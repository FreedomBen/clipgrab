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



#include "video_tudou.h"

video_tudou::video_tudou()
{
    this->_name = "Tudou";
    this->_supportsTitle = true;
    this->_supportsDescription = false;
    this->_supportsThumbnail = false;
    this->_supportsSearch = false;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http://.*tudou\\.com/programs/view/.*", Qt::CaseInsensitive);
    _treeItem = NULL;
}

video* video_tudou::createNewInstance()
{
    return new video_tudou();
}

void video_tudou::parseVideo(QString xml)
{
    switch (this->_step)
    {
        case 1:
        //
        // Retrieving the video id
        //
        {
            QRegExp expression;
            QString iid;
            expression = QRegExp("var iid = ([0-9]+)");
            expression.indexIn(xml);
            if (expression.indexIn(xml) !=-1)
            {
                iid = expression.cap(1);
                tempFile = new QTemporaryFile(QDir::tempPath() + "/clipgrab-info--XXXXXX");
                _finished = false;
                if (this->_url.isValid())
                {
                    _step = 2;
                    _redirectLevel = 0;
                    QNetworkRequest request;
                    request.setUrl(QUrl("http://v2.tudou.com/v2/cdn?id="+iid));
                    _currentNetworkReply = this->_networkAccessManager->get(request);
                    this->_redirectLevel = 0;
                }
            }
            else
            {
                emit error("Could not retrieve video iid.", this);
            }

            break;
        }

        case 2:
        {
            QRegExp expression;
            expression = QRegExp("ti?tl?e?=\"(.*)\"");
            expression.setMinimal(true);
            expression.indexIn(xml);
            qDebug() << xml;
            if (expression.indexIn(xml) !=-1)
            {
                _title = expression.cap(1);
                qDebug() << _title;

                expression = QRegExp("<f.*>(.*)</f>");
                expression.setMinimal(true);
                if (expression.indexIn(xml) !=-1)
                {
                    videoQuality newQuality;
                    newQuality.quality = tr("normal");
                    newQuality.videoUrl = expression.cap(1).replace("&amp;", "&");
                    newQuality.containerName = ".flv";
                    _supportedQualities.append(newQuality);
                    qDebug() << "------"+newQuality.videoUrl+"------";
                    emit analysingFinished();
                }
                else
                {
                    emit error("Could not retrieve video url");
                }
            }
            else
            {
                emit error("Could not retrieve video title");
            }
        }
    }
}
