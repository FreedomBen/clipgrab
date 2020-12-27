/*
    ClipGrab³
    Copyright (C) The ClipGrab Project
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



#include <QApplication>
#include <QGuiApplication>
#include <QTranslator>
#include <QDebug>
#include "mainwindow.h"
#include "clipgrab.h"
#include "video.h"

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("ClipGrab");
    QCoreApplication::setOrganizationDomain("clipgrab.org");
    QCoreApplication::setApplicationName("ClipGrab");
    QCoreApplication::setApplicationVersion(QString(STRINGIZE_VALUE_OF(CLIPGRAB_VERSION)).replace("\"", ""));

    QCommandLineParser parser;
    parser.setApplicationDescription("ClipGrab");
    parser.addVersionOption();
    parser.addHelpOption();
    QCommandLineOption startMinimizedOption(QStringList() << "start-minimized", "Hide the ClipGrab window on launch");
    parser.addOption(startMinimizedOption);
    parser.process(app);

    QSettings settings;
    if (settings.allKeys().isEmpty()) {
        static const QChar key[] = {
            0x0050, 0x0068, 0x0069, 0x006c, 0x0069, 0x0070, 0x0070, 0x0020, 0x0053, 0x0063, 0x0068, 0x006d, 0x0069, 0x0065, 0x0064, 0x0065, 0x0072};
        QSettings legacySettings(QString::fromRawData(key, sizeof(key) / sizeof(QChar)), "ClipGrab");
        QStringList legacyKeys = legacySettings.allKeys();
        QStringList ignoredKeys = {"youtubePlayerUrl", "youtubePlayerJS", "youtubePlayerSignatureMethodName"};
        for (int i = 0; i < legacyKeys.length(); i++) {
            if (ignoredKeys.contains(legacyKeys.at(i))) continue;
            settings.setValue(legacyKeys.at(i), legacySettings.value(legacyKeys.at(i)));
        }
        legacySettings.clear();
    }

    ClipGrab* cg = new ClipGrab();

    QTranslator translator;
    QString locale = settings.value("Language", "auto").toString();
    if (locale == "auto")
    {
        locale = QLocale::system().name();
    }
    translator.load(QString(":/lng/clipgrab_") + locale);
    app.installTranslator(&translator);
    for (int i=0; i < cg->languages.length(); i++)
    {
        if (cg->languages[i].code == locale) {
            if (cg->languages[i].isRTL)
            {
                QApplication::setLayoutDirection(Qt::RightToLeft);
            }
            break;
        }
    }

    MainWindow w(cg);
    w.init();
    if (!parser.isSet(startMinimizedOption)) {
        w.show();
    }

    QTimer::singleShot(0, [=] {
       cg->getUpdateInfo();
       QObject::connect(cg, &ClipGrab::updateInfoProcessed, [cg] {
           bool force = QSettings().value("forceYoutubeDlDownload", false).toBool();
           if (force) QSettings().setValue("forceYoutubeDlDownload", false);
           cg->downloadYoutubeDl(force);
       });
    });

    return app.exec();
}
