#include <Python.h>

// Define the struct
typedef struct {
    PyObject_HEAD
    int num_vertices;
    int (*adj_list)[16]; // Array of arrays (adjacency list)
} AdjacencyList;

// Define the initialization function
static int AdjacencyList_init(AdjacencyList *self, PyObject *args, PyObject *kwds) {
    // Parse arguments and initialize the struct
    if (!PyArg_ParseTuple(args, "i", &self->num_vertices)) {
        return -1;
    }
    // Allocate memory for the adjacency list
    self->adj_list = (int(*)[16])malloc(self->num_vertices * sizeof(int[16]));
    if (self->adj_list == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
        return -1;
    }
    // Initialize the adjacency list with zeros
    for (int i = 0; i < self->num_vertices; ++i) {
        for (int j = 0; j < 16; ++j) {
            self->adj_list[i][j] = 0;
        }
    }
    return 0;
}

// Define the deallocation function
static void AdjacencyList_dealloc(AdjacencyList *self) {
    // Free allocated memory
    if (self->adj_list != NULL) {
        free(self->adj_list);
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Define the methods
static PyMethodDef AdjacencyList_methods[] = {
    {NULL}  /* Sentinel */
};

// Define the type object
static PyTypeObject AdjacencyListType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "simple_graphs.AdjacencyList",
    .tp_doc = "Adjacency List object",
    .tp_basicsize = sizeof(AdjacencyList),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)AdjacencyList_init,
    .tp_dealloc = (destructor)AdjacencyList_dealloc,
    .tp_methods = AdjacencyList_methods,
};

// Define the module initialization function
static PyModuleDef simple_graphs_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "simple_graphs",
    .m_doc = "Module containing simple graph classes",
    .m_size = -1,
};

// Define the module initialization function
PyMODINIT_FUNC PyInit_simple_graphs(void) {
    PyObject *m;
    if (PyType_Ready(&AdjacencyListType) < 0)
        return NULL;

    m = PyModule_Create(&simple_graphs_module);
    if (m == NULL)
        return NULL;

    Py_INCREF(&AdjacencyListType);
    PyModule_AddObject(m, "AdjacencyList", (PyObject *)&AdjacencyListType);
    return m;
}
