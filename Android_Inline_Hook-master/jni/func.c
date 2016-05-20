#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include <jni.h>
#include <stdlib.h>
#include <EGL/egl.h>  
#include <GLES/gl.h>
#include "hook.h"

struct hook_t eph;
extern EGLBoolean new_eglSwapBuffers_arm(EGLDisplay dpy, EGLSurface surf);

EGLBoolean new_eglSwapBuffers(EGLDisplay dpy, EGLSurface surf)
{
	EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surf);
	old_eglSwapBuffers = (void*)eph.orig;
	hook_precall(&eph);																										//将原始的eph赋值回去
	EGLBoolean res = old_eglSwapBuffers(dpy,surf);
	hook_postcall(&eph);																								  //将自定义的eph覆盖原始的
	LOGD("new_eglSwapBuffers\n");       
	return res;
}

int hook_entry(char * a)
{
	LOGD("hook start\n");
	hook(&eph, getpid(), "libEGL.", "eglSwapBuffers", new_eglSwapBuffers_arm, new_eglSwapBuffers);
	return 0;
}
