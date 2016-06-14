#include <stdio.h>
#include <stdlib.h>
#include "randexp.h"
#include "regexvm.h"
#include "test_common.h"
#include "lex.h"

#define BLOCK_SIZE          50
#define MAX_REP             10

#define SIMPLE_CHOICES      2
#define BASIC_CHOICES       5
#define REPS_CHOICES        4
#define ELEM_CHOICES        4
/*#define ELEM_CHOICES        6 */
#define SETITEM_CHOICES     2

#define NUM_META            12

#define LITERALS            (((PRINTABLE_HIGH - PRINTABLE_LOW) + \
                            (WS_HIGH - WS_LOW)) - NUM_META)

static const char *meta = "()*+-.?[]{|}";
static char charmap[LITERALS + NUM_META];

static void nonterm_re       (cfg_t *cfg);
static void nonterm_union    (cfg_t *cfg);
static void nonterm_simple   (cfg_t *cfg);
static void nonterm_concat   (cfg_t *cfg);
static void nonterm_basic    (cfg_t *cfg);
static void nonterm_zero     (cfg_t *cfg);
static void nonterm_one      (cfg_t *cfg);
static void nonterm_onezero  (cfg_t *cfg);
static void nonterm_rep      (cfg_t *cfg);
static void nonterm_reps     (cfg_t *cfg);
static void nonterm_repn     (cfg_t *cfg);
static void nonterm_reprange (cfg_t *cfg);
static void nonterm_repmore  (cfg_t *cfg);
static void nonterm_repless  (cfg_t *cfg);
static void nonterm_elem     (cfg_t *cfg);
static void nonterm_group    (cfg_t *cfg);
static void nonterm_any      (cfg_t *cfg);
/*static void nonterm_sol      (cfg_t *cfg);*/
/*static void nonterm_eol      (cfg_t *cfg);*/
static void nonterm_char     (cfg_t *cfg);
static void nonterm_char_ccs (cfg_t *cfg);
static void nonterm_set      (cfg_t *cfg);
static void nonterm_setitems (cfg_t *cfg);
static void nonterm_setitem  (cfg_t *cfg);
static void nonterm_range    (cfg_t *cfg);

typedef void (*nonterm_t)(cfg_t*);

nonterm_t simple_choices[SIMPLE_CHOICES] = {nonterm_concat, nonterm_basic};
nonterm_t basic_choices[BASIC_CHOICES] =
    {nonterm_zero, nonterm_one, nonterm_onezero, nonterm_rep, nonterm_elem};
nonterm_t reps_choices[REPS_CHOICES] =
    {nonterm_repn, nonterm_reprange, nonterm_repmore, nonterm_repless};
nonterm_t elem_choices[ELEM_CHOICES] =
    {nonterm_group, nonterm_any, /*nonterm_eol, nonterm_sol,*/ nonterm_char,
    nonterm_set};
nonterm_t setitem_choices[SETITEM_CHOICES] = {nonterm_range, nonterm_char_ccs};

static int ismeta (char c)
{
    int i;

    for (i = 0; meta[i]; ++i) {
        if (meta[i] == c)
            return 1;
    }

    return 0;
}

static void init_charmap ()
{
    int i;
    char c;

    i = 0;
    do {
        charmap[i] = i + WS_LOW;
    } while ((i++ + WS_LOW) < WS_HIGH);

    c = PRINTABLE_LOW;
    while (i < LITERALS) {
        if (!ismeta(c)) {
            charmap[i++] = c;
        }
        ++c;
    }

    for (i = 0; i < NUM_META; ++i) {
        charmap[LITERALS + i] = meta[i];
    }
}

static char get_rand_literal (uint8_t meta)
{
    unsigned int hi;

    hi = (meta) ? LITERALS + NUM_META : LITERALS;
    return charmap[rand_range(WS_LOW, hi)];
}

static void enlarge_buf (cfg_t *cfg, int blocks)
{
    char *temp;
    int size;

    size = BLOCK_SIZE * blocks;
    if ((temp = realloc(cfg->buf, cfg->space + size)) == NULL) {
         printf("Error: can't allocate any more memory.\n");
         printf("Current heap allocation size: %u bytes\n", cfg->space);
         exit(-1);
    }

    cfg->space += size;
    cfg->buf = temp;
}

static void buf_addc (cfg_t *cfg, char c)
{
    if ((cfg->size + 1) > cfg->space) {
        enlarge_buf(cfg, 1);
    }

    cfg->buf[cfg->size++] = c;
}

static void buf_adds (cfg_t *cfg, char *s, int n)
{
    int i;
    int blocks;

    if ((cfg->size + n) > cfg->space) {
        blocks = (((cfg->size + n) - cfg->space) / BLOCK_SIZE) + 1;
        enlarge_buf(cfg, blocks);
    }

    for (i = 0; i < n && s[i]; ++i) {
        cfg->buf[cfg->size++] = s[i];
    }
}

