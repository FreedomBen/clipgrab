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



#include "video_clipfish.h"

video_clipfish::video_clipfish()
{
    this->_name = "Clipfish";
    this->_supportsTitle = true;
    this->_supportsDescription = false;
    this->_supportsThumbnail = true;
    this->_supportsSearch = false;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http://(www\\.)*clipfish\\.de/.*video/[0-9]+/", Qt::CaseInsensitive);
    _treeItem = NULL;
}

video* video_clipfish::createNewInstance()
{
    return new video_clipfish();
}


bool video_clipfish::setUrl(QString url)
{
    if (_url.isEmpty())
    {
        QRegExp expression("video/([0-9]+)/", Qt::CaseInsensitive);
        if (expression.indexIn(url))
        {
            this->_url = QUrl("http://www.clipfish.de/devxml/videoinfo/" + expression.cap(1));
        }

        if (_url.isValid())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


void video_clipfish::parseVideo(QString xml)
{
    QRegExp expression;
    expression = QRegExp("<title><\\!\\[CDATA\\[(.*)\\]\\]></title>");
    expression.setMinimal(true);
    if (expression.indexIn(xml) !=-1)
    {
        _title = QString(expression.cap(1));

        expression = QRegExp("<filename><\\!\\[CDATA\\[(.*)\\]\\]></filename>");
        expression.setMinimal(true);
        if (expression.indexIn(xml)!=-1)
        {

            videoQuality newQuality;
            newQuality.quality = tr("normal");
            newQuality.videoUrl = expression.cap(1);
            newQuality.containerName = "." + newQuality.videoUrl.split(".").last();
            _supportedQualities.append(newQuality);

            expression = QRegExp("<imageurl>(.*)</imageurl>");
            if (expression.indexIn(xml)!=-1)
            {
                _urlThumbnail = QUrl(expression.cap(1));
            }
        }

        else
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
