#ifndef EVOLVER_H
#define EVOLVER_H

#include <cstdlib>
#include <vector>
#include <SFML/Graphics.hpp>
#include "solution.h"
#include <utility>
#include <functional>

class Evolver
{
public:
    using ParentPair = std::pair<std::reference_wrapper<Solution>, std::reference_wrapper<Solution>>;
    using Population = std::vector<Solution>;

private:
    using uint = unsigned int;

    double totalFitness;
    uint populationSize = 1600;
    uint threadCount = 8;
    uint generation;
    const sf::Uint8* target;
    uint solutionLength;
    Population population;

    Evolver::ParentPair selectParents();
    void mutate(Solution& solution);
    Solution crossover(const Solution& a, const Solution& b);
    uint selectRoulette(bool invert = false);
    Population createChildren(uint count);
    void sortPopulation();
    static bool isAlpha(uint value);

    Evolver::uint selectTournament(double pressure, uint count);
    std::vector<Solution> loadSolutions(const std::string &path, sf::Vector2u size, unsigned int amount);
    inline double calculateMutation();
public:
    Evolver();

    Evolver::Population generatePopulation(uint amount, uint solutionLength);
    void evaluateSolution(Solution& solution);
    void prepare(const sf::Uint8 *target, sf::Vector2u size, std::string path = std::string());
    void evolve();

    uint getGeneration() const;
    Evolver::Population& getPopulation();
    uint getSolutionLength() const;
    unsigned int getPopulationSize() const;
};

#endif // EVOLVER_H

