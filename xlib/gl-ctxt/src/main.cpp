/**
 * @file        main.cpp
 * @description Create opengl context using x11
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

#include <GL/gl.h>
#include <GL/glext.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include "X11/Xlib.h"
#include "cstdlib"
#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/keysymdef.h>
#include "X11/XKBlib.h"

#define GLX_MAJOR_MIN 1
#define GLX_MINOR_MIN 2

int main()
{
    Display* dpy                 = nullptr;
    Window root                  = 0UL;
    Window w                     = 0UL;
    int scr                      = 0;
    GLint glxMajor               = 0;
    GLint glxMinor               = 0;
    XVisualInfo* vi              = nullptr;
    XSetWindowAttributes xattr   = {};
    bool globalAbortFlag         = false;
    static Atom wm_delete_window = 0;
    GLXContext ctxt              = nullptr;

    dpy = XOpenDisplay(NULL);
    if (!glXQueryVersion(dpy, &glxMajor, &glxMinor))
    {
        std::cerr << "Failed to query glx version\n";
        exit(EXIT_FAILURE);
    }
    if (glxMajor < GLX_MAJOR_MIN && glxMinor < GLX_MINOR_MIN)
    {
        std::cerr << "GLX version >=1.2 is required\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "glx version is " << glxMajor << "." << glxMinor << std::endl;

    scr  = DefaultScreen(dpy);
    root = XDefaultRootWindow(dpy);

    // clang-format off
    GLint glxAttriutes[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_SAMPLE_BUFFERS, 0,
        GLX_SAMPLES, 0,
        None
    };
    // clang-format on
    vi = glXChooseVisual(dpy, scr, glxAttriutes);
    if (nullptr == vi)
    {
        std::cerr << "Could not create required visual window\n";
        exit(EXIT_FAILURE);
    }
    xattr.border_pixel      = BlackPixel(dpy, scr);
    xattr.background_pixel  = WhitePixel(dpy, scr);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, vi->visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask;

    w = XCreateWindow(dpy, root, 0, 0, 1024, 612, 0, vi->depth, InputOutput, vi->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &xattr);
    XStoreName(dpy, w, "Rohit Nimkar: OpenGL demo with X11");
    ctxt = glXCreateContext(dpy, vi, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, ctxt);

    /* free XVisual as it is not required */
    XFree(vi);
    vi = nullptr;

    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);
    XMapWindow(dpy, w);

    while (!globalAbortFlag)
    {
        XEvent evt;
        XNextEvent(dpy, &evt);
        switch (evt.type)
        {
        case Expose:
        {
            std::cout << "Expose" << std::endl;

            break;
        }
        case ClientMessage:
        {
            if (evt.xclient.data.l[0] == wm_delete_window) { globalAbortFlag = true; }

            break;
        }
        case KeyPress:
        {
            KeySym sym = XkbKeycodeToKeysym(dpy, evt.xkey.keycode, 0, 0);
            if (XK_Escape == sym) { globalAbortFlag = true; }
            break;
        }
        case MapNotify:
        {
            std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << "\n";
            std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << "\n";
            std::cout << "GL Version: " << glGetString(GL_VERSION) << "\n";
            std::cout << "GL Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
            break;
        }
        default:
        {
            std::cout << "Default event: " << evt.type << std::endl;
            break;
        }
        }

        /* redraw frame */
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);
        glEnd();

        glXSwapBuffers(dpy, w);
    }

    /* resource cleanup */
    glXDestroyContext(dpy, ctxt);
    XFreeColormap(dpy, xattr.colormap);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
    dpy = nullptr;
    return (0);
}
