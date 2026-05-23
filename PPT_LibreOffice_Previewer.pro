#插件只能以Debug模式编译,不能以Release模式编译
#若需优化,使用strip命令
QT += widgets core
TEMPLATE = lib
CONFIG += plugin c++11
TARGET = PPT_LibreOffice_Previewer

DISTFILES += \
    PPT_LibreOffice_Previewer.json

HEADERS += \
    ppt_libreoffice_previewer.h \
    interfaces/easy_desktop_class.h \
    interfaces/media-widget-action/media_widgetaction_interface.h \
    interfaces/file-preview/file_preview_plugin.h \
    interfaces/file-preview/preview_file_interface.h

SOURCES += \
    ppt_libreoffice_previewer.cpp \
    interfaces/easy_desktop_class.cpp
