#define __USE_MATH_DEFINES
#include <math.h>

#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <cstdio>
#include <cstdlib>

#include "mandlebrot.h"

const int gwidth  = 2880;
const int gheight = 1740;

RGB points[gwidth][gheight];

Display   *display;
Window     window;
GLXContext glContext;
XRectangle rect        = {0}; // window dimentions rectangle
bool       gbAbortFlag = false;
//
void resize(GLsizei width, GLsizei height)
{
    if (height <= 0)
        height = 1;

    glMatrixMode(GL_PROJECTION); // for matrix calculation while resizing use GL_PROJECTION
    glLoadIdentity();            // take identity matrix for beginning

    GLdouble gldHeight = (GLdouble)(tan(M_PI / 8.0f) * 0.1);
    GLdouble gldWidth  = gldHeight * ((GLdouble)width / (GLdouble)height);

    /* after setting projection matrix, ensure that VIEW matrix is also set */
    glFrustum(-gldWidth, gldWidth, -gldHeight, gldHeight, 0.1, 100.0);
    gluLookAt(0.0, 0.0, 2.5, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glViewport(0, 0, width, height); // view complete window
}

void calculateMandleBrot()
{
    for (uint32_t idx = 0; idx < gwidth; idx++)
    {
        for (uint32_t jdx = 0; jdx < gheight; jdx++)
        {
            double n = (julia(idx, jdx));
            if (n < MAX_ITERATIONS - 1)
            {
                n                = (n / MAX_ITERATIONS) * 360.0f;
                RGB color        = HSBtoRGB(120.0, 1.0f, 1.0f);
                points[idx][jdx] = color;
            }
            else
            {
                points[idx][jdx] = {0, 0, 0};
            }
        }
    }
}

void createWindow()
{
    display = XOpenDisplay(nullptr);

    if (display == nullptr)
    {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(display);

    // Define the attributes for the visual context
    static int visualAttributes[] = {
        GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_SAMPLE_BUFFERS, 0, GLX_SAMPLES, 0,
        None};

    XVisualInfo *visual = glXChooseVisual(display, screen, visualAttributes);
    if (visual == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    Colormap             colormap = XCreateColormap(display, RootWindow(display, screen), visual->visual, AllocNone);
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap   = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    window = XCreateWindow(display, RootWindow(display, screen), 0, 0, gwidth, gheight, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

    XMapWindow(display, window);
    XStoreName(display, window, "OpenGL MandleBrot");

    glContext = glXCreateContext(display, visual, nullptr, GL_TRUE);
    glXMakeCurrent(display, window, glContext);
    resize(gwidth, gheight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    calculateMandleBrot();
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POINTS);
    for (float idx = -5.0f; idx < 5.0f; idx += 2.0f / (float)gwidth)
    {
        for (float idy = -5.0f; idy < 5.0f; idy += 2.0f / (float)gheight)
        {
            // double n = (mandle(idx, idy));
            double n = (julia(idx, idy));
            if (n < MAX_ITERATIONS - 1)
            {
                n         = (n / MAX_ITERATIONS) * 360.0f;
                RGB color = HSBtoRGB(n, 1.0f, 1.0f);
                glColor3ubv((const GLubyte *)&color);
            }
            else
            {
                glColor3f(0.0f, 0.0f, 0.0f);
            }

            glVertex2f(idx, idy);
        }
    }
    glEnd();

    glXSwapBuffers(display, window);
}

int main(int argc, char *argv[])
{
    createWindow();

    while (false == gbAbortFlag)
    {
        XEvent event;
        if (XPending(display))
        {

            XNextEvent(display, &event);
            switch (event.type)
            {
                case Expose:
                {
                    renderScene();
                    break;
                }

                case KeyPress:
                {

                    KeySym sym = XkbKeycodeToKeysym(display, event.xkey.keycode, 0, 0);

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
            }
        }
        // renderScene();
    }

    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
