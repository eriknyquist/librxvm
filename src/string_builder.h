#ifndef _STRING_BUILDER_H_
#define _STRING_BUILDER_H_

typedef struct strb strb_t;

struct strb {
    uint64_t size;
    uint64_t space;

    char *buf;
    unsigned int block_size;
};

void strb_init (strb_t *cfg, unsigned int block_size);
void strb_addc (strb_t *cfg, char c);
void strb_addc (strb_t *cfg, char c);
void strb_addu (strb_t *cfg, unsigned int i);

#endif
