#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>

#include "mynfs_client.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;

static struct options {	
	MyNFS* nfs_client;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void show_help(const char *progname){
	printf("usage: %s [options] <mountpoint>\n\n", progname);
}

static void *client_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
    (void) conn;
    cfg->kernel_cache = 1;
    return NULL;
}

static int client_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	return options.nfs_client->rpc_getattr(path, stbuf);
}

static int client_mkdir(const char *path, mode_t mode) {
    return options.nfs_client->rpc_mkdir(path, mode);
}

static int client_unlink(const char *path)
{
    return options.nfs_client->rpc_unlink(path);    
}

static int client_rmdir(const char *path)
{
    return options.nfs_client->rpc_rmdir(path);
}

static int client_rename(const char *from, const char *to, unsigned int flags)
{
    return options.nfs_client->rpc_rename(from, to, flags);
}

static int client_open(const char *path, struct fuse_file_info *fi)
{
    return options.nfs_client->rpc_open(path, fi);
}

static int client_read(const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
    return options.nfs_client->rpc_read(path, buf, size, offset, fi);
}

static int client_write(const char *path, const char *buf, size_t size,
             off_t offset, struct fuse_file_info *fi)
{
    return options.nfs_client->rpc_write(path, buf, size, offset, fi);
}

static int client_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi,
               enum fuse_readdir_flags flags)
{
    return options.nfs_client->rpc_readdir(path, buf, filler);
}

static int client_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    return options.nfs_client->rpc_create(path, mode, fi);
}


static int client_utimens(const char *path, const struct timespec ts[2],
		       struct fuse_file_info *fi)
{

    return options.nfs_client->rpc_utimens(path, ts, fi);
}

static int client_mknod(const char *path, mode_t mode, dev_t rdev)
{
    return options.nfs_client->rpc_mknod(path, mode, rdev);
}

static struct client_operations : fuse_operations {
    client_operations(){
        init = client_init;
        getattr = client_getattr;
        readdir = client_readdir;
        open = client_open;
        read = client_read;
        write = client_write;
        create  = client_create;
        mkdir   = client_mkdir;
        rmdir = client_rmdir;
        unlink  = client_unlink;
        rename = client_rename;
        utimens = client_utimens;
        mknod = client_mknod;
    }

} nfs_oper;

int main(int argc, char* argv[]){

    int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	options.nfs_client = new MyNFS(grpc::CreateChannel("18.191.94.70:50051", grpc::InsecureChannelCredentials()));

    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
	   return 1;

    if (options.show_help) {
        show_help(argv[0]);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0][0] = '\0'; 
    }

    ret = fuse_main(args.argc, args.argv, &nfs_oper, &options);
    fuse_opt_free_args(&args);
    return ret;

}
