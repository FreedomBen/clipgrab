/*
	ClipGrab³
    Copyright (C) Philipp Schmieder
    http://clipgrab.de
    feedback [at] clipgrab [dot] de

    This file is part of ClipGrab.
    ClipGrab is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    ClipGrab is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ClipGrab.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "clipgrab.h"

ClipGrab::ClipGrab()
{
    //*
    //* Set current version
    //*
    version = QCoreApplication::applicationVersion();

    //*
    //* Initialise languages
    //*

    language newLang;
    newLang.name = tr("Automatic language recognition");
    newLang.code = "auto";
    languages.append(newLang);

    newLang.name = QString::fromLocal8Bit("Česky");
    newLang.code = "cs";
    languages.append(newLang);

    newLang.name = "Deutsch";
    newLang.code = "de";
    languages.append(newLang);

    newLang.name = "English";
    newLang.code = "en";
    languages.append(newLang);

    newLang.name = QString::fromLocal8Bit("Español");
    newLang.code = "es";
    languages.append(newLang);


    newLang.name = QString::fromLocal8Bit("Français");
    newLang.code = "fr";
    languages.append(newLang);


    newLang.name = "Italiano";
    newLang.code = "it";
    languages.append(newLang);

    newLang.name = "Polski";
    newLang.code = "pl";
    languages.append(newLang);

    newLang.name = "Romana";
    newLang.code = "ro";
    languages.append(newLang);

    newLang.name = "Suomi";
    newLang.code = "fi";
    languages.append(newLang);

    newLang.name = QString::fromLocal8Bit("Ελληνικά¨");
    newLang.code = "el";
    languages.append(newLang);

    newLang.name = QString::fromLocal8Bit("Русский");
    newLang.code = "ru";
    languages.append(newLang);

    newLang.name = QString::fromLocal8Bit("日本語");
    newLang.code = "ja";
    languages.append(newLang);

    //*
    //* Initialise the supported portals
    //*
    portals.append(new video_youtube);
    portals.append(new video_myvideo);
    portals.append(new video_vimeo);
    portals.append(new video_dailymotion);
    portals.append(new video_myspass);
    portals.append(new video_clipfish);
    portals.append(new video_metacafe);


    //*
    //* Initialise the heuristic
    //*
    this->heuristic = new video_heuristic();

    //*
    //* Initialise the supported converters
    //*
    converters.append(new converter_copy);
    converters.append(new converter_ffmpeg);

    converter* tmpConverter;
    QList<QString> tmpModes;
    format tmpFormat;
    for (int i = 0; i < converters.size(); ++i)
    {
        tmpConverter = converters.at(i);
        connect(tmpConverter, SIGNAL(error(QString)), this, SLOT(errorHandler(QString)));
        if (tmpConverter->isAvailable())
        {
            tmpModes = tmpConverter->getModes();
            for (int i = 0; i < tmpModes.size(); ++i)
            {
                tmpFormat._converter = tmpConverter;
                tmpFormat._mode = i;
                tmpFormat._name = tmpModes.at(i);
                formats.append(tmpFormat);
            }
        }
    }

    activateProxySettings();
    QNetworkAccessManager* obeyatorManager = new QNetworkAccessManager;
    QNetworkRequest obeyatorRequest;
    QString sys = "x11";

    #if defined Q_WS_WIN
        sys = "win";
    #endif
    #if defined Q_WS_MAC
        #ifdef Q_WS_MAC64
            sys = "mac";
        #else
            sys = "mac-legacy";
        #endif
    #endif

    QDateTime startedDateTime = QDateTime::currentDateTime();
    if (settings.value("firstStarted", 0).toInt() == 0)
    {
        settings.setValue("firstStarted", startedDateTime.toTime_t());
    }
    if (settings.value("firstStarted-" + version, 0) == 0)
    {
        settings.setValue("firstStarted-" + version, startedDateTime.toTime_t());
    }
    obeyatorRequest.setUrl("http://clipgrab.de/or.php?version=" + version +"&lang="+QLocale::system().name().split("_")[0]+"&sys="+sys+"&current=" + settings.value("firstStarted-" + version, startedDateTime.toTime_t()).toString()+"&first="+settings.value("firstStarted", startedDateTime.toTime_t()).toString());
    obeyatorManager->get(obeyatorRequest);
    connect(obeyatorManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(obey(QNetworkReply*)));

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));

}

void ClipGrab::obey(QNetworkReply* reply)
{
    if (reply->bytesAvailable())
    {
        QStringList commands = QString(reply->readAll()).split("\n");
        qDebug() << commands;
        for (int i = 0; i < commands.size(); ++i)
        {
            QStringList command = commands.at(i).split("|");
            if (!command.isEmpty())
            {
                if (command.at(0) == "url")
                {
                    if (command.size() == 2 || (command.size() == 3 && settings.value(command.at(2)).toString() != "true"))
                    {
                        if  (!(command.at(1).contains("update")  && this->settings.value("DisableUpdateNotifications", false) == true))
                        {
                            QDesktopServices::openUrl(QUrl(command.at(1)));
                        }
                        if (command.size() == 3)
                        {
                            settings.setValue(command.at(2), "true");
                        }
                    }
                }
                else if (command.at(0) == "set" && command.size() == 3)
                {
                    settings.setValue(command.at(1), command.at(2));
                }
                else if (command.at(0) == "die")
                {
                	QApplication::quit();
                }
            }
        }
    }
    else {qDebug() << "nothing";
    }
}

void ClipGrab::determinePortal(QString url)
{

    if (url.startsWith("http://") || url.startsWith("https://"))
    {
        video* portal = 0;
        bool found = false;
        for (int i = 0; i < portals.size(); ++i)
        {
            if (portals.at(i)->compatibleWithUrl(url))
            {
                found = true;
                portal = portals.at(i);
            }
        }
        emit compatiblePortalFound(found, portal);
    }
    else
    {
        emit compatiblePortalFound(false, 0);
    };
}

void ClipGrab::errorHandler(QString error)
{
    QMessageBox box;
    box.setText(error);
    box.exec();
}

void ClipGrab::errorHandler(QString error, video* /*video*/)
{
    bool solved = false;
    if (!solved)
    {
        QMessageBox box;
        box.setText(error);
        box.exec();
    }

}

