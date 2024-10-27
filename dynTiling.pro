QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    contentbutton.cpp \
    contentedit.cpp \
    dynaddrmbutton.cpp \
    main.cpp \
    mainwindow.cpp \
    profiledialog.cpp \
    profilemenu.cpp

HEADERS += \
    contentbtncount.h \
    contentbutton.h \
    contentedit.h \
    dynaddrmbutton.h \
    mainwindow.h \
    profiledialog.h \
    profilemenu.h \
    timedpopup.h

FORMS += \
    contentedit.ui \
    mainwindow.ui \
    profiledialog.ui \
    profilemenu.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
