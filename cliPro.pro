QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/apputils.cpp \
    src/buttonedit.cpp \
    src/contentbutton.cpp \
    src/dynbutton.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/movebutton.cpp \
    src/profilemenu.cpp \
    src/profilenamedialog.cpp

HEADERS += \
    src/apputils.h \
    src/buttonedit.h \
    src/contentbtncount.h \
    src/contentbutton.h \
    src/dynbutton.h \
    src/mainwindow.h \
    src/movebutton.h \
    src/profilemenu.h \
    src/profilenamedialog.h

FORMS += \
    src/buttonedit.ui \
    src/mainwindow.ui \
    src/movebutton.ui \
    src/profilemenu.ui \
    src/profilenamedialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
