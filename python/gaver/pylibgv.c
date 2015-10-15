#include <Python.h>
#include <errno.h>

/*
 * El objeto solamente tiene un campo que representa el descriptor de 
 * archivo
 */

typedef struct {
    PyObject_HEAD
    int socket;
} pylibgvObject;


static PyObject *
pylibgv_New (PyTypeObject *type)
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
pylibgv_init(pylibgvObject *self, PyObject *sdfileno)
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

static void
pylibgv_dealloc(pylibgvObject *self)
{
    self->ob_type->tp_free((PyObject *)self);
}


static PyObject *
pylibgv_listen(pylibgvObject *self, PyObject *arg)
{
    int backlog;

    if (!PyInt_Check(sdfileno))
    {
	PyErr_SetString(PyExc_AttributeError, "Argument must be a int");
	return NULL;
    }

    backlog = PyInt_AsLong(arg);

    errno = 0;
    if (gv_listen_api_msg(self->socket,backlog)==-1)
    {
	if (gv_err == OSERROR)
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

    if (!PyArg_ParseTuple(args,"iiis",&addr,&port,&vport, path)) 
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
	if (gv_err == OSERROR)
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

    if (!PyArg_ParseTuple(args,"iiis",&addr,&port,&vport, path)) 
    {
	PyErr_SetString(PyExc_AttributeError, "Error in arguments");
	return NULL;
    }
    /*
     * ToDo test len of path
     */
    errno = 0;
    if (gv_accept_api_msg(self->socket, addr, port, vport, path)==-1)
    {
	if (gv_err == OSERROR)
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
    if (gv_bind_api_msg(self->socket, addr, port, vport)==-1)
    {
	if (gv_err == OSERROR)
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

