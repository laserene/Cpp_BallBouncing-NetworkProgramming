#include "iostream"
#include "random"
#include "helper.h"

int getRandomNumber(const int min, const int max) {
    std::random_device rd; // Seed for random number engine
    std::mt19937 gen(rd()); // Mersenne Twister random number engine
    std::uniform_int_distribution<> dist(min, max); // Define range
    return dist(gen);
}
