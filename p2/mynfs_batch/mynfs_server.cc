#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include <grpc++/grpc++.h>
#include "mynfs.grpc.pb.h"

#include "serverWriteCache.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;

using namespace mynfs;
using namespace std;

void convertPath(const char* client_path, char* server_path){
    strcat(server_path, "./server_root");
    strcat(server_path + 13, client_path);
    server_path[strlen(server_path)] = '\0';
}

class MyNFSService final : public NFS::Service {
	Status mynfs_getattr(ServerContext* context, const Path* p, 
					 Stat* reply) override {
		char server_path[1000]={0};
		convertPath(p->path().c_str(), server_path);
        struct stat st;
		int res = lstat(server_path, &st);
        if(res == -1){
		    //perror(strerror(errno));
		    reply->set_err(errno);
		}
		else{
            reply->set_ino(st.st_ino);
            reply->set_mode(st.st_mode);
            reply->set_nlink(st.st_nlink);
            reply->set_uid(st.st_uid);
            reply->set_gid(st.st_gid);
            reply->set_rdev(st.st_rdev);
            reply->set_size(st.st_size);
            reply->set_blksize(st.st_blksize);
            reply->set_blocks(st.st_blocks);
            reply->set_atime(st.st_atime);
            reply->set_mtime(st.st_mtime);
            reply->set_ctime(st.st_ctime);
		    reply->set_err(0);
		}
		
        return Status::OK;
	}
	
    Status mynfs_mkdir(ServerContext* context, const MkdirRequest* request,
                        ErrorMessage* reply) override {
        char server_path[1000]={0};
        convertPath(request->path().c_str(), server_path);
        int res = mkdir(server_path, request->mode());

        if(res == -1){
            //perror(strerror(errno)); 
            reply->set_err(errno);
        } 
        else{
            reply->set_err(0);
        }

        return Status::OK;
    }

    Status mynfs_unlink(ServerContext* context, const Path* request,
                        ErrorMessage* reply) override {
        char server_path[1000]={0};
        convertPath(request->path().c_str(), server_path);
        int res = unlink(server_path);
        if(res == -1){
            //perror(strerror(errno));
            reply->set_err(errno);
        } 
        else{
            reply->set_err(0);
        }
        return Status::OK;
    }

    Status mynfs_rmdir(ServerContext* context, const Path* request,
                        ErrorMessage* reply) override {
        char server_path[1000]={0};
        convertPath(request->path().c_str(), server_path);
        int res = rmdir(server_path);
        if(res == -1){
            //perror(strerror(errno));
            reply->set_err(errno);
        } 
        else{
            reply->set_err(0);
        }
        return Status::OK;
    }

    Status mynfs_rename(ServerContext* context, const RenameRequest* request,
                        ErrorMessage* reply) override {
        if (request->flags()) {
            //perror(strerror(errno));
            reply->set_err(EINVAL);
            return Status::OK;
        }

        char from[1000]={0};
        char to[1000] = {0};
        convertPath(request->from().c_str(), from);
        convertPath(request->to().c_str(), to);

        int res = rename(from, to);
        if(res == -1){
            //perror(strerror(errno));
            reply->set_err(errno);
        } 
        else{
            reply->set_err(0);
        }
        return Status::OK;
    }

    Status mynfs_open(ServerContext* context, const OpenRequest* request,
                        OpenReply* reply) override {
        
        char server_path[1000] = {0};
        
        convertPath(request->path().c_str(), server_path);
        int fd = open(server_path, request->flags());
        if(fd == -1){
            //perror(strerror(errno));
            reply->set_err(errno);
        }
        else{
            reply->set_fh(fd);        
            reply->set_err(0);
        }
        return Status::OK;
    }

    Status mynfs_read(ServerContext* context, const ReadRequest* request, 
                        ReadReply* reply) override {
        char server_path[1000] = {0};
        char *buf = new char[request->size()];
        convertPath(request->path().c_str(), server_path);

        int res = pread(request->fh(), buf, request->size(), request->offset());
        if (res == -1){
            reply->set_err(errno);
            //perror(strerror(errno));
        }
        else{
            reply->set_bytesread(res);
            reply->set_buffer(buf);
            reply->set_err(0);
        }  

        free(buf);
        
        return Status::OK;
    }

