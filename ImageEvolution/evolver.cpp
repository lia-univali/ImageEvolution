#include "evolver.h"
#include "utility.h"
#include <algorithm>
#include <utility>
#include <thread>
#include <future>
#include <unordered_set>
#include <chrono>
#include <set>
#include "avir.h"

Evolver::Evolver()
{

}

std::vector<Solution> Evolver::loadSolutions(const std::string& path, sf::Vector2u size, unsigned int amount) {
    sf::Image image;
    image.loadFromFile(path);

    avir::CImageResizer<> resizer(8);
    Solution buffer(size.x * size.y * 4);
    resizer.resizeImage(image.getPixelsPtr(), image.getSize().x, image.getSize().y, 0, &buffer[0], size.x, size.y, 4, 0);

    std::vector<Solution> solutions;
    for (unsigned int i = 0; i < amount; i++) {
        solutions.push_back(buffer);
    }

    return solutions;
}

void Evolver::prepare(const sf::Uint8 *target, sf::Vector2u size, std::string path){
    this->target = target;
    this->solutionLength = size.x * size.y * 4;

    if (population.empty()) {
        this->population = generatePopulation(populationSize, solutionLength);
    } else {
        this->population = loadSolutions(path, size, populationSize);
    }

    for (Solution& s : population) {
        evaluateSolution(s);
    }
    sortPopulation();

    generation = 0;
}

double Evolver::calculateMutation()
{
    //if (globalBest > 0.9) return 0.01;
    //return std::max(0.01, std::min(-2 * globalBest + 1, 0.9));
    //return std::max(0.01, std::min(-1.7 * globalBest + 1.54, 0.9));
    //return std::max(0.01, std::min(-3.5625 * globalBest + 2.76625, 0.9));
    //return std::min(-5.5625 * globalBest + 5.01625, 0.9);
    //return std::min(-3.916 * globalBest + 3.87784, 1.0);
    /*if (globalBest.load() < 0.66) return 0.3;
    if (globalBest.load() < 0.72) return 0.2;
    if (globalBest.load() < 0.78) return 0.1;
    if (globalBest.load() < 0.84) return 0.075;*/
    return 0.05;
}

Evolver::Population Evolver::createChildren(uint count)
{
    Population children;
    children.reserve(count);

    for (uint i = 0; i < count; i++) {
        ParentPair p = selectParents();
        Solution child = crossover(p.first, p.second);

        if (Utility::saferandom() < calculateMutation()) {
            mutate(child);
        }

        evaluateSolution(child);
        children.push_back(child);
    }

    return children;
}

void Evolver::evolve()
{
    totalFitness = 0;
    for (Solution& s : population) {
        totalFitness += s.fitness;
    }

    uint childrenPerThread = populationSize / threadCount;

    std::vector<std::future<Population>> futures;
    std::vector<std::thread> threads;

    for (uint k = 1; k < threadCount; k++) {
        std::packaged_task<Population()> task([&]{ return createChildren(childrenPerThread); });

        futures.push_back(task.get_future());
        threads.push_back(std::thread(std::move(task)));
    }

    Population p = createChildren(childrenPerThread);
    population.insert(population.end(), p.begin(), p.end());

    for (std::thread& thread : threads) {
        thread.join();
    }

    for (std::future<Population>& future : futures) {
        p = future.get();
        population.insert(population.end(), p.begin(), p.end());
    }

    sortPopulation();
    population.erase(population.begin() + populationSize, population.end());

    /*
    std::vector<Solution> nextGeneration;
    nextGeneration.reserve(populationSize);

    uint eliteCount = populationSize * 0.05;
    for (uint i = 0; i < eliteCount; i++) {
        nextGeneration.push_back(population[i]);
        population.erase(population.begin() + i);
    }


    while (nextGeneration.size() < populationSize) {
        uint index = selectRoulette();
        nextGeneration.push_back(population[index]);
        population.erase(population.begin() + index);
    }

    population = nextGeneration;*/
    //std::cout << population.size() << "\n";

    //std::cout << "Best: " << globalBest << "\n";
    generation++;
}

Evolver::uint Evolver::getGeneration() const
{
    return generation;
}

Evolver::Population& Evolver::getPopulation()
{
    return population;
}

void Evolver::sortPopulation()
{
    std::sort(population.begin(), population.end(), [&](const Solution& a, const Solution& b) {
        return a.fitness > b.fitness; //Descrescent
    });
}

Evolver::uint Evolver::getSolutionLength() const
{
    return solutionLength;
}

unsigned int Evolver::getPopulationSize() const
{
    return populationSize;
}

Evolver::ParentPair Evolver::selectParents()
{
    double pressure = 0.65;
    uint count = 2;
    uint first = selectTournament(pressure, count);
    uint second;
    do {
        second = selectTournament(pressure, count);
    } while (first == second);

    return ParentPair(population[first], population[second]);
}

