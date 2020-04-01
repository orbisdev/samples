/*
  standard user_nfs helpers
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <nfsc/libnfs.h>
#include <nfsc/libnfs-raw-mount.h>

#ifdef __PS4__
 #include <debugnet.h>

#else
 #define  debugNetPrintf  fprintf
 #define  DEBUG           stderr
 #define  ERROR           stderr
 #define  INFO            stdout

#endif


int process_server(const char *server)
{
    debugNetPrintf(DEBUG, "mount_getexports(%s):\n", server);
    struct exportnode *exports;
    struct exportnode *export;

    exports = mount_getexports(server);
    if (exports == NULL) {
        debugNetPrintf(ERROR, "Failed to get exports for server %s.\n", server);
        return -1;
    }
    for (export=exports; export; export = export->ex_next) {
        debugNetPrintf(INFO, "nfs://%s%s\n", server, export->ex_dir);
    }
    mount_free_export_list(exports);
    return 0;
}


// globals, just one at once
struct nfs_context *nfs = NULL;
struct nfs_url     *url = NULL;
struct nfsfh       *fh  = NULL;
struct nfs_stat_64  st;
static uint64_t     off = 0;


// rewind() alike: reset stream position
void user_seek(long offset, int whence)
{
    switch(whence)
    {
        default:
        case SEEK_SET: off = offset; break;
    }
}


size_t user_read(unsigned char *dst, size_t size)
{
    size_t readed = nfs_pread(nfs, fh, off, size, dst);
           off   += readed;

    if(readed != size) fprintf(stderr, "asked for %zu, but readed %zu!\n", size, readed);

    return readed;
}


#define BUFSIZE /* (DRMP3_DATA_CHUNK_SIZE) */ ( 16 *1024 )

// read chunks of max BUFSIZE in bytes
void user_refill_buffer(unsigned char *pData, int offinFixedBuffer)
{
    if(off < st.nfs_size)
    {
        int count = st.nfs_size - off;
        if(count > BUFSIZE) count = BUFSIZE;

        count -= offinFixedBuffer;
        count  = user_read(pData + offinFixedBuffer, count);

        if(count < 0) { fprintf(stderr, "Failed to read from file\n"); return; }
        //debugNetPrintf(DEBUG, "nfs readchunk: %lldb %db @ %p + %d\n", off, count, pData, offinFixedBuffer);
    }
}

/// setup nfs contex, mount export
int user_init(void)
{
    nfs = nfs_init_context();

    nfs_set_debug(nfs, /*debug_level*/0); // quite down

    int ret = 0;
#if 0 // optional, list server exports
    static const char *srv = "10.0.0.2";
    ret = process_server(srv);
    debugNetPrintf(DEBUG, "process_server(nfs://%s): %d\n", srv, ret);
#endif

    // parse url and mount export
    url = nfs_parse_url_dir(nfs, "nfs://10.0.0.2/hostapp");
    ret = nfs_mount(nfs, url->server, url->path);
    debugNetPrintf(DEBUG, "nfs_mount(%s, %s) return: %d\n", url->server, url->path, ret);

    return 1;
}

// we have opened just one file at once
size_t user_stat(void)
{
    if(nfs_fstat64(nfs, fh, &st))
        { debugNetPrintf(DEBUG, "Failed to stat file : %s\n", nfs_get_error(nfs)); return -1; }
    return st.nfs_size;
}

// just open, don't read; return the nfs_file_handle
struct nfsfh *user_open(const char *filename)
{
    int flags  = 0;
        off    = 0;
        flags |= O_RDONLY;

    if(nfs_open(nfs, filename, flags, &fh))
        { debugNetPrintf(DEBUG, "Failed to open(%s)\n", nfs_get_error(nfs)); return NULL; }
    return fh;
}

void user_close(void)
{
    debugNetPrintf(DEBUG,"user_close, %p, %p, %p\n", nfs, url, fh);
    if(nfs_close(nfs, fh))
         { debugNetPrintf(DEBUG, "Failed to close(): %s\n", nfs_get_error(nfs)); return; }
}


/// cleanup, umount export, destroy nfs context
void user_end(void)
{
    debugNetPrintf(DEBUG,"user_end, %p, %p, %p\n", nfs, url, fh);
    int ret = -1;
    goto finished;

    if(nfs && fh)
    {
        ret = nfs_close(nfs, fh);
        if(ret)
        {
            debugNetPrintf(DEBUG, "Failed to close(): %s\n", nfs_get_error(nfs));
        }
    }

finished:
    ret = nfs_umount(nfs);
    debugNetPrintf(DEBUG, "nfs_umount return: %x\n", ret);

    nfs_destroy_url(url);
    nfs_destroy_context(nfs);

    return;
}
