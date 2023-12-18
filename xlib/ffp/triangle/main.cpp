#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <cstdio>
#include <cstdlib>

Display *display;
Window window;
GLXContext glContext;

void createWindow() {
    display = XOpenDisplay(nullptr);

    if (display == nullptr) {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(display);

    // Define the attributes for the visual context
    static int visualAttributes[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
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

    XVisualInfo *visual = glXChooseVisual(display, screen, visualAttributes);
    if (visual == nullptr) {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    Colormap colormap = XCreateColormap(display, RootWindow(display, screen), visual->visual, AllocNone);
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask;

    window = XCreateWindow(display, RootWindow(display, screen), 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

    XMapWindow(display, window);
    XStoreName(display, window, "OpenGL Triangle");

    glContext = glXCreateContext(display, visual, nullptr, GL_TRUE);
    glXMakeCurrent(display, window, glContext);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT);
    glRotatef(0.5f, 0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 0.0, 0.0);  // Red
    glVertex2f(0.0, 1.0);
    glColor3f(0.0, 1.0, 0.0);  // Green
    glVertex2f(-1.0, -1.0);
    glColor3f(0.0, 0.0, 1.0);  // Blue
    glVertex2f(1.0, -1.0);
    glEnd();

    glXSwapBuffers(display, window);
}

int main(int argc, char *argv[]) {
    createWindow();

    while (1) {
        XEvent event;
        if(XPending(display))
        {

        XNextEvent(display, &event);

        if (event.type == Expose) {
            renderScene();
        }

        if (event.type == KeyPress) {
            break;
        }
        }
            renderScene();
    }

    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}

