// Pulls in the Python API
#include <Python.h>
#include <rxvm.h>
#include "structmember.h"

#define NUM_RXVM_EXC 16

char *exception_descs[NUM_RXVM_EXC][2] = {
    {"rxvm.BadOperator", "Operator used incorrectly"},
    {"rxvm.BadClass", "Unexpected character class closing character"},
    {"rxvm.BadRepetition", "Unexpected repetition closing character"},
    {"rxvm.BadParenthesis", "Unexpected closing parenthesis"},
    {"rxvm.MissingClosingParenthesis", "Missing closing parenthesis"},
    {"rxvm.MissingClosingClass", "Missing character class closing character"},
    {"rxvm.MissingClosingRepetition", "Missing repetition closing character"},
    {"rxvm.IncompleteCharacterRange", "Incomplete range within character class"},
    {"rxvm.ExtraRepetitionComma", "Extra comma in repetition"},
    {"rxvm.NonNumericRepetition", "Non-numerical character in repetition"},
    {"rxvm.EmptyRepetition", "Empty repetition"},
    {"rxvm.TrailingEscape", "Tailing escape character"},
    {"rxvm.InvalidSymbol", "Unsupported symbol in input stream"},
    {"rxvm.MemoryAllocationError", "Failed to allocate memory"},
    {"rxvm.InvalidParameter", "Invalid parameter passed to librxvm function"},
    {"rxvm.IOError", "FILE pointer error in rxvm_fsearch()"}
};

PyObject *exceptions[NUM_RXVM_EXC];

PyObject *UnknownError;
PyObject *m;

typedef struct {
    PyObject_HEAD
    rxvm_t compiled;
} Pyrxvm_t;

static void Pyrxvm_dealloc(Pyrxvm_t* self)
{
    rxvm_free(&self->compiled);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *Pyrxvm_Raise(int err)
{
    err = -err;

    if (err > NUM_RXVM_EXC) {
        return PyErr_Format(UnknownError, "Unknown error code %d", -err);
    }

    PyErr_SetString(exceptions[err - 1], exception_descs[err - 1][1]);
    return NULL;
}

static int Pyrxvm_init (Pyrxvm_t *self, PyObject *args, PyObject *kwargs)
{
    int ret;
    char *regex;

    if (!PyArg_ParseTuple(args, "s", &regex)) {
        return -1;
    }

    ret = rxvm_compile(&self->compiled, regex);
    if (ret < 0) {
        Pyrxvm_Raise(ret);
    }

    return 0;
}

static PyObject *Pyrxvm_fsearch (Pyrxvm_t *self, PyObject *args,
    PyObject *kwargs)
{
    PyObject *ret;
    PyObject *File;
    PyFileObject *PyFile;

    FILE *fp;
    char *match;
    int flags = 0;
    char *kwlist[] = {"file", "flags", NULL};

    uint64_t match_size;
    int err;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!|i", kwlist,
            &PyFile_Type, &File, &flags)) {
        return NULL;
    }

    PyFile = (PyFileObject *)File;
    fp = PyFile_AsFile(File);
    PyFile_IncUseCount(PyFile);

    err = rxvm_fsearch(&self->compiled, fp, &match_size, flags);

    if (err < 0) {
        ret = Pyrxvm_Raise(err);

    } else if (err == 0) {
        ret = Py_None;

    } else {
        if ((match = malloc(match_size + 1)) == NULL) {
            ret = Pyrxvm_Raise(RXVM_EMEM);

        } else if (fread(match, 1, match_size, fp) != match_size) {
            ret = Pyrxvm_Raise(RXVM_IOERR);

        } else {
            match[match_size] = '\0';
            ret = Py_BuildValue("s", match);
            free(match);
        }
    }

    PyFile_DecUseCount(PyFile);
    return ret;
}

static PyObject *Pyrxvm_gen (Pyrxvm_t *self, PyObject *args, PyObject *kwargs)
{
    PyObject *ret;
    char *gen;
    char *kwlist[] = {"generosity", "whitespace", "limit", NULL};
    rxvm_gencfg_t gencfg;

    int generosity = 50;
    int limit = 1000;
    int whitespace = 10;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|iii", kwlist, &generosity,
            &whitespace, &limit)) {
        return NULL;
    }

    gencfg.generosity = (generosity > 100) ? 100 : generosity;
    gencfg.whitespace = (whitespace > 100) ? 100 : whitespace;
    gencfg.limit = limit;

    gen = rxvm_gen(&self->compiled, &gencfg);
    if (gen == NULL) {
        return Pyrxvm_Raise(RXVM_EMEM);
    }

    ret = Py_BuildValue("s", gen);
    free(gen);

    return ret;
}

