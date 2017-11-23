QT += core serialport sql

QT -= gui

DESTDIR = ../OTDRServer/build

CONFIG += c++11

TARGET = otdrd

TEMPLATE = app

SOURCES +=  otdrd/main.cpp \
            otdrd/OTDR.cpp \
            otdrd/reflectogram.cpp \
            otdrd/otdrprocessing.cpp \
            otdrd/compare.cpp \
            otdrd/lineevent.cpp

HEADERS +=  otdrd/OTDR.h \
            otdrd/reflectogram.h \
            otdrd/otdrprocessing.h \
            otdrd/compare.h \
            otdrd/lineevent.h
