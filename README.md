# PPT_LibreOffice_Previewer
基于Ext_Preview_PluginInterface的预览插件,用于预览PPT

该项目为Ext_Preview_PluginInterface插件开发的演示

## 使用方法

在[Easy_Desktop](https://github.com/3084793958/Easy_Desktop)中的预览控件中载入插件

# 26.5.28更新
1.将加载换位异步

2.增加文件支持(支持libreoffice所支持的格式)(math使用的是打印法,而不是转换法)

# 初代

## 原理
利用 libreoffice 命令行将PPT文件转为pdf,再由预览控件读取

libreoffice转换需要时间,这里直接阻断

## Tips
注释会教你怎么写的

