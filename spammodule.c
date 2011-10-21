#include <Python.h>

static PyObject *
spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    return Py_BuildValue("i", sts);
}


/*
const char *file;
int bufsize = 0;
ok = PyArg_ParseTuple(args, "si", &file, &bufsize);
*/
