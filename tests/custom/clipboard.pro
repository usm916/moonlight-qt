QT = core
CONFIG += console c++17
CONFIG -= app_bundle debug_and_release
TEMPLATE = app
TARGET = clipboard-state-test
INCLUDEPATH += ../../moonlight-common-c/moonlight-common-c/src
SOURCES += clipboard.cpp
