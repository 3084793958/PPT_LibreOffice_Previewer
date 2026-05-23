#include "ppt_libreoffice_previewer.h"
#include <QFileInfo>

PPT_LibreOffice_Previewer::PPT_LibreOffice_Previewer(QObject *parent)
    :QObject(parent)
{}
PPT_LibreOffice_Previewer::~PPT_LibreOffice_Previewer()
{
    RemovePlugin();
}
void PPT_LibreOffice_Previewer::RemovePlugin()
{
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}
QString PPT_LibreOffice_Previewer::Plugin_Name()
{
    return QStringLiteral("PPT_LibreOffice_Previewer");//使用QStringLiteral将字符串存储在只读数据段，避免跨模块动态分配内存
}
void PPT_LibreOffice_Previewer::init(Preview_File_Interface *ptr, QWidget *carrier)
{
    (void) carrier;
    preview_file_ptr = ptr;//接住指针
}
void PPT_LibreOffice_Previewer::clear()
{
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished(500);
    }
}
bool PPT_LibreOffice_Previewer::previewFile(QFileInfo &info)
{
    if (!preview_file_ptr)
    {
        return false;
    }

    QString suffix = info.suffix().toLower();
    if (suffix != "ppt" && suffix != "pptx" && suffix != "odp")
    {
        return false;
    }
    clear();
    QPair<bool, QFileInfo> result = Ppt_To_Pdf(info);
    if (result.first)
    {
        preview_file_ptr->setupPdfPreview(result.second);
        return true;
    }
    else
    {
        return false;
    }
}
QPair<bool, QFileInfo> PPT_LibreOffice_Previewer::Ppt_To_Pdf(QFileInfo &info)
{
    const QString outputPdfPath = "/tmp/Easy_Desktop/Plugin_Tmp_Data/tmp_ppt_preview.pdf";

    QStringList args;
    args << "--headless" << "--convert-to" << "pdf"
         << "--outdir" << "/tmp/Easy_Desktop/Plugin_Tmp_Data"
         << info.absoluteFilePath();

    int exit_code = m_process->execute("libreoffice", args);//这里是阻塞式的
    if (exit_code != 0)
    {
        return {false, QFileInfo()};
    }

    QString generatedPdf = QString("/tmp/Easy_Desktop/Plugin_Tmp_Data/%1.pdf").arg(info.completeBaseName());

    if (QFile::exists(generatedPdf))
    {
        QFile::remove(outputPdfPath);
        if (!QFile::rename(generatedPdf, outputPdfPath))
        {
            QFile::remove(generatedPdf);
            return {false, QFileInfo()};
        }
    }
    else
    {
        return {false, QFileInfo()};
    }

    QFileInfo pdfInfo(outputPdfPath);
    if (pdfInfo.exists() && pdfInfo.size() > 0)
    {
        return {true, pdfInfo};
    }
    else
    {
        return {false, QFileInfo()};
    }
}
