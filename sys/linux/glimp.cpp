/*****************************************************************************
                    The Dark Mod GPL Source Code
 
 This file is part of the The Dark Mod Source Code, originally based 
 on the Doom 3 GPL Source Code as published in 2011.
 
 The Dark Mod Source Code is free software: you can redistribute it 
 and/or modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation, either version 3 of the License, 
 or (at your option) any later version. For details, see LICENSE.TXT.
 
 Project: The Dark Mod (http://www.thedarkmod.com/)
 
 $Revision: 5336 $ (Revision of last commit) 
 $Date: 2012-03-11 12:13:16 +0000 (Sun, 11 Mar 2012) $ (Date of last commit)
 $Author: tels $ (Author of last commit)
 
******************************************************************************/
#include "../../idlib/precompiled.h"
#include "../../renderer/tr_local.h"
#include "local.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern "C" {
#	include "libXNVCtrl/NVCtrlLib.h"
}

idCVar sys_videoRam( "sys_videoRam", "0", CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_INTEGER, "Texture memory on the video card (in megabytes) - 0: autodetect", 0, 512 );

Display *dpy = NULL;
static int scrnum = 0;

Window win = 0;

bool dga_found = false;

static GLXContext ctx = NULL;

static bool vidmode_ext = false;
static int vidmode_MajorVersion = 0, vidmode_MinorVersion = 0;	// major and minor of XF86VidExtensions

static XF86VidModeModeInfo **vidmodes;
static int num_vidmodes;
static bool vidmode_active = false;

// backup gamma ramp
static int save_rampsize = 0;
static unsigned short *save_red, *save_green, *save_blue;

void GLimp_WakeBackEnd(void *a) {
	common->DPrintf("GLimp_WakeBackEnd stub\n");
}

#ifdef ID_GL_HARDLINK
void GLimp_EnableLogging(bool log) {
	static bool logging;
	if (log != logging)
	{
		common->DPrintf("GLimp_EnableLogging - disabled at compile time (ID_GL_HARDLINK)\n");
		logging = log;
	}
}
#endif

void GLimp_FrontEndSleep() {
	common->DPrintf("GLimp_FrontEndSleep stub\n");
}

void *GLimp_BackEndSleep() {
	common->DPrintf("GLimp_BackEndSleep stub\n");
	return 0;
}

bool GLimp_SpawnRenderThread(void (*a) ()) {
	common->DPrintf("GLimp_SpawnRenderThread stub\n");
	return false;
}

void GLimp_ActivateContext() {
	assert( dpy );
	assert( ctx );
	qglXMakeCurrent( dpy, win, ctx );
}

void GLimp_DeactivateContext() {
	assert( dpy );
	qglXMakeCurrent( dpy, None, NULL );
}

/*
=================
GLimp_SaveGamma

save and restore the original gamma of the system
=================
*/
void GLimp_SaveGamma() {
	if ( save_rampsize ) {
		return;
	}

	assert( dpy );
	
	XF86VidModeGetGammaRampSize( dpy, scrnum, &save_rampsize);
	save_red = (unsigned short *)malloc(save_rampsize*sizeof(unsigned short));
	save_green = (unsigned short *)malloc(save_rampsize*sizeof(unsigned short));
	save_blue = (unsigned short *)malloc(save_rampsize*sizeof(unsigned short));
	XF86VidModeGetGammaRamp( dpy, scrnum, save_rampsize, save_red, save_green, save_blue);
}

/*
=================
GLimp_RestoreGamma

save and restore the original gamma of the system
=================
*/
void GLimp_RestoreGamma() {
	if (!save_rampsize)
		return;
	
	XF86VidModeSetGammaRamp( dpy, scrnum, save_rampsize, save_red, save_green, save_blue);
	
	free(save_red); free(save_green); free(save_blue);
	save_rampsize = 0;
}

