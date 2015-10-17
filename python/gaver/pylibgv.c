#include <Python.h>
#include <structmember.h>
#include <errno.h>
#include <gaver/apitypes.h>
#include <gaver/gv_err.h>
#include <gaver/libgv.h>

/*
 * El objeto solamente tiene un campo que representa el descriptor de 
 * archivo
 */
typedef struct {
    PyObject_HEAD
    int socket;
} pylibgvObject;



static PyObject *
pylibgv_new (PyTypeObject *type)
{
    pylibgvObject *self;
    self = (pylibgvObject *) type->tp_alloc(type,0);
    if ( self != NULL )
    {
	return (PyObject *) self;
    }
    return NULL;
}


static PyObject *
pylibgv_set(pylibgvObject *self, PyObject *sdfileno)
{
    if (PyInt_Check(sdfileno))
    {
	self->socket = PyInt_AsLong(sdfileno);
    }
    else
    {
	PyErr_SetString(PyExc_AttributeError, "Argument must be a int");
	return NULL;
    }
    Py_RETURN_TRUE;
}

static PyObject *
pylibgv_init(pylibgvObject *self, PyObject *sdfileno)
{
    self->socket = -1;
    Py_RETURN_TRUE;
}

static void
pylibgv_dealloc(pylibgvObject *self)
{
    self->ob_type->tp_free((PyObject *)self);
}


static PyObject *
pylibgv_listen(pylibgvObject *self, PyObject *arg)
{
    int backlog;

    if (!PyInt_Check(arg))
    {
	PyErr_SetString(PyExc_AttributeError, "Argument must be a int");
	return NULL;
    }

    backlog = PyInt_AsLong(arg);

    errno = 0;
    if (gv_listen_api_msg(self->socket,backlog)==-1)
    {
	if (gv_errno == OSERROR)
	{
	    /* 
	     * ToDo change strerror
             */
	    PyErr_SetString(PyExc_IOError, strerror(errno));
	}
	else
	{
	    /*
             * ToDo change for gv error
             */
	    PyErr_SetString(PyExc_IOError, "Gv Kernel Error");
	}
	return NULL;
    }
    Py_RETURN_TRUE;
}


static PyObject *
pylibgv_connect(pylibgvObject *self, PyObject *args)
{
    u_int32_t addr;
    u_int16_t port;
    u_int16_t vport;
    char *path;

    if (!PyArg_ParseTuple(args,"iiis",&addr,&port,&vport, &path)) 
    {
	PyErr_SetString(PyExc_AttributeError, "Error in arguments");
	return NULL;
    }
    /*
     * ToDo test len of path
     */
    errno = 0;
    if (gv_connect_api_msg(self->socket, addr, port, vport, path)==-1)
    {
	if (gv_errno == OSERROR)
	{
	    /* 
	     * ToDo change strerror
             */
	    PyErr_SetString(PyExc_IOError, strerror(errno));
	}
	else
	{
	    /*
             * ToDo change for gv error
             */
	    PyErr_SetString(PyExc_IOError, "Gv Kernel Error");
	}
	return NULL;
    }
    Py_RETURN_TRUE;
}


static PyObject *
pylibgv_accept(pylibgvObject *self, PyObject *args)
{
    u_int32_t addr;
    u_int16_t port;
    u_int16_t vport;
    char *path;

    if (!PyArg_ParseTuple(args,"iiis",&addr,&port,&vport, &path)) 
    {
	PyErr_SetString(PyExc_AttributeError, "Error in arguments");
	return NULL;
    }
    /*
     * ToDo test len of path
     */
    errno = 0;
    if (gv_accept_api_msg(self->socket, &addr, &port, &vport, path)==-1)
    {
	if (gv_errno == OSERROR)
	{
	    /* 
	     * ToDo change strerror
             */
	    PyErr_SetString(PyExc_IOError, strerror(errno));
	}
	else
	{
	    /*
             * ToDo change for gv error
             */
	    PyErr_SetString(PyExc_IOError, "Gv Kernel Error");
	}
	return NULL;
    }
    Py_RETURN_TRUE;
}


static PyObject *
pylibgv_bind(pylibgvObject *self, PyObject *args)
{
    u_int32_t addr;
    u_int16_t port;
    u_int16_t vport;

    if (!PyArg_ParseTuple(args,"iii",&addr,&port,&vport)) 
    {
	PyErr_SetString(PyExc_AttributeError, "Error in arguments");
	return NULL;
    }
    errno = 0;
    if (gv_bind_api_msg(self->socket, &addr, &port, &vport)==-1)
    {
	if (gv_errno == OSERROR)
	{
	    /* 
	     * ToDo change strerror
             */
	    PyErr_SetString(PyExc_IOError, strerror(errno));
	}
	else
	{
	    /*
             * ToDo change for gv error
             */
	    PyErr_SetString(PyExc_IOError, "Gv Kernel Error");
	}
	return NULL;
    }
    Py_RETURN_TRUE;
}


static PyMemberDef pylibgv_members[] = {
    {NULL}
};

static PyMethodDef pylibgv_methods[] = {
    {"connect", (PyCFunction)pylibgv_connect, METH_VARARGS, "Connect"},
    {"listen",	(PyCFunction)pylibgv_listen,  METH_O,       "Listen"},
    {"set",	(PyCFunction)pylibgv_set,  METH_O,          "Set Socket"},
    {"bind",	(PyCFunction)pylibgv_bind,    METH_VARARGS, "Bind"},	
    {"accept",	(PyCFunction)pylibgv_accept,  METH_VARARGS, "Accept"},
    {NULL}
};


static PyTypeObject pylibgvType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Python GaVer",            /*tp_name*/
    sizeof(pylibgvObject),     /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)pylibgv_dealloc,/*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "GaVer Methods Api",       /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    pylibgv_methods,            /* tp_methods */
    pylibgv_members,            /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)pylibgv_init,     /* tp_init */
    0,                         /* tp_alloc */
    pylibgv_new,                /* tp_new */
};


#ifndef PyMODINIT_FUNC  
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initpylibgv(void) 
{
    PyObject* m;
    
    if (PyType_Ready(&pylibgvType) < 0)
        return;
                
    m = Py_InitModule3("pylibgv", pylibgv_methods, "GaVer Methods");
    
    if (m == NULL)
         return;
                                             
    Py_INCREF(&pylibgvType);
    PyModule_AddObject(m, "pylibgv", (PyObject *)&pylibgvType);
}