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



#include "video_myvideo.h"

video_myvideo::video_myvideo()
{
    this->_name = "MyVideo";
    this->_supportsTitle = true;
    this->_supportsDescription = true;
    this->_supportsThumbnail = true;
    this->_supportsSearch = true;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http://www\\.myvideo\\.de/watch/") << QRegExp("http://www\\.myvideo\\.ch/watch/") << QRegExp("http://www\\.myvideo\\.at/watch/");
}

video* video_myvideo::createNewInstance()
{
    return new video_myvideo();
}

void video_myvideo::parseVideo(QString html)
{
    QString newUrl;
    QRegExp expression;
    expression = QRegExp("<h1>(.*)</h1>");    expression.setMinimal(true);
    if (expression.indexIn(html) !=-1)
    {
        _title = QString(expression.cap(1)).replace(" - MyVideo", "");

        expression = QRegExp("<link rel='image_src' href='(.*)\\/thumbs\\/(.*)_");
        expression.setMinimal(true);
        if (expression.indexIn(html)!=-1)
        {
            newUrl.append(expression.cap(1)).append("/").append(expression.cap(2)).append(".flv");
            expression = QRegExp("\"t\":\\s*\"(.*)\"");
            expression.setMinimal(true);
            videoQuality newQuality;
            newQuality.quality = tr("normal");
            newQuality.videoUrl = newUrl.toAscii();
            newQuality.containerName = ".flv";
            _supportedQualities.append(newQuality);
            emit analysingFinished();
        }
        else
        {
            emit error("Could not retrieve video link.", this);
            emit analysingFinished();
        }
    }
    else
    {
        emit error("Could not retrieve video title.", this);
        emit analysingFinished();
    }
}
