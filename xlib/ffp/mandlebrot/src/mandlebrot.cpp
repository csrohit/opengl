#define __USE_MATH_DEFINES
#include <math.h>
#include "mandlebrot.h" 


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

RGB HSBtoRGB(double hue, double saturation, double brightness) {
    hue = fmod(hue, 360.0); // Ensure hue is within [0, 360) degrees
    saturation = (saturation > 1.0) ? 1.0 : ((saturation < 0.0) ? 0.0 : saturation); // Limit saturation within [0, 1]
    brightness = (brightness > 1.0) ? 1.0 : ((brightness < 0.0) ? 0.0 : brightness); // Limit brightness within [0, 1]

    double c = saturation * brightness;
    double x = c * (1 - fabs(fmod(hue / 60.0, 2) - 1));
    double m = brightness - c;

    double r1, g1, b1;

    if (hue >= 0 && hue < 60) {
        r1 = c;
        g1 = x;
        b1 = 0;
    } else if (hue >= 60 && hue < 120) {
        r1 = x;
        g1 = c;
        b1 = 0;
    } else if (hue >= 120 && hue < 180) {
        r1 = 0;
        g1 = c;
        b1 = x;
    } else if (hue >= 180 && hue < 240) {
        r1 = 0;
        g1 = x;
        b1 = c;
    } else if (hue >= 240 && hue < 300) {
        r1 = x;
        g1 = 0;
        b1 = c;
    } else {
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
