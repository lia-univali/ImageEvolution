#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <vector>
#include <string>
#include <functional>
#include <SFML/Graphics.hpp>

class Visualizer
{
private:
    sf::RenderWindow window;
    float margin = 30;
    float firstFitnessValue;
    float currentFitness;
    unsigned int scaling;
    unsigned int progressScaling;
    unsigned int maxGenerations;
    unsigned int chartHeight;
    sf::Texture mainTexture;
    std::vector<sf::Texture> progressTextures;
    sf::Font font;
    sf::Vector2u imageSize;
    std::vector<sf::Text> inferiorLabels;
    std::vector<sf::Vector2f> fitnessPoints;
    std::string modeString;

    void drawChart(sf::Color chartColor);
public:
    Visualizer(unsigned int width, unsigned int height, unsigned int scaling = 5);
    void displayImage(char* data, unsigned int size);
    void displayMessage(const std::string& message);
    sf::Texture& getMainTexture();
    std::vector<sf::Texture>& getProgressTextures();
    void addInferiorLabel(std::string str);
    sf::Vector2u getSize() const;
    void draw(bool display = true, sf::Color chartColor = sf::Color::Red);
    void addPoint(float value);
    bool closeRequested();
    void prepare(sf::Vector2u imageSize, unsigned int maxGenerations);
    bool waitFor(int ms, std::string message);
    void setModeString(const std::string& value);
};

#endif // VISUALIZER_H
