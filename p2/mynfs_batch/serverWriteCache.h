#include <iostream>
#include <string>
#include <map>
#include <mutex>

class cacheEntry
{
public:
	int fd;
	char * buf;
	size_t size;
	off_t offset;

	cacheEntry(int fd, const char *buf, size_t size, off_t offset)
	{
		this->fd = fd;
		this->buf = strdup(buf);
		this->size = size;
		this->offset = offset;
	}
};

class serverWriteCache
{
public:
	std::map<int, std::vector<cacheEntry> > entries;

	std::mutex g_mutex;

	serverWriteCache(){
	}
	void writeAll(int ID){
		g_mutex.lock();
		for(std::vector<cacheEntry>::iterator iter = entries[ID].begin();iter!=entries[ID].end(); iter++){
			int res = pwrite(iter->fd, iter->buf, iter->size, iter->offset);
		}
		for(std::vector<cacheEntry>::iterator iter = entries[ID].begin();iter!=entries[ID].end(); iter++){
			fsync(iter->fd);
		}
		entries[ID].clear();
		g_mutex.unlock();
	}

	void insert(int ID, cacheEntry entry){
		entries[ID].push_back(entry);
	}

};