static PyObject *Pyrxvm_match (Pyrxvm_t *self, PyObject *args, PyObject *kwargs)
{
    char *input;
    int flags = 0;
    int ret;

    char *kwlist[] = {"input", "flags", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|i", kwlist, &input,
            &flags)) {
        return NULL;
    }

    ret = rxvm_match(&self->compiled, input, flags);
    if (ret < 0) {
        return Pyrxvm_Raise(ret);
    }

    return (ret) ? Py_True : Py_False;
}

static PyObject *Pyrxvm_search (Pyrxvm_t *self, PyObject *args,
    PyObject *kwargs)
{
    PyObject *ret;
    char *input;
    char *match;
    char *start, *end;
    int flags = 0;
    int index = 0;
    int err;
    int msize;

    char *kwlist[] = {"input", "index", "flags", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|ii", kwlist, &input,
            &index, &flags)) {
        return NULL;
    }

    err = rxvm_search(&self->compiled, input + index, &start, &end, flags);
    if (err < 0) {
        return Pyrxvm_Raise(err);
    }

    if (err) {
        msize = (end - start) + 1;
        if ((match = malloc(msize + 1)) == NULL) {
            return Pyrxvm_Raise(RXVM_EMEM);
        }

        memcpy(match, start, msize);
        match[msize] = '\0';

        ret = Py_BuildValue("(si)", match, (end - input) + 1);
        free(match);
        return ret;
    }

    return Py_BuildValue("(Oi)", Py_None, index);
}

static PyMethodDef Pyrxvm_methods[] = {
    {"match", (PyCFunction)Pyrxvm_match, METH_VARARGS | METH_KEYWORDS,
        "Checks if the input string matches the compiled expression"},
    {"search", (PyCFunction)Pyrxvm_search, METH_VARARGS | METH_KEYWORDS,
        "Searches a string for next occurrence of the compiled expression"},
    {"fsearch", (PyCFunction)Pyrxvm_fsearch, METH_VARARGS | METH_KEYWORDS,
        "Searches file pointer for next occurence of the compiled expression"},
    {"gen", (PyCFunction)Pyrxvm_gen, METH_VARARGS | METH_KEYWORDS,
        "Generates a pseudo-random string matching the compiled expression"},
    {NULL}
};

static PyTypeObject Pyrxvm_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "rxvm.Rxvm",                                         /* tp_name */
    sizeof(Pyrxvm_t),                                    /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)Pyrxvm_dealloc,                          /* tp_dealloc */
    0,                                                   /* tp_print */
    0,                                                   /* tp_getattr */
    0,                                                   /* tp_setattr */
    0,                                                   /* tp_compare */
    0,                                                   /* tp_repr */
    0,                                                   /* tp_as_number */
    0,                                                   /* tp_as_sequence */
    0,                                                   /* tp_as_mapping */
    0,                                                   /* tp_hash */
    0,                                                   /* tp_call */
    0,                                                   /* tp_str */
    0,                                                   /* tp_getattro */
    0,                                                   /* tp_setattro */
    0,                                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                                  /* tp_flags */
    "Compiled Pyrxvm object",                            /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    Pyrxvm_methods,                                      /* tp_methods */
    0,                                                   /* tp_members */
    0,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    (initproc)Pyrxvm_init,                               /* tp_init */
    0,                                                   /* tp_alloc */
    0,                                                   /* tp_new */
};

static PyObject *Pyrxvm_compile (PyObject *self, PyObject *args,
    PyObject *kwargs)
{
    PyObject *compiled;

    compiled = PyObject_Call((PyObject *)&Pyrxvm_Type, args, kwargs);

    if (PyErr_Occurred()) {
        return NULL;
    }

    return compiled;
}

static PyMethodDef module_methods[] = {
    {"compile", (PyCFunction)Pyrxvm_compile, METH_VARARGS | METH_KEYWORDS,
        "Compile a regular expression"},
    {NULL}
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initrxvm(void)
{
    int i;

    Pyrxvm_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&Pyrxvm_Type) < 0)
        return;

    m = Py_InitModule3("rxvm", module_methods, "Python bindings for librxvm");
    Py_INCREF(&Pyrxvm_Type);
    PyModule_AddObject(m, "pyrxvm", (PyObject *)&Pyrxvm_Type);

    /* Create exceptions to wrap librxvm error codes */
    for (i = 0; i < NUM_RXVM_EXC; i++) {
        exceptions[i] = PyErr_NewException(exception_descs[i][0], NULL, NULL);
        PyModule_AddObject(m, exception_descs[i][0] + 5, exceptions[i]);
    }

    /* Default exception for unknown error codes */
    UnknownError = PyErr_NewException("rxvm.UnknownError", NULL, NULL);
    PyModule_AddObject(m, "UnknownError", UnknownError);

    /* Module globals for rxvm flag constants */
    PyModule_AddObject(m, "ICASE", Py_BuildValue("i", RXVM_ICASE));
    PyModule_AddObject(m, "NONGREEDY", Py_BuildValue("i", RXVM_NONGREEDY));
    PyModule_AddObject(m, "MULTILINE", Py_BuildValue("i", RXVM_MULTILINE));
}
