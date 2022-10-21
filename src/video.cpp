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



#include "video.h"

video::video() {
    selectedQuality = -1;
    state = state::empty;

    youtubeDl = nullptr;

    targetConverter = nullptr;
    cachedDownloadSize = 0;
    cachedDownloadProgress = 0;
    audioOnly = false;
}

void video::startYoutubeDl(QStringList arguments) {
    if (youtubeDl != nullptr) youtubeDl->deleteLater();

    youtubeDl = YoutubeDl::instance(arguments);
    connect(youtubeDl , QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &video::handleProcessFinished);
    connect(youtubeDl , &QProcess::readyRead, this, &video::handleProcessReadyRead);
    youtubeDl->start();
}

void video::fetchPlaylistInfo(QString url) {
    if (state != state::empty) return;
    state = state::fetching;

    this->url = url;

    QStringList arguments;
    arguments << "-J" << url;
    arguments << "--yes-playlist";
    arguments << "--flat-playlist";
    startYoutubeDl(arguments);
}

void video::fetchInfo(QString url) {
    if (state != state::empty) return;
    state = state::fetching;

    // youtube-dl fails on YouTube links that include ?list parameter
    QUrl parsedUrl = QUrl(url);
    if (parsedUrl.host() == "www.youtube.com" && parsedUrl.path() == "/watch") {
        QString v = QUrlQuery(parsedUrl.query()).queryItemValue("v");
        parsedUrl.setQuery("v=" + v);
        url = parsedUrl.toString();
    } else if (parsedUrl.host() == "youtu.be") {
        QString v = parsedUrl.path();
        parsedUrl.setUrl("https://www.youtube.com/watch?v=" + v);
    }

    this->url = url;

    QStringList arguments;
    arguments << "-J";
    arguments << "--no-playlist";
    arguments << url;

    startYoutubeDl(arguments);
}

void video::download() {
    if ((state != state::fetched && state != state::paused && state != state::canceled) || selectedQuality <= -1) return;
    state = state::downloading;

    videoQuality quality = qualities.at(selectedQuality);
    downloadSize = quality.videoFileSize + quality.audioFileSize;

    QStringList arguments;

    arguments << "--newline";
    arguments << "--no-playlist";
    arguments << "--no-mtime";

    QString fileTemplate = QDir::cleanPath(
                QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
                QDir::separator() +
                "/cg-youtube-dl-%(id)s-%(format_id)s.%(ext)s"
    );
    arguments << "-o" << fileTemplate;

    if (quality.audioFormat.isEmpty()) {
        arguments << "-f" << quality.videoFormat;
    } else if (audioOnly) {
        arguments << "-f" << quality.audioFormat;
    } else {
        arguments << "-f" << quality.videoFormat + "+" + quality.audioFormat;
    }

    arguments << url;

    startYoutubeDl(arguments);
}

void video::cancel() {
    if (state != state::downloading && state != state::paused) return;

    state = state::canceling;
    if (youtubeDl != nullptr) youtubeDl->terminate();
}

void video::pause() {
    if (state!= state::downloading) return;

    state = state::pausing;
    if (youtubeDl != nullptr) youtubeDl->terminate();
}

void video::resume() {
    if (state != state::paused) return;

    download();
}

void video::restart() {
    if (state != state::canceled && state != state::error) return;

    download();
}

void video::fromJson(QByteArray data) {
    if (state != state::empty) return;
    handleInfoJson(data);
}

