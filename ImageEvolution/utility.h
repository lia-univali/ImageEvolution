/*
 * Utility.h
 *
 *  Created on: 4 Jul 2016
 *      Author: Fernando
 */

#ifndef UTILITY_H_
#define UTILITY_H_
#include <unordered_set>
#include <sstream>
#include <iostream>

class Utility {

public:
    static double random(double min = 0, double max = 1);
    static int irandom(double min = 0, double max = 1);
    static double pi(double multiplier = 1, double denominator = 1);

    static double saferandom(double min = 0, double max = 1);
    static int safeirandom(double min = 0, double max = 1);
    static std::unordered_set<unsigned int> chooseRandomly(unsigned int count, unsigned int min, unsigned int max);

    template <typename T>
    static std::string createString(T t) {
        std::ostringstream oss;
        oss << t;
        return oss.str();
    }

    template <typename T, typename... Args>
    static std::string createString(T first, Args... args) {
        std::ostringstream oss;
        oss << first;
        return oss.str() + createString(args...);
    }

    template <typename T, typename U, typename V>
    static double normalize(T value, U min, V max) {
        return (value - min) / static_cast<double>(max - min);
    }
};

#endif /* UTILITY_H_ */
