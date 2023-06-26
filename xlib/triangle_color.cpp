/**
 * @file        triangle_color.cpp
 * @description Create a blank window
 * @author      Rohit Nimkar
 * @version     1.0
 * @date        2023-06-26
 * @copyright   Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */ 

#include<stdio.h>
#include<stdlib.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

/* global variable declaration */
Display                 *dpy;   /**< connection to x-server */
Window                  root;   /**< parent window */
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
Window                  win;    /**< window handle */
GLXContext              glc;
XWindowAttributes       gwa;
XEvent                  x_evt;  /**< event emitted by xserver */

void drawTriangle() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1., 1., -1., 1., 1., 20.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

    glBegin(GL_TRIANGLES);
    
    glColor3f(1., 0., 0.);
    glVertex3f(-1., -1., 0.);
    
    glColor3f(0., 1., 0.);
    glVertex3f( 1., -1., 0.);
    
    glColor3f(0., 0., 1.);
    glVertex3f( 0,  1., 0.);
    
    glEnd();
} 

int main(int argc, char *argv[]) {

    /* show window on main display i.e. on which it is executed */
    dpy = XOpenDisplay(NULL);
    if(dpy == NULL) {
        printf("\n\tcannot connect to X server\n\n");
        exit(0);
    }
    
    /* default desktop background */
    root = DefaultRootWindow(dpy);

    vi = glXChooseVisual(dpy, 0, att);

    if(vi == NULL) {
        printf("\n\tno appropriate visual found\n\n");
        exit(0);
    } 
    else {
        printf("\n\tvisual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
    }

    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask;

    win = XCreateWindow(
        dpy,                     /* connection to x server */
        root,                       /* specifies parent window */
        0,                          /* top position */
        0,                          /* left position */
        800,                        /* inner width of window excluding borders */
        600,                        /* inner height of window excluding borders */
        0,                          /* border width */  
        vi->depth,                  /* depth */
        InputOutput,                /* Class of xwindow */
        vi->visual,                 /* visual type */
        CWColormap | CWEventMask,   /* specifies which arguments are defined in arguments param */
        &swa                        /* arguments list */  
    );

    XMapWindow(dpy, win);
    XStoreName(dpy, win, "Rohit Nimkar - Native linux window");

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);

    glEnable(GL_DEPTH_TEST); 

    while(1) {
        XNextEvent(dpy, &x_evt);

        if(x_evt.type == Expose) {
            XGetWindowAttributes(dpy, win, &gwa);
            glViewport(0, 0, gwa.width, gwa.height);
            drawTriangle(); 
            glXSwapBuffers(dpy, win);
        }

        else if(x_evt.type == KeyPress) {
            glXMakeCurrent(dpy, None, NULL);
            glXDestroyContext(dpy, glc);
            XDestroyWindow(dpy, win);
            XCloseDisplay(dpy);
            exit(0);
        }
    }
} 

