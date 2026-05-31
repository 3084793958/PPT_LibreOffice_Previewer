#include "ppt_libreoffice_previewer.h"
#include <QFileInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QSet>
#include <QTextEdit>

PPT_LibreOffice_Previewer::PPT_LibreOffice_Previewer(QObject *parent)
    :QObject(parent)
{
    connect(watcher, &QFutureWatcher<QPair<bool, QFileInfo>>::finished, this, [=]
    {
        QFuture<QPair<bool, QFileInfo>> future = watcher->future();
        if (!future.isResultReadyAt(0))
        {
            return;
        }
        if (future.result().first)
        {
            preview_file_ptr->clearCurrentPreview();
            preview_file_ptr->setupPdfPreview(future.result().second);
        }
    });
}
PPT_LibreOffice_Previewer::~PPT_LibreOffice_Previewer()
{
    RemovePlugin();
}
void PPT_LibreOffice_Previewer::RemovePlugin()
{
    disconnect();
    watcher->setPaused(true);
    watcher->future().cancel();
    watcher->cancel();
    if (m_process && m_process->state() != QProcess::NotRunning)
    {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}
QString PPT_LibreOffice_Previewer::Plugin_Name()
{
    return QString("PPT_LibreOffice_Previewer-V") + plugin_version.VersionName();
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
static const QSet<QString> convertibleMimeTypes = //允许转换文件的mimeData
{

    // LibreOffice
    "application/vnd.oasis.opendocument.text",
    "application/vnd.oasis.opendocument.spreadsheet",
    "application/vnd.oasis.opendocument.presentation",
    "application/vnd.oasis.opendocument.graphics",
    "application/vnd.oasis.opendocument.formula",
    "application/vnd.oasis.opendocument.text-template",
    "application/vnd.oasis.opendocument.spreadsheet-template",
    "application/vnd.oasis.opendocument.presentation-template",
    "application/vnd.oasis.opendocument.graphics-template",
    "application/vnd.oasis.opendocument.formula-template",
    "application/vnd.oasis.opendocument.image",
    "application/vnd.oasis.opendocument.base",

    // ODF变体(x-vnd.)
    "application/x-vnd.oasis.opendocument.text",
    "application/x-vnd.oasis.opendocument.spreadsheet",
    "application/x-vnd.oasis.opendocument.presentation",
    "application/x-vnd.oasis.opendocument.graphics",
    "application/x-vnd.oasis.opendocument.formula",
    "application/x-vnd.oasis.opendocument.image",

    // StarOffice遗留
    "application/vnd.sun.xml.writer",
    "application/vnd.sun.xml.writer.template",
    "application/vnd.sun.xml.writer.global",
    "application/vnd.sun.xml.calc",
    "application/vnd.sun.xml.calc.template",
    "application/vnd.sun.xml.impress",
    "application/vnd.sun.xml.impress.template",
    "application/vnd.sun.xml.draw",
    "application/vnd.sun.xml.draw.template",
    "application/vnd.sun.xml.math",

    // Microsoft Office 97-2003 (OLE)
    "application/msword",
    "application/msword.template",
    "application/vnd.ms-excel",
    "application/vnd.ms-excel.template.macroEnabled.12",
    "application/vnd.ms-excel.addin.macroEnabled.12",
    "application/vnd.ms-excel.sheet.binary.macroEnabled.12",
    "application/vnd.ms-excel.sheet.macroEnabled.12",
    "application/vnd.ms-powerpoint",
    "application/vnd.ms-powerpoint.template.macroEnabled.12",
    "application/vnd.ms-powerpoint.addin.macroEnabled.12",
    "application/vnd.ms-powerpoint.slideshow.macroEnabled.12",

    // OLE变体
    "application/x-msword",
    "application/x-msexcel",
    "application/x-mspowerpoint",

    // Microsoft Office 2007+ (OOXML)

    // Word
    "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
    "application/vnd.openxmlformats-officedocument.wordprocessingml.template",
    "application/vnd.ms-word.document.macroEnabled.12",
    "application/vnd.ms-word.template.macroEnabled.12",

    // Excel
    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.template",
    "application/vnd.ms-excel.sheet.macroEnabled.12",
    "application/vnd.ms-excel.template.macroEnabled.12",
    "application/vnd.ms-excel.addin.macroEnabled.12",
    "application/vnd.ms-excel.sheet.binary.macroEnabled.12",

    // PowerPoint
    "application/vnd.openxmlformats-officedocument.presentationml.presentation",
    "application/vnd.openxmlformats-officedocument.presentationml.template",
    "application/vnd.openxmlformats-officedocument.presentationml.slideshow",
    "application/vnd.ms-powerpoint.presentation.macroEnabled.12",
    "application/vnd.ms-powerpoint.template.macroEnabled.12",
    "application/vnd.ms-powerpoint.slideshow.macroEnabled.12",
    "application/vnd.ms-powerpoint.addin.macroEnabled.12",

    // WordPerfect
    "application/vnd.wordperfect",
    "application/wordperfect",
    "application/wordperfect5.1",
    "application/x-wordperfect",
    "application/vnd.wordperfect"
};
bool PPT_LibreOffice_Previewer::previewFile(QFileInfo &info)
{
    if (!preview_file_ptr)
    {
        return false;
    }

    is_math = false;

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(info);
    QString mimeName = mime.name();
    if (!convertibleMimeTypes.contains(mimeName))
    {
        return false;
    }
    if (mimeName == "application/vnd.oasis.opendocument.formula" ||
        mimeName == "application/x-vnd.oasis.opendocument.formula" ||
        mimeName == "application/vnd.oasis.opendocument.formula-template" ||
        mimeName == "application/vnd.sun.xml.math")
    {
        is_math = true;
    }
    clear();
    preview_file_ptr->get_m_textEdit()->setPlainText(tr("加载中..."));
    preview_file_ptr->get_m_textEdit()->show();
    Ppt_To_Pdf(info);
    return true;
}
bool PPT_LibreOffice_Previewer::willThrowFileToNextPreview()
{
    return true;
}
void PPT_LibreOffice_Previewer::Ppt_To_Pdf(QFileInfo &info)
{
    QString generatedPdf = QString("/tmp/Easy_Desktop/Plugin_Tmp_Data/%1.pdf").arg(info.completeBaseName());
    QStringList args;

    if (is_math)
    {
        args << "--headless" << "--print-to-file"
             << "--outdir" << "/tmp/Easy_Desktop/Plugin_Tmp_Data"
             << info.absoluteFilePath();
    }
    else
    {
        args << "--headless" << "--convert-to" << "pdf"
             << "--outdir" << "/tmp/Easy_Desktop/Plugin_Tmp_Data"
             << info.absoluteFilePath();
    }

    QFuture<QPair<bool, QFileInfo>> future = QtConcurrent::run([this, args, generatedPdf]() -> QPair<bool, QFileInfo>
    {
        int exit_code = m_process->execute("libreoffice", args);
        m_process->deleteLater();
        m_process = nullptr;
        if (exit_code != 0)
        {
            return {false, QFileInfo()};
        }

        const QString outputPdfPath = "/tmp/Easy_Desktop/Plugin_Tmp_Data/tmp_ppt_preview.pdf";

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
    });

    watcher->setFuture(future);
}
