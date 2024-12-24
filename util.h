#ifndef UTIL_H
#define UTIL_H

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

int collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

#endif //UTIL_H
