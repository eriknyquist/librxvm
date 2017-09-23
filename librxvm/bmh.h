#ifndef BMH_H_
#define BMH_H_

void bmh_reset (void);
void bmh_init (FILE *file, char *pat, unsigned int length);
int bmh (threads_t *tm);

#endif
