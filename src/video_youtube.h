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



#ifndef video_YOUTUBE_H
#define video_YOUTUBE_H

#include "video.h"
#include "QtWebKit"
#include <QtGui>
#include "ui_youtube_password.h"

#include "QFile"

struct fmtQuality
{
    QString quality;
    QString video;
    QString audio;

    fmtQuality(QString quality, QString video, QString audio="")
    {
        this->quality = quality;
        this->video = video;
        if (!audio.isEmpty())
        {
            this->audio = audio;
        }
    }
};

class video_youtube : public video
{
    Q_OBJECT

    private:
        QString _maxfmt;

        QString parseSignature(QString s);
        QString getFmtLink(QStringList, QString fmt);
        QString getUrlFromFmtLink(QString link);
        void parseJS(QString js);
        void parseLoginForm(QString html);
        void parseLoginFormResponse(QString html);
        QString loginFormError;
        QString html;
        QString js;
        QString downloading;

    public:
        virtual bool setUrl(QString);
        video_youtube();
        video* createNewInstance();
        virtual void parseVideo(QString);
};

#endif // video_YOUTUBE_H
