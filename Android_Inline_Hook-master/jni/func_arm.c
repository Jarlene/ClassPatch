#include <sys/types.h>
#include <EGL/egl.h>  
#include <GLES/gl.h>

extern int new_eglSwapBuffers(EGLDisplay dpy, EGLSurface surf);

int new_eglSwapBuffers_arm(EGLDisplay dpy, EGLSurface surf)
{
	return new_eglSwapBuffers(dpy,surf);
}