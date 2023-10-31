/**
 * @file        main.cpp
 * @description Draw colored cube using x11-opengl
 * @author      Rohit Nimkar
 * @version     1.0
 * @date        2023-07-01
 * @copyright   Copyright 2023 Rohit Nimkar
 *
 * @attention
 *  Use of this source code is governed by a BSD-style
 *  license that can be found in the LICENSE file or at
 *  opensource.org/licenses/BSD-3-Clause
 */

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "X11/Xlib.h"
#include "cstdlib"
#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/keysymdef.h>
#include "X11/XKBlib.h"
#include "shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include "vmath.h"
#include "stb_image.h"

#define GLX_MAJOR_MIN 1
#define GLX_MINOR_MIN 2

struct Header
{
    int nVertex;
    int nIndexes;
};
struct Vertex
{
    float x;
    float y;
    float z;
    float u;
    float v;
};
int main()
{
    Display*             dpy              = nullptr;
    Window               root             = 0UL;
    Window               w                = 0UL;
    int                  scr              = 0;
    GLint                glxMajor         = 0;
    GLint                glxMinor         = 0;
    XVisualInfo*         vi               = nullptr;
    XSetWindowAttributes xattr            = {};
    bool                 globalAbortFlag  = false;
    static Atom          wm_delete_window = 0;
    GLXContext           ctxt             = nullptr;
    GLint                result           = 0;
    GLuint               program          = 0U;
    GLuint               vertexBuffer     = 0U;
    GLuint               texture          = 0U;
    GLuint               colorBuffer      = 0U;
    GLboolean            shouldDraw       = false;
    GLint                width            = 0;
    GLint                height           = 0;
    GLint                nrChannels       = 0;

    struct Vertex*      vertices = NULL;
    struct Header header;
    FILE*         pFile = fopen("./model.hex", "rb");
    if (NULL == pFile)
    {
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }

    fread(&header, sizeof(header), 1, pFile);
    printf("number of vertices: %d\n", header.nVertex);
    vertices = (struct Vertex*)malloc(sizeof(struct Vertex) * header.nVertex);
    fread(vertices, sizeof(struct Vertex), header.nVertex, pFile);

    for (uint32_t idx = 0; idx < header.nVertex; ++idx)
    {
        printf("[%f %f %f : %f %f]\n", vertices[idx].x, vertices[idx].y, vertices[idx].z, vertices[idx].u, vertices[idx].v);
    }

    fclose(pFile);

    // clang-format off
    static const GLfloat vertexBufferData[] = {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };
    static GLfloat colorBufferData[] = {

        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f,
         1.0, 0.0, 0.0,
         1.0, 1.0, 0.0,
         0.0, 1.0, 1.0,
         0.0, 0.0, 1.0,
         1.0, 0.0, 1.0,
         1.0, 0.5, 1.0,
         0.5, 1.0, 0.5,
         0.25, 0.5, 0.75,
         1.0, 0.0, 0.0,
         1.0, 1.0, 0.0,
         0.0, 1.0, 1.0,
         0.0, 0.0, 1.0,
         1.0, 0.0, 0.0,
         1.0, 1.0, 0.0,
         0.0, 1.0, 1.0,
         0.0, 0.0, 1.0,
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

    /* set window attributes */
    xattr.border_pixel      = BlackPixel(dpy, scr);
    xattr.background_pixel  = WhitePixel(dpy, scr);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, vi->visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask | PointerMotionMask;

    /* create window */
    w = XCreateWindow(dpy, root, 0, 0, 1024, 768, 0, vi->depth, InputOutput, vi->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &xattr);
    XStoreName(dpy, w, "Rohit Nimkar: OpenGL demo with X11");

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

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /* initialize vertex buffer */
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * header.nVertex, vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)0);

    /* initialize color buffer */
    // glGenBuffers(1, &colorBuffer);
    // glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(colorBufferData), colorBufferData, GL_STATIC_DRAW);

    /* vertex texture position */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)(3 * sizeof(GLfloat)));

    /* load and create texture */

    /* set the texture wrapping parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    /* set texture filtering parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* load image, create texture and generate mipmaps */
    unsigned char* data = stbi_load("./wall.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // Create and compile our GLSL program from the shaders
    result = LoadShaders("vertex.glsl", "fragment.glsl", &program);

    if (GL_TRUE != result)
    {
        std::cerr << "Failed to link program\n";
        return -1;
    }
    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);
    XMapWindow(dpy, w);

    /* generate transformation matrix */
    GLuint      MatrixID   = glGetUniformLocation(program, "MVP");
    vmath::mat4 Projection = vmath::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    vmath::mat4 View       = vmath::lookat(vmath::vec3(1.0f, 3.0f, 5.0f), vmath::vec3(0.0f, 0.0f, 0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
    // glm::mat4 View       = glm::lookAt(
    //     glm::vec3(4, 3, -3), // Camera is at (4,3,3), in World Space
    //     glm::vec3(0, 0, 0),  // and looks at the origin
    //     glm::vec3(0, 1, 0)   // Head is up (set to 0,-1,0 to look upside-down)
    // );
    // glm::mat4 Model = glm::mat4(1.0f);
    // glm::mat4 MVP   = Projection * View * Model;
    //
    vmath::mat4 Model = vmath::mat4::identity();
    vmath::mat4 MVP   = Projection * View * Model;
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
                    if (!shouldDraw)
                        shouldDraw = true;
                    break;
                }
                case ClientMessage:
                {
                    if (evt.xclient.data.l[0] == wm_delete_window)
                    {
                        globalAbortFlag = true;
                    }
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
                    // std::cout << "Default event: " << evt.type << std::endl;
                    break;
                }
            }
        }

        if (!shouldDraw)
            continue;

        /* redraw frame */
        // std::cout << "redrawing frame" << std::endl;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static GLfloat theta = 3;
        theta += 0.5;
        // if (theta > glm::radians(360.0))
        // {
        //     theta = 0.0;
        // }
        Model = vmath::translate(0.0f, 0.0f, 0.0f) * vmath::rotate(theta, 0.0f, 1.0f, 0.0f) * vmath::scale(1.0f, 1.0f, 1.0f);
        MVP   = Projection * View * Model;

        glUseProgram(program);
        glEnableVertexAttribArray(1);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        /* enable vertex buffer */
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        /* enable color buffer */
        // glEnableVertexAttribArray(1);
        // glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawArrays(GL_TRIANGLES, 0, header.nVertex);
        glXSwapBuffers(dpy, w);
    }

    free(vertices);
    /* resource cleanup */
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteProgram(program);
    glXDestroyContext(dpy, ctxt);
    XFreeColormap(dpy, xattr.colormap);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);
    dpy = nullptr;
    return (0);
}
