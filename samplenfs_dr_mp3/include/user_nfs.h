#if defined HAVE_NFS
/// from user_nfs.c
#include <nfsc/libnfs.h>
int    user_init (void);
size_t user_stat (void);
struct
nfsfh *user_open (const char *filename);
void   user_seek (long offset, int whence);
size_t user_read (unsigned char *dst, size_t size);
void   user_close(void);
void   user_end  (void);
#endif