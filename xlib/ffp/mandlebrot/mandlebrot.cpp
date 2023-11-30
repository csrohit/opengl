#include <stdio.h>
#include <stdint.h>
#include <math.h> // Include math.h for fmod function

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;

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

int main() {
    double hue = 120.0; // Replace with desired hue value (0 to 360)
    double saturation = 1.0; // Replace with desired saturation value (0 to 1)
    double brightness = 1.0; // Replace with desired brightness value (0 to 1)

    RGB rgb = HSBtoRGB(hue, saturation, brightness);
    printf("RGB: Red = %u, Green = %u, Blue = %u\n", rgb.red, rgb.green, rgb.blue);
    return 0;
}