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



#include "video_vimeo.h"

video_vimeo::video_vimeo()
{
    this->_name = "Vimeo";
    this->_supportsTitle = true;
    this->_supportsDescription = true;
    this->_supportsThumbnail = true;
    this->_supportsSearch = true;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http[s]?://(www\\.)*vimeo\\.com/[0-9]+", Qt::CaseInsensitive);
    _treeItem = NULL;
}

video* video_vimeo::createNewInstance()
{
    return new video_vimeo();
}


bool video_vimeo::setUrl(QString url)
{
    if (_url.isEmpty())
    {
        this->_url = QUrl(url.replace("vimeo.com/", "player.vimeo.com/config/"));
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


void video_vimeo::parseVideo(QString xml)
{
    QRegExp expression;
    expression = QRegExp("\"title\":\"(.*)\",");
    expression.setMinimal(true);
    if (expression.indexIn(xml) !=-1)
    {
        _title = QString(expression.cap(1));
        QRegExp expression2;
        QRegExp expression3;

        expression = QRegExp("\"id\":(.*),");
        expression.setMinimal(true);
        expression2 = QRegExp("\"signature\":\"(.*)\",");
        expression2.setMinimal(true);
        expression3 = QRegExp("\"timestamp\":(.*),");
        expression3.setMinimal(true);
        if (expression.indexIn(xml)!=-1 && expression.cap(1) != "" && expression2.indexIn(xml)!=-1 && expression2.cap(1) != ""&& expression3.indexIn(xml)!=-1 && expression3.cap(1) != "")
        {
            QRegExp expression4;
            expression4 = QRegExp("\"hd\":(1),");
            expression4.setMinimal(true);
            expression4.indexIn(xml);
            if (expression4.indexIn(xml)!=-1 && expression4.cap(1).trimmed() == "1" )
            {
                videoQuality newQuality;
                newQuality.quality = tr("HD");
                newQuality.videoUrl = "http://player.vimeo.com/play_redirect?clip_id=" + expression.cap(1) + "&sig=" + expression2.cap(1) + "&time=" + expression3.cap(1)+"&quality=hd";
                newQuality.containerName = ".mp4";
                _supportedQualities.append(newQuality);
            }

            videoQuality newQuality;
            newQuality.quality = tr("high");
            newQuality.videoUrl = "http://player.vimeo.com/play_redirect?clip_id=" + expression.cap(1) + "&sig=" + expression2.cap(1) + "&time=" + expression3.cap(1);
            newQuality.containerName = ".mp4";
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