/*
=================
GLimp_SetGamma

gamma ramp is generated by the renderer from r_gamma and r_brightness for 256 elements
the size of the gamma ramp can not be changed on X (I need to confirm this)
=================
*/
void GLimp_SetGamma(unsigned short red[256], unsigned short green[256], unsigned short blue[256]) {
	if ( dpy ) {		
		int size;
		
		GLimp_SaveGamma();
		XF86VidModeGetGammaRampSize( dpy, scrnum, &size);
		common->DPrintf("XF86VidModeGetGammaRampSize: %d\n", size);
		if ( size > 256 ) {
			// silly generic resample
			int i;
			unsigned short *l_red, *l_green, *l_blue;
			l_red = (unsigned short *)malloc(size*sizeof(unsigned short));
			l_green = (unsigned short *)malloc(size*sizeof(unsigned short));
			l_blue = (unsigned short *)malloc(size*sizeof(unsigned short));
			//int r_size = 256;
			int r_i; float r_f;
			for(i=0; i<size-1; i++) {
				r_f = (float)i*255.0f/(float)(size-1);
				r_i = (int)floor(r_f);
				r_f -= (float)r_i;
				l_red[i] = (int)round((1.0f-r_f)*(float)red[r_i]+r_f*(float)red[r_i+1]);
				l_green[i] = (int)round((1.0f-r_f)*(float)green[r_i]+r_f*(float)green[r_i+1]);
				l_blue[i] = (int)round((1.0f-r_f)*(float)blue[r_i]+r_f*(float)blue[r_i+1]);				
			}
			l_red[size-1] = red[255]; l_green[size-1] = green[255]; l_blue[size-1] = blue[255];
			XF86VidModeSetGammaRamp( dpy, scrnum, size, l_red, l_green, l_blue );
			free(l_red); free(l_green); free(l_blue);
		} else {
			XF86VidModeSetGammaRamp( dpy, scrnum, size, red, green, blue );
		}
	}
}

void GLimp_Shutdown() {
	if ( dpy ) {
		
		Sys_XUninstallGrabs();
	
		GLimp_RestoreGamma();

		qglXDestroyContext( dpy, ctx );
		
#if !defined( ID_GL_HARDLINK )
		GLimp_dlclose();
#endif
		
		XDestroyWindow( dpy, win );
		if ( vidmode_active ) {
			XF86VidModeSwitchToMode( dpy, scrnum, vidmodes[0] );
		}

		XFlush( dpy );

		// FIXME: that's going to crash
		//XCloseDisplay( dpy );

		vidmode_active = false;
		dpy = NULL;
		win = 0;
		ctx = NULL;
	}
}

void GLimp_SwapBuffers() {
	assert( dpy );
	qglXSwapBuffers( dpy, win );
}

/*
GLX_TestDGA
Check for DGA	- update in_dgamouse if needed
*/
void GLX_TestDGA() {

#if defined( ID_ENABLE_DGA )
	int dga_MajorVersion = 0, dga_MinorVersion = 0;
	assert( dpy );
	if ( !XF86DGAQueryVersion( dpy, &dga_MajorVersion, &dga_MinorVersion ) ) {
		// unable to query, probalby not supported
		common->Printf( "Failed to detect DGA DirectVideo Mouse\n" );
		cvarSystem->SetCVarBool( "in_dgamouse", false );
		dga_found = false;
	} else {
		common->Printf( "DGA DirectVideo Mouse (Version %d.%d) initialized\n",
				   dga_MajorVersion, dga_MinorVersion );
		dga_found = true;
	}
#else
    dga_found = false;
#endif
}

/*
** XErrorHandler
**   the default X error handler exits the application
**   I found out that on some hosts some operations would raise X errors (GLXUnsupportedPrivateRequest)
**   but those don't seem to be fatal .. so the default would be to just ignore them
**   our implementation mimics the default handler behaviour (not completely cause I'm lazy)
*/
int idXErrorHandler(Display * l_dpy, XErrorEvent * ev) {
	char buf[1024];
	common->Printf( "Fatal X Error:\n" );
	common->Printf( "  Major opcode of failed request: %d\n", ev->request_code );
	common->Printf( "  Minor opcode of failed request: %d\n", ev->minor_code );
	common->Printf( "  Serial number of failed request: %lu\n", ev->serial );
	XGetErrorText( l_dpy, ev->error_code, buf, 1024 );
	common->Printf( "%s\n", buf );
	return 0;
}

