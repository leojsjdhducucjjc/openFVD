# Shared first-party application sources.
#
# The production app and the regression test target both include this file so
# the tests exercise the same project, track, UI, and serialization code that
# ships in FVD++. main.cpp remains target-specific.

SOURCES += \
    $$OPENFVD_ROOT/trackbuilder/buildereditorwidget.cpp \
    $$OPENFVD_ROOT/trackbuilder/buildersegment.cpp \
    $$OPENFVD_ROOT/trackbuilder/secbuilder.cpp \
    $$OPENFVD_ROOT/core/undohandler.cpp \
    $$OPENFVD_ROOT/core/undoaction.cpp \
    $$OPENFVD_ROOT/core/trackhandler.cpp \
    $$OPENFVD_ROOT/core/track.cpp \
    $$OPENFVD_ROOT/core/subfunction.cpp \
    $$OPENFVD_ROOT/core/smoothhandler.cpp \
    $$OPENFVD_ROOT/core/sectionhandler.cpp \
    $$OPENFVD_ROOT/core/section.cpp \
    $$OPENFVD_ROOT/core/secstraight.cpp \
    $$OPENFVD_ROOT/core/secgeometric.cpp \
    $$OPENFVD_ROOT/core/secforced.cpp \
    $$OPENFVD_ROOT/core/seccurved.cpp \
    $$OPENFVD_ROOT/core/secbezier.cpp \
    $$OPENFVD_ROOT/core/saver.cpp \
    $$OPENFVD_ROOT/core/nolimitsimporter.cpp \
    $$OPENFVD_ROOT/core/mnode.cpp \
    $$OPENFVD_ROOT/core/function.cpp \
    $$OPENFVD_ROOT/core/exportfuncs.cpp \
    $$OPENFVD_ROOT/core/secnlcsv.cpp \
    $$OPENFVD_ROOT/osx/common.cpp \
    $$OPENFVD_ROOT/renderer/trackmesh.cpp \
    $$OPENFVD_ROOT/renderer/mytexture.cpp \
    $$OPENFVD_ROOT/renderer/myshader.cpp \
    $$OPENFVD_ROOT/renderer/myframebuffer.cpp \
    $$OPENFVD_ROOT/renderer/glviewwidget.cpp \
    $$OPENFVD_ROOT/ui/transitionwidget.cpp \
    $$OPENFVD_ROOT/ui/trackwidget.cpp \
    $$OPENFVD_ROOT/ui/trackproperties.cpp \
    $$OPENFVD_ROOT/ui/smoothui.cpp \
    $$OPENFVD_ROOT/ui/qcustomplot.cpp \
    $$OPENFVD_ROOT/ui/projectwidget.cpp \
    $$OPENFVD_ROOT/ui/optionsmenu.cpp \
    $$OPENFVD_ROOT/ui/objectexporter.cpp \
    $$OPENFVD_ROOT/ui/mytreewidget.cpp \
    $$OPENFVD_ROOT/ui/myqdoublespinbox.cpp \
    $$OPENFVD_ROOT/ui/mainwindow.cpp \
    $$OPENFVD_ROOT/ui/importui.cpp \
    $$OPENFVD_ROOT/ui/graphwidget.cpp \
    $$OPENFVD_ROOT/ui/graphhandler.cpp \
    $$OPENFVD_ROOT/ui/exportui.cpp \
    $$OPENFVD_ROOT/ui/draglabel.cpp \
    $$OPENFVD_ROOT/ui/conversionpanel.cpp

HEADERS += \
    $$OPENFVD_ROOT/trackbuilder/buildereditorwidget.h \
    $$OPENFVD_ROOT/trackbuilder/buildersegment.h \
    $$OPENFVD_ROOT/trackbuilder/secbuilder.h \
    $$OPENFVD_ROOT/core/undohandler.h \
    $$OPENFVD_ROOT/core/undoaction.h \
    $$OPENFVD_ROOT/core/trackhandler.h \
    $$OPENFVD_ROOT/core/track.h \
    $$OPENFVD_ROOT/core/subfunction.h \
    $$OPENFVD_ROOT/core/smoothhandler.h \
    $$OPENFVD_ROOT/core/sectionhandler.h \
    $$OPENFVD_ROOT/core/section.h \
    $$OPENFVD_ROOT/core/secstraight.h \
    $$OPENFVD_ROOT/core/secgeometric.h \
    $$OPENFVD_ROOT/core/secforced.h \
    $$OPENFVD_ROOT/core/seccurved.h \
    $$OPENFVD_ROOT/core/secbezier.h \
    $$OPENFVD_ROOT/core/saver.h \
    $$OPENFVD_ROOT/core/nolimitsimporter.h \
    $$OPENFVD_ROOT/core/mnode.h \
    $$OPENFVD_ROOT/core/function.h \
    $$OPENFVD_ROOT/core/exportfuncs.h \
    $$OPENFVD_ROOT/core/secnlcsv.h \
    $$OPENFVD_ROOT/osx/common.h \
    $$OPENFVD_ROOT/renderer/trackmesh.h \
    $$OPENFVD_ROOT/renderer/mytexture.h \
    $$OPENFVD_ROOT/renderer/myshader.h \
    $$OPENFVD_ROOT/renderer/myframebuffer.h \
    $$OPENFVD_ROOT/renderer/glviewwidget.h \
    $$OPENFVD_ROOT/ui/transitionwidget.h \
    $$OPENFVD_ROOT/ui/trackwidget.h \
    $$OPENFVD_ROOT/ui/trackproperties.h \
    $$OPENFVD_ROOT/ui/smoothui.h \
    $$OPENFVD_ROOT/ui/qcustomplot.h \
    $$OPENFVD_ROOT/ui/projectwidget.h \
    $$OPENFVD_ROOT/ui/optionsmenu.h \
    $$OPENFVD_ROOT/ui/objectexporter.h \
    $$OPENFVD_ROOT/ui/mytreewidget.h \
    $$OPENFVD_ROOT/ui/myqdoublespinbox.h \
    $$OPENFVD_ROOT/ui/mainwindow.h \
    $$OPENFVD_ROOT/ui/importui.h \
    $$OPENFVD_ROOT/ui/graphwidget.h \
    $$OPENFVD_ROOT/ui/graphhandler.h \
    $$OPENFVD_ROOT/ui/exportui.h \
    $$OPENFVD_ROOT/ui/draglabel.h \
    $$OPENFVD_ROOT/ui/conversionpanel.h \
    $$OPENFVD_ROOT/lenassert.h

FORMS += \
    $$OPENFVD_ROOT/ui/transitionwidget.ui \
    $$OPENFVD_ROOT/ui/trackwidget.ui \
    $$OPENFVD_ROOT/ui/trackproperties.ui \
    $$OPENFVD_ROOT/ui/smoothui.ui \
    $$OPENFVD_ROOT/ui/projectwidget.ui \
    $$OPENFVD_ROOT/ui/optionsmenu.ui \
    $$OPENFVD_ROOT/ui/objectexporter.ui \
    $$OPENFVD_ROOT/ui/mainwindow.ui \
    $$OPENFVD_ROOT/ui/importui.ui \
    $$OPENFVD_ROOT/ui/graphwidget.ui \
    $$OPENFVD_ROOT/ui/exportui.ui \
    $$OPENFVD_ROOT/ui/conversionpanel.ui
