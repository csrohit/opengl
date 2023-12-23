#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#define _USE_MATH_DEFINES
#include "stb_image.h"
#include <math.h>

/* function declaration */
static void initialize();
static void uninitialize();
static void display();
static void update();
static void resize(GLsizei width, GLsizei height);
static void toggleFullscreen(Display *display, Window window);

void drawTriangleGround(void);
void setShadowMatrix(GLfloat *destMat, float *lightPos, float *plane);
void DrawGround(void);

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
GLfloat     yPos          = 0.0f;

/* Light properties */
GLfloat lightAmbient[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 5.0f, 0.0f, 1.0f};
GLfloat spotDirection[] = {0.0f, -1.0f, 0.0f};


GLfloat blueDiffuse[] = {0.0f, 0.0f, 1.0f};
GLfloat GreenDiffuse[] = {0.0f, 1.0f, 0.0f};
GLfloat RedDiffuse[] = {1.0f, 0.0f, 0.0f};

GLfloat bluePosition[] = {5.0f, 5.0f, 5.0f, 1.0f};
GLfloat greenPosition[] = {5.0f, 5.0f, -5.0f, 1.0f};
GLfloat redPosition[] = {-5.0f, 5.0f, -5.0f, 1.0f};

/* Material Diffuse */
GLfloat materialAmbient[]  = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat materialDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess  = 128.0f;

GLuint textureGround = 0;
int ground;

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

    w = XCreateWindow(dpy, root, 0, 0, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &xattr);
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
    int               width, height, nrChannels;
    XWindowAttributes xattr;
    XGetWindowAttributes(dpy, w, &xattr);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClearDepth(1.0f);      // this bit will be set in depth buffer after calling glClear()
    glEnable(GL_DEPTH_TEST); // enable depth test
    glDepthFunc(GL_LEQUAL);  // Which function to use for testing

    glEnable(GL_CULL_FACE);

    /* per light initialization */
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &textureGround);
    glBindTexture(GL_TEXTURE_2D, textureGround);
    /* alignment and unpacking */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    /* set texture filtering parameters */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /* load image, create texture and generate mipmaps */
    unsigned char *data = stbi_load("./floor.bmp", &width, &height, &nrChannels, 0);
    if (data)
    {
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    pQuadric = gluNewQuadric();
    

    float cuttoff = 30.0f;
    float exponent = 40.0f;
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDirection);
    glLightfv(GL_LIGHT0, GL_SPOT_EXPONENT, &exponent);
    glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, &cuttoff);

    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);
    glLightfv(GL_LIGHT1, GL_SPOT_EXPONENT, &exponent);
    glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, &cuttoff);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, blueDiffuse);
    
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotDirection);
    glLightfv(GL_LIGHT2, GL_SPOT_EXPONENT, &exponent);
    glLightfv(GL_LIGHT2, GL_SPOT_CUTOFF, &cuttoff);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, GreenDiffuse);

    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spotDirection);
    glLightfv(GL_LIGHT3, GL_SPOT_EXPONENT, &exponent);
    glLightfv(GL_LIGHT3, GL_SPOT_CUTOFF, &cuttoff);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, RedDiffuse);

    // glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, &materialShininess);

    // Set the clipping plane equation
    resize(xattr.width, xattr.height);
    toggleFullscreen(dpy, w);

    ground = glGenLists(2);
    glNewList(ground, GL_COMPILE);
    drawTriangleGround();
    glEndList();

    glNewList(ground + 1, GL_COMPILE);
    gluSphere(pQuadric, 0.2, 90, 90);
    glEndList();
    
}

void uninitialize()
{
    glDeleteTextures(1, &textureGround);
    gluDeleteQuadric(pQuadric);
    glDeleteLists(ground, 2);
}

float angle = 0.0f;

double planeEquation[]  = {0.0, 0.01, 0.0, 0.0}; // This example sets a clipping plane along the x=0 plane
double planeEquation1[] = {0.0, 0.00, 0.0, 0.0}; // This example sets a clipping plane along the x=0 plane
void   drawScene();

static void display()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    gluLookAt(0.0f, yPos, 15.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT1, GL_POSITION, bluePosition);
    glLightfv(GL_LIGHT2, GL_POSITION, greenPosition);
    glLightfv(GL_LIGHT3, GL_POSITION, redPosition);

    glCallList(ground);

    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);
    glColor3fv(lightDiffuse);
    glCallList(ground + 1);
    glPopAttrib();
    glPopMatrix();
    
    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glTranslatef(bluePosition[0], bluePosition[1], bluePosition[2]);
    glColor3fv(blueDiffuse);
    glCallList(ground + 1);
    glPopAttrib();
    glPopMatrix();
    
    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glTranslatef(greenPosition[0], greenPosition[1], greenPosition[2]);
    glColor3fv(GreenDiffuse);
    glCallList(ground + 1);
    glPopAttrib();
    glPopMatrix();

    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glTranslatef(redPosition[0], redPosition[1], redPosition[2]);
    glColor3fv(RedDiffuse);
    glCallList(ground + 1);
    glPopAttrib();
    glPopMatrix();

}

static void update()
{
    angle += 1;
    if (angle >= 360.0f)
    {
        angle -= 360.0f;
    }
}

static void resize(GLsizei width, GLsizei height)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
    glViewport(0, 0, width, height);
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
    GLfloat fExtent = 5.0f;
    GLfloat fStep   = 0.3f;
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
// Draw the ground as a series of triangle strips
void drawTriangleGround(void)
{
    GLfloat fExtent = 20.0f;
    GLfloat fStep   = 0.05f;
    GLfloat y       = -0.4f;
    GLfloat iStrip, iRun;
    GLfloat s       = 0.0f;
    GLfloat t       = 0.0f;
    GLfloat texStep = 1.0f / (fExtent * .075f);
    // Ground is a tiling texture
    glBindTexture(GL_TEXTURE_2D, textureGround);
    // Lay out strips and repeat textures coordinates
    for (iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
    {
        t = 0.0f;
        glBegin(GL_TRIANGLE_STRIP);
        for (iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
        {
            glTexCoord2f(s, t);
            glNormal3f(0.0f, 1.0f, 0.0f); // All Point up
            glVertex3f(iStrip, y, iRun);
            glTexCoord2f(s + texStep, t);
            glNormal3f(0.0f, 1.0f, 0.0f); // All Point up
            glVertex3f(iStrip + fStep, y, iRun);
            t += texStep;
        }
        glEnd();
        s += texStep;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}
