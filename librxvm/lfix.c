static unsigned int ltemp0;
static unsigned int ltempn;
static unsigned int lfix0;
static unsigned int lfixn;
static unsigned int lfixs;

void lfix_zero (void)
{
    ltemp0 = ltempn = lfix0 = lfixn = 0;
}

void lfix_start (char *orig, char *lp1)
{
    lfixs = 1;
    ltemp0 = ltempn = lp1 - orig;
}

void lfix_continue (char *orig, char *lp1)
{
    if (lfixs) ++ltempn;
    else lfix_start(orig, lp1);
}

unsigned int lfix_longest (void)
{
    return lfixn - lfix0;
}

unsigned int lfix_current (void)
{
    return ltempn - ltemp0;
}

void lfix_set_longest (void)
{
    lfix0 = ltemp0;
    lfixn = ltempn;
}

void lfix_update_longest (void)
{
    if (lfix_current() > lfix_longest()) {
        lfix_set_longest();
    }
}

void lfix_stop (void)
{
    lfix_update_longest();
    lfixs = ltemp0 = ltempn = 0;
}

void lfix_dec (void)
{
    if (ltempn > ltemp0) --ltempn;
}

unsigned int get_lfix0 (void)
{
    return lfix0;
}

unsigned int get_lfixn (void)
{
    return lfixn;
}

void set_lfixn (unsigned int val)
{
    lfixn = val;
}

void set_ltempn (unsigned int val)
{
    ltempn = val;
}

unsigned int get_ltempn (void)
{
    return ltempn;
}