void video::handleInfoJson(QByteArray data) {
    QJsonObject json = QJsonDocument::fromJson(data).object();

    title = json.value("title").toString();
    id = json.value("id").toString();

    if (json.value("_type").toString() == "playlist") {
        QJsonArray playlist = json.value("entries").toArray();
        for (int i = 0; i < playlist.size(); i++) {
            video* video = new class video;
            video->fromJson(QJsonDocument(playlist.at(i).toObject()).toJson());
            playlistVideos << video;
        }

        state = state::fetched;
        return;
    } else if (json.value("_type").toString() == "url") {
        portal = json.value("ie_key").toString().toLower();
        if (portal == "generic") portal = QUrl(this->url).host();

        url = json.value("url").toString();
        if (QUrl(url).scheme().isEmpty() && portal == "youtube") {
            url = "https://www.youtube.com/watch?v=" + url;
        }

        state = state::unfetched;
        return;
    }

    portal = json.value("extractor").toString().toLower();
    if (portal.isEmpty()) portal = json.value("ie_key").toString().toLower();
    else if (portal == "generic") portal = QUrl(this->url).host();

    url = json.value("webpage_url").toString();
    if (url.isEmpty()) url = json.value("url").toString();
    if (QUrl(url).scheme().isEmpty() && portal == "youtube") {
        url = "https://www.youtube.com/watch?v=" + url;
    }

    artist = json.value("artist").toString();
    if (artist.isEmpty()) artist = json.value("user").toString();
    if (artist.isEmpty()) artist = json.value("uploader").toString();

    duration = json.value("duration").toDouble();

    QJsonArray formats = json.value("formats").toArray();
    QList<QJsonObject> videoFormats;
    QList<QJsonObject> audioFormats;
    QStringList acceptedExts = {"mp4", "m4a"};
    if (QSettings().value("UseWebM", false).toBool()) {
        acceptedExts << "webm" << "opus";
    }
    for (int i = 0; i < formats.size(); i++) {
        QJsonObject format = formats.at(i).toObject();
        if (acceptedExts.contains(format.value("ext").toString())) {
            if (format.value("vcodec").toString() == "none") {
                audioFormats << format;
            } else {
                videoFormats << format;
            }
        }
    }

    // Sort audio formats by bitrate
    std::sort(audioFormats.begin(), audioFormats.end(), [](QJsonObject a, QJsonObject b) {
        double tbrA = a.value("tbr").toDouble();
        double tbrB = b.value("tbr").toDouble();

        return tbrA > tbrB;
    });

    // Sort video formats by format and resolution
    std::sort(videoFormats.begin(), videoFormats.end(), [](QJsonObject a, QJsonObject b) {
        int heightA = a.value("height").toInt();
        int heightB = b.value("height").toInt();
        if (heightA != heightB) return heightA > heightB;

        int fpsA = a.value("fps").toInt();
        int fpsB = b.value("fps").toInt();
        if (fpsA != fpsB) return fpsA > fpsB;

        QString extA = a.value("ext").toString();
        QString extB = b.value("ext").toString();
        if (extA != extB) return extA > extB;

        // This makes sure AVC1 comes before AV01 and AV01 is removed as a duplicate
        QString vcodecA = a.value("vcodec").toString();
        QString vcodecB = b.value("vcodec").toString();
        if (vcodecA != vcodecB) return vcodecA > vcodecB;

        QString acodecA = a.value("acodec").toString();
        QString acodecB = b.value("acodec").toString();
        if (acodecA != acodecB) {
            if (acodecA == "none") return true;
            if (acodecB == "none") return false;
        }
        return false;
    });

    // Remove duplicates
    videoFormats.erase(
        std::unique(videoFormats.begin(), videoFormats.end(), [](QJsonObject a, QJsonObject b) {
            int heightA = a.value("height").toInt();
            int heightB = b.value("height").toInt();
            if (heightA != heightB) return false;

            int fpsA = a.value("fps").toInt();
            int fpsB = b.value("fps").toInt();
            return fpsA == fpsB;
        }),
        videoFormats.end()
    );

    for (int i = 0; i < videoFormats.size(); i ++) {
        QJsonObject videoFormat = videoFormats.at(i);
        int height = videoFormat.value("height").toInt();
        QRegularExpression heightExp("^(\\d+)p");
        QRegularExpressionMatch match = heightExp.match(videoFormat.value("format_note").toString());
        if (match.hasMatch()) height = match.captured(1).toInt();
        int fps = videoFormat.value("fps").toInt();

        QString name = QString::number(height) + "p";
        if (name == "0p") name = tr("unknown");
        else if (fps >= 59) name.append("60");

        if (height >= 4000) {
            name.append(" (8K)");
        } else if (height >= 2000) {
            name.append(" (4K)");
        } else if (height >= 700) {
            name.append(" (HD)");
        }

        if (videoFormat.value("ext").toString() == "webm") {
            name.append(" WebM");
        }

        videoQuality quality(name, videoFormat.value("format_id").toString());
        quality.resolution = height;
        quality.videoFileSize = videoFormat.value("filesize").toInt();
        quality.audioFileSize = 0;
        quality.containerName = videoFormat.value("ext").toString();

        QList<QJsonObject> compatibleAudioFormats(audioFormats);
        QStringList compatibleAudioExts = {"aac", "m4a"};
        if (videoFormat.value("ext") == "webm") {
             compatibleAudioExts.clear();
             compatibleAudioExts << "webm" << "ogg";
        }
        compatibleAudioFormats.erase(std::remove_if(compatibleAudioFormats.begin(), compatibleAudioFormats.end(), [videoFormat, compatibleAudioExts](QJsonObject audioFormat) {
            QString ext = audioFormat.value("ext").toString();

            return !compatibleAudioExts.contains(ext);
        }), compatibleAudioFormats.end());

        if (videoFormat.value("acodec") == "none" && audioFormats.size() > 0) {
            int audioIndex = (compatibleAudioFormats.size() -1) * i / videoFormats.size();
            quality.audioFormat = compatibleAudioFormats.at(audioIndex).value("format_id").toString();
            quality.audioFileSize = compatibleAudioFormats.at(audioIndex).value("filesize").toInt();
        }

        qualities << quality;
    }

    state = state::fetched;
}

