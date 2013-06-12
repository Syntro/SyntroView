# This file is part of Syntro
#
# Copyright (c) 2012 Pansenti, LLC. All rights reserved.
#

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += DisplayStats.h \
    DisplayStatsData.h \
    ImageWindow.h \
    ViewClient.h \
    SyntroView.h \
    ViewSingleCamera.h \
    SelectStreamsDlg.h

SOURCES += DisplayStats.cpp \
    DisplayStatsData.cpp \
    ImageWindow.cpp \
    main.cpp \
    SyntroView.cpp \
    ViewClient.cpp \
    ViewSingleCamera.cpp \
    SelectStreamsDlg.cpp

FORMS += SyntroView.ui \
    ViewSingleCamera.ui \
    DisplayStats.ui 