bool GLimp_OpenDisplay( void ) {
	if ( dpy ) {
		return true;
	}

	if ( cvarSystem->GetCVarInteger( "net_serverDedicated" ) == 1 ) {
		common->DPrintf( "not opening the display: dedicated server\n" );
		return false;
	}

	common->Printf( "Setup X display connection\n" );

	// that should be the first call into X
	if ( !XInitThreads() ) {
		common->Printf("XInitThreads failed\n");
		return false;
	}
	
	// set up our custom error handler for X failures
	XSetErrorHandler( &idXErrorHandler );

	if ( !( dpy = XOpenDisplay(NULL) ) ) {
		common->Printf( "Couldn't open the X display\n" );
		return false;
	}
	scrnum = DefaultScreen( dpy );
	return true;
}

/*
===============
GLX_Init
===============
*/
int GLX_Init(glimpParms_t a) {
	int attrib[] = {
		GLX_RGBA,				// 0
		GLX_RED_SIZE, 8,		// 1, 2
		GLX_GREEN_SIZE, 8,		// 3, 4
		GLX_BLUE_SIZE, 8,		// 5, 6
		GLX_DOUBLEBUFFER,		// 7
		GLX_DEPTH_SIZE, 24,		// 8, 9
		GLX_STENCIL_SIZE, 8,	// 10, 11
		GLX_ALPHA_SIZE, 8, // 12, 13
		None
	};
	// these match in the array
#define ATTR_RED_IDX 2
#define ATTR_GREEN_IDX 4
#define ATTR_BLUE_IDX 6
#define ATTR_DEPTH_IDX 9
#define ATTR_STENCIL_IDX 11
#define ATTR_ALPHA_IDX 13
	Window root;
	XVisualInfo *visinfo;
	XSetWindowAttributes attr;
	XSizeHints sizehints;
	unsigned long mask;
	int colorbits, depthbits, stencilbits;
	int tcolorbits, tdepthbits, tstencilbits;
	int actualWidth, actualHeight;
	int i;
	const char *glstring;

	if ( !GLimp_OpenDisplay() ) {
		return false;
	}

	common->Printf( "Initializing OpenGL display\n" );

	root = RootWindow( dpy, scrnum );

	actualWidth = glConfig.vidWidth;
	actualHeight = glConfig.vidHeight;

	// Get video mode list
	if ( !XF86VidModeQueryVersion( dpy, &vidmode_MajorVersion, &vidmode_MinorVersion ) ) {
		vidmode_ext = false;
		common->Printf("XFree86-VidModeExtension not available\n");
	} else {
		vidmode_ext = true;
		common->Printf("Using XFree86-VidModeExtension Version %d.%d\n",
				   vidmode_MajorVersion, vidmode_MinorVersion);
	}

	GLX_TestDGA();

	if ( vidmode_ext ) {
		int best_fit, best_dist, dist, x, y;

		XF86VidModeGetAllModeLines( dpy, scrnum, &num_vidmodes, &vidmodes );

		// Are we going fullscreen?  If so, let's change video mode
		if ( a.fullScreen ) {
			best_dist = 9999999;
			best_fit = -1;

			for (i = 0; i < num_vidmodes; i++) {
				if (a.width > vidmodes[i]->hdisplay ||
					a.height > vidmodes[i]->vdisplay)
					continue;

				x = a.width - vidmodes[i]->hdisplay;
				y = a.height - vidmodes[i]->vdisplay;
				dist = (x * x) + (y * y);
				if (dist < best_dist) {
					best_dist = dist;
					best_fit = i;
				}
			}

			if (best_fit != -1) {
				actualWidth = vidmodes[best_fit]->hdisplay;
				actualHeight = vidmodes[best_fit]->vdisplay;

				// change to the mode
				XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[best_fit]);
				vidmode_active = true;

				// Move the viewport to top left
				// FIXME: center?
				XF86VidModeSetViewPort(dpy, scrnum, 0, 0);

				common->Printf( "Free86-VidModeExtension Activated at %dx%d\n", actualWidth, actualHeight );

			} else {
				a.fullScreen = false;
				common->Printf( "Free86-VidModeExtension: No acceptable modes found\n" );
			}
		} else {
			common->Printf( "XFree86-VidModeExtension: not fullscreen, ignored\n" );
		}
	}
	// color, depth and stencil
	colorbits = 24;
	depthbits = 24;
	stencilbits = 8;

	for (i = 0; i < 16; i++) {
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if ((i % 4) == 0 && i) {
			// one pass, reduce
			switch (i / 4) {
			case 2:
				if (colorbits == 24)
					colorbits = 16;
				break;
			case 1:
				if (depthbits == 24)
					depthbits = 16;
				else if (depthbits == 16)
					depthbits = 8;
			case 3:
				if (stencilbits == 24)
					stencilbits = 16;
				else if (stencilbits == 16)
					stencilbits = 8;
			}
		}

		tcolorbits = colorbits;
		tdepthbits = depthbits;
		tstencilbits = stencilbits;

		if ((i % 4) == 3) {		// reduce colorbits
			if (tcolorbits == 24)
				tcolorbits = 16;
		}

		if ((i % 4) == 2) {		// reduce depthbits
			if (tdepthbits == 24)
				tdepthbits = 16;
			else if (tdepthbits == 16)
				tdepthbits = 8;
		}

		if ((i % 4) == 1) {		// reduce stencilbits
			if (tstencilbits == 24)
				tstencilbits = 16;
			else if (tstencilbits == 16)
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}

		if (tcolorbits == 24) {
			attrib[ATTR_RED_IDX] = 8;
			attrib[ATTR_GREEN_IDX] = 8;
			attrib[ATTR_BLUE_IDX] = 8;
		} else {
			// must be 16 bit
			attrib[ATTR_RED_IDX] = 4;
			attrib[ATTR_GREEN_IDX] = 4;
			attrib[ATTR_BLUE_IDX] = 4;
		}
		
		attrib[ATTR_DEPTH_IDX] = tdepthbits;	// default to 24 depth
		attrib[ATTR_STENCIL_IDX] = tstencilbits;

		visinfo = qglXChooseVisual(dpy, scrnum, attrib);
		if (!visinfo) {
			continue;
		}

		common->Printf( "Using %d/%d/%d Color bits, %d Alpha bits, %d depth, %d stencil display.\n",
			 attrib[ATTR_RED_IDX], attrib[ATTR_GREEN_IDX],
			 attrib[ATTR_BLUE_IDX], attrib[ATTR_ALPHA_IDX],
			 attrib[ATTR_DEPTH_IDX],
			 attrib[ATTR_STENCIL_IDX]);

		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;
		break;
	}

	if (!visinfo) {
		common->Printf("Couldn't get a visual\n");
		return false;
	}
	// window attributes
	attr.background_pixel = BlackPixel(dpy, scrnum);
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = X_MASK;
	if (vidmode_active) {
		mask = CWBackPixel | CWColormap | CWSaveUnder | CWBackingStore |
			CWEventMask | CWOverrideRedirect;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
	} else {
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	}

	win = XCreateWindow(dpy, root, 0, 0,
						actualWidth, actualHeight,
						0, visinfo->depth, InputOutput,
						visinfo->visual, mask, &attr);

	XStoreName(dpy, win, GAME_NAME);

	// don't let the window be resized
	// FIXME: allow resize (win32 does)
	sizehints.flags = PMinSize | PMaxSize;
	sizehints.min_width = sizehints.max_width = actualWidth;
	sizehints.min_height = sizehints.max_height = actualHeight;

	XSetWMNormalHints(dpy, win, &sizehints);

	XMapWindow( dpy, win );

	if ( vidmode_active ) {
		XMoveWindow( dpy, win, 0, 0 );
	}

	XFlush(dpy);
	XSync(dpy, False);
	ctx = qglXCreateContext(dpy, visinfo, NULL, True);
	XSync(dpy, False);

	// Free the visinfo after we're done with it
	XFree(visinfo);

	qglXMakeCurrent(dpy, win, ctx);

	glstring = (const char *) qglGetString(GL_RENDERER);
	common->Printf("GL_RENDERER: %s\n", glstring);
	
	glstring = (const char *) qglGetString(GL_EXTENSIONS);
	common->Printf("GL_EXTENSIONS: %s\n", glstring);

	// FIXME: here, software GL test

	glConfig.isFullscreen = a.fullScreen;
	
	if ( glConfig.isFullscreen ) {
		Sys_GrabMouseCursor( true );
	}
	
	return true;
}

