  #include <grpc++/grpc++.h>
  #include "mynfs.grpc.pb.h"
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>

  using grpc::Channel;
  using grpc::Status;
  using grpc::ClientContext;
  using grpc::ClientReader;

  using namespace mynfs;
  using namespace std;

  class MyNFS {
   public:
    MyNFS(std::shared_ptr<Channel> channel)
        : stub_(NFS::NewStub(channel)) {}

      int rpc_getattr(string path, struct stat* stbuf){
        Stat reply;
        ClientContext context;
        Path p;
        p.set_path(path);
        //memset(stbuf, 0, sizeof(struct stat));
        Status status = stub_->mynfs_getattr(&context, p, &reply);
        
        if(reply.err() == 0){
          stbuf->st_ino = reply.ino();
          stbuf->st_mode = reply.mode();
          stbuf->st_nlink = reply.nlink();
          stbuf->st_uid = reply.uid();
          stbuf->st_gid = reply.gid();
          stbuf->st_rdev = reply.rdev();
          stbuf->st_size = reply.size();
          stbuf->st_blksize = reply.blksize();
          stbuf->st_blocks = reply.blocks();
          stbuf->st_atime = reply.atime();
          stbuf->st_mtime = reply.mtime();
          stbuf->st_ctime = reply.ctime();
          return 0;
        }
        else{
          return -reply.err();
        }
      }

      int rpc_mkdir(const char* path, mode_t mode){
        MkdirRequest request;
        ClientContext context;
        request.set_path(path);
        request.set_mode(mode);
        ErrorMessage reply;

        Status status = stub_->mynfs_mkdir(&context, request, &reply);
        return -reply.err();
      }

      int rpc_unlink(const char* path) {
        Path request;
        ClientContext context;
        request.set_path(path);
        ErrorMessage reply;

        Status status = stub_->mynfs_unlink(&context, request, &reply);
        return -reply.err();
      }
    
      int rpc_rmdir(string path) {
        Path request;
        ClientContext context;
        request.set_path(path);
        ErrorMessage reply;

        Status status = stub_->mynfs_rmdir(&context, request, &reply);
        return -reply.err();
      }

      int rpc_rename(const char *from, const char *to, unsigned int flags) {
        RenameRequest request;
        ClientContext context;
        request.set_from(from);
        request.set_to(to);
        request.set_flags(flags);
        ErrorMessage reply;

        Status status = stub_->mynfs_rename(&context, request, &reply);
        return -reply.err();
      }

      int rpc_open(const char* path, struct fuse_file_info* fi){
        ClientContext clientContext;
        OpenRequest request;
        OpenReply reply;
        Status status; 

        request.set_path(path);
        request.set_flags(fi->flags);
        
        status = stub_->mynfs_open(&clientContext, request, &reply);
        if(reply.err() == 0)
          fi->fh = reply.fh();

        return -reply.err();
      }

      int rpc_read(const char *path, char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi){
        ClientContext clientContext;
        ReadRequest request;
        ReadReply reply;
        request.set_path(path);
        request.set_size(size);
        request.set_offset(offset);
        request.set_fh(fi->fh);

        Status status = stub_->mynfs_read(&clientContext, request, &reply);
        if(reply.err() == 0){
            strcpy(buf, reply.buffer().c_str());
            return reply.bytesread();
        }else{
            return -reply.err();
        }        
      }

      int rpc_write(const char *path, const char *buf, size_t size,
               off_t offset, struct fuse_file_info *fi){
        ClientContext clientContext;
        WriteRequest request;
        request.set_path(path);
        request.set_size(size);
        request.set_offset(offset);
        request.set_buffer(buf);
        request.set_fh(fi->fh);
        
        WriteReply reply;

        Status status = stub_->mynfs_write(&clientContext, request, &reply);
        if(reply.err() == 0){
            return reply.nbytes();
        } else{
            return -reply.err(); 
        }
      }

      int rpc_readdir(const char *path, void *buf, fuse_fill_dir_t filler){
          Path request;
          ReaddirReply reply;
          ClientContext clientContext;
          struct stat st;
          enum fuse_fill_dir_flags fill_flags = static_cast<fuse_fill_dir_flags>(0);
          request.set_path(path);

          std::unique_ptr<ClientReader<ReaddirReply> >reader(
              stub_->mynfs_readdir(&clientContext, request));
          while(reader->Read(&reply)){
              memset(&st, 0, sizeof(st));
              st.st_ino = reply.ino();
              st.st_mode = reply.type() << 12;

              if (filler(buf, reply.name().c_str(), &st, 0, fill_flags))
                  break;
              }

          Status status = reader->Finish();	

          return -reply.err(); 
      }
      
      int rpc_create(const char *path, mode_t mode, struct fuse_file_info *fi)
      {
          ClientContext clientContext; 
          CreateRequest request;
          CreateReply reply;
          request.set_path(path);
          request.set_flags(fi->flags);
          request.set_mode(mode);
          
          Status status = stub_->mynfs_create(&clientContext, request, &reply);
          if(reply.err() == 0)
              fi->fh = reply.fh();   
   
          return -reply.err();
      }


    int rpc_utimens(const char *path, const struct timespec *ts, struct fuse_file_info *fi) {
        UtimensRequest request;
        ClientContext context;
        ErrorMessage reply;
        request.set_path(path);
        request.set_sec1(ts[0].tv_sec);
        request.set_nsec2(ts[0].tv_nsec);
        request.set_sec2(ts[1].tv_sec);
        request.set_nsec2(ts[1].tv_nsec);
        Status status = stub_->mynfs_utimens(&context, request, &reply);
        return -reply.err();
    }


    int rpc_mknod(const char *path, mode_t mode, dev_t rdev) {
        MknodRequest request;
        ClientContext context;
        ErrorMessage reply;
        request.set_path(path);
        request.set_mode(mode);
        request.set_rdev(rdev);
        Status status = stub_->mynfs_mknod(&context, request, &reply);
        return -reply.err();

    }



   private:
      std::unique_ptr<NFS::Stub> stub_;
  };
