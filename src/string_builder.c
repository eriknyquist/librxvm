#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "string_builder.h"
#include "rxvm_err.h"

#define DEFAULT_BLOCK_SIZE     10

static int enlarge_buf (strb_t *cfg, int blocks)
{
    char *temp;
    int size;

    size = cfg->block_size * blocks;
    if ((temp = realloc(cfg->buf, cfg->space + size)) == NULL) {
         return RXVM_EMEM;
    }

    cfg->space += size;
    cfg->buf = temp;
    return 0;
}

int strb_init (strb_t *cfg, unsigned int block_size)
{
    if (!cfg) return RXVM_EPARAM;

    memset(cfg, 0, sizeof(strb_t));
    cfg->block_size = (block_size) ? block_size : DEFAULT_BLOCK_SIZE;

    if ((cfg->buf = malloc(cfg->block_size)) == NULL) {
        return RXVM_EMEM;
    }

    cfg->space = cfg->block_size;
    return 0;
}

int strb_addc (strb_t *cfg, char c)
{
    if ((cfg->size + 1) > cfg->space) {
        if (enlarge_buf(cfg, 1) < 0) {
            return RXVM_EMEM;
        }
    }

    cfg->buf[cfg->size++] = c;
    return 0;
}

int strb_adds (strb_t *cfg, char *s, int n)
{
    int i;
    int blocks;

    if ((cfg->size + n) > cfg->space) {
        blocks = (((cfg->size + n) - cfg->space) / cfg->block_size) + 1;
        if (enlarge_buf(cfg, blocks) < 0) {
            return RXVM_EMEM;
        }
    }

    for (i = 0; i < n && s[i]; ++i) {
        cfg->buf[cfg->size++] = s[i];
    }

    return 0;
}

int strb_addu (strb_t *cfg, unsigned int i)
{
    int chars;
    char num[20];

    chars = snprintf(num, sizeof(num), "%u", i);
    return strb_adds(cfg, num, chars);
}
