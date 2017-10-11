# These are just a few basic sanity checks to ensure the python bindings are
# properly passing things through to/from librxvm. The full test suite for
# librxvm can be found under the top-level 'test' directory of the main librxvm
# repository

import os
import unittest
import rxvm

search_patterns = [
    ("abc", "abababcabcabc", ["abc", "abc", "abc"]),
    ("abc+", "abababcabccabc", ["abc", "abcc", "abc"]),
    ("xyz+", "abbbbababbabbxyzxyzzababaxyzzz", ["xyz", "xyzz", "xyzzz"])
]

class RXVMTest(unittest.TestCase):
    def test_compile_exceptions(self):
        # Verify that the mapping between librxvm error codes and exception
        # types is correct
        self.assertRaises(rxvm.BadOperator, rxvm.compile, "+")
        self.assertRaises(rxvm.BadOperator, rxvm.compile, "*")
        self.assertRaises(rxvm.BadOperator, rxvm.compile, "?")
        self.assertRaises(rxvm.BadOperator, rxvm.compile, "{5}")
        self.assertRaises(rxvm.BadClass, rxvm.compile, "]")
        self.assertRaises(rxvm.BadClass, rxvm.compile, "[a]]")
        self.assertRaises(rxvm.BadRepetition, rxvm.compile, "b}")
        self.assertRaises(rxvm.BadRepetition, rxvm.compile, "b{2}}")
        self.assertRaises(rxvm.BadParenthesis, rxvm.compile, "b)")
        self.assertRaises(rxvm.BadParenthesis, rxvm.compile, "(z))")
        self.assertRaises(rxvm.MissingClosingParenthesis, rxvm.compile, "(")
        self.assertRaises(rxvm.MissingClosingParenthesis, rxvm.compile, "(abc")
        self.assertRaises(rxvm.MissingClosingClass, rxvm.compile, "[")
        self.assertRaises(rxvm.MissingClosingClass, rxvm.compile, "[a-f")
        self.assertRaises(rxvm.MissingClosingRepetition, rxvm.compile, "a{2")
        self.assertRaises(rxvm.MissingClosingRepetition, rxvm.compile, "a{2,}{")
        self.assertRaises(rxvm.IncompleteCharacterRange, rxvm.compile, "[a-]")
        self.assertRaises(rxvm.ExtraRepetitionComma, rxvm.compile, "e{3,,}")
        self.assertRaises(rxvm.ExtraRepetitionComma, rxvm.compile, "e{3,8,}")
        self.assertRaises(rxvm.NonNumericRepetition, rxvm.compile, "q{q}")
        self.assertRaises(rxvm.NonNumericRepetition, rxvm.compile, "q{3,t}")
        self.assertRaises(rxvm.EmptyRepetition, rxvm.compile, "4{}")
        self.assertRaises(rxvm.EmptyRepetition, rxvm.compile, "4{,}")
        self.assertRaises(rxvm.TrailingEscape, rxvm.compile, "\\")
        self.assertRaises(rxvm.TrailingEscape, rxvm.compile, "abc\\")

    def test_search(self):
        # Verify that rxvm.search behaves as expected
        for regex, target, matches in search_patterns:
            p = rxvm.compile(regex)
            i = 0
            m = 0

            while m < len(matches):
                match, i = p.search(target, i)
                self.assertEqual(match, matches[m])
                m += 1

            self.assertEqual((None, i), p.search(target, i))

    def test_fsearch(self):
        # Verify that rxvm.fsearch behaves as expected
        fname = ".pyrxvm_fsearch_testfile.txt"

        for regex, target, matches in search_patterns:
            m = 0
            p = rxvm.compile(regex)

            with open(fname, 'w') as fh:
                fh.write(target)

            with open(fname, 'r') as fh:
                while m < len(matches):
                    match = p.fsearch(fh)
                    self.assertEqual(match, matches[m])
                    m += 1

                self.assertEqual(None, p.fsearch(fh))

            os.remove(fname)

    def test_match(self):
        # Verify that rxvm.match behaves as expected
        p = rxvm.compile("abc")
        self.assertTrue(p.match("abc"))
        self.assertFalse(p.match("ab"))
        self.assertFalse(p.match("abcc"))
        self.assertFalse(p.match(""))

        p = rxvm.compile("a(xy|zz*)?bc+")
        self.assertTrue(p.match("abc"))
        self.assertTrue(p.match("abccc"))
        self.assertTrue(p.match("axybccc"))
        self.assertTrue(p.match("azbc"))
        self.assertTrue(p.match("azzzzzbcc"))
        self.assertFalse(p.match("ab"))

def main():
    suite = unittest.TestLoader().loadTestsFromTestCase(RXVMTest)
    unittest.TextTestRunner(verbosity=2).run(suite)

if __name__ == '__main__':
    main()
