#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "string_builder.h"

#define DEFAULT_BLOCK_SIZE     10

static void enlarge_buf (strb_t *cfg, int blocks)
{
    char *temp;
    int size;

    size = cfg->block_size * blocks;
    if ((temp = realloc(cfg->buf, cfg->space + size)) == NULL) {
         printf("Error: can't allocate any more memory for strb_t\n");
         printf("Current heap allocation size: %lu bytes\n", cfg->space);
         exit(-1);
    }

    cfg->space += size;
    cfg->buf = temp;
}

void strb_init (strb_t *cfg, unsigned int block_size)
{
    if (!cfg) return;

    memset(cfg, 0, sizeof(strb_t));
    cfg->block_size = (block_size) ? block_size : DEFAULT_BLOCK_SIZE;

    if ((cfg->buf = malloc(cfg->block_size)) == NULL) {
        printf("Can't allocate initial memory for strb_t\n");
        exit(1);
    }

    cfg->space = cfg->block_size;
}

void strb_addc (strb_t *cfg, char c)
{
    if ((cfg->size + 1) > cfg->space) {
        enlarge_buf(cfg, 1);
    }

    cfg->buf[cfg->size++] = c;
}

void strb_adds (strb_t *cfg, char *s, int n)
{
    int i;
    int blocks;

    if ((cfg->size + n) > cfg->space) {
        blocks = (((cfg->size + n) - cfg->space) / cfg->block_size) + 1;
        enlarge_buf(cfg, blocks);
    }

    for (i = 0; i < n && s[i]; ++i) {
        cfg->buf[cfg->size++] = s[i];
    }
}

void strb_addu (strb_t *cfg, unsigned int i)
{
    int chars;
    char num[20];

    chars = snprintf(num, sizeof(num), "%d", i);
    strb_adds(cfg, num, chars);
}
