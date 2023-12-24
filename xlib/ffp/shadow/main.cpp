#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>
#include <memory.h> // for memset
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include "stb_image.h"
#include <math.h>

#define TRUE  1
#define FALSE 0
// Transformation matrix to project Shadows
bool bFullscreen;

/* light properties */
GLfloat lightAmbient[]  = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {-1.0f, 5.0f, -2.0f, 1.0f};

/* material properties */
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

/* function declaration */
static void initialize();
static void uninitialize();
static void display();
static void update();
static void resize(GLsizei width, GLsizei height);
static void toggleFullscreen(Display *display, Window window);
void        drawSurface(void);

void        aDrawGround(void);
void        drawScene(bool bShadow);
static void doughnut(GLfloat r, GLfloat R, GLint nsides, GLint rings);
void        setShadowMatrix(GLfloat *result, float *lightPost, float *plane);

/* Windowing related variables */
Display   *dpy          = nullptr; // connection to server
Window     w            = 0UL;     // handle of current window
bool       gbAbortFlag  = false;   // Global abort flag
GLint      result       = 0;       // variable to get value returned by APIS
GLXContext glCtxt       = nullptr; // handle to OpenGL context
XRectangle rect         = {0};     // window dimentions rectangle
bool       gbFullscreen = false;   // should display in fullscreen mode
bool       shouldDraw   = false;   // should scene be rendered

/*--- Program specific variables ---*/
GLUquadric *pQuadric;
GLfloat     colorWhite[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat     colorBlack[4] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat     yPos          = 2.1f;
GLfloat     temp          = 0.0f;
bool        bDebugToggle  = false;

/* Light properties */
GLfloat lightPositionMirror[4] = {0.0f, 0.0f, 0.0f, 1.0f};

GLfloat materialRed[4]    = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialBlue[4]   = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat materialGreen[4]  = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat materialYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
GLfloat materialCyan[4]   = {0.0f, 1.0f, 1.0f, 1.0f};
GLfloat floorDiffuse[4]   = {1.0f, 1.0f, 1.0f, 0.5f};

GLfloat shadowMatrix[16];

/* Material Diffuse */
GLfloat materialDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float   plane[4]           = {0.0, 1.0, 0.0, 0.0};
/*-----*/
int main(int argc, char *argv[])
{
    static Atom wm_delete_window = 0;   // atomic variable to detect close button click
    Window      root             = 0UL; // handle of root window [Desktop]
    dpy                          = XOpenDisplay(nullptr);

    if (dpy == nullptr)
    {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(1);
    }

    int screen = DefaultScreen(dpy);
    root       = XDefaultRootWindow(dpy);

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

    XVisualInfo *visual = glXChooseVisual(dpy, screen, glxAttriutes);
    if (visual == nullptr)
    {
        fprintf(stderr, "Error: No appropriate visual found\n");
        exit(1);
    }

    XSetWindowAttributes xattr;
    xattr.event_mask        = ExposureMask | KeyPressMask;
    xattr.border_pixel      = BlackPixel(dpy, screen);
    xattr.background_pixel  = WhitePixel(dpy, screen);
    xattr.override_redirect = true;
    xattr.colormap          = XCreateColormap(dpy, root, visual->visual, AllocNone);
    xattr.event_mask        = ExposureMask | KeyPressMask | StructureNotifyMask;

    w = XCreateWindow(dpy, root, 0, 0, 1600, 1200, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &xattr);
    /* register for window close event */
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, w, &wm_delete_window, 1);
    printf("delete atom %lu\n", wm_delete_window);

    XStoreName(dpy, w, "Rohit Nimkar: Reflection");
    XMapWindow(dpy, w);

    glCtxt = glXCreateContext(dpy, visual, nullptr, GL_TRUE);
    glXMakeCurrent(dpy, w, glCtxt);
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
                                yPos += 0.1;
                            }
                            else
                            {
                                yPos -= 0.1;
                            }
                            break;
                        }
                        case XK_f:
                        {
                            toggleFullscreen(dpy, w);
                            gbFullscreen = !gbFullscreen;
                            break;
                        }
                        case XK_l:
                        {
                            static bool bLight = false;
                            if (false == bLight)
                            {
                                glEnable(GL_LIGHTING);
                            }
                            else
                            {
                                glDisable(GL_LIGHTING);
                            }
                            bLight = !bLight;
                            break;
                        }

                        case XK_r:
                        {
                            if (event.xkey.state & ShiftMask)
                            {
                                /* handle A */
                                temp += 0.1;
                            }
                            else
                            {
                                temp -= 0.1;
                            }
                            break;
                        }
                        case XK_t:
                        {
                            bDebugToggle = !bDebugToggle;
                            break;
                        }

                        case XK_p:
                        {
                            void printReport();
                            printReport();
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

    uninitialize();
    glXMakeCurrent(dpy, None, nullptr);
    glXDestroyContext(dpy, glCtxt);
    XDestroyWindow(dpy, w);
    XCloseDisplay(dpy);

    return 0;
}
GLuint torus;

