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



#include "video_metacafe.h"

video_metacafe::video_metacafe()
{
    this->_name = "Metacafe";
    this->_supportsTitle = true;
    this->_supportsDescription = false;
    this->_supportsThumbnail = false;
    this->_supportsSearch = false;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http[s]?://\\w*\\.metacafe\\.com/watch/[0-9]+/.*", Qt::CaseInsensitive);
}

video* video_metacafe::createNewInstance()
{
    return new video_metacafe();
}


void video_metacafe::parseVideo(QString html)
{

    QRegExp expression;

    expression = QRegExp("<meta property=\"og:title\" content=\"(.*)\"");
    expression.setMinimal(true);
    if (expression.indexIn(html) !=-1)
    {
        _title = QString(expression.cap(1)).replace("&amp;quot;", "\"").replace("&amp;amp;", "&").replace("&#39;", "'").replace("&quot;", "\"");
        expression = QRegExp("name=\"flashvars\" value=\"(.*)\"");
        expression.setMinimal(true);
        if (expression.indexIn(html)!=-1 && expression.cap(1) != "")
        {
            html = QUrl::fromEncoded(expression.cap(1).toAscii()).toString();

            expression = QRegExp("\"highDefinitionMP4\":\\{.*mediaURL\":\"(.*)\"");
            expression.setMinimal(true);

            if (expression.indexIn(html) != -1 && expression.cap(1) != "")
            {
                videoQuality newQuality;
                newQuality.quality = "HD (720p)";
                newQuality.containerName = ".mp4";
                newQuality.videoUrl = expression.cap(1).remove("\\").remove("%5BFrom%20www.metacafe.com%5D%20");
                newQuality.chunkedDownload = newQuality.videoUrl.contains("factor");
                _supportedQualities.append(newQuality);
            }

            expression = QRegExp("\\{\"MP4\":\\{.*mediaURL\":\"(.*)\"");
            expression.setMinimal(true);

            if (expression.indexIn(html) != -1 && expression.cap(1) != "")
            {
                videoQuality newQuality;
                newQuality.quality = "360p";
                newQuality.containerName = ".mp4";
                newQuality.videoUrl = expression.cap(1).remove("\\").remove("%5BFrom%20www.metacafe.com%5D%20");
                newQuality.chunkedDownload = newQuality.videoUrl.contains("factor");
                _supportedQualities.append(newQuality);
            }

        }

        if (_supportedQualities.isEmpty())
        {
            emit error("Could not retrieve video link.", this);
        }

    }
    else
    {
        emit error("Could not retrieve video title.", this);
    }


    emit analysingFinished();
}
