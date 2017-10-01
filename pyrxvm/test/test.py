# These are just some basic sanity checks to ensure the python bindings are
# properly passing things through to librxvm. The full test suite for librxvm
# can be found under the top-level 'test' directory

import unittest
import rxvm

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
