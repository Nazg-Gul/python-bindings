/**
 * Extended functionality of Python bindings
 *
 * Copyright 2009 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "iface.h"
#include <wchar.h>

#define _GET_FIELD_VALUE(err_val, check_cond, cast) \
  PyObject *field; \
   \
  if (!obj || !attr_name) \
    { \
      return err_val; \
    } \
 \
  field = extpy_get_attr_string (obj, attr_name); \
 \
  if (!field) \
    { \
      return err_val; \
    } \
 \
  if (!(check_cond)) \
    { \
      return err_val; \
    } \
  \
  result = cast (field); \
  Py_DECREF (field);


/**
 * Fill stdout and stderr fields of running result
 */
static inline void
fill_result_outputs (extpy_run_result_t *result)
{
  result->stdout = py_tracer_get_buffer (PY_STDOUT);
  result->stderr = py_tracer_get_buffer (PY_STDERR);
}

/**
 * Create error object for return
 *
 * @param error_msg - error message
 * @return error object to return
 */
PyObject*
extpy_return_pyobj_error (PyObject *type, wchar_t *error_msg)
{
  char *mbmsg;

  WCS2MBS (mbmsg, error_msg);

  PyErr_SetString (type, mbmsg);

  free (mbmsg);

  return NULL;
}

/*
 * Set dictionary key's value without causing memory leaks.
 * When some object is just created to be added to the dictionary,
 * its reference count needs to be decremented so it can be reclaimed.
 *
 * @param dict - dictionary to operate on
 * @param key - dictionary's key
 * @param value - value to set
 */
int
extpy_dict_set_item_str (PyObject *dict, wchar_t *key, PyObject *value)
{
  char *mbkey;

  WCS2MBS (mbkey, key);

  /* Add value to dictionary */
  int ret = PyDict_SetItemString (dict, mbkey, value);

  free (mbkey);

  /* Delete original */
  Py_DECREF (value);

  return ret;
}

/**
 * Run python file
 *
 * @param filename - name of file to run
 * @return run result
 * @sideeffect allocate memory for output value. USe extpy_run_free() to free
 */
extpy_run_result_t*
extpy_run_file (wchar_t *filename)
{
  extpy_run_result_t *result;

  MALLOC_ZERO (result, sizeof (extpy_run_result_t));

  py_tracer_truncate_buffer (PY_STDOUT);
  py_tracer_truncate_buffer (PY_STDERR);
  result->result = py_run_file (filename);
  fill_result_outputs (result);

  return result;
}

/**
 * Run python script
 *
 * @param script - script to run
 * @return run result
 * @sideeffect allocate memory for output value. USe extpy_run_free() to free
 */
extpy_run_result_t*
extpy_run_script (py_script_t *script)
{
  extpy_run_result_t *result;

  MALLOC_ZERO (result, sizeof (extpy_run_result_t));

  py_tracer_truncate_buffer (PY_STDOUT);
  py_tracer_truncate_buffer (PY_STDERR);
  result->result = py_run_script (script);
  fill_result_outputs (result);

  return result;
}

/**
 * Free running results
 *
 * @param result - results to be freed
 */
void
extpy_run_free (extpy_run_result_t* result)
{
  if (!result)
    {
      return;
    }

  if (result->result)
    {
      Py_DECREF (result->result);
    }

  SAFE_FREE (result->stdout);
  SAFE_FREE (result->stderr);
  SAFE_FREE (result);
}

/****
 * Object's attributes manipulation
 */

/**
 * Retrieve an attribute from object
 *
 * @param obj - object to get attr of
 * @param attr_name - name of attribute to get
 * @return attribute's value
 */
PyObject*
extpy_get_attr_string (PyObject *obj, const wchar_t *attr_name)
{
  char *mbname;
  PyObject *result;

  if (!obj || !attr_name)
    {
      return NULL;
    }

  WCS2MBS (mbname, attr_name);

  if (!mbname)
    {
      return NULL;
    }

  result = PyObject_GetAttrString (obj, mbname);

  free (mbname);

  return result;
}

/**
 * Set the value of an attribute
 *
 * @param obj - object to set attribute of
 * @param attr_name - name of attribute to set
 * @param val - new value of attribute
 */
int
extpy_set_attr_string (PyObject *obj, const wchar_t *attr_name, PyObject *val)
{
  char *mbname;
  int result;

  if (!obj || !attr_name)
    {
      return -1;
    }

  WCS2MBS (mbname, attr_name);

  if (!mbname)
    {
      return -1;
    }

  result = PyObject_SetAttrString (obj, mbname, val);

  free (mbname);

  return result;
}

/**
 * Check if object has specified attribute
 *
 * @param obj - object to check attribute of
 * @param attr_name - name of attribute to check
 * @return zero if there is no such attribute in object, non-zero otherwise
 */
int
extpy_has_attr_string (PyObject *obj, const wchar_t *attr_name)
{
  char *mbname;
  int result;

  if (!obj || !attr_name)
    {
      return 0;
    }

  WCS2MBS (mbname, attr_name);

  if (!mbname)
    {
      return 0;
    }

  result = PyObject_HasAttrString (obj, mbname);

  free (mbname);

  return result;
}

/**
 * Get long-value object's attribute
 *
 * @param obj - object to get attribute's value of
 * @param attr_name - name of attribute
 * @return specified attribute's value
 */
long
extpy_get_long_attr (PyObject *obj, const wchar_t *attr_name)
{
  long result;

  _GET_FIELD_VALUE (0, (PyInt_Check (field) || PyLong_Check (field)),
                    PyLong_AsLong);

  return result;
}

/**
 * Get double-value object's attribute
 *
 * @param obj - object to get attribute's value of
 * @param attr_name - name of attribute
 * @return specified attribute's value
 */
double
extpy_get_double_attr (PyObject *obj, const wchar_t *attr_name)
{
  double result;

  _GET_FIELD_VALUE (0, PyFloat_Check (field), PyFloat_AsDouble);

  return result;
}

/**
 * Get string-value object's attribute
 *
 * @param obj - object to get attribute's value of
 * @param attr_name - name of attribute
 * @return specified attribute's value
 * @sideeffect allocate memory for output value
 */
wchar_t*
extpy_get_string_attr (PyObject *obj, const wchar_t *attr_name)
{
  char *result;
  wchar_t *wcs_result;

  _GET_FIELD_VALUE (NULL, PyString_Check (field), PyString_AsString);

  MBS2WCS (wcs_result, result);

  return wcs_result;
}
