#include "uploadthread.h"
#include "global.h"

#include <QDir>
#include <QFile>
#include <QDate>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QNetworkReply>

#ifdef USE_FSTREAM
#include <fstream>
using std::fstream;
using std::string;
using std::ios_base;
#endif


NBFtpUpload::NBFtpUpload(QObject *parent) : QObject(parent)
{
    m_man = new QNetworkAccessManager(this);
    QString filename = QDir(GlobalConfig::ProjectPath).dirName() + "-" + QDate::currentDate().toString("yyyyMMdd") +
#if defined(Q_OS_WIN)
        ".7z"
#elif defined(Q_OS_LINUX)
#if defined(LINUX_X86)
        "-x86"
#elif defined(LINUX_X64)
        "-x64"
#else
        "-xxx"
#endif
        + ".tar.gz"
#endif
    ;
    QString uploadUrl = GlobalConfig::FtpPath;
    if (!uploadUrl.endsWith("/"))
        uploadUrl.append("/");

    uploadUrl.append(filename);

    m_url = new QUrl(uploadUrl);
    m_url->setUserName(GlobalConfig::FtpUserName);
    m_url->setPassword(GlobalConfig::FtpPassword);

    m_filePath = QDir(GlobalConfig::DeployPath).absoluteFilePath(filename);
}

NBFtpUpload::~NBFtpUpload()
{
    delete m_url;
}

void NBFtpUpload::start()
{
#ifndef USE_FSTREAM
    QFile *f = new QFile(m_filePath, this);
    f->open(QIODevice::ReadOnly);
#else
    QFileInfo file(m_filePath);
    string path = QDir::toNativeSeparators(file.canonicalFilePath()).toStdString();
    fstream fs;
    fs.open(path, ios_base::in | ios_base::binary);

    fs.seekg(0, ios_base::end);
    fstream::pos_type size = fs.tellg();
    fs.seekg(0, ios_base::beg);

    QByteArray f(size, 0);
    fs.read(f.data(), size);
#endif
    QNetworkReply *r = m_man->put(QNetworkRequest(*m_url), f);
    connect(r, &QNetworkReply::finished, this, &NBFtpUpload::finished);

}

NBUploadThread::NBUploadThread()
{

}

NBUploadThread::~NBUploadThread()
{

}

void NBUploadThread::run()
{
    if (!GlobalConfig::FtpPath.toLower().startsWith("ftp://")) {
        QDir ftpDir(GlobalConfig::FtpPath);
        QList<QFileInfo> f = ftpDir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);
        if (f.length() > 14)
            QFile::remove(f.first().absoluteFilePath());

        QString fileName = QDir(GlobalConfig::ProjectPath).dirName() + "-" + QDate::currentDate().toString("yyyyMMdd") +
#if defined(Q_OS_WIN)
            ".7z"
#elif defined(Q_OS_LINUX)
#if defined(LINUX_X86)
            "-x86"
#elif defined(LINUX_X64)
            "-x64"
#else
            "-xxx"
#endif
            + ".tar.gz"
#endif
        ;

        QDir dplyDir(GlobalConfig::DeployPath);

        QFile::copy(dplyDir.absoluteFilePath(fileName), ftpDir.absoluteFilePath(fileName));
    } else {
        NBFtpUpload *upload = new NBFtpUpload;
        connect(upload, &NBFtpUpload::finished, this, &NBUploadThread::uploadFinished);
        connect(upload, &NBFtpUpload::finished, upload, &NBFtpUpload::deleteLater);
        upload->start();

        exec();
    }
}

void NBUploadThread::uploadFinished()
{
    delete sender();
    exit(0);
}
