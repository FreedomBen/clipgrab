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



#include "video_collegehumor.h"

video_collegehumor::video_collegehumor()
{
    this->_name = "Collegehumor";
    this->_supportsTitle = true;
    this->_supportsDescription = true;
    this->_supportsThumbnail = true;
    this->_supportsSearch = true;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http://(www\\.)*collegehumor\\.com/video/[0-9]+/.*", Qt::CaseInsensitive);
    _treeItem = NULL;
}

video* video_collegehumor::createNewInstance()
{
    return new video_collegehumor();
}


bool video_collegehumor::setUrl(QString url)
{
    if (_url.isEmpty())
    {
        this->_url = QUrl(url.replace("collegehumor.com/", "collegehumor.com/moogaloop/"));
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


void video_collegehumor::parseVideo(QString xml)
{
    QRegExp expression;
    expression = QRegExp("<caption>(.*)</caption>");
    expression.setMinimal(true);
    if (expression.indexIn(xml) !=-1)
    {
        _title = QString(expression.cap(1));

        expression = QRegExp("<file><\\!\\[CDATA\\[(.*)\\]\\]></file>");
        expression.setMinimal(true);
        if (expression.indexIn(xml)!=-1 && !expression.cap(1).isEmpty())
        {
            videoQuality newQuality;
            newQuality.quality = tr("normal");
            newQuality.videoUrl = expression.cap(1).trimmed();
            newQuality.containerName = newQuality.videoUrl.right(4);
            _supportedQualities.append(newQuality);
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
