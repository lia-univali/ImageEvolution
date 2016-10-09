#include "visualizer.h"
#include "utility.h"

sf::Texture& Visualizer::getMainTexture()
{
    return mainTexture;
}

std::vector<sf::Texture>& Visualizer::getProgressTextures()
{
    return progressTextures;
}

void Visualizer::addInferiorLabel(std::string str)
{
    inferiorLabels.push_back(sf::Text(str, font, 14));
}

void Visualizer::setModeString(const std::string& value)
{
    modeString = value;
}

void Visualizer::drawChart(sf::Color chartColor)
{
    float top = window.getSize().y - chartHeight - margin;

    sf::RectangleShape background(sf::Vector2f(window.getSize().x - (margin * 2), chartHeight));
    background.setPosition(margin, top);
    background.setFillColor(sf::Color(255, 255, 255, 20));
    window.draw(background);

    sf::Text text(L"Aptidão", font, 24);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition(window.getSize().x / 2 - (bounds.width / 2), top - bounds.height - 24);
    window.draw(text);

    unsigned int lines = 5;
    for (unsigned int i = 0; i < lines; i++) {
        sf::RectangleShape line(sf::Vector2f(window.getSize().x - (margin * 2), 1));
        line.setFillColor(sf::Color(255, 255, 255, 100));
        line.setPosition(margin, top + ((i / static_cast<float>(lines - 1)) * chartHeight));

        window.draw(line);
    }

    for (unsigned int i = 0; i < fitnessPoints.size(); i++) {
        sf::Vector2f& p = fitnessPoints[i];

        sf::CircleShape point(5);
        point.setPosition(p.x - 3.5, p.y - 3.5);
        point.setFillColor(sf::Color(chartColor.r, chartColor.g, chartColor.b, 25));
        window.draw(point);

        point.setRadius(1.5);
        point.setPosition(p);
        point.setFillColor(chartColor);
        window.draw(point);
    }
}

void Visualizer::draw(bool display, sf::Color chartColor)
{
    window.clear();
    sf::Vector2u windowSize = window.getSize();

    sf::Sprite sprite(mainTexture);
    sprite.setOrigin(imageSize.x / 2.0, 0);
    sprite.setScale(scaling, scaling);
    sprite.setPosition((windowSize.x / 2.0), margin);
    window.draw(sprite);

    /*for (unsigned int i = 0; i < progressTextures.size(); i++) {
        unsigned int x = imageSize.x * i * progressScaling;
        unsigned int y = windowSize.y - imageSize.y * progressScaling;

        sf::Sprite sprite(progressTextures[i]);
        sprite.setPosition(x, y);
        sprite.setScale(progressScaling, progressScaling);
        window.draw(sprite);
    }

    for (unsigned int i = 0; i < inferiorLabels.size(); i++) {
        unsigned int x = imageSize.x * i * progressScaling;
        unsigned int y = windowSize.y - imageSize.y * progressScaling;

        sf::Text& text = inferiorLabels[i];
        text.setPosition(x + (text.getLocalBounds().width / 2), y - 30);
        window.draw(text);
    }*/

    drawChart(chartColor);

    float percentage = Utility::normalize(currentFitness, firstFitnessValue, 1) * 100.0;

    sf::Text percentageText(L"Taxa de aproximação", font, 20);
    percentageText.setPosition(window.getSize().x / 2 - (percentageText.getLocalBounds().width / 2), margin + imageSize.y * scaling + 35);
    window.draw(percentageText);

    percentageText.setString(Utility::createString(percentage, "%"));
    percentageText.setPosition(window.getSize().x / 2 - (percentageText.getLocalBounds().width / 2), margin + imageSize.y * scaling + 65);
    window.draw(percentageText);

    sf::Text modeText(modeString, font, 15);
    modeText.setPosition(margin, margin);
    window.draw(modeText);

    if (display) window.display();
}

void Visualizer::addPoint(float value)
{
    currentFitness = value;

    if (fitnessPoints.empty()) {
        firstFitnessValue = value;
    }

    float x = margin + (fitnessPoints.size() / static_cast<float>(maxGenerations)) * (window.getSize().x - (margin * 2));
    float y = window.getSize().y - (Utility::normalize(value, firstFitnessValue, 1) * chartHeight) - margin;

    fitnessPoints.push_back(sf::Vector2f(x, y));
}

bool Visualizer::closeRequested()
{
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            return true;
        }
    }

    return false;
}

void Visualizer::prepare(sf::Vector2u imageSize, unsigned int maxGenerations)
{
    this->imageSize = imageSize;
    this->maxGenerations = maxGenerations;
    fitnessPoints.clear();
    mainTexture.create(imageSize.x, imageSize.y);

    for (sf::Texture& tex : progressTextures) {
        tex.create(imageSize.x, imageSize.y);
    }
}

bool Visualizer::waitFor(int ms, std::string message)
{
    int current = 0;
    sf::Clock clock;

    while (current < ms) {
        current = clock.getElapsedTime().asMilliseconds();

        std::string str;
        int pixelShift;
        float remaining = (ms - current) / 1000.0;
        if (remaining < 1E-5) {
            str = "Inicializando...";
            pixelShift = 50;
        } else {
            str = Utility::createString(message, " em ", remaining, " s");
            pixelShift = 90;
        }


        sf::Text text(str, font, 18);

        float x = window.getSize().x / 2 - pixelShift;
        float y = ((margin + imageSize.y * scaling + 80) + (window.getSize().y - margin - chartHeight - 60)) / 2.0;
        text.setPosition(x, y);

        draw(false, sf::Color::Green);
        window.draw(text);
        window.display();

        if (closeRequested()) {
            return false;
        }
    }

    return true;
}

Visualizer::Visualizer(unsigned int width, unsigned int height, unsigned int scaling)
    : window(sf::VideoMode::getDesktopMode(), "Evolucao de Imagens", sf::Style::Fullscreen, sf::ContextSettings(0, 0, 8)),
      scaling(scaling), progressTextures(6)
{
    progressScaling = scaling / 2.0;
    chartHeight = std::round(window.getSize().y * 0.15);

    font.loadFromFile("fonts/Times_New_Roman.ttf");
}

void Visualizer::displayImage(char* data, unsigned int size)
{
    window.clear();

    sf::Texture texture;
    texture.loadFromMemory(data, size);

    sf::Sprite sprite(texture);
    sprite.setPosition(window.getSize().x / 2 - texture.getSize().x / 2, window.getSize().y / 2 - texture.getSize().y / 2);

    window.draw(sprite);
    window.display();
}

void Visualizer::displayMessage(const std::string &message)
{
    window.clear();

    sf::Text text(message, font, 30);
    float x = window.getSize().x / 2 - (text.getLocalBounds().width / 2);
    float y = window.getSize().y / 2 - (text.getLocalBounds().height / 2);
    text.setPosition(x, y);

    window.draw(text);
    window.display();
}

sf::Vector2u Visualizer::getSize() const
{
    return window.getSize();
}



