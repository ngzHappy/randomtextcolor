
QT       += core gui

TARGET = randomtextcolor
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++17
CONFIG += console

SOURCES += \
        main.cpp \
    RandomTextColor.cpp

HEADERS += \
    RandomTextColor.hpp

FORMS +=


CONFIG(debug,debug|release){
    DESTDIR = $$PWD/bin/debugDir
}else{
    DESTDIR = $$PWD/bin/releaseDir
}

