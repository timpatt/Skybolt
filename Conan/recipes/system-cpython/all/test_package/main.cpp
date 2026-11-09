#include <Python.h>

int main()
{
	Py_Initialize();
	if (!Py_IsInitialized())
	{
		return 1;
	}

	PyObject* module = PyImport_ImportModule("sys");
	if (!module)
	{
		PyErr_Print();
		Py_FinalizeEx();
		return 2;
	}

	Py_DECREF(module);
	return Py_FinalizeEx() < 0 ? 3 : 0;
}
