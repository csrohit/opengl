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
#include "math3d.h"
#include "stb_image.h"
#include <math.h>

#define TRUE  1
#define FALSE 0
// Transformation matrix to project Shadows
M3DMatrix44f   shadowMat;
bool           bFullscreen;
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;

/* light properties */
GLfloat lightAmbient[]  = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat lightDiffuse[]  = {0.7f, 0.7f, 0.7f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {-5.0f, 5.0f, -5.0f, 0.0f};

/* material properties */
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

/* function declaration */
static void initialize();
static void uninitialize();
static void display();
static void update();
static void resize(GLsizei width, GLsizei height);
static void toggleFullscreen(Display *display, Window window);
void        DrawGround(void);
void        drawCube();

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

/* Light properties */
GLfloat lightPositionMirror[4] = {0.0f, 0.0f, 0.0f, 1.0f};

GLfloat materialRed[4]    = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialBlue[4]   = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat materialGreen[4]  = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat materialYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
GLfloat materialCyan[4]   = {0.0f, 1.0f, 1.0f, 1.0f};

GLfloat fFogColor[4] = {0.847656f, 0.84375f, 0.83984f, 1.0f};

float g_rotationAngle = 0.0;

GLfloat g_lightPos[4] = {5.0, 5.0, 5.0, 1.0};
GLfloat g_shadowMatrix[16];

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

    XStoreName(dpy, w, "Rohit Nimkar: Doughnut");
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
                                yPos += 0.1;
                            }
                            else
                            {
                                yPos -= 0.1;
                            }
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

GLuint cube;

static void initialize()
{
    pQuadric = gluNewQuadric();
    XWindowAttributes xattr;
    XGetWindowAttributes(dpy, w, &xattr);
    // Any three points on ground (CCW order)
    M3DVector3f points[3] = {{-30.0f, 0.0f, -20.0f}, {-30.0f, 0.0f, 20.0f}, {40.0f, 0.0f, 20.0f}};

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST); // Hidden surface removal
    glFrontFace(GL_CCW);     // Counterclockwise Winding
    glEnable(GL_CULL_FACE);  // Do not calculate inside of JET

    // Lighting
    glEnable(GL_LIGHTING);

    // Setup light 0
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);

    // Material
    // Enable color tracking
    glEnable(GL_COLOR_MATERIAL);

    // front material ambient and diffuse
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMateriali(GL_FRONT, GL_SHININESS, 128);

    // get plane eqation from three points on ground
    M3DVector4f vPlaneEquation;
    m3dGetPlaneEquation(vPlaneEquation, points[0], points[1], points[2]);

    setShadowMatrix(g_shadowMatrix, lightPosition, plane);

    // calculate projection matrix to draw shadows on ground
    m3dMakePlanarShadowMatrix(shadowMat, vPlaneEquation, lightPosition);

    printf("Shadow matrix:\n");
    printf("Plane equation: %.2f %.2f %.2f %.2f\n", vPlaneEquation[0], vPlaneEquation[1], vPlaneEquation[2], vPlaneEquation[3]);
    printf("%.2f %.2f %.2f %.2f\n", shadowMat[0], shadowMat[2], shadowMat[2], shadowMat[3]);
    printf("%.2f %.2f %.2f %.2f\n", shadowMat[4], shadowMat[5], shadowMat[6], shadowMat[7]);
    printf("%.2f %.2f %.2f %.2f\n", shadowMat[8], shadowMat[9], shadowMat[10], shadowMat[11]);
    printf("%.2f %.2f %.2f %.2f\n", shadowMat[12], shadowMat[13], shadowMat[14], shadowMat[15]);

    printf("My Shadow matrix:\n");
    printf("Plane equation: %.2f %.2f %.2f %.2f\n", plane[0], plane[1], plane[2], plane[3]);
    printf("%.2f %.2f %.2f %.2f\n", g_shadowMatrix[0], g_shadowMatrix[2], g_shadowMatrix[2], g_shadowMatrix[3]);
    printf("%.2f %.2f %.2f %.2f\n", g_shadowMatrix[4], g_shadowMatrix[5], g_shadowMatrix[6], g_shadowMatrix[7]);
    printf("%.2f %.2f %.2f %.2f\n", g_shadowMatrix[8], g_shadowMatrix[9], g_shadowMatrix[10], g_shadowMatrix[11]);
    printf("%.2f %.2f %.2f %.2f\n", g_shadowMatrix[12], g_shadowMatrix[13], g_shadowMatrix[14], g_shadowMatrix[15]);

    /* My shadow matrix */
    // rescale normals to unit length
    glEnable(GL_NORMALIZE);

    return;
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

    glClearDepth(1.0f);      // this bit will be set in depth buffer after calling glClear()
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL);  // Which function to use for testing

    glEnable(GL_CULL_FACE);

    /* per light initialization */
    glEnable(GL_LIGHT0);
    // glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    pQuadric = gluNewQuadric();
    cube     = glGenLists(2);
    glNewList(cube, GL_COMPILE);
    glTranslatef(0.0f, 1.0f, 0.0f);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialRed);
    doughnut(0.5f, 1.0f, 50, 50);
    glEndList();

    /* ground */
    glNewList(cube + 1, GL_COMPILE);
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    DrawGround();
    glPopAttrib();
    glEndList();

    /* Sphere */
    glNewList(cube + 2, GL_COMPILE);
    gluSphere(pQuadric, 0.2f, 20, 20);
    glEndList();

    // use color tracking using glColor
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT, GL_SPECULAR, colorWhite);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    // Set the clipping plane equation
    resize(xattr.width, xattr.height);
    // toggleFullscreen(dpy, w);
    setShadowMatrix(g_shadowMatrix, lightPosition, plane);
}

