#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>
#include <SFML/Graphics.hpp>

class Solution : public std::vector<sf::Uint8>
{    
public:
    double fitness = -1;
    Solution();
    Solution(unsigned int size);
};

#endif // SOLUTION_H