/*
===================
GLimp_Init

This is the platform specific OpenGL initialization function.  It
is responsible for loading OpenGL, initializing it,
creating a window of the appropriate size, doing
fullscreen manipulations, etc.  Its overall responsibility is
to make sure that a functional OpenGL subsystem is operating
when it returns to the ref.

If there is any failure, the renderer will revert back to safe
parameters and try again.
===================
*/
bool GLimp_Init( glimpParms_t a ) {

	if ( !GLimp_OpenDisplay() ) {
		return false;
	}
	
#ifndef ID_GL_HARDLINK
	if ( !GLimp_dlopen() ) {
		return false;
	}
#endif
	
	if (!GLX_Init(a)) {
		return false;
	}
	
	return true;
}

/*
===================
GLimp_SetScreenParms
===================
*/
bool GLimp_SetScreenParms( glimpParms_t parms ) {
	return true;
}

/*
================
Sys_GetVideoRam
returns in megabytes
open your own display connection for the query and close it
using the one shared with GLimp_Init is not stable
================
*/
int Sys_GetVideoRam( void ) {
	static int run_once = 0;
	int major, minor, value;
	Display *l_dpy;
	int l_scrnum;

	if ( run_once ) {
		return run_once;
	}

	if ( sys_videoRam.GetInteger() ) {
		run_once = sys_videoRam.GetInteger();
		return sys_videoRam.GetInteger();
	}

	// try a few strategies to guess the amount of video ram
	common->Printf( "guessing video ram ( use +set sys_videoRam to force ) ..\n" );
	if ( !GLimp_OpenDisplay( ) ) {
		run_once = 64;
		return run_once;
	}
	l_dpy = dpy;
	l_scrnum = scrnum;
	// go for nvidia ext first
	if ( XNVCTRLQueryVersion( l_dpy, &major, &minor ) ) {
		common->Printf( "found XNVCtrl extension %d.%d\n", major, minor );
		if ( XNVCTRLIsNvScreen( l_dpy, l_scrnum ) ) {
			if ( XNVCTRLQueryAttribute( l_dpy, l_scrnum, 0, NV_CTRL_VIDEO_RAM, &value ) ) {
				run_once = value / 1024;
				return run_once;
			} else {
				common->Printf( "XNVCtrlQueryAttribute NV_CTRL_VIDEO_RAM failed\n" );
			}
		} else {
			common->Printf( "default screen %d is not controlled by NVIDIA driver\n", l_scrnum );
		}
	}
	// try ATI /proc read ( for the lack of a better option )
	int fd;
	if ( ( fd = open( "/proc/dri/0/umm", O_RDONLY ) ) != -1 ) {
		int len;
		char umm_buf[ 1024 ];
		char *line;
		if ( ( len = read( fd, umm_buf, 1024 ) ) != -1 ) {
			// should be way enough to get the full file
			// grab "free  LFB = " line and "free  Inv = " lines
			umm_buf[ len-1 ] = '\0';
			line = umm_buf;
			line = strtok( umm_buf, "\n" );
			int total = 0;
			while ( line ) {
				if ( strlen( line ) >= 13 && strstr( line, "max   LFB =" ) == line ) {
					total += atoi( line + 12 );
				} else if ( strlen( line ) >= 13 && strstr( line, "max   Inv =" ) == line ) {
					total += atoi( line + 12 );
				}
				line = strtok( NULL, "\n" );
			}
			if ( total ) {
				run_once = total / 1048576;
				// round to the lower 16Mb
				run_once &= ~15;
				return run_once;
			}
		} else {
			common->Printf( "read /proc/dri/0/umm failed: %s\n", strerror( errno ) );
		}
	}
	common->Printf( "guess failed, return default low-end VRAM setting ( 64MB VRAM )\n" );
	run_once = 64;
	return run_once;
}