    Status mynfs_write(ServerContext* context, const WriteRequest* request, 
            WriteReply* reply) override {

        cacheEntry entry(static_cast<int>(request->fh()), request->buffer().c_str(), static_cast<size_t>(request->size()), static_cast<off_t>(request->offset()));
        writeCache.insert(request->id(), entry);
        printf("fd: %d\n", entry.fd);
        printf("entry_size%d\n",writeCache.entries[request->id()].size());

        reply->set_nbytes(request->size());
        reply->set_err(0);

        return Status::OK;
    }

	Status mynfs_readdir(ServerContext* context, const Path* s,
						ServerWriter<ReaddirReply>* writer) override {
		DIR *dp;
		struct dirent *entry;
		ReaddirReply reply;

		char server_path[1000]={0};
		convertPath(s->path().c_str(), server_path);

		dp = opendir(server_path);
		if (dp == NULL){
			//perror(strerror(errno));
			reply.set_err(errno);
		}
		else{
            while(1){
                entry = readdir(dp);
                if(!entry)
                    break;
                reply.set_ino(entry->d_ino);
                reply.set_name(entry->d_name);
                reply.set_type(entry->d_type);
                reply.set_err(0);
                writer->Write(reply);
            }
            closedir(dp);
        }

		return Status::OK;
	}


    Status mynfs_create(ServerContext* context, const CreateRequest* req,
            CreateReply* reply) override {

        char server_path[1000] = {0};
        convertPath(req->path().c_str(), server_path);

        int fd = open(server_path, req->flags(), req->mode());
        if(fd == -1){
            //perror(strerror(errno));
            reply->set_err(errno);
        }
        else{
            reply->set_fh(fd);
            reply->set_err(0);
        }
        return Status::OK;
    }



    Status mynfs_utimens(ServerContext* context, const UtimensRequest* request,
                        ErrorMessage* reply) override {
    	char server_path[1000]={0};
        convertPath(request->path().c_str(), server_path);

        struct timespec ts[2];
    	ts[0].tv_sec = request->sec1();
    	ts[0].tv_nsec = request->nsec1();
    	ts[1].tv_sec = request->sec2();
    	ts[1].tv_nsec = request->nsec2();

        int res = utimensat(0, server_path, ts, AT_SYMLINK_NOFOLLOW);

    	if (res == -1) {
            //perror(strerror(errno));
            reply->set_err(errno);
        } 
        else{
            reply->set_err(0);
        }
        return Status::OK;
    }

    Status mynfs_mknod(ServerContext* context, const MknodRequest* request,
                        ErrorMessage* reply) override {
        char server_path[1000]={0};
        convertPath(request->path().c_str(), server_path);

        int res;
	    mode_t mode = request->mode();
	    dev_t rdev = request->rdev();

	    if (S_ISFIFO(mode))
		    res = mkfifo(server_path, mode);
	    else
		    res = mknod(server_path, mode, rdev);
	    
        if (res == -1) {
            //perror(strerror(errno));
	        reply->set_err(errno);
	    }
        else{
            reply->set_err(0);
        }
        return Status::OK;
    }

    Status mynfs_release(ServerContext* context, const ReleaseRequest* request, 
                        ErrorMessage* reply) override{
        close(request->fh());
        reply->set_err(0);
        return Status::OK;
    }

    Status mynfs_clear(ServerContext* context, const ClearRequest* request,
                        ErrorMessage* reply) override{
        writeCache.entries[request->id()].clear();
        reply->set_err(0);
        return Status::OK;
    }

    Status mynfs_fsync(ServerContext* context, const FsyncRequest* request, 
                        FsyncReply* reply) override {

        if (request->size()!=writeCache.entries[request->id()].size()){
            reply->set_err(-1);
            return Status::OK;
        }
        writeCache.writeAll(request->id());
        reply->set_err(1);
        return Status::OK;
    }

    serverWriteCache writeCache;

};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  MyNFSService service;

  ServerBuilder builder;

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}

