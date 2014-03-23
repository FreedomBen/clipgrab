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



#include "converter_ffmpeg.h"

void ffmpegThread::run()
{
    QSettings settings;
    QString videoCodec;
    QString videoBitrate;
    QString audioCodec;

    QObject* parent = new QObject;

    bool audioCodecAccepted = false;
    bool videoCodecAccepted = false;

    ffmpegCall = settings.value("ffmpegPath", "ffmpeg").toString() + " -y";

    if (!concatFiles.empty())
    {
        for (int i=0; i < concatFiles.size(); i++)
        {
            ffmpegCall.append(" -i \"" + concatFiles.at(i)->fileName() + "\"" );
        }
        ffmpegCall.append(" -acodec copy -vcodec copy -f mp4 \"" + concatTarget->fileName() + "\"");
    }
    else
    {
        ffmpegCall.append(" -i \"" + inputFile->fileName() + "\"" );


        ffmpeg = new QProcess(parent);
        ffmpeg->start(ffmpegCall);
        ffmpeg->waitForFinished(-1);
        QString videoInfo = ffmpeg->readAllStandardError();
        ffmpeg->close();

        QRegExp expression;
        expression = QRegExp("Audio: (.*),");
        expression.setMinimal(true);
        if (expression.indexIn(videoInfo) !=-1)
        {
            audioCodec = expression.cap(1);
        }
        expression = QRegExp("Video: (.*),");
        expression.setMinimal(true);
        if (expression.indexIn(videoInfo) !=-1)
        {
            videoCodec = expression.cap(1);
        }

        expression = QRegExp("Video:.*([0-9]+) kb/s");
        expression.setMinimal(true);
        if (expression.indexIn(videoInfo) !=-1)
        {
            videoBitrate = expression.cap(1);
        }

        qDebug() << "Source video: " << videoCodec << videoBitrate << audioCodec;
        qDebug() << "Target video: " << acceptedVideoCodec << acceptedAudioCodec;

        for (int i = 0; i < acceptedAudioCodec.size(); ++i)
        {
            if (audioCodec.contains(acceptedAudioCodec[i]))
            {
                audioCodecAccepted = true;
            }
        }

        for (int i = 0; i < acceptedVideoCodec.size(); ++i)
        {
            if (videoCodec.contains(acceptedVideoCodec[i]))
            {
                videoCodecAccepted = true;
            }
        }

        if (acceptedAudioCodec[0] == "none")
        {
            ffmpegCall = ffmpegCall + " -an";
        }
        else
        {
            if (audioCodecAccepted == true)
            {
                ffmpegCall = ffmpegCall + " -acodec copy";
            }
            else
            {
                if (acceptedAudioCodec[0] == "libvorbis")
                {
                    if (videoInfo.contains("libvorbis"))
                    {
                        ffmpegCall = ffmpegCall + " -acodec libvorbis -aq 9";
                    }
                    else
                    {
                        ffmpegCall = ffmpegCall + " -acodec vorbis -aq 9 -strict experimental";
                    }
                }
                else
                {
                    ffmpegCall = ffmpegCall + " -acodec " + acceptedAudioCodec[0] + " -ab 256k";
                }
            }
        }

        if (acceptedVideoCodec[0] == "none")
        {
            ffmpegCall = ffmpegCall + " -vn";
        }
        else
        {
            if (videoCodecAccepted == true)
            {
                ffmpegCall = ffmpegCall + " -vcodec copy";
            }
            else
            {
                if (videoBitrate.toInt() > 100)
                {
                    ffmpegCall = ffmpegCall + " -vb " + QString::number(videoBitrate.toInt()*1.2) + "k" + " -vcodec " + acceptedVideoCodec[0];
                }
                else
                {
                    ffmpegCall = ffmpegCall + " -vcodec " + acceptedVideoCodec[0];
                }
            }
        }

        ffmpegCall = ffmpegCall + " -metadata title=\"" + metaTitle + "\"";
        ffmpegCall = ffmpegCall + " -metadata author=\"" + metaArtist + "\"";
        ffmpegCall = ffmpegCall + " -metadata artist=\"" + metaArtist + "\"";

        ffmpegCall = ffmpegCall + " \"" + target + "\"";
    }

    qDebug() << "Executing ffmpeg: " << ffmpegCall;

    ffmpeg = new QProcess(parent);
    ffmpeg->start(ffmpegCall);
    ffmpeg->waitForFinished(-1);
    qDebug() << ffmpeg->readAllStandardError();
    qDebug() << ffmpeg->readAllStandardOutput();
    ffmpeg->close();

    // patch added for Qt Version 4.5.0 by Günther Bauer
    #if QT_VERSION == 0x040500
        QFile::remove(randomFile);
    #endif
    // end of patch
};