Evolver::uint Evolver::selectRoulette(bool invert)
{
    double target = Utility::saferandom(0, totalFitness);
    double current = 0;

    for (uint i = 0; i < population.size(); i++) {
        current += invert ? (1 - population[i].fitness) : population[i].fitness;
        if (current > target) {
            return i;
        }
    }

    return population.size() - 1;
}

Evolver::uint Evolver::selectTournament(double pressure, uint count) {
    std::unordered_set<uint> individuals = Utility::chooseRandomly(count, 0, population.size() - 1);

    using Comparator = std::function<bool(uint, uint)>;
    std::set<uint, Comparator> sortedIndividuals(individuals.begin(), individuals.end(), [&](const uint& a, const uint& b) {
        return population[a].fitness > population[b].fitness;
    });

    uint i = 0;
    for (const uint& v : sortedIndividuals) {
        double random = Utility::saferandom(0, 1);

        //Se tiver sorte ou se for o ultimo elemento
        if (i == sortedIndividuals.size() - 1 || random < pressure * std::pow(1 - pressure, i) ) {
            return v;
        }

        i++;
    }


    return -1;
}

Evolver::Population Evolver::generatePopulation(uint amount, uint solutionLength)
{
    Population pop;

    for (uint i = 0; i < amount; i++) {
        Solution s;
        s.reserve(solutionLength);

        for (uint i = 0; i < solutionLength; i++) {
            if (isAlpha(i)) { //Canal alpha
                s.push_back(255);
            } else {
                s.push_back(Utility::irandom(0, 255));
            }
        }

        pop.push_back(s);
    }
    return pop;
}

bool Evolver::isAlpha(uint value) {
    return (value + 1) % 4 == 0;
}

Solution Evolver::crossover(const Solution& a, const Solution& b)
{
    Solution child;
    child.reserve(solutionLength);

    bool parent = false;
    uint value = 0;
    uint min = solutionLength * 0.0005;
    uint max = solutionLength * 0.001;
    for (uint i = 0; i < solutionLength; i++) {
        if (value == 0) {
            parent = !parent;
            value = Utility::safeirandom(min, max);
        }

        //if (!isAlpha(i)) {
            value--;
            child.push_back(parent ? a[i] : b[i]);
        /*} else {
            child.push_back(255);
        }*/
    }


    //uint mid = solutionLength / 2;
    /*
    std::copy(a.begin(), a.begin() + mid, std::back_inserter(child));
    std::copy(b.begin() + mid, b.end(), std::back_inserter(child));*/

    /*for (uint i = 0; i < solutionLength; i++) {
        child.push_back(i < mid ? a[i] : b[i]);
    }*/


    /*bool parent = true;
    double chance = 0.0;
    for (uint i = 0; i < solutionLength; i++) {
        double val = std::rand();
        if (Utility::random() < chance) {
            parent = !parent;
        }

        child.push_back(parent ? a[i] : b[i]);
    }*/

    /*

    std::set<uint> points;
    for (uint i = 0; i < Utility::irandom(solutionLength / 4, solutionLength / 2); i++) {
        points.insert(Utility::irandom(0, solutionLength - 1));
    }

    bool mother = false;
    uint current = 0;
    for (uint point : points) {
        for (; current < point; current++) {
            child[current] = mother ? a[current] : b[current];
        }
        mother = !mother;
    }*/

    return child;
}

void Evolver::mutate(Solution& solution)
{
    uint mutations = Utility::safeirandom(solutionLength * 0.001, solutionLength * 0.01);

    std::vector<uint> indices;
    for (uint i = 0; i < mutations; i++) {
        uint index;
        do {
            index = Utility::safeirandom(0, solutionLength - 1);
        } while ((index + 1) % 4 == 0 || std::find(indices.begin(), indices.end(), index) != indices.end());
        //While the index is the alpha channel or the vector contains the index

        indices.push_back(index);
    }

    for (uint i : indices) {
        solution[i] = Utility::safeirandom(0, 255);
        //solution[i] += Utility::safeirandom(-solution[i], 255 - solution[i]);
        //solution[i] += Utility::safeirandom(-solution[i] / 2, 255 - solution[i] / 2);
        //solution[i] = solution[i] % 256;
        //solution[i] = target[i];
    }
}

void Evolver::evaluateSolution(Solution& solution)
{
    double fitness = 0;
    for (uint i = 0; i < solutionLength; i++) {
        if (isAlpha(i)) { //Canal alpha
            fitness += 1.0;
        } else {
            fitness += 1.0 - (std::abs(solution[i] - target[i]) / 255.0);
        }
    }

    solution.fitness = fitness / solutionLength;
}

