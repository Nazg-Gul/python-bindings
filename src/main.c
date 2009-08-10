#include <stdlib.h>
#include "python/iface.h"

PY_METHOD(my_method)
  char *s;

  PY_PARSE_TUPLE ("s", L"Method expects one string argument", &s);

  printf ("Method `my_method`: %s\n", s);
PY_METH_END

PY_BEGIN_METHMAP(methods)
  PY_METHMAP_DEF (L"my_method", my_method, METH_VARARGS, L"Some documentation")
PY_END_METHMAP

PY_INITTAB_PROC(test_init, L"Test", L"My first test module", methods)
  PY_DEF_INT_CONST (L"TRUE", 1);
  PY_DEF_INT_CONST (L"FALSE", 0);
PY_INITTAB_END_PROC

PY_BEGIN_INITTAB(inittab_modules)
  PY_INITTAB_DEF ("Test", test_init)
PY_END_INITTAB

int
main (int argc, char **argv)
{
  if (python_init (argc, argv, inittab_modules))
    {
      return EXIT_FAILURE;
    }

  py_run_file (L"../t/main.py");

  python_done ();

  return EXIT_SUCCESS;
}
