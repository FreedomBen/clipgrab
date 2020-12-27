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



#ifndef VIDEO_H
#define VIDEO_H

#include <QtGui>
#include <QtNetwork>
#include <QtWidgets>
#include <QDebug>
#include "converter.h"
#include "youtube_dl.h"

struct videoQuality
{
    QString name;
    QString videoFormat;
    QString audioFormat;
    QString videoCodec;
    QString audioCodec;
    QString containerName;
    qint64 videoFileSize;
    qint64 audioFileSize;
    int resolution;

    videoQuality(QString name) : name(name) {};
    videoQuality(QString name, QString videoFormat) : name(name), videoFormat(videoFormat) {};

    bool operator<(videoQuality _other) const
    {
        return false;
        // TODO! Implement comparison!
    }
};

class video : public QObject
{
    Q_OBJECT

public:
    video();

    enum class state {empty, error, unfetched, fetching, fetched, downloading, converting, pausing, paused, canceling, canceled, finished};

    operator QString() const { return "<video:" + title + ":" + url + ">"; }

    //*
    //*Video Access
    //*

    void fetchPlaylistInfo(QString url);
    virtual void fetchInfo(QString url);
    virtual void fromJson(QByteArray data);
    virtual void download();
    virtual void pause();
    virtual void resume();
    virtual void cancel();
    virtual void restart();

    virtual QString getPortalName();
    virtual QString getTargetFormatName();
    virtual QString getTitle();
    virtual QString getArtist();
    virtual QString getUrl();
    virtual QString getThumbnail();
    virtual qint64 getDuration();
    QList<videoQuality> getQualities();
    virtual QString getSelectedQualityName();
    virtual QString getSafeFilename();
    QString getTargetFilename() {return targetFilename;};
    QString getFinalFilename() {return finalFilename;};
    virtual state getState() {return state;};
    virtual qint64 getDownloadSize();
    virtual qint64 getDownloadProgress();

    virtual QList<video*> getPlaylistVideos();

    virtual void setMetaTitle(QString title);
    virtual void setMetaArtist(QString artist);
    virtual void setTargetFilename(QString filename);
    virtual void setConverter(converter*, int);
    virtual bool setQuality(int n);

protected:
    QString url;
    QString portal;
    QString id;
    QString title;
    QString artist;
    QString metaTitle;
    QString metaArtist;
    int duration;
    int selectedQuality;
    QList<videoQuality> qualities;

    QStringList downloadFilenames;
    qint64 downloadSize;
    QList<qint64> downloadSizeEstimates;
    qint64 cachedDownloadSize;
    qint64 cachedDownloadProgress;

    QString finalDownloadFilename;
    QProcess* youtubeDl;
    void startYoutubeDl(QStringList);
    video::state state;

    bool audioOnly;
    converter* targetConverter;
    int targetConverterMode;
    QString targetFilename;
    QString finalFilename;

    QList<video*> playlistVideos;

    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessReadyRead();
    void handleInfoJson(QByteArray);
    void handleDownloadInfo(QString line);
    void handleConversionFinished();
    void handleConversionError(QString);
    void removeTempFiles();


signals:
    void infoReady();
    void stateChanged();
    void downloadProgressChanged(qint64, qint64);
};

#endif // ABSTRACT_video_H
