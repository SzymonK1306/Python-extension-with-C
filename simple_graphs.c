#include <Python.h>

const int MAX_VERTICES = 16;

typedef struct {
    PyObject_HEAD
    short vertices;
    PyObject **adj_list; 
} AdjacencyList;

int count_bits(short num) {
    int count = 0;
    while (num) {
        count += num & 1; 
        num >>= 1;        
    }
    return count;
}

static int AdjacencyList_init(AdjacencyList *self, PyObject *args, PyObject *kwds) {
    const char *g6 = "?";
    if (!PyArg_ParseTuple(args, "s", &g6)) {
        return -1;
    }
    
    int len = strlen(g6);

    for (int i = 0; i < g6[0] - 63; i++) {
        self->vertices = self->vertices << 1;
        self->vertices = self->vertices | 0x0001;
    }

    self->adj_list = (PyObject **)malloc(MAX_VERTICES * sizeof(PyObject *));
    if (self->adj_list == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory");
        return -1;
    }

    for (int i = 0; i < MAX_VERTICES; ++i) {
        self->adj_list[i] = PyList_New(0);
        if (self->adj_list[i] == NULL) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create Python list");
            return -1;
        }
    }

    int k = 0; 
    int i = 1;
    for (int v = 1; v < count_bits(self->vertices); ++v) {
        for (int u = 0; u < v; ++u) {
            int c;
            if (k==0)
            {
                c = g6[i] - 63;
                i++;
                k=6;
            }
            k--;
            if ((c & (1 << k)) != 0)
            {
                PyList_Append(self->adj_list[u], PyLong_FromLong(v));
                PyList_Append(self->adj_list[v], PyLong_FromLong(u));
            }
        }
    }

    // Sort list
    for (int i = 0; i < count_bits(self->vertices); ++i) {
        PyList_Sort(self->adj_list[i]);
    }

    return 0;
}

// Destructor 
static void AdjacencyList_dealloc(AdjacencyList *self) {
    for (int i = 0; i < MAX_VERTICES; ++i) {
        Py_DECREF(self->adj_list[i]);
    }
    free(self->adj_list);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *number_of_vertices(AdjacencyList *self) {
    return PyLong_FromLong(count_bits(self->vertices));
}

static PyObject *Alist(AdjacencyList *self) {
    return self->adj_list[1];
}

static PyObject *vertices(AdjacencyList *self) {
    PyObject *vertices_set = PySet_New(NULL);
    if (!vertices_set) {
        return NULL;
    }
    short num = self->vertices;

    for (int i = 0; i < MAX_VERTICES; i++) {
        if ((num >> i) & 1) {
            PyObject *item = PyLong_FromLong(i);
            PySet_Add(vertices_set, item);
            Py_DECREF(item);
        }
    }

    return vertices_set;
}

static PyObject *number_of_edges(AdjacencyList *self) {
    int sum = 0;
    for (int i = 0; i < count_bits(self->vertices); ++i) {
        sum += PyList_Size(self->adj_list[i]);
    }
    sum = sum / 2;
    return PyLong_FromLong(sum);
}

static PyObject *edges(AdjacencyList *self) {
    PyObject *edges_set = PySet_New(NULL);
    if (edges_set == NULL) {
        return NULL;
    }
    for (int i = 0; i < MAX_VERTICES; ++i) {
        for (int j = 0; j < PyList_Size(self->adj_list[i]); ++j) {
            PyObject *vertex_j = PyList_GetItem(self->adj_list[i], j);
            if (vertex_j == NULL) { // error handle
                Py_DECREF(edges_set);
                return NULL;
            }
            int smaller_vertex = i < PyLong_AsLong(vertex_j) ? i : PyLong_AsLong(vertex_j);
            int larger_vertex = i < PyLong_AsLong(vertex_j) ? PyLong_AsLong(vertex_j) : i;

            PyObject *edge = PyTuple_Pack(2, PyLong_FromLong(smaller_vertex), PyLong_FromLong(larger_vertex));

            if (edge == NULL) {
                Py_DECREF(edges_set);
                return NULL;
            }

            int result = PySet_Add(edges_set, edge);
            Py_DECREF(edge); 
            if (result == -1) {
                Py_DECREF(edges_set);
                return NULL;
            }
            Py_DECREF(vertex_j);
        }
    }
    return edges_set;
}

static PyObject *is_edge(AdjacencyList *self, PyObject *args) {
    int v, u;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &v, &u);
    }
    int number_of_neighbours = PyList_Size(self->adj_list[v]);
    int result = 0;
    for (int i=0; i < number_of_neighbours; ++i)
    {
        PyObject *vertex_i = PyList_GetItem(self->adj_list[v], i);
        if (PyLong_AsLong(vertex_i) == u)
        {
            result = 1;
            break;
        }
    }

    return PyBool_FromLong(result);
}

