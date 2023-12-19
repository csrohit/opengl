#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/X.h>

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

/* Function prototypes */
void initialize();
void display();
void update();
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
bool                 shouldDraw       = false;

int main(int argc, char *argv[])
{
    dpy = XOpenDisplay(nullptr);

    if (dpy == nullptr)
    {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(dpy);

    // Define the attributes for the visual context
    static int visualAttributes[] = {
        GLX_RGBA, GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_SAMPLE_BUFFERS, 0, GLX_SAMPLES, 0,
        None};

    XVisualInfo *visual = glXChooseVisual(dpy, screen, visualAttributes);
    if (visual == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    Colormap             colormap = XCreateColormap(dpy, RootWindow(dpy, screen), visual->visual, AllocNone);
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap   = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    w = XCreateWindow(dpy, RootWindow(dpy, screen), 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

    XMapWindow(dpy, w);
    XStoreName(dpy, w, "Torus");

    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);

    glCtxt = glXCreateContext(dpy, visual, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, glCtxt);

    initialize();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

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
                        std::cout << "Terminating application\n";
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
                            std::cout << "Terminating application\n";
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
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);

    return 0;
}

void initialize()
{
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    resize(800, 600);
}

void update()
{
}

void display()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    // glTranslatef(0.0f, 0.0f, -3.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0); // Red
    glVertex2f(0.0, 1.0);
    glColor3f(0.0, 1.0, 0.0); // Green
    glVertex2f(-1.0, -1.0);
    glColor3f(0.0, 0.0, 1.0); // Blue
    glVertex2f(1.0, -1.0);
    glEnd();

    glXSwapBuffers(dpy, w);
}

void resize(int32_t width, int32_t height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    gluLookAt(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glViewport(0, 0, (GLsizei)width, (GLsizei)height); // bioscope/Binoculor => focus on which are to be see in window => here we telling to focus on whole window
}
