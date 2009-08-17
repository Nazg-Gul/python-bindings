/**
 * Extended functionality of Python bindings
 *
 * Copyright 2008 Sergey I. Sharybin <g,ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#ifndef PYTHON_IFACE_H
#  error "Do not include this file directly. Include iface.h instead."
#endif

#define EXTPY_CALL_OBJECT(result, object, format, args...) \
  { \
    PyObject *o_args = Py_BuildValue (format, ##args); \
    result = PyEval_CallObject (object, o_args); \
    Py_DECREF (o_args); \
  }

#define EXTPY_VOIDCALL_OBJECT(object, format, args...) \
  { \
    PyObject *o_args = Py_BuildValue (format, ##args), *result; \
    result = PyEval_CallObject (object, o_args); \
    Py_DECREF (o_args); \
    if (result) \
      { \
        Py_DECREF (result); \
      } \
  }

typedef struct {
  PyObject *result;

  wchar_t *stdout;
  wchar_t *stderr;
} extpy_run_result_t;

/* Create error object for return */
PyObject*
extpy_return_pyobj_error (PyObject *type, wchar_t *error_msg);

/* Set dictionary key's value without causing memory leaks */
int
extpy_dict_set_item_str (PyObject *dict, wchar_t *key, PyObject *value);

/* Run python file */
extpy_run_result_t*
extpy_run_file (wchar_t *filename);

/* Run python script */
extpy_run_result_t*
extpy_run_script (py_script_t *script);

/* Free running results */
void
extpy_run_free (extpy_run_result_t* result);
