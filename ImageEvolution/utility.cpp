/*
 * Utility.cpp
 *
 *  Created on: 4 Jul 2016
 *      Author: Fernando
 */

#include "utility.h"
#include <cstdlib>
#include <cmath>
#include <random>
#include <ctime>
#include <thread>
#include <utility>

double Utility::random(double min, double max)
{
    return ((std::rand() / static_cast<double>(RAND_MAX)) * (max - min)) + min;
}

int Utility::irandom(double min, double max)
{
   return std::round(random(min, max));
}

double Utility::pi(double multiplier, double denominator)
{
    return (3.14159265359 * multiplier) / denominator;
}

double Utility::saferandom(double min, double max)
{
    static thread_local std::mt19937* generator = nullptr;
    if (!generator) generator = new std::mt19937(std::clock() + std::hash<std::thread::id>()(std::this_thread::get_id()));
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(*generator);
}

int Utility::safeirandom(double min, double max)
{
    return std::round(saferandom(min, max));
}

std::unordered_set<unsigned int> Utility::chooseRandomly(unsigned int count, unsigned int min, unsigned int max) {
    std::unordered_set<unsigned int> elements;
    while (elements.size() < count) {
        elements.insert(Utility::safeirandom(min, max));
    }

    return elements;
}


