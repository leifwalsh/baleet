/* Copyright 2010 Leif Walsh.  See COPYING for details. */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PROG() (prog ? prog : argv[0])

#ifdef _DAMMIT_GIMME_THE_CRACK_ALREADY

#define PERROR(...) 
#define PFAIL(...) 
#define PFAILIF(...) 
#define ERR(...) 
#define FAIL(...) 
#define FAILIF(...) 

#define malloc_chk(p, type, sz) do { (p) = type malloc(sz); } while (0)
#define free_chk(p) free(p)
#define open_chk(fd, path, flags...) do { (fd) = open((path), flags); } while (0)
#define close_chk(fd) close(fd)

#else  /* this is the safe one */

#define PERROR() perror(PROG())

#define PFAIL()                                 \
  do {                                          \
      PERROR();                                 \
      ret = errno;                              \
      goto cleanup;                             \
  } while (0)

#define PFAILIF(cond)                           \
  do {                                          \
      if (cond) {                               \
          PFAIL();                              \
      }                                         \
  } while (0)

#define ERR(fmt, ...) fprintf(stderr, "%s: " fmt, PROG(), ##__VA_ARGS__)

#define FAIL(ret_, fmt, ...)                    \
  do {                                          \
      ERR(fmt, ##__VA_ARGS__);                  \
      ret = (ret_);                             \
      goto cleanup;                             \
  } while (0)

#define FAILIF(cond, ret_, fmt, ...)            \
  do {                                          \
      if (cond) {                               \
          FAIL((ret_), fmt, ##__VA_ARGS__);     \
      }                                         \
  } while (0)

#define malloc_chk(p, type, sz)                 \
  do {                                          \
      (p) = type malloc(sz);                    \
      PFAILIF(!(p));                            \
  } while (0)

#define free_chk(p)                             \
  do {                                          \
      if (p) {                                  \
          free(p);                              \
          (p) = NULL;                           \
      }                                         \
  } while (0)

#define open_chk(fd, path, flags...)            \
  do {                                          \
      (fd) = open((path), flags);               \
      PFAILIF((fd) < 0);                        \
  } while (0)

#define close_chk(fd)                           \
  do {                                          \
      if ((fd) >= 0) {                          \
          close(fd);                            \
          fd = -1;                              \
      }                                         \
  } while (0)

#endif

int main(const int argc, const char * const argv[])
{
    int ret = 0;
    char *prog = NULL;
    int *fds = NULL;
    unsigned char *buf = NULL;
    int rand_fd = -1;

    /* lengthy */
    {
        char *tmp1 = NULL;
        char *tmp2 = NULL;
        tmp1 = strdup(argv[0]);
        tmp2 = basename(tmp1);
        prog = strdup(tmp2);
        free_chk(tmp1);
    }

    FAILIF((argc < 2), 2, "what file(s) do you want to baleet?\n");

    /* allocate one extra just to make the indexing simpler */
    malloc_chk(fds, (int *), argc * sizeof(*fds));
    for (int i = 0; i < argc; ++i) {
        fds[i] = -1;
    }

    /* lol, don't try to delete ourselves, start at 1 */
    for (int i = 1; i < argc; ++i) {
        struct stat st;

        ret = stat(argv[i], &st);
        PFAILIF(ret < 0);

        FAILIF((!S_ISREG(st.st_mode)), 1, "%s is not a regular file!\n",
               argv[i]);
    }

    rand_fd = open("/dev/urandom", O_RDONLY);
    if (rand_fd < 0) {
        FAILIF((errno == 0), 1, "unknown error\n");
        switch (errno) {
        case EACCES:
        case EISDIR:
        case ELOOP:
        case ENAMETOOLONG:
        case ENODEV:
        case ENOENT:
        case ENXIO:
        case EPERM:
            rand_fd = open("/dev/random", O_RDONLY);
            PFAILIF(rand_fd < 0);
            break;
        default:
            PFAIL();
        }
    }

    for (int i = 1; i < argc; ++i) {
        for (int j = 0; j < 8; ++j) {  // overwrite 8 times, just to be sure
            struct stat st;
            blksize_t blksize = 0;
            off_t size = 0;
            ssize_t sz_written = 0;

            open_chk(fds[i], argv[i], O_WRONLY);
            ret = fstat(fds[i], &st);
            PFAILIF(ret < 0);

            blksize = st.st_blksize;
            size = st.st_size;

            malloc_chk(buf, (unsigned char *), blksize);

            while (sz_written < size) {
                ssize_t sz_read = 0;
                ssize_t cur_written = 0;

                while (sz_read < blksize) {
                    ssize_t tmp = read(rand_fd, buf, blksize - sz_read);
                    PFAILIF(tmp < 0);
                    FAILIF(tmp == 0, 1, "random device hit EOF.\n");
                    sz_read += tmp;
                }
                FAILIF(sz_read != blksize, 1,
                       "couldn't read enough random data.\n");

                while (cur_written < blksize) {
                    ssize_t tmp = write(fds[i], buf + cur_written,
                                        blksize - cur_written);
                    PFAILIF(tmp < 0);
                    FAILIF(tmp == 0, 1, "unknown error\n");
                    cur_written += tmp;
                }
                FAILIF(cur_written != blksize, 1,
                       "couldn't write enough random data.\n");

                sz_written += cur_written;
            }

            free_chk(buf);

            close_chk(fds[i]);
        }

        ret = unlink(argv[i]);
        PFAILIF(ret < 0);
    }

cleanup:
    if (fds != NULL) {
        for (int i = 1; i < argc; ++i) {
            close_chk(fds[i]);
        }
    }
    free_chk(fds);
    free_chk(buf);
    close_chk(rand_fd);
    free_chk(prog);
    exit(ret);
}
