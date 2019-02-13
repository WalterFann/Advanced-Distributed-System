#include <iostream>
#include <string.h>
#include <stdlib.h>      
#include <time.h>
#include <map>
#include "mynfs_client.h"


class cacheEntry
{
public:
	int fd;
	char * path;
	char * buf;
	size_t size;
	off_t offset;
	struct fuse_file_info *fi;

	cacheEntry(int fd, const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
	{
		this->fd = fd;
		this->path = strdup(path);
		this->buf = strdup(buf);
		this->size = size;
		this->offset = offset;
		this->fi = fi;
	}
};

class clientWriteCache
{
public:
	std::vector<cacheEntry> entries;
	MyNFS* nfs_client;
	int ID;
	void reinitialize(){
		this->entries.clear();
	}
	int writeAll(){
		int res = 1;
		res = nfs_client->rpc_fsync(ID, entries.size());
		//printf("fail: %d\n", res);
		if(res!=-1)//fail
		{
			return res;
		}
		return res;
	}

	void recover(){
		if(entries.size()==0){
			return;
		}
		nfs_client->rpc_clear(ID);
		struct fuse_file_info *fi;
		std::map<std::string, int> openedFiles;
		int temp;

		for(std::vector<cacheEntry>::iterator iter = entries.begin();iter!=entries.end(); iter++){
			std::string p(iter->path);
			if(openedFiles.find(p)==openedFiles.end()){
				fi = new fuse_file_info();
				fi->flags = O_WRONLY;
				temp = nfs_client->rpc_open(iter->path, fi);
				openedFiles[p] = fi->fh;
			}
			fi = new fuse_file_info();
			fi->fh = openedFiles[p];
			int res = nfs_client->rpc_write(iter->buf, iter->size, iter->offset, fi, ID);
		}
	}

	void insert(cacheEntry entry){
		entries.push_back(entry);
		printf("fd%d\n", entry.fd);
		printf("entry size: %d\n", entries.size());
	}

	clientWriteCache(MyNFS* client){
		this->nfs_client = client;
		srand((unsigned)time(0));
		this->ID = rand();
		printf("ID = %d\n", ID);
	}

};