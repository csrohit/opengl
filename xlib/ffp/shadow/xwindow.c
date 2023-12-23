/*--- System Headers ---*/
#include <memory.h> //memset()
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*--- XWindows Headers ---*/
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h> // visual

#define WIN_WIDTH  800U
#define WIN_HEIGHT 600U

/*--- OpenGL headers ---*/

/*--- Function declarations ---*/
void uninitialize(void);

/*--- Global variable declarations ---*/
Display *pDisplay = NULL;
Colormap colormap = 0UL;
Window   window   = 0UL;

int main(void)
{
    int                  defaultScreen    = 0;
    int                  defaultDepth     = 0;
    XVisualInfo          visualInfo       = {0};
    Status               statuc           = 0;
    XSetWindowAttributes windowAttributes = {0};
    int                  styleMask        = 0;
    Atom                 wmDelete         = 0;
    XEvent               event            = {0};
    KeySym               keySym           = 0UL;

    /* establish connection with x-server */
    pDisplay = XOpenDisplay(NULL);
    if (NULL == pDisplay)
    {
        printf("%s: XOpenDisplay failed\n", __func__);
        uninitialize();
        exit(1);
    }

    /* get default screen from display */
    defaultScreen = XDefaultScreen(pDisplay);

    /* Step3: get default depth*/
    defaultDepth = XDefaultDepth(pDisplay, defaultScreen);

    /* Step4: get visual */
    memset((void *)&visualInfo, 0, sizeof(XVisualInfo));
    Status status = XMatchVisualInfo(pDisplay, defaultScreen, defaultDepth, TrueColor, &visualInfo);
    if (status == 0)
    {
        printf("XMatchVisualInfo failed\n");
        uninitialize();
        exit(1);
    }

    /* Step5: Set window properties */
    memset((void *)&windowAttributes, 0, sizeof(XSetWindowAttributes));
    windowAttributes.border_pixel      = 0UL;
    windowAttributes.background_pixel  = XBlackPixel(pDisplay, visualInfo.screen);
    windowAttributes.background_pixmap = 0UL;
    windowAttributes.colormap          = XCreateColormap(pDisplay,                                 // pointer to display
                                                         RootWindow(pDisplay, visualInfo.screen), // window
                                                         visualInfo.visual,                        // pointer to visual
                                                         AllocNone);                               // do not allocate memory, won't need to use this later
    /* Step6: assign this colormap to global colormap */
    colormap = windowAttributes.colormap;

    /* Step7: specify which styles we wish to applied to the window */
    styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    /* Step8: create window */
    window = XCreateWindow(pDisplay,          // pointer to display
                           RootWindow(pDisplay, visualInfo.screen),                 // parent window [Desktop]
                           0,                 // x
                           0,                 // y
                           WIN_WIDTH,         // window width
                           WIN_HEIGHT,        // window dimentions
                           0,                 // border width
                           visualInfo.depth,  // depth
                           InputOutput,       // class of window
                           visualInfo.visual, // visual
                           styleMask,         // which styles to be applied
                           &windowAttributes  // window attributes
    );

    if (0 == window)
    {
        printf("XCreateWindow failed\n");
        uninitialize();
        exit(1);
    }

    /* Step9: State which events we want to receive */
    XSelectInput(pDisplay, window, ExposureMask | VisibilityChangeMask | StructureNotifyMask | KeyPressMask | ButtonPressMask | PointerMotionMask);

    wmDelete = XInternAtom(pDisplay, "WM_DELETE_WINDOW", True); // cristopher tronche, kenton lee, hpux

    /* Step10: add/set as protocol for window manager */
    XSetWMProtocols(pDisplay, window, &wmDelete, 1);

    /* Step11: set window title/caption/name */
    XStoreName(pDisplay, window, "RTR5: Rohit Nimkar");

    /* Step12: Show window */
    XMapWindow(pDisplay, window);

    /* Event Loop */
    while (1)
    {
        XNextEvent(pDisplay, &event);
        switch (event.type)
        {
            case KeyPress:
                keySym = XkbKeycodeToKeysym(pDisplay,           // pointer to display
                                            event.xkey.keycode, // actual keycode
                                            0,                  // key sym group
                                            0                   // is shift pressed
                );
                switch (keySym)
                {
                    case XK_Escape:
                    {
                        uninitialize();
                        exit(0);
                        break;
                    }
                    default:
                        break;
                }
                break;
            case 33:
            {
                uninitialize();
                exit(0);
                break;
            }
            default:
                break;
        }
    }

    uninitialize();
    return (0);
}

void uninitialize(void)
{
    if (0UL < colormap)
    {
        XFreeColormap(pDisplay, colormap);
    }
    if (NULL != pDisplay)
    {
        XCloseDisplay(pDisplay);
    }
}
