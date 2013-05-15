### eqmake4 was here ###
CONFIG -= debug_and_release debug
CONFIG += release

VERSION = 0.2.31
QT += dbus
unix {
    PREFIX=/usr
    LIBDIR = $$PREFIX/lib
}
CONFIG += link_pkgconfig meegotouch mobility qt 
PKGCONFIG += meegotouch 
MOBILITY+=contacts

TEMPLATE = app
SUBDIRS = theme
MOC_DIR = .moc
OBJECTS_DIR = .obj
INCLUDEPATH += ./src
INCLUDEPATH += /usr/include/qt4/QtContacts 
LIBS += -lseaside
MODEL_HEADERS += \
    src/seasideslidermodel.h
STYLE_HEADERS += \
    src/imagelabelstyle.h \
    src/seasidesliderstyle.h
HEADERS += \
    $$MODEL_HEADERS \
    $$STYLE_HEADERS \
    src/imagelabel.h \
    src/people.h \
    src/peopleapp.h \
    src/peoplegroupedview.h \
    src/person.h \
    src/personcard.h \
    src/personcardview.h \
    src/persondetailview.h \
    src/personeditview.h \
    src/slider.h \
    src/sliderview.h \
    src/window.h \
    src/personcommspage.h \
    src/peopledbusadaptor.h
SOURCES = \
    src/imagelabel.cpp \
    src/main.cpp \
    src/people.cpp \
    src/peopleapp.cpp \
    src/peoplegroupedview.cpp \
    src/person.cpp \
    src/personcard.cpp \
    src/personcardview.cpp \
    src/persondetailview.cpp \
    src/personeditview.cpp \
    src/slider.cpp \
    src/window.cpp \
    src/sliderview.cpp \
    src/personcommspage.cpp \
    src/peopledbusadaptor.cpp

LIBS += -L$$PREFIX/lib
INCLUDEPATH += $$PREFIX/include
TARGET = meego-handset-people
target.path = $$PREFIX/bin

# Theme
M_THEME_DIR = $$PREFIX/share/themes/base/meegotouch
PEOPLE_THEME_DIR = $$M_THEME_DIR/$$TARGET
theme.files = theme/*
theme.path = $$PEOPLE_THEME_DIR
theme.CONFIG += no_check_exist

desktop.files = meego-handset-people.desktop
desktop.path = $$PREFIX/share/applications

INSTALLS += \
    theme \
    desktop \
    dbus_service \
    target

OTHER_FILES = \
    theme/style/meego-handset-people.css \
    theme/meego-handset-people.conf \
    meego-handset-people.desktop

MGEN_OUTDIR = .gen
mgen.output = $$GEN_DIR/gen_${QMAKE_FILE_BASE}data.cpp
mgen.clean += $$GEN_DIR/gen_*

# DBUS Service files
dbus_service.files = com.meego.people.service
dbus_service.path = $$INSTALL_ROOT/usr/share/dbus-1/services
