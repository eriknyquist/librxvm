#ifndef BMH_H_
#define BMH_H_

#ifdef _WIN32
#define fseek _fseeki64
#define ftell _ftelli64
#endif

#ifdef linux
#define fseek fseeko
#define ftell ftello
#endif

void bmh_reset (void);
void bmh_init (FILE *file, char *pat, unsigned int length);
int64_t bmh (threads_t *tm);

#endif
