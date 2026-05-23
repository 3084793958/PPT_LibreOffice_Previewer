#ifndef PPT_LIBREOFFICE_PREVIEWER_H
#define PPT_LIBREOFFICE_PREVIEWER_H

#include "interfaces/file-preview/file_preview_plugin.h"

#include <QProcess>

class PPT_LibreOffice_Previewer : public QObject, public Ext_Preview_PluginInterface//插件接口
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.Easy.Desktop.Ext_Preview_PluginInterface" FILE "PPT_LibreOffice_Previewer.json")
    //metaData,用于与Easy_Desktop的首次交流,以判断是否为Ext_Preview_PluginInterface

    //PPT_LibreOffice_Previewer.json中需要说明该插件支持Ext_Preview_Plugin,这么写即可
    //
    //{
    //    "Ext_Preview_Plugin": ""
    //}


    Q_INTERFACES(Ext_Preview_PluginInterface)//告诉QT为插件
public:
    explicit PPT_LibreOffice_Previewer(QObject *parent = nullptr);
    virtual ~PPT_LibreOffice_Previewer() override;//析构函数
    virtual void RemovePlugin() override;//移除插件时被调用
    virtual QString Plugin_Name() override;//插件名
    virtual void init(Preview_File_Interface *ptr, QWidget *carrier) override;//初始化函数,给出对应指针,方便方法调用
    virtual void clear() override;//清理由插件产生的预览的函数
    virtual bool previewFile(QFileInfo &info) override;//预览文件函数,需要返回一个bool,告诉Easy_Desktop自己能否预览
private:
    Preview_File_Interface *preview_file_ptr = nullptr;//预览控件的指针
    QProcess *m_process = new QProcess(this);//调用libreoffice转化ppt到pdf
    //不知道什么原因,在QtCreator中无法正常启动libreoffice

    QPair<bool, QFileInfo> Ppt_To_Pdf(QFileInfo &info);//转化函数<是否成功,转化后的文件路径>
};

#endif // PPT_LIBREOFFICE_PREVIEWER_H
