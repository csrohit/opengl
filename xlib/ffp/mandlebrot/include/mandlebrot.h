#ifndef MANDLEBROT_H
#define MANDLEBROT_H
#include <stdint.h>

#define MAX_ITERATIONS 500.0f

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;
uint32_t mandle(float x, float y);

uint32_t julia(float zx, float zy);
RGB HSBtoRGB(double hue, double saturation, double brightness);

#endif // !MANDLEBROT_H
