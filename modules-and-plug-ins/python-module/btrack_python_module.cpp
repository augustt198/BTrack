#include <iostream>
#include <algorithm>
#include <Python.h>
#include "../../src/OnsetDetectionFunction.h"
#include "../../src/BTrack.h"
#include <numpy/arrayobject.h>

//=======================================================================
static PyObject * btrack_trackBeats(PyObject *dummy, PyObject *args)
{
    PyObject *arg1=NULL;
    PyObject *arr1=NULL;
    
    if (!PyArg_ParseTuple(args, "O", &arg1))
    {
        return NULL;
    }
    
    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_IN_ARRAY);
    if (arr1 == NULL)
    {
        return NULL;
    }
    
    
    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = (double*) PyArray_DATA(arr1);
    
    // get array size
    long signal_length = PyArray_Size((PyObject*)arr1);
    
    
    ////////// BEGIN PROCESS ///////////////////
    int hopSize = 512;
    int frameSize = 1024;
    
    int numframes;
    double buffer[hopSize];	// buffer to hold one hopsize worth of audio samples
    
    
    // get number of audio frames, given the hop size and signal length
	numframes = (int) floor(((double) signal_length) / ((double) hopSize));
    
    
    BTrack b(hopSize,frameSize);
    
    
    double beats[5000];
    int beatnum = 0;
    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (int i=0;i < numframes;i++)
	{
		// add new samples to frame
		for (int n = 0;n < hopSize;n++)
		{
			buffer[n] = data[(i*hopSize)+n];
		}
		
        // process the current audio frame
        b.processAudioFrame(buffer);
        
        // if a beat is currently scheduled
		if (b.beatDueInCurrentFrame())
		{
			beats[beatnum] = BTrack::getBeatTimeInSeconds(i,hopSize,44100);
            beatnum = beatnum + 1;
		}
		
	}
	
	///////// End Processing Loop /////////////
	///////////////////////////////////////////
    
    
    ////////// END PROCESS ///////////////////
    
    double beats_out[beatnum];          // create output array
    
    // copy beats into output array
    for (int i = 0;i < beatnum;i++)
    {
        beats_out[i] = beats[i];
    }
    
    
    
    ////////// CREATE ARRAY AND RETURN IT ///////////////////
    int nd=1;
    npy_intp m= beatnum;
    //double fArray[5] = {0,1,2,3,4};
    
    PyObject* c=PyArray_SimpleNew(nd, &m, NPY_DOUBLE);
    
    void *arr_data = PyArray_DATA((PyArrayObject*)c);
    
    memcpy(arr_data, beats_out, PyArray_ITEMSIZE((PyArrayObject*) c) * m);
    
    
    Py_DECREF(arr1);
    Py_INCREF(Py_None);
    //return Py_None;
    
    return (PyObject *)c;
}


//=======================================================================
static PyObject * btrack_calculateOnsetDF(PyObject *dummy, PyObject *args)
{
    PyObject *arg1=NULL;
    PyObject *arr1=NULL;
    
    if (!PyArg_ParseTuple(args, "O", &arg1)) 
    {
        return NULL;
    }
    
    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_IN_ARRAY); 
    if (arr1 == NULL) 
    {
        return NULL;
    }


    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = (double*) PyArray_DATA(arr1);
    
    // get array size
    long signal_length = PyArray_Size((PyObject*)arr1);
    
    ////////// BEGIN PROCESS ///////////////////
    int hopSize = 512;
    int frameSize = 1024;
    int df_type = 6;
    int numframes;
    double buffer[hopSize];	// buffer to hold one hopsize worth of audio samples

    
    // get number of audio frames, given the hop size and signal length
	numframes = (int) floor(((double) signal_length) / ((double) hopSize));
    
    OnsetDetectionFunction onset(hopSize,frameSize,df_type,1);

    double df[numframes];
    

    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (int i=0;i < numframes;i++)
	{		
		// add new samples to frame
		for (int n = 0;n < hopSize;n++)
		{
			buffer[n] = data[(i*hopSize)+n];
		}
		
		df[i] = onset.calculateOnsetDetectionFunctionSample(buffer);
		
	}
	
	///////// End Processing Loop /////////////
	///////////////////////////////////////////
    
    

    
    ////////// CREATE ARRAY AND RETURN IT ///////////////////
    int nd=1;
    npy_intp m= numframes;

    
    PyObject* c=PyArray_SimpleNew(nd, &m, NPY_DOUBLE);
    
    void *arr_data = PyArray_DATA((PyArrayObject*)c);
        
    memcpy(arr_data, df, PyArray_ITEMSIZE((PyArrayObject*) c) * m); 
    
     
    Py_DECREF(arr1);  
    Py_INCREF(Py_None); 
    //return Py_None;
    
    return (PyObject *)c;
}


