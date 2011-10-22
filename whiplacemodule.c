#include <Python.h>

/* whiplace module function */
static PyObject *
whiplace(PyObject *self, PyObject *args)
{

   char *fname;
   FILE *f;

   PyTupleObject *keyTuple
   PyTupleObject *valueTuple

   if (! PyArg_ParseTuple(args, "OOs", &keyTuple, &valueTuple,
            &fname)) {
       return NULL;
   }
   else {
      f = PySys_GetFile(fname, stdin);
      /* Format the tuples to arrays */
      int nkeys = PyTuple_Size(keyArray);
      if (PyTuple_Size(valueArray) != nkeys) {
         return NULL;
      }

      char *keyArray[], *valueArray[]; // <<<<<<<< DYNAMIC ALLOCATION
      /* FILE *PySys_GetFile(char *name, FILE *def) */
   }

}

/* registration table */
static PyMethodDef whiplace_methods[] = {
   {"whiplace", whiplace, METH_VARARGS,
      "Fast string replacement."},
   {NULL, NULL, 0, NULL}
};

/* module initializer */
void initwhiplace() {
   Py_InitModule("whiplace", whiplace_methods);
}
