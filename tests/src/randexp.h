#ifndef _RANDEXP_H_
#define _RANDEXP_H_

/* holds the probability values to control the randomness */
typedef struct gencfg cfg_t;

struct gencfg {
    char *buf;
    int size;
    int space;

    int literals;   /* 0-100; higher means more literals */
    int escapes;    /* 0-100; higher means more escaped literals */
    int tokens;     /* 0-100; higher means more tokens */
};

char *gen_randexp (cfg_t *cfg);

#endif
