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



#include "video_sevenload.h"

video_sevenload::video_sevenload()
{
    this->_name = "sevenload";
    this->_supportsTitle = true;
    this->_supportsDescription = true;
    this->_supportsThumbnail = true;
    this->_supportsSearch = true;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http://.*sevenload\\.com/.*/([^-]*)-.*$", Qt::CaseInsensitive);
    _treeItem = NULL;
}

video* video_sevenload::createNewInstance()
{
    return new video_sevenload();
}


bool video_sevenload::setUrl(QString url)
{
    if (_url.isEmpty())
    {
        QRegExp expression("http://.*sevenload\\.com/.*/([^-/]*)-[^/]*$", Qt::CaseInsensitive);
        expression.indexIn(url);
        if (expression.indexIn(url) != -1)
        {
            this->_url = QUrl("http://flash.sevenload.com/player?itemId=" + expression.cap(1));
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


void video_sevenload::parseVideo(QString xml)
{
    QRegExp expression;
    expression = QRegExp("<items>.*<title>(.*)</title>");
    expression.setMinimal(true);
    expression.indexIn(xml);
    if (expression.indexIn(xml) !=-1)
    {
        _title = QString(expression.cap(1));

        expression = QRegExp("<stream quality=\"normal\".*(http://.*\\.flv)");
        expression.setMinimal(true);
        if (expression.indexIn(xml)!=-1)
        {
            videoQuality newQuality;
            newQuality.quality = tr("normal");
            newQuality.videoUrl = expression.cap(1);
            newQuality.containerName = ".flv";
            _supportedQualities.append(newQuality);
        }
        expression = QRegExp("<stream quality=\"high\".*(http://.*\\.mp4)");
        expression.setMinimal(true);
        if (expression.indexIn(xml)!=-1)
        {
            videoQuality newQuality;
            newQuality.quality = tr("HD");
            newQuality.videoUrl = expression.cap(1);
            newQuality.containerName = ".mp4";
            _supportedQualities.append(newQuality);
        }

        expression = QRegExp("<images>.*(http://.*\\.mp3)");
        if (expression.indexIn(xml)!=-1)
        {
            _urlThumbnail = QUrl(expression.cap(1));
        }

        if (_supportedQualities.isEmpty())
        {
            emit error(tr("This video appears to be protected and cannot be downloaded."), this);
            _title = "";
        }
    }
    else
    {
        emit error("Could not retrieve video title.", this);
    }
    emit analysingFinished();
}