void video::handleDownloadInfo(QString line) {
    qDebug() << line;
    QRegularExpression re;
    QRegularExpressionMatch match;

    re.setPattern("^\\[download\\] Destination: (.+)");
    match = re.match(line);
    if (!match.captured(1).isNull()) {
        QString filename = match.captured(1);
        if (!downloadFilenames.contains(filename)) {
            downloadFilenames << match.captured(1);
            downloadSizeEstimates << 0;
        }
        return;
    }

    re.setPattern("^\\[download\\] (.+?) has already been downloaded( and merged)?");
    match = re.match(line);
    if (!match.captured(1).isNull()) {
        finalDownloadFilename = match.captured(1);
        return;
    }
    re.setPattern("^\\[ffmpeg\\] Merging formats into \"([^\"]+)\"");
    match = re.match(line);
    if (!match.captured(1).isNull()) {
        finalDownloadFilename = match.captured(1);
        return;
    }

    re.setPattern("^\\[download\\]\\s+(\\d+\\.\\d)%\\s+of\\s+~?(\\d+\\.\\d+)(T|G|M|K)iB");
    match = re.match(line);
    if (match.hasMatch() && !downloadFilenames.isEmpty()) {
        qint64 downloadProgress = 0;
        for (int i = 0; i < downloadFilenames.size(); i++) {
            downloadProgress += QFileInfo(downloadFilenames.at(i)).size();
            downloadProgress += QFileInfo(downloadFilenames.at(i) + ".part").size();
        }

        if (downloadSize > 0 && downloadProgress > 0) {
            cachedDownloadSize = downloadSize;
            cachedDownloadProgress = downloadProgress;
            emit downloadProgressChanged(downloadSize, downloadProgress);
        } else if (downloadProgress > 0) {
            QStringList prefixes {"K", "M", "G", "T"};
            qint64 downloadSizeEstimate = match.captured(2).toFloat() * pow(1024, 1 + prefixes.indexOf(match.captured(3)));
            downloadSizeEstimates.replace(downloadSizeEstimates.size() - 1, downloadSizeEstimate);

            qint64 totalDownloadSizeEstimate = 0;
            for (int i = 0; i < downloadSizeEstimates.size(); i++) {
                totalDownloadSizeEstimate += downloadSizeEstimates.at(i);
            }

            if (totalDownloadSizeEstimate > 0) {
                cachedDownloadSize = totalDownloadSizeEstimate;
                cachedDownloadProgress = downloadProgress;
                emit downloadProgressChanged(totalDownloadSizeEstimate, downloadProgress);
            }
        }
    }

    re.setPattern("ERROR:\\s+(.*)");
    match = re.match(line);
    if (match.hasMatch()) {
        qDebug() << "ERROR!" << match.captured(1);
        state = state::error;
        emit stateChanged();
        youtubeDl->kill();
    }
}

bool video::setQuality(int index) {
    if (index >= qualities.size()) return false;

    selectedQuality = index;
    return true;
}


void video::setTargetFilename(QString filename) {
    this->targetFilename = filename;
}

