#ifndef YOUTUBEDL_H
#define YOUTUBEDL_H

#include <QtCore>
#include <QDebug>

class YoutubeDl
{
public:
    YoutubeDl();

    static const char * executable;
    static const char * download_url;
    static const char * homepage_url;
    static const char * homepage_short;

    static QProcess* instance(QStringList arguments);
    static QProcess* instance(QString path, QStringList arguments);
    static QString getVersion();
    static QString getPythonVersion();
    static QString find(bool force = false);
    static QString findPython();

    static QString path;
};

#endif // YOUTUBEDL_H
