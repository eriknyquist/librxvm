#ifndef RANDEXP_H_

typedef struct randexp_cfg randexp_cfg_t;

struct randexp_cfg {
    strb_t *strb;
    uint8_t literals;   /* 0-100; higher means more literals */
    uint8_t escapes;    /* 0-100; higher means more escaped literals */
    uint8_t tokens;     /* 0-100; higher means more tokens */

    size_t limit;
};

char *gen_randexp (randexp_cfg_t *cfg, size_t *len);
void init_charmap (void);

#endif
