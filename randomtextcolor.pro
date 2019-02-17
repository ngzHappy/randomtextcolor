
QT       += core gui
QT += widgets

TARGET = randomtextcolor
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17
CONFIG += console

SOURCES += \
        main.cpp \
        MainWindow.cpp \
    RandomTextColor.cpp

HEADERS += \
        MainWindow.hpp \
    RandomTextColor.hpp

FORMS += \
        MainWindow.ui


