#include "random"
#include "utils.h"

int getRandomNumber() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 50);
    return distrib(gen);
}