static void buf_addu (cfg_t *cfg, unsigned int i)
{
    int chars;
    char num[20];

    chars = snprintf(num, sizeof(num), "%d", i);
    buf_adds(cfg, num, chars);
}

static int choice (int prob)
{
    return (rand_range(0, 100) < prob);
}

void nonterm_range (cfg_t *cfg)
{
    buf_addc(cfg, get_rand_literal(0));
    buf_addc(cfg, '-');
    buf_addc(cfg, get_rand_literal(0));
}

void nonterm_setitem (cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = setitem_choices[rand_range(0, (SETITEM_CHOICES -1))];
    nonterm(cfg);
}

void nonterm_setitems (cfg_t *cfg)
{
    if (rand_range(0, 1)) {
        nonterm_setitem(cfg);
    } else {
        nonterm_setitem(cfg);
        nonterm_setitems(cfg);
    }
}

void nonterm_set (cfg_t *cfg)
{
    buf_addc(cfg, '[');
    nonterm_setitems(cfg);
    buf_addc(cfg, ']');
}

void nonterm_char_ccs (cfg_t *cfg)
{
    buf_addc(cfg, get_rand_literal(1));
}

void nonterm_char (cfg_t *cfg)
{
    if (choice(cfg->escapes)) {
        buf_addc(cfg, '\\');
        buf_addc(cfg, meta[rand_range(0, NUM_META - 1)]);
    } else {
        buf_addc(cfg, get_rand_literal(0));
    }
}
/*
void nonterm_eol (cfg_t *cfg)
{
    buf_addc(cfg, '$');
}

void nonterm_sol (cfg_t *cfg)
{
    buf_addc(cfg, '^');
}
*/
void nonterm_any (cfg_t *cfg)
{
    buf_addc(cfg, '.');
}

void nonterm_group (cfg_t *cfg)
{
    buf_addc(cfg, '(');
    nonterm_re(cfg);
    buf_addc(cfg, ')');
}

void nonterm_elem (cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = elem_choices[rand_range(0, (ELEM_CHOICES - 1))];
    nonterm(cfg);
}

void nonterm_repless (cfg_t *cfg)
{
    buf_addc(cfg, ',');
    buf_addu(cfg, rand_range(1, MAX_REP));
}

void nonterm_repmore (cfg_t *cfg)
{
    buf_addu(cfg, rand_range(1, MAX_REP));
    buf_addc(cfg, ',');
}

void nonterm_reprange (cfg_t *cfg)
{
    buf_addu(cfg, rand_range(1, MAX_REP));
    buf_addc(cfg, ',');
    buf_addu(cfg, rand_range(1, MAX_REP));
}

void nonterm_repn (cfg_t *cfg)
{
    buf_addu(cfg, rand_range(1, MAX_REP));
}

void nonterm_reps (cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = reps_choices[rand_range(0, (REPS_CHOICES - 1))];
    nonterm(cfg);
}

void nonterm_rep (cfg_t *cfg)
{
    nonterm_elem(cfg);
    buf_addc(cfg, '{');
    nonterm_reps(cfg);
    buf_addc(cfg, '}');
}

void nonterm_onezero (cfg_t *cfg)
{
    nonterm_elem(cfg);
    buf_addc(cfg, '?');
}

void nonterm_one (cfg_t *cfg)
{
    nonterm_elem(cfg);
    buf_addc(cfg, '+');
}

void nonterm_zero (cfg_t *cfg)
{
    nonterm_elem(cfg);
    buf_addc(cfg, '*');
}

void nonterm_basic (cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = basic_choices[rand_range(0, (BASIC_CHOICES - 1))];
    nonterm(cfg);
}

void nonterm_concat (cfg_t *cfg)
{
    nonterm_simple(cfg);
    nonterm_basic(cfg);
}

void nonterm_simple (cfg_t *cfg)
{
    if (choice(cfg->tokens)) {
        nonterm_concat(cfg);
    } else {
        nonterm_basic(cfg);
    }
}

void nonterm_union (cfg_t *cfg)
{
    nonterm_re(cfg);
    buf_addc(cfg, '|');
    nonterm_simple(cfg);
}

void nonterm_re (cfg_t *cfg)
{
    if (rand_range(0, 1)) {
        nonterm_union(cfg);
    } else {
        nonterm_simple(cfg);
    }
}

char *gen_randexp (cfg_t *cfg)
{
    if ((cfg->buf = malloc(BLOCK_SIZE)) == NULL) {
        return NULL;
    }

    cfg->space = BLOCK_SIZE;
    cfg->size = 0;

    init_charmap();
    nonterm_re(cfg);

    buf_addc(cfg, '\0');
    return cfg->buf;
}
