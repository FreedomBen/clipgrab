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



#include "video_myspass.h"

video_myspass::video_myspass()
{
    this->_name = "Myspass";
    this->_supportsTitle = true;
    this->_supportsDescription = true;
    this->_supportsThumbnail = true;
    this->_supportsSearch = true;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http://(www\\.)*myspass\\.de/myspass/.*--/[0-9]", Qt::CaseInsensitive);
    _treeItem = NULL;
}

video* video_myspass::createNewInstance()
{
    return new video_myspass();
}


bool video_myspass::setUrl(QString url)
{
    if (_url.isEmpty())
    {
        QRegExp expression("myspass\\.de/myspass/.*--/([0-9]+)", Qt::CaseInsensitive);
        if (expression.indexIn(url))
        {
            this->_url = QUrl("http://www.myspass.de/myspass/includes/apps/video/getvideometadataxml.php?id=" + expression.cap(1));
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


void video_myspass::parseVideo(QString xml)
{
    QRegExp expression;
    expression = QRegExp("<title><\\!\\[CDATA\\[(.*)\\]\\]></title>");
    expression.setMinimal(true);
    if (expression.indexIn(xml) !=-1)
    {
        _title = QString(expression.cap(1));

        expression = QRegExp("<url_flv><\\!\\[CDATA\\[(.*)\\]\\]></url_flv>");
        expression.setMinimal(true);
        if (expression.indexIn(xml)!=-1)
        {

            videoQuality newQuality;
            newQuality.quality = tr("high");
            newQuality.videoUrl = expression.cap(1);
            newQuality.containerName = ".flv";
            _supportedQualities.append(newQuality);

            expression = QRegExp("<imagePreview><\\!\\[CDATA\\[(.*)\\]\\]></imagePreview>");
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