converter_ffmpeg::converter_ffmpeg()
{
    QSettings settings;
    this->_modes.append(tr("MPEG4"));
    this->_modes.append(tr("WMV (Windows)"));
    this->_modes.append(tr("OGG Theora"));
    this->_modes.append(tr("MP3 (audio only)"));
    this->_modes.append(tr("OGG Vorbis (audio only)"));
}


QString converter_ffmpeg::getExtensionForMode(int mode)
{
    switch (mode)
    {
        case 0:
            return "mp4";
            break;
        case 1:
            return "wmv";
            break;
        case 2:
            return "ogg";
            break;
        case 3:
            return "mp3";
            break;
        case 4:
            return "ogg";
            break;
    }
    return "";
}



void converter_ffmpeg::startConversion(QFile* inputFile, QString& target, QString /*originalExtension*/, QString metaTitle, QString metaArtist, int mode)
{
    QStringList acceptedAudio;
    QStringList acceptedVideo;
    QString container;
    QDir fileCheck;

    switch (mode)
    {
    case 0:
        acceptedAudio <<  "libvo_aacenc" << "aac" << "mp3";
        acceptedVideo << "mpeg4" << "h264";
        container = "mp4";
        break;
    case 1:
        acceptedAudio << "wmav2";
        acceptedVideo << "wmv2";
        container = "wmv";
        break;
    case 2:
        acceptedAudio << "libvorbis" << "vorbis";
        acceptedVideo << "libtheora" << "theora";
        container = "ogv";
        break;
    case 3:
        acceptedAudio << "libmp3lame" << "mp3";
        acceptedVideo << "none";
        container = "mp3";
        break;
    case 4:
        acceptedAudio << "libvorbis" << "vorbis";
        acceptedVideo << "none";
        container = "ogg";
        break;
    }


    if (fileCheck.exists(target + "." + container))
    {
        int i = 1;
        while (fileCheck.exists(target + "-" + QString::number(i) + container))
        {
            i++;
        }
        target.append("-");
        target.append(QString::number(i));
    }

    target.append("." + container);

    ffmpeg.inputFile = inputFile;
    ffmpeg.acceptedAudioCodec = acceptedAudio;
    ffmpeg.acceptedVideoCodec = acceptedVideo;
    ffmpeg.metaTitle = metaTitle;
    ffmpeg.metaArtist = metaArtist;
    ffmpeg.target = target;
    connect(&ffmpeg, SIGNAL(finished()), this, SLOT(emitFinished()));
    ffmpeg.start();

}

void converter_ffmpeg::concatenate(QList<QFile *> files, QFile *target)
{
    ffmpeg.concatFiles = files;
    ffmpeg.concatTarget = target;
    connect(&ffmpeg, SIGNAL(finished()), this, SLOT(emitFinished()));
    ffmpeg.start();

}

converter* converter_ffmpeg::createNewInstance()
{
    return new converter_ffmpeg();
}

void converter_ffmpeg::emitFinished()
{
    emit conversionFinished();
}

bool converter_ffmpeg::isAvailable()
{
    QSettings settings;

    QString ffmpegPath;
    QProcess testProcess;

    #ifdef Q_WS_MAC
        ffmpegPath =  "\"" + QApplication::applicationDirPath() + "/ffmpeg\"";
    #else

        testProcess.start("avconv -v quiet");
        if (testProcess.waitForFinished())
        {
            ffmpegPath = "avconv";
        }
        else
        {
            testProcess.start("ffmpeg -v quiet");
            if (testProcess.waitForFinished())
            {
                ffmpegPath = "ffmpeg";
            }
            else
            {
                emit error(tr("No installed version of avconv or ffmpeg coud be found. Converting files and downloading 1080p videos from YouTube is not supported."));
                settings.setValue("ffmpegPath", "");
                return false;
            }
        }
    #endif

    testProcess.start(ffmpegPath + " -formats");
    testProcess.waitForFinished();
    QString supportedFormats = testProcess.readAllStandardOutput();

    settings.setValue("ffmpegPath", ffmpegPath);
    settings.setValue("DashSupported", (bool) supportedFormats.contains("hls"));
    if (settings.value("DashSupported", false) == false)
    {
        emit error(tr("The installed version of %1 is outdated.\nDownloading 1080p videos from YouTube is not supported.").arg(ffmpegPath));
    }

    return true;

}
