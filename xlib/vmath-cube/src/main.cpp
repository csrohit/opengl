/**
 * @version     1.0
 * @date        2023-07-01
 * @copyright   Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

/* OpenGL Header files */
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>

/* X11 header files */
#include "X11/XKBlib.h"
#include "X11/Xlib.h"
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

/* Standard C++ Header files */
#include <cstddef>
#include <cstdlib>
#include <iostream>

/* Project level header files */
#include "shader.h"

/* For mathematical operations */
#include <vmath.h>

#define GLX_MAJOR_MIN 1
#define GLX_MINOR_MIN 2

int main()
{
    Display*             dpy               = nullptr;
    Window               root              = 0UL;
    Window               w                 = 0UL;
    int                  scr               = 0;
    GLint                glxMajor          = 0;
    GLint                glxMinor          = 0;
    XVisualInfo*         vi                = nullptr;
    XSetWindowAttributes xattr             = {};
    bool                 globalAbortFlag   = false;
    static Atom          wm_delete_window  = 0;
    GLXContext           ctxt              = nullptr;
    GLint                result            = 0;
    GLuint               program           = 0U;
    GLuint               vertexBuffer      = 0U;
    GLuint               vertexArrayBuffer = 0U;
    GLuint               indexBuffer       = 0U;
    GLboolean            shouldDraw        = false;

    // clang-format off
    static const GLfloat vertexBufferData[] = {
        -1.0, -1.0 , 1.0, 1.0, 0.0, 0.0,
        -1.0, 1.0, 1.0, 1.0, 1.0, 0.0,
        1.0, 1.0, 1.0, 0.0, 1.0, 1.0,
        1.0, -1.0, 1.0, 0.0, 0.0, 1.0,
        -1.0, -1.0, -1.0, 1.0, 0.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, 0.5, 1.0,
        1.0, 1.0, -1.0, 0.5, 1.0, 0.5,
        1.0, -1.0, -1.0, 0.25, 0.5, 0.75,

        0.0, 0.0 , 0.0, 1.0, 0.0, 0.0,
        0.0, 2.0, 0.0, 1.0, 1.0, 0.0,
        2.0, 2.0, 0.0, 0.0, 1.0, 1.0,
        2.0, 0.0, 0.0, 0.0, 0.0, 1.0,
        0.0, 0.0 , 2.0, 1.0, 0.0, 0.0,
        0.0, 2.0, 2.0, 1.0, 1.0, 0.0,
        2.0, 2.0, 2.0, 0.0, 1.0, 1.0,
        2.0, 0.0, 2.0, 0.0, 0.0, 1.0,
    };

    static const unsigned int indices[] = {
        /* font-face */
        0, 1, 2,
        0, 3, 2,

        /* rear face */
        4, 5, 6,
        4, 7, 6,

        /* left face */
        0, 1, 4,
        5, 1, 4,

        /* top-face */
        1, 5, 2,
        6, 5, 2,

        /* bottom-face */
        0, 4, 3,
        7, 4, 3,

        /* right-face */
        2, 3, 7,
        2, 6, 7, /**< first cube */

        /* front-face */
        8,9,10,
        8,11,10,

        /* back-face */
        12, 13, 14,
        12, 15, 14,

        /* left-face */
        9,8,12,
        9,13, 12,

        /* bottom-face */
        11, 8, 12,
        11, 15, 12,

        /* top-face */
        10, 9, 13,
        10, 14, 13,

        /* right-face */
        10, 11, 15,
        10, 14, 15

    };

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

    vi = glXChooseVisual(dpy, scr, glxAttriutes);
    if (nullptr == vi)
    {
        std::cerr << "Could not create required visual window\n";
        exit(EXIT_FAILURE);
    }

    /* set window attributes */
    xattr.border_pixel      = BlackPixel(dpy, scr);
    xattr.background_pixel  = WhitePixel(dpy, scr);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, vi->visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask;

    /* create window */
    w = XCreateWindow(dpy, root, 0, 0, 1024, 768, 0, vi->depth, InputOutput, vi->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &xattr);
    XStoreName(dpy, w, "Rohit Nimkar: OpenGL demo with X11");

    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);

    /* create opengl context */
    ctxt = glXCreateContext(dpy, vi, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, ctxt);

    /* initialize glew */
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize glew\n";
        XFree(vi);
        XFreeColormap(dpy, xattr.colormap);
        glXDestroyContext(dpy, ctxt);
        XDestroyWindow(dpy, w);
        XCloseDisplay(dpy);
        return -1;
    }

    /* free XVisual as it is not required */
    XFree(vi);
    vi = nullptr;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* initialize vertex buffer */
    glGenVertexArrays(1, &vertexArrayBuffer);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &indexBuffer);

    glBindVertexArray(vertexArrayBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Create and compile our GLSL program from the shaders
    result = LoadShaders("vertex.glsl", "fragment.glsl", &program);
    if (GL_TRUE != result)
    {
        std::cerr << "Failed to link program\n";
        return -1;
    }

    XMapWindow(dpy, w);

    /* generate transformation matrix */

    static GLfloat theta      = 3;
    GLuint         MatrixID   = glGetUniformLocation(program, "MVP");
    vmath::mat4    Model      = vmath::translate(vmath::vec3(0.0f, 0.0f, -3.0f)) * vmath::rotate(45.0f, 0.0f, 0.0f, 1.0f) * vmath::scale(1.0f, 1.0f, 1.0f);
    vmath::mat4    View       = vmath::lookat(vmath::vec3(1.0f, 3.0f, 5.0f), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
    vmath::mat4    Projection = vmath::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    vmath::mat4    MVP        = Projection * View * Model;

    while (!globalAbortFlag)
    {
        XEvent evt;
        if (XPending(dpy))
        {
            XNextEvent(dpy, &evt);
            switch (evt.type)
            {
            case Expose:
            {
                if (!shouldDraw) shouldDraw = true;
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
                if (XK_Escape == sym)
                {
                    globalAbortFlag = true;
                    shouldDraw      = false;
                }
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
        }

        if (!shouldDraw) continue;
        // static GLfloat x, y = 3.0, z;
        theta += 0.5;
        Model = vmath::translate(vmath::vec3(0.0f, 0.0f, 0.0f)) * vmath::rotate(theta, 0.0f, 0.0f, 1.0f) * vmath::scale(1.0f, 1.0f, 1.0f);
        MVP   = Projection * View * Model;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glBindVertexArray(vertexArrayBuffer);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glXSwapBuffers(dpy, w);
    }

    /* resource cleanup */
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vertexArrayBuffer);
    glDeleteProgram(program);
    glXDestroyContext(dpy, ctxt);
    XFreeColormap(dpy, xattr.colormap);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
    dpy = nullptr;
    return (0);
}


