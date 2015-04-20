QT       += sql

INCLUDEPATH += $$PWD

SOURCES += $$PWD/qdbmanager.cpp \
           $$PWD/interfaces/qsqlite.cpp \
           $$PWD/entities/abstractEntity.cpp \
           $$PWD/entities/baseEntity.cpp \
           $$PWD/entityfactory.cpp

HEADERS += $$PWD/qdbmanager.h\
           $$PWD/interfaces/qsqlite.h \
           $$PWD/entities/abstractEntity.h \
           $$PWD/entities/baseEntity.h \
           $$PWD/entityfactory.h
