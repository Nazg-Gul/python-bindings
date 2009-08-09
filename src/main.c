#include <stdlib.h>
#include <python/iface.h>

static py_module_t *module;

PY_METHOD_FORWARD(my_method);

PY_BEGIN_METHMAP(methods)
  PY_METHMAP_DEF (L"my_method", my_method, METH_VARARGS, L"Some documentation")
PY_END_METHMAP

PY_METHOD(my_method)
  char *s;

  PY_PARSE_TUPLE ("s", L"Method expects one string argument", &s);

  printf ("Method `my_method`: %s\n", s);
PY_METH_END

static void
test_init (void)
{
  module = py_module_new (L"Test", L"My first test module", methods);

  PyModule_AddIntConstant (module->handle, "TRUE", 1);
  PyModule_AddIntConstant (module->handle, "FALSE", 0);
}

static struct _inittab inittab_modules[] = {
  {"Test", test_init},
  {NULL, NULL}
};

int
main (int argc, char **argv)
{
  py_script_t *script;

  if (python_init (argc, argv, inittab_modules))
    {
      return EXIT_FAILURE;
    }

  PyImport_ImportModule ("Test");

  script = py_script_new_file (L"../t/main.py");
  py_run_script (script);
  py_script_free (script);

  py_module_free (module);

  python_done ();

  return EXIT_SUCCESS;
}