static void initialize()
{
    pQuadric = gluNewQuadric();
    XWindowAttributes xattr;
    XGetWindowAttributes(dpy, w, &xattr);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClearDepth(1.0f);      // this bit will be set in depth buffer after calling glClear()
    glEnable(GL_DEPTH_TEST); // Hidden surface removal
    glFrontFace(GL_CCW);     // Counterclockwise Winding
    glEnable(GL_CULL_FACE);  // Do not calculate inside of JET

    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);

    torus = glGenLists(2);
    glNewList(torus, GL_COMPILE);
    glTranslatef(0.0f, 1.0f, 0.0f);
    doughnut(0.25f, 0.75f, 50, 50);
    glEndList();

    /* ground */
    glNewList(torus + 1, GL_COMPILE);
    drawSurface();
    glEndList();

    /* Sphere */
    glNewList(torus + 2, GL_COMPILE);
    glTranslatef(0.0f, 0.0f, 1.0f);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
    gluSphere(pQuadric, 0.2f, 20, 20);
    glEndList();

    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Vendor: %s\n", glGetString(GL_VENDOR));

    resize(xattr.width, xattr.height);
    // toggleFullscreen(dpy, w);
}

void uninitialize()
{
    glDeleteLists(torus, 3);
    gluDeleteQuadric(pQuadric);
}

float lightAngle  = 0.0f;
float sphereAngle = 0.0f;

static void display()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    gluLookAt(0.0f, yPos, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glDisable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
    glCallList(torus + 1);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    /* Now, only render where stencil is set to 1. */

    glStencilFunc(GL_EQUAL, 1, 0xffffffff);
    /* draw if ==1 */
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    /* Draw here */
    glPushMatrix();
    glScalef(1.0f, -1.0f, 1.0f);
    glFrontFace(GL_CW);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    drawScene(false);
    glFrontFace(GL_CCW);
    glPopMatrix();

    // draw the shadow
    glPushMatrix();
    // draw the shadow as black, blended with the surface, with no lighting, and not
    // preforming the depth test
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    // make sure that we don't draw at any raster position more than once
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColor4f(0.0, 0.0, 0.0, 1.5f);

    // project the cube through the shadow matrix
    glMultMatrixf(shadowMatrix);
    drawScene(true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glDisable(GL_STENCIL_TEST);

    /* draw ground */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCallList(torus + 1);
    glDisable(GL_BLEND);

    /* Original scene */
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glPushMatrix();
    drawScene(false);
    glPopMatrix();

    return;
}

void drawScene(bool bShadow)
{
    glMaterialf(GL_FRONT, GL_SHININESS, 128);
    if (false == bShadow)
    {
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialRed);
    }
    glMaterialfv(GL_FRONT, GL_EMISSION, colorBlack);
    glCallList(torus);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, colorBlack);
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_EMISSION, materialGreen);
    glTranslatef(1.0f, 0.0f, 0.0f);
    glRotatef(sphereAngle, 0.0f, 1.0f, 0.0f);
    glCallList(torus + 2);
    glPopMatrix();
