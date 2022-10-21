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



#include "video_heuristic.h"

video_heuristic::video_heuristic()
{
    this->_name = "Heuristic";
    this->_supportsTitle = true;
    this->_supportsDescription = false;
    this->_supportsThumbnail = false;
    this->_supportsSearch = false;
    this->_icon = 0;

    QNetworkCookieJar* cookieJar = new QNetworkCookieJar;
    QList<QNetworkCookie> cookieList;
    cookieList << QNetworkCookie("age_check", "1");
    cookieList << QNetworkCookie("age_verified", "1");
    cookieJar->setCookiesFromUrl(cookieList, _url);
    handler->networkAccessManager->setCookieJar(cookieJar);
}

video* video_heuristic::createNewInstance()
{
    return new video_heuristic();
}

void video_heuristic::parseVideo(QString html)
{
    QRegExp expression;
    expression = QRegExp("<title>(.*)</title>");
    expression.setMinimal(true);
    if (expression.indexIn(html) !=-1)
    {
        _title = QString(expression.cap(1));
        expression = QRegExp("var flashvars = \\{.*'srv': '(.*)'.*'file': '(.*)'");
        expression.setMinimal(true);
        if (expression.indexIn(html) !=-1)
        {
            videoQuality newQuality;
            newQuality.containerName = ".flv";
            newQuality.videoUrl = expression.cap(1) + "/flv2/" + expression.cap(2);
            newQuality.quality = tr("normal");
            _supportedQualities.append(newQuality);
            emit analysingFinished();
            return;
        }

        expression = QRegExp("<p><a href=\"(http://download\\..*&t=dd)\">");
        expression.setMinimal(true);
        if (expression.indexIn(html) !=-1)
        {
            videoQuality newQuality;
            newQuality.containerName = ".flv";
            newQuality.videoUrl = expression.cap(1);
            newQuality.quality = tr("normal");
            _supportedQualities.append(newQuality);
            emit analysingFinished();
            return;
        }
        expression = QRegExp("\"(http://[^\"%]*\\.(flv|mp4|f4v|webm)[^\"]*)\"");
        expression.setMinimal(true);
        if (expression.indexIn(html) !=-1)
        {
            videoQuality newQuality;
            newQuality.containerName = expression.cap(2).prepend(".");
            newQuality.videoUrl = expression.cap(1);
            newQuality.quality = tr("normal");
            _supportedQualities.append(newQuality);
            emit analysingFinished();
            return;
        }

        expression = QRegExp("'(http://[^\"'%]*\\.(flv|mp4|f4v|webm)[^\"]*)'");
        expression.setMinimal(true);
        if (expression.indexIn(html) !=-1)
        {
            videoQuality newQuality;
            newQuality.containerName = expression.cap(2).prepend(".");
            newQuality.videoUrl = expression.cap(1);
            newQuality.quality = tr("normal");
            _supportedQualities.append(newQuality);
            emit analysingFinished();
            return;
        }


        expression = QRegExp("(http%3a%2f%2f[a-z0-9\\.%-_~]*.(flv|mp4|f4v|webm)[a-z0-9\\.%-_~]*)[&\"]");
        expression.setMinimal(true);
        expression.setCaseSensitivity(Qt::CaseInsensitive);
        if (expression.indexIn(html) !=-1)
        {
            videoQuality newQuality;
            newQuality.containerName = expression.cap(2).prepend(".");
            newQuality.videoUrl = QUrl::fromPercentEncoding(expression.cap(1).toAscii());
            newQuality.quality = tr("normal");
            _supportedQualities.append(newQuality);
            emit analysingFinished();
            return;
        }


        expression = QRegExp("'(http://[^\"']*\\.(flv|mp4|f4v|webm)[^\"]*)'");
        expression.setMinimal(true);
        if (expression.indexIn(html) !=-1)
        {
            videoQuality newQuality;
            newQuality.containerName = expression.cap(2).prepend(".");
            newQuality.videoUrl = QUrl::fromPercentEncoding(expression.cap(1).toAscii());
            newQuality.quality = tr("normal");
            _supportedQualities.append(newQuality);
            emit analysingFinished();
            return;
        }

        expression = QRegExp("(http://[a-zA-Z0-9\\.%-_~/\\?=]*.flv)");
        expression.setMinimal(true);
        if (expression.indexIn(html) !=-1)
        {
            videoQuality newQuality;
            newQuality.containerName = ".flv";
            newQuality.videoUrl = expression.cap(1);
            newQuality.quality = tr("normal");
            _supportedQualities.append(newQuality);
            emit analysingFinished();
            return;
        }

    }
    else
    {
        emit error("Could not retrieve video title.", this);
    }
    _title = "";
    emit analysingFinished();
}
