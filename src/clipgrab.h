/*
    ClipGrab³
    Copyright (C) The ClipGrab Project
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



#ifndef CLIPGRAB_H
#define CLIPGRAB_H

#include <QApplication>
#include <QtGui>
#include <QtNetwork>
#include <QtXml>
#include <QtDebug>
#include <QtWidgets>

#include "video.h"
#include "converter.h"
#include "converter_copy.h"
#include "converter_ffmpeg.h"

#include "ui_update_message.h"
#include "ui_helper_downloader.h"
#include "message_dialog.h"

struct format
{
    converter* _converter;
    QString _name;
    int _mode;
    bool _audioOnly;
};

struct language
{
    language(): isRTL(false) {}

    QString name;
    QString code;
    bool isRTL;
};

struct updateInfo
{
    QString version;
    QString url;
    QString sha1;
    QDomNodeList domNodes;

    friend bool operator>(const updateInfo &a, const updateInfo &b)
    {
        return compare(a, b) > 0;
    }

    friend bool operator<(const updateInfo &a, const updateInfo &b)
    {
        return compare(a, b) < 0;
    }

    friend bool operator==(const updateInfo &a, const updateInfo &b)
    {
        return compare(a, b) == 0;
    }

    static int compare(const updateInfo &a, const updateInfo&b)
    {
        QRegExp regexp("^(\\d+)\\.(\\d+)\\.(\\d+)(-.*)?$");
        if (regexp.indexIn(a.version) > -1 && regexp.indexIn((b.version)) > -1)
        {
            regexp.indexIn(a.version);
            int majorA = regexp.cap(1).toInt();
            int minorA = regexp.cap(2).toInt();
            int patchA = regexp.cap(3).toInt();
            QString restA = regexp.cap(4);

            regexp.indexIn(b.version);
            int majorB = regexp.cap(1).toInt();
            int minorB = regexp.cap(2).toInt();
            int patchB = regexp.cap(3).toInt();
            QString restB = regexp.cap(4);

            if (majorA > majorB) return 1;
            if (majorA < majorB) return -1;
            if (minorA > minorB) return 1;
            if (minorA < minorB) return -1;
            if (patchA > patchB) return 1;
            if (patchA < patchB) return -1;
            if (restA.isEmpty() && !restB.isEmpty()) return 1;
            if (!restA.isEmpty() && restB.isEmpty()) return -1;
            return restA.compare(restB);
        }
        return b.version.compare(a.version);
    }
};


class ClipGrab : public QObject
{
    Q_OBJECT

    public:
    ClipGrab();

    QList<format> formats;
    QList<video*> downloads;
    QList<converter*> converters;
    QSettings settings;
    QSystemTrayIcon trayIcon;
    bool isKnownVideoUrl(QString url);
    QClipboard* clipboard;
    QString clipboardUrl;
    QString version;
    void openTargetFolder(video*);
    void openDownload(video*);

    QList<language> languages;

    video* getCurrentVideo();
    int getRunningDownloadsCount();
    QPair<qint64, qint64> getDownloadProgress();

    void getUpdateInfo();

    void downloadYoutubeDl(bool force = false);
    void updateYoutubeDl();
    QProcess* runYouTubeDl(QStringList);
    void startYoutubeDlDownload();

    void clearTempFiles();

    // Helpers
    QString humanizeBytes(qint64);
    QString humanizeSeconds(qint64);

    protected:
        QDialog* updateMessageDialog;
        Ui::UpdateMessage* updateMessageUi;
        QList<updateInfo> availableUpdates;
        QTemporaryFile* updateFile;
        QDialog* helperDownloaderDialog;
        Ui::HelperDownloader* helperDownloaderUi;
        QFile* youtubeDlFile;
        QNetworkReply* updateReply;
        video* currentVideo;
        video* currentSearch;
        QString youtubeDlPath;
        QProcess *youtubeDlUpdateProcess;

    public slots:
        void errorHandler(QString);
        void errorHandler(QString, video*);
        void parseUpdateInfo(QNetworkReply* reply);

        void fetchVideoInfo(const QString & url);
        void clearCurrentVideo();
        void enqueueDownload(video* video);
        void cancelAllDownloads();
        void search(QString keywords = "");
        void clipboardChanged();
        void activateProxySettings();

        void startUpdateDownload();
        void skipUpdate();
        void updateDownloadFinished();
        void updateDownloadProgress(qint64, qint64);
        void updateReadyRead();

    signals:
        void currentVideoStateChanged(video*);
        void downloadEnqueued();
        void downloadFinished(video*);
        void searchFinished(video*);
        void youtubeDlDownloadFinished();
        void compatibleUrlFoundInClipboard(QString url);
        void allDownloadsCanceled();
        void updateInfoProcessed();
};

#endif // CLIPGRAB_H
