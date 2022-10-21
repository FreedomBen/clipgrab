# #####################################################################
# Automatically generated by qmake (2.01a) Mo 26. Okt 18:26:00 2009
# #####################################################################
TEMPLATE = app
TARGET = clipgrab
DEPENDPATH += . \
    release
INCLUDEPATH += . src
QT += core \
    widgets
QT += gui
QT += network
QT += xml
QT += webenginewidgets

DEFINES += "USE_YTDLP=1"

MOC_DIR = temp
UI_DIR = temp
OBJECTS_DIR = temp
QM_FILES_INSTALL_PATH = i18n

# Input
HEADERS += \
    src/converter.h \
    src/converter_copy.h \
    src/converter_ffmpeg.h \
    src/download_list_model.h \
    src/helper_downloader.h \
    src/mainwindow.h \
    src/video.h \
    src/notifications.h \
    src/message_dialog.h \
    src/clipgrab.h \
    src/web_engine_view.h \
    src/youtube_dl.h
FORMS += \
    src/ui/metadata-dialog.ui \
    src/ui/helper_downloader.ui \
    src/ui/mainwindow.ui \
    src/ui/update_message.ui \
    src/ui/message_dialog.ui
SOURCES += \
    src/converter.cpp \
    src/converter_copy.cpp \
    src/converter_ffmpeg.cpp \
    src/download_list_model.cpp \
    src/helper_downloader.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/video.cpp \
    src/notifications.cpp \
    src/message_dialog.cpp \
    src/clipgrab.cpp \
    src/web_engine_view.cpp \
    src/youtube_dl.cpp
RESOURCES += resources.qrc
TRANSLATIONS += ts/clipgrab_bg.ts \
                ts/clipgrab_bn.ts \
                ts/clipgrab_ca.ts \
                ts/clipgrab_cs.ts \
                ts/clipgrab_de.ts \
                ts/clipgrab_el.ts \
                ts/clipgrab_es.ts \
                ts/clipgrab_eu.ts \
                ts/clipgrab_fa.ts \
                ts/clipgrab_fr.ts \
                ts/clipgrab_fi.ts \
                ts/clipgrab_hr.ts \
                ts/clipgrab_hu.ts \
                ts/clipgrab_id.ts \
                ts/clipgrab_it.ts \
                ts/clipgrab_ja.ts \
                ts/clipgrab_ko.ts \
                ts/clipgrab_mk.ts \
                ts/clipgrab_lt.ts \
                ts/clipgrab_nl.ts \
                ts/clipgrab_no.ts \
                ts/clipgrab_pa.ts \
                ts/clipgrab_pl.ts \
                ts/clipgrab_pt.ts \
                ts/clipgrab_ro.ts \
                ts/clipgrab_ru.ts \
                ts/clipgrab_si.ts \
                ts/clipgrab_sr.ts \
                ts/clipgrab_sv.ts \
                ts/clipgrab_sw.ts \
                ts/clipgrab_tr.ts \
                ts/clipgrab_vi.ts \
                ts/clipgrab_zh.ts \
                ts/clipgrab_strings.ts
CODECFORTR = UTF-8
win32:RC_FILE = windows_icon.rc
macx { 
    TARGET = ClipGrab
    ICON = clipgrab.icns
    RC_FILE = clipgrab.icns
    QMAKE_INFO_PLIST = ClipGrab.plist
    LIBS += -framework AppKit -framework Foundation
}
VERSION = 3.9.6
DEFINES += CLIPGRAB_VERSION=$$VERSION