static PyObject *vertex_degree(AdjacencyList *self, PyObject *args) {
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }

    int degree = PyList_Size(self->adj_list[v]);
    return PyLong_FromLong(degree);
}

static PyObject *vertex_neighbors(AdjacencyList *self, PyObject *args) {
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }

    PyObject *neighbours_set = PySet_New(NULL);
    if (!neighbours_set) {
        return NULL;
    }

    for (int i = 0; i < PyList_Size(self->adj_list[v]); i++)
    {
        PyObject *vertex_i = PyList_GetItem(self->adj_list[v], i);
        PySet_Add(neighbours_set, vertex_i);
        Py_DECREF(vertex_i);
    }
    return neighbours_set;
}

static PyObject *add_vertex(AdjacencyList *self, PyObject *args) {
    int v;

    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }

    short tmp = (0x0001 << v);
    self->vertices = self->vertices | tmp;
    Py_RETURN_NONE;
}

static PyObject *delete_vertex(AdjacencyList *self, PyObject *args) {
    int v;
    if (args != NULL) {
        PyArg_ParseTuple(args, "i", &v);
    }
    Py_DECREF(self->adj_list[v]);
    self->adj_list[v] = PyList_New(0);

    for (int i = 0; i < MAX_VERTICES; i++)
    {
        if (PySequence_Contains(self->adj_list[i], PyLong_FromLong(v)))
        {
            int idx = PySequence_Index(self->adj_list[i], PyLong_FromLong(v));
            if (idx != -1)
            {
                PyList_SetSlice(self->adj_list[i], idx, idx + 1, NULL);
            }
        }
    }

    self->vertices &= ~(1 << v);
    Py_RETURN_NONE;
}

static PyObject *add_edge(AdjacencyList *self, PyObject *args) {
    int v, u;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &v, &u);
    }
    PyList_Append(self->adj_list[v], PyLong_FromLong(u));
    PyList_Sort(self->adj_list[u]);

    PyList_Append(self->adj_list[u], PyLong_FromLong(v));
    PyList_Sort(self->adj_list[v]);

    Py_RETURN_NONE;
}

static PyObject *delete_edge(AdjacencyList *self, PyObject *args) {
    int v, u;

    if (args != NULL) {
        PyArg_ParseTuple(args, "ii", &v, &u);
    }

    int idx_v = PySequence_Index(self->adj_list[u], PyLong_FromLong(v));
    PyList_SetSlice(self->adj_list[u], idx_v, idx_v + 1, NULL);

    int idx_u = PySequence_Index(self->adj_list[v], PyLong_FromLong(u));
    PyList_SetSlice(self->adj_list[v], idx_u, idx_u + 1, NULL);

    Py_RETURN_NONE;
}

static PyObject *degree_sequence(AdjacencyList *self) {
    PyObject *degree_sequence_list = PyList_New(0);

    short num = self->vertices;
    for (int i = 0; i < MAX_VERTICES; ++i) {
        if ((num >> i) & 1) 
        {
            int degree = PyList_Size(self->adj_list[i]);
            PyList_Append(degree_sequence_list, PyLong_FromLong(degree));
        }
    }
    PyList_Sort(degree_sequence_list);
    PyList_Reverse(degree_sequence_list);

    return degree_sequence_list;
}

static PyObject *get_ver(AdjacencyList *self) {
    return PyLong_FromLong(self->vertices);
}

// Define the methods
static PyMethodDef AdjacencyList_methods[] = {
    {"number_of_vertices", (PyCFunction)number_of_vertices, METH_NOARGS},
    {"vertices", (PyCFunction)vertices, METH_NOARGS},
    {"number_of_edges", (PyCFunction)number_of_edges, METH_NOARGS},
    {"edges", (PyCFunction)edges, METH_NOARGS},
    {"is_edge", (PyCFunction)is_edge, METH_VARARGS},
    {"vertex_degree", (PyCFunction)vertex_degree, METH_VARARGS},
    {"vertex_neighbors", (PyCFunction)vertex_neighbors, METH_VARARGS},
    {"add_vertex", (PyCFunction)add_vertex, METH_VARARGS},
    {"delete_vertex", (PyCFunction)delete_vertex, METH_VARARGS},
    {"add_edge", (PyCFunction)add_edge, METH_VARARGS},
    {"delete_edge", (PyCFunction)delete_edge, METH_VARARGS},
    {"degree_sequence", (PyCFunction)degree_sequence, METH_NOARGS},
    {"Alist", (PyCFunction)Alist, METH_NOARGS},
    {"get_ver", (PyCFunction)get_ver, METH_NOARGS},
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
