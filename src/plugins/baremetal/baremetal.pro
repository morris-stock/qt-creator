QT += network
include(../../qtcreatorplugin.pri)

# BareMetal files

SOURCES += baremetalplugin.cpp \
    baremetalcustomrunconfiguration.cpp\
    baremetaldevice.cpp \
    baremetalrunconfiguration.cpp \
    baremetalgdbcommandsdeploystep.cpp \
    baremetaldeviceconfigurationwizardpages.cpp \
    baremetaldeviceconfigurationwizard.cpp \
    baremetaldeviceconfigurationwidget.cpp \
    baremetaldebugsupport.cpp \
    gdbserverproviderprocess.cpp \
    gdbserverproviderssettingspage.cpp \
    gdbserverprovider.cpp \
    gdbserverproviderchooser.cpp \
    gdbserverprovidermanager.cpp \
    openocdgdbserverprovider.cpp \
    defaultgdbserverprovider.cpp \
    stlinkutilgdbserverprovider.cpp \
    iarewtoolchain.cpp \

HEADERS += baremetalplugin.h \
    baremetalconstants.h \
    baremetalcustomrunconfiguration.h \
    baremetaldevice.h \
    baremetalrunconfiguration.h \
    baremetalgdbcommandsdeploystep.h \
    baremetaldeviceconfigurationwidget.h \
    baremetaldeviceconfigurationwizard.h \
    baremetaldeviceconfigurationwizardpages.h \
    baremetaldebugsupport.h \
    gdbserverproviderprocess.h \
    gdbserverproviderssettingspage.h \
    gdbserverprovider.h \
    gdbserverproviderchooser.h \
    gdbserverprovidermanager.h \
    openocdgdbserverprovider.h \
    defaultgdbserverprovider.h \
    stlinkutilgdbserverprovider.h \
    iarewtoolchain.h \

RESOURCES += \
    baremetal.qrc
