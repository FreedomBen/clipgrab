#include "video_youku.h"

video_youku::video_youku()
{
    this->_name = "Youku";
    this->_supportsTitle = true;
    this->_supportsDescription = false;
    this->_supportsThumbnail = false;
    this->_supportsSearch = false;
    this->_icon = 0;
    this->_urlRegExp << QRegExp("http[s]?://(www\\.)*v\\.youku\\.com/v_show/id_.+\\.html.*", Qt::CaseInsensitive);
    _treeItem = NULL;
}

video* video_youku::createNewInstance()
{
    return new video_youku();
}

void video_youku::parseVideo(QString html)
{
    switch (this->_step)
    {
        case 1:
        //
        // Retrieving the video title
        //
        {
            QRegExp expression;

            expression = QRegExp("<h1 class=\"title\">(.+)</h1>");
            expression.setMinimal(true);
            if (expression.indexIn(html) !=-1 && !expression.cap(1).isEmpty())
            {
                _title = expression.cap(1);
                _finished = false;

                expression = QRegExp("http://v.youku.com/v_show/id_(.+)\\.html");
                expression.setMinimal(true);
                if (expression.indexIn(_url.toString()) !=-1 && !expression.cap(1).isEmpty())
                {
                    this->_url = QUrl("http://v.youku.com/player/getM3U8/vid/" + expression.cap(1) + "/type/mp4/v.m3u8");
                    if (_url.isValid())
                    {
                        _step = 2;
                        _redirectLevel = 0;
                        tempFile = new QTemporaryFile(QDir::tempPath() + "/clipgrab-info--XXXXXX");
                        _finished = false;
                        QNetworkRequest request;
                        request.setUrl(QUrl("http://v.youku.com/player/getM3U8/vid/" + expression.cap(1) + "/type/mp4/v.m3u8"));
                        qDebug() << request.url().toString();
                        _currentNetworkReply = this->_networkAccessManager->get(request);
                        connect(_currentNetworkReply, SIGNAL(readyRead()), this, SLOT(dataHandler()));
                    }
                    else
                    {
                        emit error("Could not retrieve video link");
                    }
                }

            }
            else {
                emit error("Could not retrieve video title");
            }

            break;
        }
        case 2:
        //
        // Retrieving the video url
        //
        {
        qDebug() << "2";
            QRegExp expression;

            expression = QRegExp("http://.+\\.(mp4|flv)\\.ts");
            expression.setMinimal(true);
            qDebug() << html;
            if (expression.indexIn(html) !=-1 && !expression.cap(1).isEmpty())
            {
                qDebug() << html;
                videoQuality newQuality;
                newQuality.quality = tr("normal");
                newQuality.videoUrl = expression.cap(0);
                newQuality.containerName = "." + expression.cap(1);
                _supportedQualities.append(newQuality);
                emit analysingFinished();

            }
            else
            {
                emit error("Could not retrieve video link.", this);
            }
        }
    }
}
