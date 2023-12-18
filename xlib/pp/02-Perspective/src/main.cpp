/**
 * @file      main.cpp
 * @brief     Perspective Projection
 * @author    Rohit Nimkar
 * @version   1.0
 * @date      2023-12-19
 * @copyright Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

// clang-format off
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
// clang-format on

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include "shader.h"
#include "vmath.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#define gpFILE stdout

#define WIN_WIDTH  800
#define WIN_HEIGHT 600

void display();
void update();
int  initialize();
void resize(int32_t width, int32_t height);

/* Windowing related variables */
Display             *dpy              = nullptr; // connection to server
Window               root             = 0UL;     // handle of root window [Desktop]
Window               w                = 0UL;     // handle of current window
int                  scr              = 0;       // handle to DefaultScreen
XVisualInfo         *vi               = nullptr; // Pointer to current visual info
XSetWindowAttributes xattr            = {};      // structure for windows attributes
bool                 gbAbortFlag      = false;   // Global abort flag
GLint                result           = 0;       // variable to get value returned by APIS
GLXContext           glCtxt           = nullptr; // handle to OpenGL context
static Atom          wm_delete_window = 0;       // atomic variable to detect close button click
XRectangle           rect             = {0};     // window dimentions rectangle

/* generate transformation matrix */
GLuint      MatrixID;
vmath::mat4 Projection;
vmath::mat4 View;
vmath::mat4 Model;
vmath::mat4 MVP;
GLuint      vertexBufferId = 0;
GLuint      program        = 0;
// clang-format off
static const GLfloat vertexBufferData[] = {
    -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f
};
// clang-format on

/* Variables related to current program */
GLboolean shouldDraw = false; // decide to render or not

int main()
{
    dpy = XOpenDisplay(nullptr);

    if (dpy == nullptr)
    {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    scr  = DefaultScreen(dpy);
    root = XDefaultRootWindow(dpy); // default window [Desktop]

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
    if (vi == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    xattr.event_mask        = ExposureMask | KeyPressMask;
    xattr.border_pixel      = BlackPixel(dpy, scr);
    xattr.background_pixel  = WhitePixel(dpy, scr);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, vi->visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask;

    /* create window */
    w = XCreateWindow(dpy,                                                    // connection to x-server
                      root,                                                   // root window
                      0,                                                      // x-coordinate of top-left corner
                      0,                                                      // y co-ordinate of top-left corner
                      WIN_WIDTH,                                              // window width
                      WIN_HEIGHT,                                             // window height
                      0,                                                      // boder width
                      vi->depth,                                              // depth of the window
                      InputOutput,                                            // class of window
                      vi->visual,                                             // pointer to visual structure
                      CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, // mask indicating which values are
                                                                              // passed in attributes
                      &xattr);                                                // pointer to attributes structure

    XStoreName(dpy, w, "Rohit Nimkar: xWindows");

    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);

    /* make window visible */
    XMapWindow(dpy, w);
    glCtxt = glXCreateContext(dpy, vi, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, glCtxt);
    /* initialize glew */
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize glew\n";
        XFree(vi);
        XFreeColormap(dpy, xattr.colormap);
        glXDestroyContext(dpy, glCtxt);
        XDestroyWindow(dpy, w);
        XCloseDisplay(dpy);
        return -1;
    }

    initialize();

    shouldDraw = false;
    while (!gbAbortFlag)
    {
        XEvent event;
        if (XPending(dpy))
        {
            XNextEvent(dpy, &event);
            switch (event.type)
            {
                case Expose:
                {
                    if (!shouldDraw)
                        shouldDraw = true;
                    break;
                }
                case ClientMessage:
                {
                    if (event.xclient.data.l[0] == wm_delete_window)
                    {
                        gbAbortFlag = true;
                    }

                    break;
                }
                case KeyPress:
                {
                    KeySym sym = XkbKeycodeToKeysym(dpy, event.xkey.keycode, 0, 0);

                    switch (sym)
                    {
                        case XK_a:
                        {
                            if (event.xkey.state & ShiftMask)
                            {
                                /* handle A */
                            }
                            else
                            {
                            }
                            break;
                        }
                        case XK_r:
                        {
                            break;
                        }
                        case XK_Escape:
                        {
                            gbAbortFlag = true;
                            break;
                        }
                    }
                    break;
                }
                case ConfigureNotify:
                {
                    if (rect.width != event.xconfigure.width || rect.height != event.xconfigure.height)
                    {
                        resize(event.xconfigure.width, event.xconfigure.height);
                        rect.width  = event.xconfigure.width;
                        rect.height = event.xconfigure.height;
                    }
                    break;
                }
            }
        }

        if (!shouldDraw)
            continue;
        update();
        display();

        glXSwapBuffers(dpy, w);
    }

    glXMakeCurrent(dpy, None, nullptr);
    glXDestroyContext(dpy, glCtxt);
    free(vi);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
}

int initialize()
{
    fprintf(gpFILE, "%-20s:%s\n", "GPU Vendor", glGetString(GL_VENDOR));
    fprintf(gpFILE, "%-20s:%s\n", "Version String", glGetString(GL_VERSION));
    fprintf(gpFILE, "%-20s:%s\n", "Graphics Renderer", glGetString(GL_RENDERER));
    fprintf(gpFILE, "%-20s:%s\n", "GL Shading Language", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Create and compile our GLSL program from the shaders
    result = LoadShaders("vertex.glsl", "fragment.glsl", &program);
    if (GL_TRUE != result)
    {
        fprintf(gpFILE, "[%s] Failed to link program\n", __func__);
        return -1;
    }

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexBufferId);

    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    /* Enable 0th vertexAttribute for specifying position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,                   // attribute index
                          3,                   // nElements per vertex
                          GL_FLOAT,            // type
                          GL_FALSE,            // OGL should not normalize data, as it is already normalized
                          6 * sizeof(GLfloat), // byte offset between generic vertex attributes
                          (void *)0            // offset to first vertex
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,                            // attribute index
                          3,                            // nElements per vertex
                          GL_FLOAT,                     // type
                          GL_FALSE,                     // OGL should not normalize data, as it is already normalized
                          6 * sizeof(GLfloat),          // byte offset between generic vertex attributes
                          (void *)(3 * sizeof(GLfloat)) // offset to first vertex
    );

    /* generate transformation matrix */
    MatrixID = glGetUniformLocation(program, "MVP");
    View     = vmath::lookat(vmath::vec3(0.0f, 0.0f, 5.0f), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));

    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    return (0);
}

void resize(int32_t width, int32_t height)
{
    Projection = vmath::perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
    glViewport(0, 0, (GLsizei)width, (GLsizei)height); // bioscope/Binoculor => focus on which are to be see in window => here we telling to focus on whole window
}

void display()
{
    Model = vmath::mat4::identity(); 
    MVP = Projection * View * Model;
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);

    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    /* Set the vertextBuffer as active buffer */
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

    /* Draw triangle from data in currently active buffer */
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void update()
{
}
