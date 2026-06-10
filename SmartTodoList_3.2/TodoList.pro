QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++17

TARGET   = TodoList
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    datetimepicker.cpp

HEADERS += \
    mainwindow.h \
    task.h \
    datetimepicker.h

# Suppress deprecation warnings
DEFINES += QT_DEPRECATED_WARNINGS

# Windows: no console window
win32: QMAKE_LFLAGS += -mwindows

RC_ICONS =
