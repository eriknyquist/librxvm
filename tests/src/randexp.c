#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "rxvm.h"
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

#define NUM_META            15

#define LITERALS            (((PRINTABLE_HIGH - PRINTABLE_LOW) + \
                            (WS_HIGH - WS_LOW)) - NUM_META)

static const char *meta = "()*+-.?[]{|}^$\\";
static char charmap[LITERALS + NUM_META];

static void nonterm_re       (randexp_cfg_t *cfg);
static void nonterm_union    (randexp_cfg_t *cfg);
static void nonterm_simple   (randexp_cfg_t *cfg);
static void nonterm_concat   (randexp_cfg_t *cfg);
static void nonterm_basic    (randexp_cfg_t *cfg);
static void nonterm_zero     (randexp_cfg_t *cfg);
static void nonterm_one      (randexp_cfg_t *cfg);
static void nonterm_onezero  (randexp_cfg_t *cfg);
static void nonterm_rep      (randexp_cfg_t *cfg);
static void nonterm_reps     (randexp_cfg_t *cfg);
static void nonterm_repn     (randexp_cfg_t *cfg);
static void nonterm_reprange (randexp_cfg_t *cfg);
static void nonterm_repmore  (randexp_cfg_t *cfg);
static void nonterm_repless  (randexp_cfg_t *cfg);
static void nonterm_elem     (randexp_cfg_t *cfg);
static void nonterm_group    (randexp_cfg_t *cfg);
static void nonterm_any      (randexp_cfg_t *cfg);
/*static void nonterm_sol      (randexp_cfg_t *cfg);*/
/*static void nonterm_eol      (randexp_cfg_t *cfg);*/
static void nonterm_char     (randexp_cfg_t *cfg);
static void nonterm_char_ccs (randexp_cfg_t *cfg);
static void nonterm_set      (randexp_cfg_t *cfg);
static void nonterm_setitems (randexp_cfg_t *cfg);
static void nonterm_setitem  (randexp_cfg_t *cfg);
static void nonterm_range    (randexp_cfg_t *cfg);

typedef void (*nonterm_t)(randexp_cfg_t*);

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

void init_charmap ()
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

static int choice (int prob)
{
    return (rand_range(0, 100) < prob);
}

void nonterm_range (randexp_cfg_t *cfg)
{
    strb_addc(cfg->strb, get_rand_literal(0));
    strb_addc(cfg->strb, '-');
    strb_addc(cfg->strb, get_rand_literal(0));
}

void nonterm_setitem (randexp_cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = setitem_choices[rand_range(0, (SETITEM_CHOICES -1))];
    nonterm(cfg);
}

void nonterm_setitems (randexp_cfg_t *cfg)
{
    if (rand_range(0, 1)) {
        nonterm_setitem(cfg);
    } else {
        nonterm_setitem(cfg);
        nonterm_setitems(cfg);
    }
}

void nonterm_set (randexp_cfg_t *cfg)
{
    strb_addc(cfg->strb, '[');
    nonterm_setitems(cfg);
    strb_addc(cfg->strb, ']');
}

void nonterm_char_ccs (randexp_cfg_t *cfg)
{
    char c;

    c = get_rand_literal(1);
    if (c == ']' || c == '-') ++c;
    strb_addc(cfg->strb, c);
}

void nonterm_char (randexp_cfg_t *cfg)
{
    if (choice(cfg->escapes)) {
        strb_addc(cfg->strb, '\\');
        strb_addc(cfg->strb, meta[rand_range(0, NUM_META - 1)]);
    } else {
        strb_addc(cfg->strb, get_rand_literal(0));
    }
}
/*
void nonterm_eol (randexp_cfg_t *cfg)
{
    strb_addc(cfg->strb, '$');
}

void nonterm_sol (randexp_cfg_t *cfg)
{
    strb_addc(cfg->strb, '^');
}
*/
void nonterm_any (randexp_cfg_t *cfg)
{
    strb_addc(cfg->strb, '.');
}

void nonterm_group (randexp_cfg_t *cfg)
{
    strb_addc(cfg->strb, '(');
    nonterm_re(cfg);
    strb_addc(cfg->strb, ')');
}

void nonterm_elem (randexp_cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = elem_choices[rand_range(0, (ELEM_CHOICES - 1))];
    nonterm(cfg);
}

void nonterm_repless (randexp_cfg_t *cfg)
{
    strb_addc(cfg->strb, ',');
    strb_addu(cfg->strb, rand_range(1, MAX_REP));
}

void nonterm_repmore (randexp_cfg_t *cfg)
{
    strb_addu(cfg->strb, rand_range(1, MAX_REP));
    strb_addc(cfg->strb, ',');
}

void nonterm_reprange (randexp_cfg_t *cfg)
{
    strb_addu(cfg->strb, rand_range(1, MAX_REP));
    strb_addc(cfg->strb, ',');
    strb_addu(cfg->strb, rand_range(1, MAX_REP));
}

void nonterm_repn (randexp_cfg_t *cfg)
{
    strb_addu(cfg->strb, rand_range(1, MAX_REP));
}

void nonterm_reps (randexp_cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = reps_choices[rand_range(0, (REPS_CHOICES - 1))];
    nonterm(cfg);
}

void nonterm_rep (randexp_cfg_t *cfg)
{
    nonterm_elem(cfg);
    strb_addc(cfg->strb, '{');
    nonterm_reps(cfg);
    strb_addc(cfg->strb, '}');
}

void nonterm_onezero (randexp_cfg_t *cfg)
{
    nonterm_elem(cfg);
    strb_addc(cfg->strb, '?');
}

void nonterm_one (randexp_cfg_t *cfg)
{
    nonterm_elem(cfg);
    strb_addc(cfg->strb, '+');
}

void nonterm_zero (randexp_cfg_t *cfg)
{
    nonterm_elem(cfg);
    strb_addc(cfg->strb, '*');
}

void nonterm_basic (randexp_cfg_t *cfg)
{
    nonterm_t nonterm;

    nonterm = basic_choices[rand_range(0, (BASIC_CHOICES - 1))];
    nonterm(cfg);
}

void nonterm_concat (randexp_cfg_t *cfg)
{
    nonterm_simple(cfg);
    nonterm_basic(cfg);
}

void nonterm_simple (randexp_cfg_t *cfg)
{
    uint8_t val;

    if (cfg->strb->size >= cfg->limit) {
        val = 0;
    } else {
        val = cfg->tokens;
    }

    if (choice(val)) {
        nonterm_concat(cfg);
    } else {
        nonterm_basic(cfg);
    }
}

void nonterm_union (randexp_cfg_t *cfg)
{
    nonterm_re(cfg);
    strb_addc(cfg->strb, '|');
    nonterm_simple(cfg);
}

void nonterm_re (randexp_cfg_t *cfg)
{
    if (rand_range(0, 1)) {
        nonterm_union(cfg);
    } else {
        nonterm_simple(cfg);
    }
}

char *gen_randexp (randexp_cfg_t *cfg, uint64_t *len)
{
    strb_t strb;

    strb_init(&strb, 128);
    cfg->strb = &strb;

    nonterm_re(cfg);

    if (len) {
        *len = strb.size;
    }

    strb_addc(&strb, '\0');
    return strb.buf;
}