return;
    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_EMISSION, materialYellow);
    glTranslatef(0.0f, 1.0f, 0.0f);
    glRotatef(-sphereAngle + 90.0f, 1.0f, 0.0f, 0.0f);
    glCallList(torus + 2);
    glPopMatrix();

    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_EMISSION, materialCyan);
    glTranslatef(0.0f, -1.0f, 0.0f);
    glRotatef(sphereAngle + 180.0f, 1.0f, 0.0f, 0.0f);
    glCallList(torus + 2);
    glPopMatrix();

    glPushMatrix();
    glMaterialfv(GL_FRONT, GL_EMISSION, materialBlue);
    glTranslatef(-1.0f, 0.0f, 0.0f);
    glRotatef(-sphereAngle + 270.0f, 0.0f, 1.0f, 0.0f);
    glCallList(torus + 2);
    glPopMatrix();
    glMaterialfv(GL_FRONT, GL_EMISSION, colorBlack);
}

static void update()
{
    int r = 5;
    lightAngle += 0.01;
    if (lightAngle >= 360.0f)
    {
        lightAngle -= 360.0f;
    }
    sphereAngle += 1;
    if (sphereAngle >= 360.0f)
    {
        sphereAngle -= 360.0f;
    }
    lightPosition[0] = r * sinf(lightAngle);
    lightPosition[2] = r * cosf(lightAngle);
    setShadowMatrix(shadowMatrix, lightPosition, plane);
}

static void resize(GLsizei width, GLsizei height)
{
    GLfloat aspectRatio;

    if (height <= 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    aspectRatio = (GLfloat)width / (GLfloat)height;

    gluPerspective(60.0f, aspectRatio, 1.0f, 500.0f);
}

static void quadloop(GLfloat r, GLfloat R, GLint nsides, GLfloat sideDelta, GLfloat cosTheta, GLfloat sinTheta, GLfloat cosTheta1, GLfloat sinTheta1)
{
    GLfloat dist, phi;
    int     j;

    glBegin(GL_QUAD_STRIP);

    dist = R + r;
    glNormal3f(cosTheta1, -sinTheta1, 0);
    glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, 0);
    glNormal3f(cosTheta, -sinTheta, 0);
    glVertex3f(cosTheta * dist, -sinTheta * dist, 0);

    phi = sideDelta;
    for (j = nsides - 2; j >= 0; j--)
    {
        GLfloat cosPhi, sinPhi;

        cosPhi = cos(phi);
        sinPhi = sin(phi);
        dist   = R + r * cosPhi;

        glNormal3f(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi);
        glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi);
        glNormal3f(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi);
        glVertex3f(cosTheta * dist, -sinTheta * dist, r * sinPhi);
        phi += sideDelta;
    }

    /* Repeat first two vertices to seam up each quad strip loop so no cracks. */
    dist = R + r;
    glNormal3f(cosTheta1, -sinTheta1, 0);
    glVertex3f(cosTheta1 * dist, -sinTheta1 * dist, 0);
    glNormal3f(cosTheta, -sinTheta, 0);
    glVertex3f(cosTheta * dist, -sinTheta * dist, 0);

    glEnd();
}

static void doughnut(GLfloat r, GLfloat R, GLint nsides, GLint rings)
{
    const GLfloat ringDelta = 2.0 * M_PI / rings;
    const GLfloat sideDelta = 2.0 * M_PI / nsides;

    GLfloat theta, theta1;
    GLfloat cosTheta, sinTheta;
    GLfloat cosTheta1, sinTheta1;
    int     i;

    theta    = 0.0;
    cosTheta = 1.0;
    sinTheta = 0.0;
    for (i = rings - 2; i >= 0; i--)
    {
        theta1    = theta + ringDelta;
        cosTheta1 = cos(theta1);
        sinTheta1 = sin(theta1);

        quadloop(r, R, nsides, sideDelta, cosTheta, sinTheta, cosTheta1, sinTheta1);

        theta    = theta1;
        cosTheta = cosTheta1;
        sinTheta = sinTheta1;
    }

    cosTheta1 = 1.0;
    sinTheta1 = 0.0;
    quadloop(r, R, nsides, sideDelta, cosTheta, sinTheta, cosTheta1, sinTheta1);
}

