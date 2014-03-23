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



#include "http_handler.h"

http_handler::http_handler()
{
    networkAccessManager = new QNetworkAccessManager;
    connect (networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleNetworkReply(QNetworkReply*)));
    connect (this, SIGNAL(downloadFinished(download*)), this, SLOT(handleFinishedDownload(download*)));
}

//Create a new QNetworkRequest
QNetworkRequest http_handler::createRequest(QUrl url)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");

    return request;
}

//Start a new download with the download handler
QNetworkReply* http_handler::addDownload(QString url, bool chunked, QByteArray postData)
{
    download* newDownload = new download;
    QNetworkRequest request = createRequest(QUrl::fromEncoded(url.toAscii()));
    newDownload->tempFile = new QTemporaryFile(QDir::tempPath() + "/clipgrab-download-XXXXXX");
    newDownload->size = 0;
    newDownload->redirectLevel = 0;
    newDownload->chunked = chunked;
    if (newDownload->chunked)
    {
        request.setRawHeader("Range", QString("bytes=0-925696").toAscii());
    }

    if (postData.isEmpty())
    {
        newDownload->reply = this->networkAccessManager->get(request);
    }
    else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        newDownload->reply = this->networkAccessManager->post(request, postData);
    }
    connect(newDownload->reply, SIGNAL(readyRead()), this, SLOT(dataHandler()));

    downloads.append(newDownload);
    return newDownload->reply;
}

//Continue the download after a redirection or successful range request
void http_handler::continueDownload(download* dl)
{
    QNetworkRequest request;
    dl->reply->close();

    //In case of redirection
    if (!dl->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().isEmpty() && dl->redirectLevel < 16)
    {
        request = createRequest(dl->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
        dl->redirectLevel ++;

        QString fileTemplate = dl->tempFile->fileTemplate();
        dl->tempFile->deleteLater();
        dl->tempFile = new QTemporaryFile(fileTemplate);
    }

    //In case of 206 status (successful range request)
    else if (dl->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 206)
    {
        request = createRequest(dl->reply->url());
        dl->progress = dl->getProgress();
        dl->currentProgress = 0;
        dl->redirectLevel = 0;

        qint64 downloadChunk = 925696;//925696;
        qint64 targetBytes = dl->progress+downloadChunk;


        if (targetBytes >= dl->size-downloadChunk)
        {
            targetBytes = dl->size;
        }

        //If the download is complete already
        if (targetBytes <= dl->progress)
        {
            emit downloadFinished(dl);
            return;
        }
        //If more parts need to be downloaded
        else
        {
            request.setRawHeader("Range", QString("bytes=" + QString::number(dl->getProgress()) + "-" + QString::number(targetBytes)).toAscii());
        }
    }
    if (!request.url().isEmpty())
    {
        dl->reply = networkAccessManager->get(request);
        connect(dl->reply, SIGNAL(readyRead()), this, SLOT(dataHandler()));
    }
}

download* http_handler::getDownload(QNetworkReply* reply)
{
    if (reply)
    {
        for (int i=0; i < this->downloads.size(); i++)
        {
            if (downloads.at(i)->reply == reply)
            {
                return downloads.at(i);
            }
        }
    }
    return NULL;
}

void http_handler::handleNetworkReply(QNetworkReply* reply)
{
    download* dl = getDownload(reply);
    if ((!dl->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().isEmpty() && dl->redirectLevel < 16) ||  (dl->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 206))
    {
        continueDownload(dl);
        return;
    }
    else if (reply->error())
    {
        qDebug() << "Error!" << reply->errorString();

        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() <= 206 && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 200)
        {
            qDebug() << "Continuing from error";
            continueDownload(dl);
            return;
        }
        else
        {
            emit error(reply->errorString() + " Error code: " + reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
            reply->close();
            handleFinishedDownload(dl);
        }
    }
    else {
        emit downloadFinished(dl);
    }
}

void http_handler::cancelAllDownloads()
{

    for (int i=0; i < this->downloads.size(); i++)
    {
        if (downloads.at(i)->reply)
        {
            downloads.at(i)->reply->disconnect();
            downloads.at(i)->reply->abort();
        }
        if (downloads.at(i)->tempFile)
        {
            downloads.at(i)->tempFile->disconnect();
            downloads.at(i)->tempFile->close();
            downloads.at(i)->tempFile->deleteLater();
        }
    }
}

void http_handler::dataHandler()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    download* dl = getDownload(reply);
	
	if (dl)
	{

		//Update current progress on chunk/download
		dl->currentProgress += reply->bytesAvailable();

		//Write data to temp files
		if (dl->tempFile && dl->tempFile->open())
		{
			dl->tempFile->write(reply->readAll());
		}

		//Update progress and file size

		if (reply->hasRawHeader("Content-Range"))
		{
			dl->size = QString(reply->rawHeader("Content-Range")).split("/").last().toLongLong();
		}
		else {
			dl->size = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
		}

		qint64 totalProgress = 0;
		qint64 totalBytes  = 0;
		for (int i=0; i < this->downloads.size(); i++)
		{
			totalProgress += this->downloads.at(i)->getProgress();
			totalBytes += this->downloads.at(i)->size;
		}
		emit downloadProgress(totalProgress, totalBytes);
	}
}

void http_handler::handleFinishedDownload(download* dl)
{
    dl->finished = true;

    bool allFinished = true;
    for (int i=0; i < this->downloads.size(); i++)
    {
        if (!this->downloads.at(i)->finished)
        {
            allFinished = false;
        }
    }
    if (allFinished)
    {
        emit allDownloadsFinished();
    }
}

void http_handler::clearDownloads()
{
    for (int i=0; i < this->downloads.size(); i++)
    {
        if (downloads.at(i)->reply)
        {
            downloads.at(i)->reply->close();
            downloads.at(i)->reply->deleteLater();
        }
        if (downloads.at(i)->tempFile)
        {
            downloads.at(i)->tempFile->deleteLater();
        }
    }
    downloads.clear();
}
