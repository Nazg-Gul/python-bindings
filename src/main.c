#include <stdlib.h>
#include "python/iface.h"

PY_METHOD(my_method)
  PyObject *o;
  wchar_t *v;

  PY_PARSE_TUPLE ("O", L"Method expects one object argument", &o);

  v = extpy_get_string_attr (o, L"stringField");

  py_proc_write (PY_STDOUT, L"longField: %ld\n",
                 extpy_get_long_attr (o, L"longField"));

  py_proc_write (PY_STDOUT, L"floatField: %lf\n",
                 extpy_get_double_attr (o, L"floatField"));

  py_proc_write (PY_STDOUT, L"stringField: %ls\n", v);

  free (v);
PY_METH_END

PY_BEGIN_METHMAP(methods)
  PY_METHMAP_DEF (L"my_method", my_method, METH_VARARGS, L"Some documentation")
PY_END_METHMAP

PY_INITTAB_PROC(test_init, L"Test", L"My first test module", methods)
  PY_DEF_INT_CONST (L"DEBUG", 1);
PY_INITTAB_END_PROC

PY_BEGIN_INITTAB(inittab_modules)
  PY_INITTAB_DEF ("Test", test_init)
PY_END_INITTAB

int
main (int argc, char **argv)
{
  extpy_run_result_t* result;

  if (python_init (argc, argv, inittab_modules))
    {
      return EXIT_FAILURE;
    }

  result = extpy_run_file (L"../t/main.py");
  printf ("Buffer from stdout:\n%ls", result->stdout);
  printf ("\nBuffer from stderr:\n%ls", result->stderr);
  extpy_run_free (result);

  python_done ();

  return EXIT_SUCCESS;
}