void uninitialize()
{
    glDeleteLists(cube, 3);
    gluDeleteQuadric(pQuadric);
}

float angle = 0.0f;

static void display()
{
    void RenderScene();
    RenderScene();
}

void drawScene()
{
}

static void update()
{
    angle += 0.01;
    if (angle >= 360.0f)
    {
        angle -= 360.0f;
    }
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

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, -4.0f, -10.0f);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    return;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glViewport(0, 0, width, height);
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

void DrawGround(void)
{
    GLfloat fExtent = 4.0f;
    GLfloat fStep   = 1.0f;
    GLfloat y       = -0.4f;
    GLfloat iStrip, iRun;

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

void RenderScene(void)
{
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw the ground
    // Darker green for background to give illusion of depth
    glBegin(GL_QUADS);
    glColor3ub((GLubyte)0, (GLubyte)32, (GLubyte)0);
    glVertex3f(10.0f, -0.0f, -10.0f);
    glVertex3f(-10.0f, -0.0f, -10.0f);
    glColor3ub((GLubyte)0, (GLubyte)255, (GLubyte)0);
    glVertex3f(-10.0f, -0.0f, 10.0f);
    glVertex3f(10.0f, -0.0f, 10.0f);
    glEnd();

    // save the matrix state and do the rotation
    glPushMatrix();
    // Draw jet at new orientation, put light in correct position
    // before rotating the JET
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glTranslatef(0.0f, 2.0f, 0.0f);
    doughnut(0.25f, 0.75f, 50, 50);
    // restore original matrix state
    glPopMatrix();

    // For drawing shadow
    // disable ligting and save the projection state
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glPushMatrix();
    // multiply by shadow projection matrix
    // glMultMatrixf((GLfloat *)shadowMat);
    glMultMatrixf(g_shadowMatrix);
    // now rotate the JET around in new flattened state
    glTranslatef(0.0f, 2.0f, 0.0f);
    // pass true to indicate drawing shadow
    doughnut(0.25f, 0.75f, 50, 50);
    // restore projection to normal
    glPopMatrix();

    // Draw the light source
    glPushMatrix();
    glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);
    glColor3ub((GLubyte)255, (GLubyte)255, (GLubyte)0);
    gluSphere(pQuadric, 0.2f, 10, 10);
    glPopMatrix();

    // restore lighting state
    glEnable(GL_DEPTH_TEST);
}
