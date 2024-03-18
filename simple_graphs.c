#include <Python.h>

// Define the struct
typedef struct {
    PyObject_HEAD
    int num_vertices;
    PyObject **adj_list; // Array of Python lists (sorted adjacency lists)
} AdjacencyList;

// Comparator function for sorting
int compare(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}

// Define the initialization function
static int AdjacencyList_init(AdjacencyList *self, PyObject *args, PyObject *kwds) {
    const char *g6 = "?"; // Input g6 format string
    if (!PyArg_ParseTuple(args, "s", &g6)) {
        return -1;
    }
    
    // Determine the number of vertices from g6 format
    int len = strlen(g6);
    self->num_vertices = g6[0] - 63;

    // Allocate memory for the adjacency list
    self->adj_list = (PyObject **)malloc(self->num_vertices * sizeof(PyObject *));
    if (self->adj_list == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
        return -1;
    }

    // Initialize adjacency lists as empty Python lists
    for (int i = 0; i < self->num_vertices; ++i) {
        self->adj_list[i] = PyList_New(0);
        if (self->adj_list[i] == NULL) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create Python list");
            return -1;
        }
    }

    // Decode g6 format and populate adjacency lists
    int k = 1; // Index to traverse g6 string
    int i = 0;
    int c;
    for (int v = 1; v < self->num_vertices; ++v) {
        for (int u = 0; u < v; ++u) {
            if (k==0)
            {
                int c = g6[i] - 63;
                i++;
                k=6;
            }
            k--;
            if ((c & (1 << k)) != 0)
            {
                PyList_Append(self->adj_list[u], PyLong_FromLong(v));
                PyList_Append(self->adj_list[v], PyLong_FromLong(u));
            }
            // if ((g6[k] >> (5 - (u % 6))) & 1) {
            //     // There is an edge between u and v
            //     PyList_Append(self->adj_list[u], PyLong_FromLong(v));
            //     PyList_Append(self->adj_list[v], PyLong_FromLong(u));
            // }
            // if (++k == len) break; // Ensure not to exceed the length of g6
        }
    }

    // Sort adjacency lists
    for (int i = 0; i < self->num_vertices; ++i) {
        PyList_Sort(self->adj_list[i]);
    }

    return 0;
}

// Destructor to deallocate memory
static void AdjacencyList_dealloc(AdjacencyList *self) {
    for (int i = 0; i < self->num_vertices; ++i) {
        Py_DECREF(self->adj_list[i]);
    }
    free(self->adj_list);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// Implementation of number_of_vertices method
static PyObject *number_of_vertices(AdjacencyList *self) {
    return PyLong_FromLong(self->num_vertices);
}

// Implementation of number_of_vertices method
static PyObject *Alist(AdjacencyList *self) {
    return self->adj_list[0];
}

static PyObject *vertices(AdjacencyList *self) {
    PyObject *vertices_set = PySet_New(NULL);
    if (!vertices_set) {
        return NULL;
    }
    for (int i = 0; i < self->num_vertices; ++i) {
        if (self->adj_list[i] != NULL)
        {
            PyObject *item = PyLong_FromLong(i);
            PySet_Add(vertices_set, item);
        };
    }

    return vertices_set;
}

static PyObject *number_of_edges(AdjacencyList *self) {
    int sum = 0;
    for (int i = 0; i < self->num_vertices; ++i) {
        sum += PyList_Size(self->adj_list[i]);
    }
    sum = sum / 2;
    return PyLong_FromLong(sum);
}

// Define the methods
static PyMethodDef AdjacencyList_methods[] = {
    {"number_of_vertices", (PyCFunction)number_of_vertices, METH_NOARGS},
    {"vertices", (PyCFunction)vertices, METH_NOARGS},
    {"number_of_edges", (PyCFunction)number_of_edges, METH_NOARGS},
    {"Alist", (PyCFunction)Alist, METH_NOARGS},
    {NULL, NULL}  /* Sentinel */
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
