#include <stdio.h>
#include <stdlib.h>
#include "rxvm.h"
#include "test_common.h"

static const char *func;
static int count;

static void log_trs (char *msg, const char *func)
{
    fprintf(trsfp, ":test-result: %s %s #%d\n", msg, func, count);
}

void verify_rxvm_match (char *regex, char *input, int ret)
{
    rxvm_t compiled;
    char *msg;
    int err;

    msg = "PASS";

    if ((err = compile_testexp(&compiled, regex)) < 0) {
        log_trs("FAIL", func);
        fprintf(logfp, "Error: compilation failed (%d): %s\n",
                err, regex);
        return;
    }

    if ((err = rxvm_match(&compiled, input, 0)) != ret) {
        fprintf(logfp, "Error: matching input %s against expression "
                "%s: Expecting %d, got %d\n", input, regex, ret, err);
        msg = "FAIL";
    }

    rxvm_free(&compiled);

    log_trs(msg, func);
    printf("%s: %s #%d\n", msg, func, ++count);
}

void test_rxvm_match (void)
{
    count = 0;
    func = __func__;

    verify_rxvm_match("abc", "abc", 1);
    verify_rxvm_match("abc", "ab", 0);
    verify_rxvm_match("abc", "", 0);
    verify_rxvm_match("abc", "abcc", 0);
    verify_rxvm_match("abc", "abcd", 0);
    verify_rxvm_match("abc", "dabc", 0);

    verify_rxvm_match("ab|", "ab", 1);
    verify_rxvm_match("ab|", "", 1);
    verify_rxvm_match("ab|", "ab", 1);
    verify_rxvm_match("ab|", "abb", 0);
    verify_rxvm_match("ab|", "b", 0);
    verify_rxvm_match("ab|", "a", 0);
    verify_rxvm_match("ab|", "x", 0);

    verify_rxvm_match("q*", "", 1);
    verify_rxvm_match("q*", "q", 1);
    verify_rxvm_match("q*", "qq", 1);
    verify_rxvm_match("q*", "qqqqqq", 1);
    verify_rxvm_match("q*", "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq", 1);
    verify_rxvm_match("q*", "qd", 0);
    verify_rxvm_match("q*", "pq", 0);
    verify_rxvm_match("q*", "x", 0);
    verify_rxvm_match("q*", "qqqqqqqqqqqqqqq7qq", 0);

    verify_rxvm_match("((cc)?)+", "", 1);
    verify_rxvm_match("((cc)?)+", "cc", 1);
    verify_rxvm_match("((cc)?)+", "cccc", 1);
    verify_rxvm_match("((cc)?)+", "cccccc", 1);
    verify_rxvm_match("((cc)?)+", "cccccccccccccccccccccccccccccccccccccc", 1);
    verify_rxvm_match("((cc)?)+", "ccccccccccccd", 0);
    verify_rxvm_match("((cc)?)+", "ccc", 0);
    verify_rxvm_match("((cc)?)+", "d", 0);

    verify_rxvm_match("(cc)*", "", 1);
    verify_rxvm_match("(cc)*", "cc", 1);
    verify_rxvm_match("(cc)*", "cccc", 1);
    verify_rxvm_match("(cc)*", "cccccc", 1);
    verify_rxvm_match("(cc)*", "ccccccccccccccccccccccccccccccccccccccccc", 0);
    verify_rxvm_match("(cc)*", "ccccccccccccd", 0);
    verify_rxvm_match("(cc)*", "ccc", 0);
    verify_rxvm_match("(cc)*", "d", 0);

    verify_rxvm_match("[A-Za-z]+", "h", 1);
    verify_rxvm_match("[A-Za-z]+", "ssefsnfugb", 1);
    verify_rxvm_match("[A-Za-z]+", "uHuiBIJHBHgDDb", 1);
    verify_rxvm_match("[A-Za-z]+", "AJIJIFHUsxskmskxmsqkoxmqsuaaGIC", 1);
    verify_rxvm_match("[A-Za-z]+", "dyRHdNJgzrdfrrhhthdlIUKJFghfKMUFNIN", 1);
    verify_rxvm_match("[A-Za-z]+", "", 0);
    verify_rxvm_match("[A-Za-z]+", "0", 0);
    verify_rxvm_match("[A-Za-z]+", "0490", 0);
    verify_rxvm_match("[A-Za-z]+", "$", 0);
    verify_rxvm_match("[A-Za-z]+", "}", 0);

    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+", "h", 1);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+", "ssefsnfugb", 1);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+",
        "uHuiBIJHBHgDDb", 1);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+",
        "AJIJIFHUsxskmskxmsqkoxmqsuaaGIC", 1);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+",
        "dyRHdilhjHRJiFIfniFNJgzrdgggrhyjfjfrrrrrhhhthhnzmrhdlIUKJFghfKMUFNIN",
        1);

    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+", "", 0);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+", "0", 0);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+", "0490", 0);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+", "$", 0);
    verify_rxvm_match("(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|X|Y|Z|a|b"
        "|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+", "}", 0);

    verify_rxvm_match("a?b*(cde)+", "abcde", 1);
    verify_rxvm_match("a?b*(cde)+", "bcde", 1);
    verify_rxvm_match("a?b*(cde)+", "cde", 1);
    verify_rxvm_match("a?b*(cde)+", "bbcdecde", 1);
    verify_rxvm_match("a?b*(cde)+", "abbbbbbbbbbbbbbbbbbbbbbcdecdecde", 1);
    verify_rxvm_match("a?b*(cde)+", "aabcde", 0);
    verify_rxvm_match("a?b*(cde)+", "abcd", 0);
    verify_rxvm_match("a?b*(cde)+", "ab", 0);
    verify_rxvm_match("a?b*(cde)+", "a", 0);
    verify_rxvm_match("a?b*(cde)+", "cdec", 0);

    verify_rxvm_match("a*|b+", "a", 1);
    verify_rxvm_match("a*|b+", "aaaaaaaaaa", 1);
    verify_rxvm_match("a*|b+", "", 1);
    verify_rxvm_match("a*|b+", "b", 1);
    verify_rxvm_match("a*|b+", "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb", 1);

    verify_rxvm_match("a*|b+", "ab", 0);
    verify_rxvm_match("a*|b+", "aaaaaaaaaaaaab", 0);
    verify_rxvm_match("a*|b+", "bbbbbbbbba", 0);
    verify_rxvm_match("a*|b+", "ba", 0);

    verify_rxvm_match("ab|xyz|q|xx", "ab", 1);
    verify_rxvm_match("ab|xyz|q|xx", "xyz", 1);
    verify_rxvm_match("ab|xyz|q|xx", "q", 1);
    verify_rxvm_match("ab|xyz|q|xx", "xx", 1);
    verify_rxvm_match("ab|xyz|q|xx", "abxyzqxx", 0);
    verify_rxvm_match("ab|xyz|q|xx", "abb", 0);
    verify_rxvm_match("ab|xyz|q|xx", "xyza", 0);
    verify_rxvm_match("ab|xyz|q|xx", "xxq", 0);
    verify_rxvm_match("ab|xyz|q|xx", "", 0);

    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "a", 1);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "abcdefg", 1);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "abcccdddefffggggggggggg",1);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "abccccccccc", 1);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "abbbbbbbbb", 1);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "abcdefggggggggga", 0);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "aa", 0);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", "accccccccccch", 0);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", " ", 0);
    verify_rxvm_match("a(b(c(d(e(f(g)*)*)*)*)*)*", ".", 0);

    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "aaa?*???", 1);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "xyY", 1);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)",
        "xY&tqqtt.qqqqtttt.", 1);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)",
        "xY&tttttttt+tttqqqqqqt.qq", 1);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "*", 1);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "a", 0);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "xyYY", 0);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "xY&?", 0);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "xY&tt++", 0);
    verify_rxvm_match("a*[?*]+|x(yY|Y&(tt*[.+]?|(qq)+)*)", "xY&ttt.qqq", 0);

    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwerty*.*?", 1);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwerty******+.*?*?*?*?*?", 1);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwerty", 1);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwerty****++++++", 1);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*",
        "qwerty***************************************", 1);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwert", 0);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwerty+", 0);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwerty*+..", 0);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwerty*?+", 0);
    verify_rxvm_match("qwerty(\\*+\\+*[*?.+])*", "qwertyy", 0);

    verify_rxvm_match("[A-Fa-f@0-9]+", "A", 1);
    verify_rxvm_match("[A-Fa-f@0-9]+", "AbCdEfFfF", 1);
    verify_rxvm_match("[A-Fa-f@0-9]+", "aB0F75EEeeEe12@3456ffff", 1);
    verify_rxvm_match("[A-Fa-f@0-9]+", "@", 1);
    verify_rxvm_match("[A-Fa-f@0-9]+", "9", 1);
    verify_rxvm_match("[A-Fa-f@0-9]+", "G", 0);
    verify_rxvm_match("[A-Fa-f@0-9]+", "", 0);
    verify_rxvm_match("[A-Fa-f@0-9]+", "abcdefg", 0);
    verify_rxvm_match("[A-Fa-f@0-9]+", "AbCdEFz", 0);
    verify_rxvm_match("[A-Fa-f@0-9]+", "+", 0);

    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "\t\n\a\n?*+?\\+", 1);
    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "\r\n\a\n?*+?\\*", 1);
    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "\v\n\a\n?*+?\\?", 1);
    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "\f\n\a\n?*+?\\.", 1);
    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "\b\n\a\n?*+?\\.", 1);

    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "b\n\a\n?*+?\\?", 0);
    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "\t\n\an?*+?\\+", 0);
    verify_rxvm_match("[\t\r\v\f\b]\n\a\n\\?\\*\\+\\?\\\\[+*?.]",
            "\t\n\a\n?*+?\\++", 0);

    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+", "a", 1);
    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+", "asd", 1);
    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+", "kdfg", 1);
    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+", "asdfghjk", 1);
    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+",
            "asgsdhjdhfgsdhjfgakdsjfkdslfskj", 1);

    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+", "", 0);
    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+", "b", 0);
    verify_rxvm_match("(a|s|d|f|g|h|j|k|l)+",
            "asjdhakdfhsdfhkdshfcsdhfsdjfhfj", 0);

    verify_rxvm_match("[asdfghjkl]+", "a", 1);
    verify_rxvm_match("[asdfghjkl]+", "asd", 1);
    verify_rxvm_match("[asdfghjkl]+", "kdfg", 1);
    verify_rxvm_match("[asdfghjkl]+", "asdfghjk", 1);
    verify_rxvm_match("[asdfghjkl]+", "asgsdhjdhfgsdhjfgakdsjfkdslfskj", 1);
    verify_rxvm_match("[asdfghjkl]+", "", 0);
    verify_rxvm_match("[asdfghjkl]+", "b", 0);
    verify_rxvm_match("[asdfghjkl]+", "asjdhakdfhsdfhkdshfcsdhfsdjfhfj", 0);

    verify_rxvm_match("abc{3}", "abccc", 1);
    verify_rxvm_match("abc{3}", "ab", 0);
    verify_rxvm_match("abc{3}", "abc", 0);
    verify_rxvm_match("abc{3}", "abcc", 0);
    verify_rxvm_match("abc{3}", "abcccc", 0);
    verify_rxvm_match("abc{3}", "abccccccccccccccc", 0);

    verify_rxvm_match("abc{4,}", "abcccc", 1);
    verify_rxvm_match("abc{4,}", "abccccc", 1);
    verify_rxvm_match("abc{4,}", "abcccccccccccc", 1);
    verify_rxvm_match("abc{4,}", "abcccccccccccccccccccc", 1);
    verify_rxvm_match("abc{4,}", "ab", 0);
    verify_rxvm_match("abc{4,}", "abc", 0);
    verify_rxvm_match("abc{4,}", "abcc", 0);
    verify_rxvm_match("abc{4,}", "abccc", 0);

    verify_rxvm_match("xyz{,6}", "xy", 1);
    verify_rxvm_match("xyz{,6}", "xyz", 1);
    verify_rxvm_match("xyz{,6}", "xyzz", 1);
    verify_rxvm_match("xyz{,6}", "xyzzzz", 1);
    verify_rxvm_match("xyz{,6}", "xyzzzzzz", 1);
    verify_rxvm_match("xyz{,6}", "xyzzzzzzz", 0);
    verify_rxvm_match("xyz{,6}", "xyzzzzzzzzzzzzz", 0);
    verify_rxvm_match("xyz{,6}", "x", 0);
    verify_rxvm_match("xyz{,6}", "xyzzzzzzzzzzzzzzzzzzzzz", 0);

    verify_rxvm_match("qqw{3,4}", "qqwww", 1);
    verify_rxvm_match("qqw{3,4}", "qqwwww", 1);
    verify_rxvm_match("qqw{3,4}", "qq", 0);
    verify_rxvm_match("qqw{3,4}", "qqw", 0);
    verify_rxvm_match("qqw{3,4}", "qqww", 0);
    verify_rxvm_match("qqw{3,4}", "qqwwwww", 0);
    verify_rxvm_match("qqw{3,4}", "qqwwwwwwwwwwwwwwwwwwww", 0);

    verify_rxvm_match("abc*(def+){2,3}", "abdefdeffdef", 1);
    verify_rxvm_match("abc*(def+){2,3}", "abcccccccdeffffffdefdeffff", 1);
    verify_rxvm_match("abc*(def+){2,3}", "abccccccccdefdef", 1);
    verify_rxvm_match("abc*(def+){2,3}", "abcdedede", 0);
    verify_rxvm_match("abc*(def+){2,3}", "abcdef", 0);
    verify_rxvm_match("abc*(def+){2,3}", "abcdeffffffffff", 0);
    verify_rxvm_match("abc*(def+){2,3}", "abcdefdefdefdef", 0);

    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "abxx", 1);
    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "abbbxxqxxdededede", 1);
    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "abbbbbbdedededeqxxdededede",1);
    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "axxdededededededede", 1);
    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "abqxxdededeqxx", 0);
    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "abqq", 0);
    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "abdededededeq", 0);
    verify_rxvm_match("ab*(q?xx|(de){4}){1,3}", "abxxdedededexxdededede", 0);

    verify_rxvm_match("[^a-f.]+", "878", 1);
    verify_rxvm_match("[^a-f.]+", "KKIIJ", 1);
    verify_rxvm_match("[^a-f.]+", "))&U&U", 1);
    verify_rxvm_match("[^a-f.]+", "_)_)_))_$EDYTR", 1);
    verify_rxvm_match("[^a-f.]+", "+", 1);
    verify_rxvm_match("[^a-f.]+", "a", 0);
    verify_rxvm_match("[^a-f.]+", ".", 0);
    verify_rxvm_match("[^a-f.]+", "00000.", 0);
    verify_rxvm_match("[^a-f.]+", "f.", 0);
    verify_rxvm_match("[^a-f.]+", "abcdef", 0);

    verify_rxvm_match("[^^B*]*", "a", 1);
    verify_rxvm_match("[^^B*]*", "dsfgdr", 1);
    verify_rxvm_match("[^^B*]*", "4649077", 1);
    verify_rxvm_match("[^^B*]*", "kdfghnblkdsngblIHN", 1);
    verify_rxvm_match("[^^B*]*", "AbCDEFGHIJKLMNOP", 1);
    verify_rxvm_match("[^^B*]*", "*", 0);
    verify_rxvm_match("[^^B*]*", "^", 0);
    verify_rxvm_match("[^^B*]*", "^^",0);
    verify_rxvm_match("[^^B*]*", "BB**BB**^^", 0);
    verify_rxvm_match("[^^B*]*", "*^B*^BBB^^BB^", 0);
}
