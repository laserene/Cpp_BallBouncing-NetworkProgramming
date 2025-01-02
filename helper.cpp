#include "helper.h"
#include "random"

int getRandomNumber(const int min, const int max) {
    return rand() % (max - min) + min;
}
