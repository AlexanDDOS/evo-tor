TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -LD:/Sources/SFML-2.5.1/lib

CONFIG(release, debug|release): LIBS += -lsfml-graphics -lsfml-main -lsfml-window -lsfml-system
CONFIG(debug, debug|release): LIBS += -lsfml-graphics-d -lsfml-main-d -lsfml-window-d -lsfml-system-d

INCLUDEPATH += D:/Sources/SFML-2.5.1/include
DEPENDPATH += D:/Sources/SFML-2.5.1/include

SOURCES += \
        main.cpp\
        beings.cpp
HEADERS += \
        beings.hpp
