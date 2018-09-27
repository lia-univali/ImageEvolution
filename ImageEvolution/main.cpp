#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "evolver.h"
#include "solution.h"
#include <chrono>
#include "avir.h"
#include "visualizer.h"
#include <QTcpSocket>
#include <cstring>
#include <thread>
#include <QCoreApplication>
#include <QDir>
#include <QList>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QString>
#include <QEventLoop>
#include <QTimer>

enum Action { Switch, Continue, None };
enum Mode { Local, Remote };

Action checkInteraction() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
        return Switch;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        return Continue;
    }

    return None;
}

QByteArray generateQRCode(std::string data) {
    std::string query("http://api.qrserver.com/v1/create-qr-code/?data=lia-opa.ddns.net%2F%3Fpass%3D" + data + "&format=jpg&size=600x600");
    QNetworkRequest req(QUrl(QString::fromStdString(query)));

    QNetworkAccessManager nam;
    QNetworkReply* reply = nam.get(req);
    QByteArray bytes;

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bytes = reply->readAll();

    return bytes;
}

sf::Vector2u calculateSize(sf::Vector2u size, sf::Vector2u windowSize) {
    float fraction = 0.1;
    sf::Vector2f bounds(windowSize.x * fraction, windowSize.y * fraction);
    float coefficient;

    if (size.x > size.y) {
        coefficient = bounds.x / size.x;
    } else {
        coefficient = bounds.y / size.y;
    }

    return sf::Vector2u(std::round(size.x * coefficient), std::round(size.y * coefficient));
}

std::vector<sf::Uint8> rescaleImage(const sf::Image& image, sf::Vector2u newSize) {
    unsigned int channels = 4;
    avir::CImageResizer<> resizer(8);
    std::vector<sf::Uint8> data(newSize.x * newSize.y * channels);

    sf::Vector2u size = image.getSize();
    resizer.resizeImage(image.getPixelsPtr(), size.x, size.y, 0, &data[0], newSize.x, newSize.y, channels, 0);

    return data;
}

sf::Image load(const std::string& path) {
    std::cout << path << "\n";
    sf::Image image;
    image.loadFromFile(path);
    return image;
}

sf::Image load(char* data, int size) {
    sf::Image image;
    image.loadFromMemory(data, size);
    return image;
}

Action begin(const sf::Image& targetImage, Visualizer& visualizer, Mode mode) {
    std::cout << "Starting up...\n";
    visualizer.setModeString("Modo " + std::string(mode == Mode::Local ? "local" : "remoto"));
    visualizer.displayMessage("Preparando...");

    sf::Vector2u windowSize = visualizer.getSize();

    sf::Vector2u imageSize = calculateSize(targetImage.getSize(), windowSize);
    std::vector<sf::Uint8> data = rescaleImage(targetImage, imageSize);

    Evolver evolver;
    evolver.prepare(&data[0], imageSize);

    unsigned int progressCount = visualizer.getProgressTextures().size() - 1;
    unsigned int generations = progressCount * 170;

    visualizer.prepare(imageSize, generations);

    Action action = Action::None;

    while (evolver.getGeneration() <= generations) {
        if (visualizer.closeRequested()) {
            exit(0);
        }

        evolver.evolve();

        std::vector<Solution>& population = evolver.getPopulation();

        Solution& best = population.front();
        visualizer.getMainTexture().update(&best[0]);
        visualizer.addPoint(best.fitness);

        /*if ((evolver.getGeneration() - 1) % (generations / progressCount) == 0) {
            unsigned int index = (evolver.getGeneration() - 1) / (generations / progressCount);

            visualizer.getProgressTextures().at(index).update(&best[0]);
        }*/

        visualizer.draw();
        action = checkInteraction();

        if (action == Action::Continue) {
            break;
        } else if (action == Action::Switch) {
            visualizer.waitFor(5000, "Alternando");
            return action;
        }
    }

    bool shouldContinue = visualizer.waitFor(10000, "Continuando");
    if (!shouldContinue) {
        exit(0);
    }

    return action;
}

