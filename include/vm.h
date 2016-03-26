#ifndef VM_H_
#define VM_H_

typedef struct threads threads_t;

struct threads {
    int *cp;             /* Threads for current input char. */
    int *np;             /* Threads for next input char. */
    uint8_t *cp_lookup;  /* Lookup table for current input char. */
    uint8_t *np_lookup;  /* Lookup table for next input char. */
    char *lastmatch;     /* Last input char. to match expression */
    int csize;           /* No. of threads queued for current input char. */
    int nsize;           /* No. of threads queued for next input. char. */
};

int vm_init (threads_t *tm, unsigned int size);
int vm_execute (threads_t *tm, regexvm_t *compiled, char **input);

#endif
