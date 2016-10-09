TEMPLATE = app
CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle
CONFIG += qt
QT -= gui
QT += network

QMAKE_CXXFLAGS += -D__NO_INLINE__ -DHAVE_IMAGE_HASH -O3
LIBS += -lpthread

win32 {
    INCLUDEPATH += "C:\Users\Fernando\Downloads\SFML-2.3.2-sources\SFML-2.3.2\include"
    LIBS += -L"C:\Users\Fernando\Downloads\SFML-2.3.2-sources\SFML-2.3.2\binaries\lib"
    LIBS += -lsfml-graphics -lsfml-window -lsfml-system
    LIBS += -lgdi32
}

!win32 {
    LIBS += -lsfml-graphics -lsfml-window -lsfml-system
}


SOURCES += main.cpp \
    evolver.cpp \
    utility.cpp \
    solution.cpp \
    visualizer.cpp

HEADERS += \
    evolver.h \
    utility.h \
    solution.h \
    avir.h \
    avir_dil.h \
    avir_float4_sse.h \
    avir_float8_avx.h \
    visualizer.h

