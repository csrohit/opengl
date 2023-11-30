#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include <cstdlib>
#define __USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>
#define MAX_ITERATIONS 500.0f

const int gwidth = 2880;
const int gheight = 1740;

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;
RGB points[gwidth][gheight];

RGB HSBtoRGB(double hue, double saturation, double brightness)
{
    hue = fmod(hue, 360.0);                                                          // Ensure hue is within [0, 360) degrees
    saturation = (saturation > 1.0) ? 1.0 : ((saturation < 0.0) ? 0.0 : saturation); // Limit saturation within [0, 1]
    brightness = (brightness > 1.0) ? 1.0 : ((brightness < 0.0) ? 0.0 : brightness); // Limit brightness within [0, 1]

    double c = saturation * brightness;
    double x = c * (1 - fabs(fmod(hue / 60.0, 2) - 1));
    double m = brightness - c;

    double r1, g1, b1;

    if (hue >= 0 && hue < 60)
    {
        r1 = c;
        g1 = x;
        b1 = 0;
    }
    else if (hue >= 60 && hue < 120)
    {
        r1 = x;
        g1 = c;
        b1 = 0;
    }
    else if (hue >= 120 && hue < 180)
    {
        r1 = 0;
        g1 = c;
        b1 = x;
    }
    else if (hue >= 180 && hue < 240)
    {
        r1 = 0;
        g1 = x;
        b1 = c;
    }
    else if (hue >= 240 && hue < 300)
    {
        r1 = x;
        g1 = 0;
        b1 = c;
    }
    else
    {
        r1 = c;
        g1 = 0;
        b1 = x;
    }

    RGB rgb;
    rgb.red = (uint8_t)((r1 + m) * 255);
    rgb.green = (uint8_t)((g1 + m) * 255);
    rgb.blue = (uint8_t)((b1 + m) * 255);

    return rgb;
}

uint32_t mandle(float x, float y)
{
    float zx = 0.0f;
    float zy = 0.0f;
    uint32_t n = 0;
    for (n = 0; n < MAX_ITERATIONS; n++)
    {

        if ((zx * zx + zy * zy) >= 4.0f)
        {
            break;
        }
        float temp = zx * zx - zy * zy + x;
        zy = 2 * zx * zy + y;
        zx = temp;
        n++;
    }
    return n;
}

uint32_t julia(float zx, float zy)
{
    float cx = -0.7269;// -0.8f; //0.0f;
    float cy = 0.1889; //0.156f; //0.0f;
    uint32_t n = 0;
    for (n = 0; n < MAX_ITERATIONS; n++)
    {

        if ((zx * zx + zy * zy) >= 4.0f)
        {
            break;
        }
        float temp = zx * zx - zy * zy + cx;
        zy = 2 * zx * zy + cy;
        zx = temp;
        n++;
    }
    return n;
}


Display *display;
Window window;
GLXContext glContext;
XRectangle rect = {0}; // window dimentions rectangle
void resize(GLsizei width, GLsizei height)
{
    if (height <= 0)
        height = 1;

    glMatrixMode(GL_PROJECTION); // for matrix calculation while resizing use GL_PROJECTION
    glLoadIdentity();            // take identity matrix for beginning

    GLdouble gldHeight = (GLdouble)(tan(M_PI / 8.0f) * 0.1);
    GLdouble gldWidth = gldHeight * ((GLdouble)width / (GLdouble)height);

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
            double n = (mandle(idx, jdx));
            if (n < MAX_ITERATIONS - 1)
            {
                n = (n / MAX_ITERATIONS) * 360.0f;
                RGB color = HSBtoRGB(120.0, 1.0f, 1.0f);
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
        None};

    XVisualInfo *visual = glXChooseVisual(display, screen, visualAttributes);
    if (visual == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    Colormap colormap = XCreateColormap(display, RootWindow(display, screen), visual->visual, AllocNone);
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = colormap;
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
            double n = (julia(idx, idy));
            if (n < MAX_ITERATIONS - 1)
            {
                n = (n / MAX_ITERATIONS) * 360.0f;
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

    while (1)
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

                if (event.type == KeyPress)
                {
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