void ClipGrab::addDownload(video* clip)
{
    downloads.append(clip);
    clip->download();
}

void ClipGrab::cancelDownload(int item)
{
    if (!downloads.count()-1 < item)
    {
        downloads.at(item)->cancel();
        downloads.at(item)->deleteLater();;
        downloads.removeAt(item);
    }
}

bool ClipGrab::isDownloadFinished(int item)
{
    if (!downloads.count()-1 < item)
    {
        return downloads.at(item)->_finished;
    }
    return "";
}

QString ClipGrab::getDownloadSaveFileName(int item)
{
    if (!downloads.count()-1 < item)
    {
        return downloads.at(item)->getSaveFileName();
    }
    return "";
}

QString ClipGrab::getDownloadTargetPath(int item)
{
    if (!downloads.count()-1 < item)
    {
        return downloads.at(item)->getTargetPath();
    }
    return "";
}


void ClipGrab::clipboardChanged()
{
    if (settings.value("Clipboard", "ask").toString() != "never")
    {
        QString url = QApplication::clipboard()->text();
        if (!url.isEmpty())
        {
            bool found = false;
            for (int i = 0; i < portals.size(); ++i)
            {
                if (portals.at(i)->compatibleWithUrl(url))
                {
                    found = true;
                    break;
                }
            }
            if (found == true)
            {
                clipboardUrl = url;
                emit compatibleUrlFoundInClipboard(url);
            }
        }
    }
}

int ClipGrab::downloadsRunning()
{
    int result = 0;
    for (int i = 0; i < downloads.size(); ++i)
    {
        if (downloads.at(i)->_finished == false)
        {
            result++;
        }
    }
    return result;
}

void ClipGrab::pauseDownload(int item)
{
    if (!downloads.count()-1 < item)
    {
        downloads.at(item)->togglePause();
    }
}

bool ClipGrab::isDownloadPaused(int item)
{
    if (!downloads.count()-1 < item)
    {
        return downloads.at(item)->isDownloadPaused();
    }
    return false;
}

QPair<qint64, qint64> ClipGrab::downloadProgress()
{
    QPair<qint64, qint64> returnValue;
    returnValue.first = 0;
    returnValue.second = 0;
    for (int i = 0; i < downloads.size(); ++i)
    {
        if (downloads.at(i)->_finished == false)
        {
            QPair<qint64, qint64> currentVideoProgress = downloads.at(i)->downloadProgress();
            returnValue.first = returnValue.first + currentVideoProgress.first;
            returnValue.second = returnValue.second + currentVideoProgress.second;
        }
    }
    return returnValue;
}

void ClipGrab::activateProxySettings()
{
    if (settings.value("UseProxy", false).toBool() == true)
    {
        QNetworkProxy proxy;
        if (settings.value("ProxyType", false).toInt() == 0)
        {
            proxy.setType(QNetworkProxy::HttpProxy);
        }
        else
        {
            proxy.setType(QNetworkProxy::Socks5Proxy);
        }
        proxy.setHostName(settings.value("ProxyHost", "").toString());
        proxy.setPort(settings.value("ProxyPort", "").toInt());
        if (settings.value("ProxyAuthenticationRequired", false).toBool() == true)
        {
            proxy.setUser(settings.value("ProxyUsername", "").toString());
            proxy.setPassword(settings.value("ProxyPassword").toString());
        }
        QNetworkProxy::setApplicationProxy(proxy);
    }
    else
    {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }

}