//=======================================================================
static PyObject * btrack_trackBeatsFromOnsetDF(PyObject *dummy, PyObject *args)
{
    PyObject *arg1=NULL;
    PyObject *arr1=NULL;
    
    if (!PyArg_ParseTuple(args, "O", &arg1)) 
    {
        return NULL;
    }
    
    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_IN_ARRAY); 
    if (arr1 == NULL) 
    {
        return NULL;
    }
    
    
    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = (double*) PyArray_DATA(arr1);
    
    // get array size
    long numframes = PyArray_Size((PyObject*)arr1);

    ////////// BEGIN PROCESS ///////////////////
    int hopSize = 512;
    int frameSize = 2*hopSize;

    BTrack b(hopSize,frameSize);
    
    double beats[5000];
    int beatnum = 0;
    double df_val;
    
    ///////////////////////////////////////////
	//////// Begin Processing Loop ////////////
	
	for (long i=0;i < numframes;i++)
	{		
        df_val = data[i] + 0.0001;
        
		b.processOnsetDetectionFunctionSample(df_val);				// process df sample in beat tracker
		
		if (b.beatDueInCurrentFrame())
		{
            beats[beatnum] = BTrack::getBeatTimeInSeconds(i,hopSize,44100);
			beatnum = beatnum + 1;	
		}
		
	}
	
	///////// End Processing Loop /////////////
	///////////////////////////////////////////
    
    
    ////////// END PROCESS ///////////////////
    
    double beats_out[beatnum];          // create output array
    
    
    // copy beats into output array
    for (int i = 0;i < beatnum;i++)     
    {
        beats_out[i] = beats[i];
    }
    
    
    ////////// CREATE ARRAY AND RETURN IT ///////////////////
    int nd=1;
    npy_intp m= beatnum;
    //double fArray[5] = {0,1,2,3,4};
    
    PyObject* c=PyArray_SimpleNew(nd, &m, NPY_DOUBLE);
    
    void *arr_data = PyArray_DATA((PyArrayObject*)c);
    
    memcpy(arr_data, beats_out, PyArray_ITEMSIZE((PyArrayObject*) c) * m); 
    
    
    Py_DECREF(arr1);  
    Py_INCREF(Py_None); 
    //return Py_None;
    
    return (PyObject *)c;
}

typedef struct {
    PyObject_HEAD
    BTrack *btrack_ptr;
    int numframes;
} PyBTrack;

static int PyBTrack_init(PyBTrack *self, PyObject *args, PyObject *kwds) {
    int hopsize, framesize;
    if (!PyArg_ParseTuple(args, "ii", &hopsize, &framesize)) {
        return -1;
    }

    self->btrack_ptr = new BTrack(hopsize, framesize);
    self->numframes = 0;

    return 0;    
}

static void PyBTrack_dealloc(PyBTrack *self) {
    delete self->btrack_ptr;
    Py_TYPE(self)->tp_free(self);
}

static PyObject * PyBTrack_trackBeats(PyBTrack *self, PyObject *args) {
    PyObject *arg1 = NULL;
    PyObject *arr1 = NULL;
    
    if (!PyArg_ParseTuple(args, "O", &arg1)) {
        return NULL;
    }
    
    arr1 = PyArray_FROM_OTF(arg1, NPY_DOUBLE, NPY_IN_ARRAY);
    if (arr1 == NULL) {
        return NULL;
    }

    BTrack *b = self->btrack_ptr;
    
    ////////// GET INPUT DATA ///////////////////
    
    // get data as array
    double* data = (double*) PyArray_DATA(arr1);
    
    // get array size
    long data_length = std::min(PyArray_Size((PyObject*) arr1), b->getHopSize());
    
    
    ////////// BEGIN PROCESS ///////////////////    
    double buffer[b->getHopSize()]; // buffer to hold one hopsize worth of audio samples
    
    memcpy(buffer, data, sizeof(double) * data_length);
    memset(buffer + data_length, 0, (b->getHopSize() - data_length));
    
    
    // process the current audio frame
    b->processAudioFrame(buffer);

    bool got_beat = false;;
    double beat_time;
        
    // if a beat is currently scheduled
    if (b->beatDueInCurrentFrame()) {
        beat_time = BTrack::getBeatTimeInSeconds(self->numframes, b->getHopSize(), 44100);
        got_beat = true;
    }
    self->numframes++;
    
    ///////// End Processing Loop /////////////
    ///////////////////////////////////////////
    
    
    ////////// END PROCESS ///////////////////
    
    if (got_beat) {
        return PyFloat_FromDouble(beat_time);
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyMethodDef PyBTrack_methods[] = {
    {"testbeat", (PyCFunction) PyBTrack_trackBeats, METH_VARARGS, "test for a beat"},
    {NULL, NULL, 0, NULL}
};

//=======================================================================
static PyMethodDef btrack_methods[] = {
    { "calculateOnsetDF",btrack_calculateOnsetDF,METH_VARARGS,"Calculate the onset detection function"},
    { "trackBeats",btrack_trackBeats,METH_VARARGS,"Track beats from audio"},
    { "trackBeatsFromOnsetDF",btrack_trackBeatsFromOnsetDF,METH_VARARGS,"Track beats from an onset detection function"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyTypeObject PyBTrackType = {PyVarObject_HEAD_INIT(NULL, 0) "btrack.BTrack"};


//=======================================================================
PyMODINIT_FUNC initbtrack(void) {
    PyBTrackType.tp_new         = PyType_GenericNew;
    PyBTrackType.tp_basicsize   = sizeof(PyBTrack);
    PyBTrackType.tp_dealloc     = (destructor) PyBTrack_dealloc;
    PyBTrackType.tp_flags       = Py_TPFLAGS_DEFAULT;
    PyBTrackType.tp_doc         = "BTrack object";
    PyBTrackType.tp_methods     = PyBTrack_methods;
    PyBTrackType.tp_init        = (initproc) PyBTrack_init;

    if (PyType_Ready(&PyBTrackType) < 0) return;

    //PyObject *m = PyModule_Create(&btrackmodule);
    //if (m == NULL) return NULL;

    Py_INCREF(&PyBTrackType);

    PyObject *m = Py_InitModule("btrack", btrack_methods);
    if (m == NULL) return;
    PyModule_AddObject(m, "BTrack", (PyObject*) &PyBTrackType);

    import_array();

    //return m;
}

//=======================================================================
int main(int argc, char *argv[])
{
    /* Pass argv[0] to the Python interpreter */
    Py_SetProgramName(argv[0]);
    
    /* Initialize the Python interpreter.  Required. */
    Py_Initialize();
    
    /* Add a static module */
    initbtrack();
}