static void toggleFullscreen(Display *display, Window window)
{
    XEvent evt;

    Atom wm_state   = XInternAtom(display, "_NET_WM_STATE", False);
    Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

    evt.xclient.type         = ClientMessage;
    evt.xclient.serial       = 0;
    evt.xclient.send_event   = True;
    evt.xclient.message_type = wm_state;
    evt.xclient.format       = 32;
    evt.xclient.window       = window;
    evt.xclient.data.l[0]    = 2; // _NET_WM_STATE_TOGGLE
    evt.xclient.data.l[1]    = fullscreen;
    evt.xclient.data.l[2]    = 0;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &evt);
}

void drawSurface(void)
{
    GLfloat fExtent = 4.0f;
    GLfloat fStep   = 1.0f;
    GLfloat y       = 0.0f;
    GLfloat iStrip, iRun;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, floorDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 128);
    glMaterialfv(GL_FRONT, GL_EMISSION, colorBlack);

    for (iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glNormal3f(0.0f, -0.4f, 0.0f);
        for (iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
        {
            glVertex3f(iStrip, y, iRun);
            glVertex3f(iStrip + fStep, y, iRun);
        }
        glEnd();
    }
}

void setShadowMatrix(GLfloat *destMat, float *lightPos, float *plane)
{
    GLfloat dot;

    // dot product of plane and light position
    dot = plane[0] * lightPos[0] + plane[1] * lightPos[1] + plane[2] * lightPos[2];

    // first column
    destMat[0]  = dot - plane[0] * lightPos[0];
    destMat[4]  = 0.0f - lightPos[0] * plane[1];
    destMat[8]  = 0.0f - lightPos[0] * plane[2];
    destMat[12] = 0.0f - lightPos[0] * plane[3];

    // second column
    destMat[1]  = 0.0f - lightPos[1] * plane[0];
    destMat[5]  = dot - lightPos[1] * plane[1];
    destMat[9]  = 0.0f - lightPos[1] * plane[2];
    destMat[13] = 0.0f - lightPos[1] * plane[3];

    // third column
    destMat[2]  = 0.0f - lightPos[2] * plane[0];
    destMat[6]  = 0.0f - lightPos[2] * plane[1];
    destMat[10] = dot - lightPos[2] * plane[2];
    destMat[14] = 0.0f - lightPos[2] * plane[3];

    // fourth column
    destMat[3]  = 0.0f - lightPos[3] * plane[0];
    destMat[7]  = 0.0f - lightPos[3] * plane[1];
    destMat[11] = 0.0f - lightPos[3] * plane[2];
    destMat[15] = dot - lightPos[3] * plane[3];
}
void aDrawGround(void)
{
    GLfloat fExtent = 5.0f;
    GLfloat fStep   = 0.5f;
    GLfloat y       = 0.0f;
    GLint   iBounce = 0;
    GLfloat iStrip, iRun, fColor;

    glShadeModel(GL_FLAT);
    for (iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
        {
            if ((iBounce % 2) == 0)
                fColor = 1.0f;
            else
                fColor = 0.0f;

            glColor4f(fColor, fColor, fColor, 0.5f);
            glVertex3f(iStrip, y, iRun);
            glVertex3f(iStrip + fStep, y, iRun);

            iBounce++;
        }
        glEnd();
    }
    glShadeModel(GL_SMOOTH);
}

void printReport()
{
    printf("temp: %.2f\n", temp);
}