QByteArray readFileFromSocket(QTcpSocket& socket) {
    QByteArray buffer;
    int dataSize;

    while(socket.bytesAvailable()) {
        socket.read((char*)&dataSize, sizeof(int));
        buffer = socket.read(dataSize);

        while(buffer.size() < dataSize) { // only part of the message has been received
            socket.waitForReadyRead(); // alternatively, store the buffer and wait for the next readyRead()
            buffer.append(socket.read(dataSize - buffer.size())); // append the remaining bytes of the message
        }
    }

    //std::cout << buffer.toStdString() << "\n";

    return buffer;
}

QByteArray readDataFromSocket(QTcpSocket& socket, int dataSize) {
    QByteArray buffer;

    while(socket.bytesAvailable()) {
        buffer = socket.read(dataSize);

        while(buffer.size() < dataSize) { // only part of the message has been received
            socket.waitForReadyRead(); // alternatively, store the buffer and wait for the next readyRead()
            buffer.append(socket.read(dataSize - buffer.size())); // append the remaining bytes of the message
        }
    }

    //std::cout << buffer.toStdString() << "\n";

    return buffer;
}

void startLocal(Visualizer& visualizer) {
    QDir directory("images");

    directory.setFilter(QDir::Files);
    std::vector<std::string> files;
    for (const QString& file : directory.entryList()) {
        files.push_back(file.toStdString());
    }

    while (true) {
        std::random_shuffle(files.begin(), files.end());
        for (const std::string& file : files) {
            Action action = begin(load("images/" + file), visualizer, Mode::Local);

            if (action == Action::Continue) {
                continue;
            } else if (action == Action::Switch) {
                return;
            }
        }
    }
}

void startSocket(Visualizer& visualizer) {
    Action action = Action::None;

    while (true) {
        QEventLoop loop;
        QTcpSocket socket;

        QObject::connect(&socket, &QTcpSocket::readyRead, [&] {
            std::string passwd = readDataFromSocket(socket, 6).toStdString();
            std::cout << "Pass = " << passwd << "\n";

            QByteArray data = generateQRCode(passwd);
            visualizer.displayImage(data.data(), data.size());

            QEventLoop pollLoop;
            QTimer timer;
            pollLoop.connect(&timer, &QTimer::timeout, [&]() {
                socket.write("count", 5);
                socket.waitForReadyRead();

                QByteArray countData = readDataFromSocket(socket, 4);
                int count = *((int*) countData.data());

                std::cout << "Count = " << count << "\n";
                if (count != 0) {
                    timer.stop();
                    pollLoop.quit();
                }
            });

            QTimer interactionTimer;
            pollLoop.connect(&interactionTimer, &QTimer::timeout, [&]() {
                action = checkInteraction();

                if (action == Action::Switch) {
                    pollLoop.quit();
                }
            });

            timer.start(1000);
            interactionTimer.start(100);
            pollLoop.exec();
            std::cout << "Action: " << action << "\n";

            if (action == Action::Switch) {
                loop.quit();
            } else {
                socket.write("get", 3);
                socket.waitForReadyRead();

                QByteArray buffer = readFileFromSocket(socket);
                action = begin(load(buffer.data(), buffer.size()), visualizer, Mode::Remote);

                socket.write("pop", 3);
                socket.waitForBytesWritten();

                loop.quit();
            }
        });

        QObject::connect(&socket, &QTcpSocket::connected, [&] { socket.write("password", 8); });

        socket.connectToHost("localhost", 3099);
        loop.exec();

        if (action == Action::Switch) {
            return;
        }
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    std::srand(std::time(nullptr));
    const int width = 1024;
    const int height = 1024;

    Visualizer visualizer(width, height);

    while (true) {
        startLocal(visualizer);
        startSocket(visualizer);
    }

    return app.exec();
}