QString video::getSafeFilename() {
    return title.replace(QRegularExpression("#|%|&|\\{|\\}|\\\\|<|>|\\*|\\?|/|\\$|!|'|\"|:|@|\\+|`|\\||=|"), "");
}

void video::setConverter(converter* targetConverter, int targetConverterMode) {
    this->targetConverter = targetConverter->createNewInstance();
    this->targetConverterMode = targetConverterMode;
    this->audioOnly = targetConverter->isAudioOnly(targetConverterMode);

    connect(this->targetConverter, &converter::conversionFinished, this, &video::handleConversionFinished);
    connect(this->targetConverter, &converter::error, this, &video::handleConversionError);
}

QString video::getTitle() {
    return title;
}

QString video::getArtist() {
    return artist;
}

void video::setMetaTitle(QString title) {
    metaTitle = title;
}

void video::setMetaArtist(QString artist) {
    metaArtist = artist;
}

QString video::getThumbnail() {
    if (portal == "youtube") {
        return "https://i.ytimg.com/vi/" + id + "/hqdefault.jpg";
    }
    return "";
}

qint64 video::getDuration() {
    return duration;
}

QString video::getUrl() {
    return url;
}

QList<videoQuality> video::getQualities() {
    return qualities;
}

QString video::getSelectedQualityName() {
    if (selectedQuality == -1) return "";

    return qualities.at(selectedQuality).name;
}

QString video::getPortalName() {
    if (portal == "youtube") return "YouTube";
    return portal;
}

qint64 video::getDownloadSize() {
    if (cachedDownloadSize < cachedDownloadProgress) return cachedDownloadProgress;
    return cachedDownloadSize;
}

qint64 video::getDownloadProgress() {
    return cachedDownloadProgress;
}

QString video::getTargetFormatName() {
    if (targetConverter == nullptr) return "";
    return targetConverter->getModes().at(targetConverterMode);
}

QList<video*> video::getPlaylistVideos() {
    return playlistVideos;
}

void video::handleProcessFinished(int /*exitCode*/, QProcess::ExitStatus exitStatus) {
    switch (state) {
    case state::fetching:
        if (exitStatus == QProcess::ExitStatus::NormalExit) {
            handleInfoJson(youtubeDl->readAllStandardOutput());
            youtubeDl->close();

            if (qualities.length() > 0) {
                qDebug() << "Discovered video: " << title;
                state = state::fetched;
            } else {
                state = state::error;
            }
        } else {
            state = state::error;
        }
        emit stateChanged();
        break;
    case state::downloading:
        if (exitStatus == QProcess::ExitStatus::NormalExit) {

            if (finalDownloadFilename.isEmpty() && !downloadFilenames.empty()) finalDownloadFilename = downloadFilenames.last();
            if (finalDownloadFilename.isEmpty()) {
                state = state::error;
                emit stateChanged();
                return;
            }

            state = state::converting;
            QFile* file = new QFile();
            file->setFileName(finalDownloadFilename);
            targetConverter->startConversion(file, targetFilename, qualities.at(selectedQuality).containerName, metaTitle, metaArtist, targetConverterMode);
        } else {
            state = state::error;
        }
        emit stateChanged();
        break;
    case state::pausing:
        state = state::paused;
        emit stateChanged();
        return;
    case state::canceling:
        removeTempFiles();
        state = state::canceled;
        emit stateChanged();
        return;
    default:
        break;
    }
}

void video::handleProcessReadyRead() {
    switch (state) {
    case state::fetching:
       // data is read all at once when process finishes
       break;
     case state::downloading:
        while (youtubeDl->canReadLine()) {
            handleDownloadInfo(QString::fromLocal8Bit(youtubeDl->readLine()));
        }
        break;
     default:
        break;
    }
}

void video::handleConversionFinished() {
    removeTempFiles();
    finalFilename = targetConverter->target;
    state = state::finished;
    emit stateChanged();
}

void video::handleConversionError(QString /*error*/) {
    state = state::error;
    emit stateChanged();
}

void video::removeTempFiles() {
    for (int i = 0; i < downloadFilenames.size(); i++) {
        QFile::remove(downloadFilenames.at(i));
        QFile::remove(downloadFilenames.at(i) + ".part");
    }
    QFile::remove(finalDownloadFilename);
}
