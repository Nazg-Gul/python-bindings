/**
 * Implementation of internal import
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "import.h"

static PyObject*
import (PyObject *self, PyObject *args,  PyObject *kw)
{
  char *name;
  PyObject *globals = NULL, *locals = NULL, *fromlist = NULL;
  PyObject *newmodule;

  static char *kwlist[] = {"name", "globals", "locals", "fromlist", 0};

  if (!PyArg_ParseTupleAndKeywords (args, kw, "s|OOO:py_import_meth", kwlist,
                                    &name, &globals, &locals, &fromlist))
    {
      return NULL;
    }

  /* import existing builtin modules or modules */
  /* that have been imported already */
  newmodule = PyImport_ImportModuleEx (name, globals, locals, fromlist);

  return newmodule;
}

PyMethodDef py_import_meth[] = {
  {"py_import_meth", import, METH_KEYWORDS, "import"}
};
