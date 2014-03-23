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



#include "video_dailymotion.h"

video_dailymotion::video_dailymotion()
{
    this->_name = "Dailymotion";
    this->_supportsTitle = true;
    this->_supportsDescription = true;
    this->_supportsThumbnail = true;
    this->_supportsSearch = true;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http://\\w*\\.dailymotion\\.com/video/.*", Qt::CaseInsensitive);
    QNetworkCookieJar* cookieJar = new QNetworkCookieJar;
    QList<QNetworkCookie> cookieList;
    cookieList << QNetworkCookie("family_filter", "off");
    cookieJar->setCookiesFromUrl(cookieList, QUrl("http://www.dailymotion.com"));
    this->handler->networkAccessManager->setCookieJar(cookieJar);

}

video* video_dailymotion::createNewInstance()
{
    return new video_dailymotion();
}

bool video_dailymotion::setUrl(QString url)
{
    if (_url.isEmpty())
    {
        this->_url = QUrl(url.replace("dailymotion.com/", "dailymotion.com/embed/"));
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

void video_dailymotion::parseVideo(QString html)
{
    QRegExp expression;
    expression = QRegExp("<title>(.*)</title>");
    expression.setMinimal(true);
    if (expression.indexIn(html) !=-1)
    {

        _title = QString(expression.cap(1));

        html = QUrl::fromEncoded(html.toAscii()).toString(QUrl::None);
        expression = QRegExp("\"stream_h264_hd1080_url\":\"(.*)\"");
        expression.setMinimal(true);
        if (expression.indexIn(html)!=-1)
        {
            videoQuality newQuality;
            newQuality.quality = tr("HD (1080p)");
            newQuality.videoUrl = expression.cap(1).replace("\\", "");
            newQuality.containerName = ".mp4";
            _supportedQualities.append(newQuality);
         }
        expression = QRegExp("\"stream_h264_hd_url\":\"(http.*)\"");
        expression.setMinimal(true);
        if (expression.indexIn(html)!=-1)
        {
            videoQuality newQuality;
            newQuality.quality = tr("HD (720p)");
            newQuality.videoUrl = expression.cap(1).replace("\\", "");
            newQuality.containerName = ".mp4";
            _supportedQualities.append(newQuality);
         }
         expression = QRegExp("\"stream_h264_hq_url\":\"(http.*)\"");
         expression.setMinimal(true);
         if (expression.indexIn(html)!=-1)
         {
             videoQuality newQuality;
             newQuality.quality = tr("high (480p)");
             newQuality.videoUrl = expression.cap(1).replace("\\", "");
             newQuality.containerName = ".mp4";
             _supportedQualities.append(newQuality);
         }
         expression = QRegExp("\"stream_h264_ld_url\":\"(http.*)\"");
         expression.setMinimal(true);
         if (expression.indexIn(html)!=-1)
         {
             videoQuality newQuality;
             newQuality.quality = tr("low (240p)");
             newQuality.videoUrl = expression.cap(1).replace("\\", "");
             newQuality.containerName = ".mp4";
             _supportedQualities.append(newQuality);
         }

    }

    if (_supportedQualities.isEmpty() | _title.isEmpty())
    {
        emit error("Could not retrieve video title.", this);

    }
    emit analysingFinished();
 }
