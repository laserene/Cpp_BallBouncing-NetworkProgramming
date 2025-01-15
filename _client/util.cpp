#include "util.h"
#include "common.h"

int collision(const int x1, const int y1, const int w1, const int h1, const int x2, const int y2, const int w2,
              const int h2) {
    return (MAX(x1, x2) < MIN(x1 + w1, x2 + w2)) && (MAX(y1, y2) < MIN(y1 + h1, y2 + h2));
}

void calcSlope(const int x1, const int y1, const int x2, const int y2, float *dx, float *dy) {
    int steps = MAX(abs(x1 - x2), abs(y1 - y2));

    if (steps == 0) {
        *dx = *dy = 0;
        return;
    }

    *dx = (x1 - x2);
    *dx /= steps;

    *dy = (y1 - y2);
    *dy /= steps;
}
