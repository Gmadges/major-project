TARGET=server.out
TEMPLATE=app
OBJECTS_DIR=obj
QT-=gui opengl core

SOURCES += $$PWD/*.cpp
HEADERS  += $$PWD/*.h     

INCLUDEPATH += $$(HOME)/libs/include/ ./ /usr/include/boost/ ../../common/
LIBS += -L$$(HOME)/libs/lib -L/usr/local/lib -lzmq -lboost_system

DESTDIR=./
CONFIG += console c++11 debug