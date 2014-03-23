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



#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    currentVideo = NULL;
    searchReply = NULL;
}


MainWindow::~MainWindow()
{
}

void MainWindow::init()
{
    //*
    //* Adding version info to the footer
    //*
    this->ui.label->setText(ui.label->text().replace("%version", "ClipGrab " + QCoreApplication::applicationVersion()));

    //*
    //* Tray Icon
    //*
    systemTrayIcon.setIcon(QIcon(":/img/icon.png"));
    systemTrayIcon.show();
    connect(&systemTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(systemTrayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(&systemTrayIcon, SIGNAL(messageClicked()), this, SLOT(systemTrayMessageClicked()));

    //*
    //* Clipboard Handling
    //*
    connect(cg, SIGNAL(compatiblePortalFound(bool, video*)), this, SLOT(compatiblePortalFound(bool, video*)));
    connect(cg, SIGNAL(compatibleUrlFoundInClipboard(QString)), this, SLOT(compatibleUrlFoundInClipBoard(QString)));

    //*
    //* Download Tab
    //*
    connect(ui.downloadStart, SIGNAL(clicked()), this, SLOT(startDownload()));
    connect(ui.downloadLineEdit, SIGNAL(textChanged(QString)), cg, SLOT(determinePortal(QString)));
    connect(this, SIGNAL(itemToCancel(int)), cg, SLOT(cancelDownload(int)));
    //connect(ui.downloadTree, SIGNAL(doubleClicked(QModelIndex)), this, openFinishedVideo(QModelIndex));
    ui.downloadTree->header()->setResizeMode(1, QHeaderView::Stretch);
    ui.downloadTree->header()->setStretchLastSection(false);
    ui.downloadTree->header()->setResizeMode(3, QHeaderView::ResizeToContents);
    ui.downloadLineEdit->setFocus(Qt::OtherFocusReason);

    int lastFormat = cg->settings.value("LastFormat", 0).toInt();
    for (int i = 0; i < this->cg->formats.size(); ++i)
    {
        this->ui.downloadComboFormat->addItem(this->cg->formats.at(i)._name);
    }
    //"Fix" for Meego: this->ui.downloadComboFormat->addItem(this->cg->formats.at(0)._name);

    this->ui.downloadComboFormat->setCurrentIndex(lastFormat);

    ui.downloadPause->hide(); //Qt does currently not handle throttling downloads properly


    //*
    //* Search Tab
    //*
    this->ui.searchWebView->setContextMenuPolicy(Qt::NoContextMenu);

    this->searchNam = new QNetworkAccessManager();

    //*
    //* Settings Tab
    //*
    connect(this->ui.settingsRadioClipboardAlways, SIGNAL(toggled(bool)), this, SLOT(settingsClipboard_toggled(bool)));
    connect(this->ui.settingsRadioClipboardNever, SIGNAL(toggled(bool)), this, SLOT(settingsClipboard_toggled(bool)));
    connect(this->ui.settingsRadioClipboardAsk, SIGNAL(toggled(bool)), this, SLOT(settingsClipboard_toggled(bool)));
    connect(this->ui.settingsRadioNotificationsAlways, SIGNAL(toggled(bool)), this, SLOT(settingsNotifications_toggled(bool)));
    connect(this->ui.settingsRadioNotificationsFinish, SIGNAL(toggled(bool)), this, SLOT(settingsNotifications_toggled(bool)));
    connect(this->ui.settingsRadioNotificationsNever, SIGNAL(toggled(bool)), this, SLOT(settingsNotifications_toggled(bool)));


    this->ui.settingsSavedPath->setText(cg->settings.value("savedPath", QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).toString());
    this->ui.settingsSaveLastPath->setChecked(cg->settings.value("saveLastPath", true).toBool());
    ui.settingsNeverAskForPath->setChecked(cg->settings.value("NeverAskForPath", false).toBool());

    ui.settingsUseMetadata->setChecked(cg->settings.value("UseMetadata", false).toBool());
    connect(this->ui.settingsUseMetadata, SIGNAL(stateChanged(int)), this, SLOT(on_settingsUseMetadata_stateChanged(int)));


    ui.settingsUseProxy->setChecked(cg->settings.value("UseProxy", false).toBool());
    ui.settingsProxyAuthenticationRequired->setChecked(cg->settings.value("ProxyAuthenticationRequired", false).toBool());
    ui.settingsProxyHost->setText(cg->settings.value("ProxyHost", "").toString());
    ui.settingsProxyPassword->setText(cg->settings.value("ProxyPassword", "").toString());
    ui.settingsProxyPort->setValue(cg->settings.value("ProxyPort", "").toInt());
    ui.settingsProxyUsername->setText(cg->settings.value("ProxyUsername", "").toString());
    ui.settingsProxyType->setCurrentIndex(cg->settings.value("ProxyType", 0).toInt());

    connect(this->ui.settingsUseProxy, SIGNAL(toggled(bool)), this, SLOT(settingsProxyChanged()));
    connect(this->ui.settingsProxyAuthenticationRequired, SIGNAL(toggled(bool)), this, SLOT(settingsProxyChanged()));
    connect(this->ui.settingsProxyHost, SIGNAL(textChanged(QString)), this, SLOT(settingsProxyChanged()));
    connect(this->ui.settingsProxyPassword, SIGNAL(textChanged(QString)), this, SLOT(settingsProxyChanged()));
    connect(this->ui.settingsProxyPort, SIGNAL(valueChanged(int)), this, SLOT(settingsProxyChanged()));
    connect(this->ui.settingsProxyUsername, SIGNAL(textChanged(QString)), this, SLOT(settingsProxyChanged()));
    connect(this->ui.settingsProxyType, SIGNAL(currentIndexChanged(int)), this, SLOT(settingsProxyChanged()));
    settingsProxyChanged();
    ui.settingsMinimizeToTray->setChecked(cg->settings.value("MinimizeToTray", false).toBool());

    if (cg->settings.value("Clipboard", "ask") == "always")
    {
        ui.settingsRadioClipboardAlways->setChecked(true);
    }
    else if (cg->settings.value("Clipboard", "ask") == "never")
    {
        ui.settingsRadioClipboardNever->setChecked(true);
    }
    else
    {
        ui.settingsRadioClipboardAsk->setChecked(true);
    }

    if (cg->settings.value("Notifications", "finish") == "finish")
    {
        ui.settingsRadioNotificationsFinish->setChecked(true);
    }
    else if (cg->settings.value("Notifications", "finish") == "always")
    {
        ui.settingsRadioNotificationsAlways->setChecked(true);
    }
    else
    {
        ui.settingsRadioNotificationsNever->setChecked(true);
    }
    ui.settingsUseWebM->setChecked(cg->settings.value("UseWebM", false).toBool());

    ui.settingsRemoveFinishedDownloads->setChecked(cg->settings.value("RemoveFinishedDownloads", false).toBool());


    int langIndex = 0;
    for (int i=0; i < cg->languages.count(); i++)
    {
        if (cg->languages.at(i).code == cg->settings.value("Language", "auto").toString())
        {
            langIndex = i;
        }
    }
    for (int i=0; i < cg->languages.count(); i++)
    {
        ui.settingsLanguage->addItem(cg->languages.at(i).name, cg->languages.at(i).code);
    }
    ui.settingsLanguage->setCurrentIndex(langIndex);


    this->ui.tabWidget->removeTab(2); //fixme!

    startTimer(500);

    //*
    //* About Tab
    //*

    #ifdef Q_WS_MAC
        this->ui.downloadOpen->hide();
        this->cg->settings.setValue("Clipboard", "always");
        this->ui.generalSettingsTabWidget->removeTab(2);
        this->setStyleSheet("#label_4{padding-top:25px}#label{font-size:10px}#centralWidget{background:#fff};#mainTab{margin-top;-20px};#label_4{padding:10px}#downloadInfoBox, #settingsGeneralInfoBox, #settingsLanguageInfoBox, #aboutInfoBox, #searchInfoBox{color: background: #00B4DE;}");
        this->ui.label_4->setMinimumHeight(120);
    #endif


   on_searchLineEdit_textChanged("");
   this->ui.mainTab->setCurrentIndex(cg->settings.value("MainTab", 0).toInt());
}

void MainWindow::startDownload()
{
    QTreeWidgetItem* tmpItem = new QTreeWidgetItem(QStringList() << currentVideo->getName() << currentVideo->title() << ui.downloadComboFormat->currentText());
    tmpItem->setSizeHint(0, QSize(16, 24));
    currentVideo->setTreeItem(tmpItem);
    currentVideo->setQuality(ui.downloadComboQuality->currentIndex());
    currentVideo->setConverter(cg->formats.at(ui.downloadComboFormat->currentIndex())._converter->createNewInstance(),cg->formats.at(ui.downloadComboFormat->currentIndex())._mode);
    QString target;
    if (cg->settings.value("NeverAskForPath", false).toBool() == false)
    {
        target = QFileDialog::getSaveFileName(this, tr("Select Target"), cg->settings.value("savedPath", QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).toString() +"/" + currentVideo->getSaveTitle());

    }
    else
    {
        target = cg->settings.value("savedPath", QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)).toString() +"/"+currentVideo->getSaveTitle();
    }
    if (!target.isEmpty())
    {
        if (cg->settings.value("saveLastPath", true) == true)
        {
            QString targetDir = target;
            targetDir.remove(targetDir.split("/", QString::SkipEmptyParts).last()).replace(QRegExp("/+"), "/");
            ui.settingsSavedPath->setText(targetDir);
        }
        currentVideo->setTargetPath(target);

        if (cg->settings.value("UseMetadata", false).toBool() == true)
        {
            if (ui.downloadComboFormat->currentIndex() == 4 || ui.downloadComboFormat->currentIndex() == 5)
            {
                metadataDialog = new QDialog;
                mdui.setupUi(metadataDialog);
                mdui.title->setText(currentVideo->title());
                metadataDialog->setModal(true);
                metadataDialog->exec();

                currentVideo->setMetaTitle(mdui.title->text());
                currentVideo->setMetaArtist(mdui.artist->text());

                delete metadataDialog;
            }
        }

        cg->addDownload(currentVideo);
        ui.downloadTree->insertTopLevelItem(0, tmpItem);

        currentVideo->_progressBar = new QProgressBar();
        currentVideo->_progressBar->setValue(0);
        currentVideo->_progressBar->setMaximum(1);
        ui.downloadTree->setItemWidget(tmpItem, 3, currentVideo->_progressBar);

        ((QProgressBar*) ui.downloadTree->itemWidget(tmpItem, 3))->setMaximum(100);
        connect(currentVideo, SIGNAL(progressChanged(int,int)),ui.downloadTree, SLOT(update()));
        connect(currentVideo, SIGNAL(downloadFinished()), currentVideo, SLOT(startConvert()));
        connect(currentVideo, SIGNAL(conversionFinished(video*)), this, SLOT(handleFinishedConversion(video*)));
        currentVideo = NULL;
        ui.downloadLineEdit->clear();
    }
}

void MainWindow::compatiblePortalFound(bool found, video* portal)
{
    disableDownloadUi(true);
    ui.downloadComboQuality->clear();;
    if (found == true)
    {
        ui.downloadLineEdit->setReadOnly(true);
        ui.downloadInfoBox->setText(tr("Please wait while ClipGrab is loading information about the video ..."));

        if (currentVideo)
        {
            currentVideo->deleteLater();;
        }
        currentVideo = portal->createNewInstance();
        currentVideo->setUrl(ui.downloadLineEdit->text());
        connect(currentVideo, SIGNAL(error(QString,video*)), cg, SLOT(errorHandler(QString,video*)));
        connect(currentVideo, SIGNAL(analysingFinished()), this, SLOT(updateVideoInfo()));
        currentVideo->analyse();

    }
    else
    {
        if (ui.downloadLineEdit->text() == "")
        {
            ui.downloadInfoBox->setText(tr("Please enter the link to the video you want to download in the field below."));
        }
        else if (ui.downloadLineEdit->text().startsWith("http://") || ui.downloadLineEdit->text().startsWith("https://"))
        {            
            ui.downloadLineEdit->setReadOnly(true);
            ui.downloadInfoBox->setText(tr("The link you have entered seems to not be recognised by any of the supported portals.<br/>Now ClipGrab will check if it can download a video from that site anyway."));

            if (currentVideo)
            {
                currentVideo->deleteLater();;
            }
            currentVideo = cg->heuristic->createNewInstance();
            currentVideo->setUrl(ui.downloadLineEdit->text());
            connect(currentVideo, SIGNAL(error(QString,video*)), cg, SLOT(errorHandler(QString,video*)));
            connect(currentVideo, SIGNAL(analysingFinished()), this, SLOT(updateVideoInfo()));
            currentVideo->analyse();
        }
    }
}

void MainWindow::updateVideoInfo()
{
    if (currentVideo && currentVideo->title() != "" && !currentVideo->getSupportedQualities().isEmpty())
    {
        ui.downloadInfoBox->setText("<strong>" + currentVideo->title() + "</strong>");
        ui.downloadLineEdit->setReadOnly(false);
        disableDownloadUi(false);
        ui.downloadComboQuality->clear();
        ui.downloadComboQuality->addItems(currentVideo->getSupportedQualities());
    }
    else
    {
        ui.downloadInfoBox->setText(tr("No downloadable video could be found.<br />Maybe you have entered the wrong link or there is a problem with your connection."));
        ui.downloadLineEdit->setReadOnly(false);
        disableDownloadUi(true);
    }
}
void MainWindow::on_settingsSavedPath_textChanged(QString string)
{
    this->cg->settings.setValue("savedPath", string);
}

void MainWindow::on_settingsBrowseTargetPath_clicked()
{
    QString newPath;
    newPath = QFileDialog::getExistingDirectory(this, tr("ClipGrab - Select target path"), ui.settingsSavedPath->text());
    if (!newPath.isEmpty())
    {
        ui.settingsSavedPath->setText(newPath);
    }
}

void MainWindow::on_downloadCancel_clicked()
{
    if (ui.downloadTree->currentIndex().row() != -1)
    {
        int item = ui.downloadTree->topLevelItemCount() - ui.downloadTree->currentIndex().row() -1;
        ui.downloadTree->takeTopLevelItem(ui.downloadTree->currentIndex().row());
        emit itemToCancel(item);
    }
}

void MainWindow::on_settingsSaveLastPath_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        cg->settings.setValue("saveLastPath", true);
    }
    else
    {
        cg->settings.setValue("saveLastPath", false);
    }
}

void MainWindow::on_downloadOpen_clicked()
{
    if (ui.downloadTree->currentIndex().row() != -1)
    {
        QString targetDir;
        targetDir = cg->getDownloadTargetPath(ui.downloadTree->topLevelItemCount()-1 - ui.downloadTree->currentIndex().row());
        targetDir.remove(targetDir.split("/").last());
        QDesktopServices::openUrl(targetDir);
    }
}


void MainWindow::compatibleUrlFoundInClipBoard(QString url)
{
    if (QApplication::activeModalWidget() == 0)
    {
        if (cg->settings.value("Clipboard", "ask") == "ask")
        {
            Notifications::showMessage(tr("ClipGrab: Video discovered in your clipboard"), tr("ClipGrab has discovered the address of a compatible video in your clipboard. Click on this message to download it now."), &systemTrayIcon);
        }
        else if (cg->settings.value("Clipboard", "ask") == "always")
        {
            this->ui.downloadLineEdit->setText(url);
            this->ui.mainTab->setCurrentIndex(1);
            this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            this->show();
            this->activateWindow();
            this->raise();
        }
    }

}

void MainWindow::systemTrayMessageClicked()
{
    if (QApplication::activeModalWidget() == 0)
    {
        this->ui.downloadLineEdit->setText(cg->clipboardUrl);
        this->ui.mainTab->setCurrentIndex(1);
        this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        this->show();
        this->activateWindow();
        this->raise();
    }
    

}

void MainWindow::systemTrayIconActivated(QSystemTrayIcon::ActivationReason)
{
    if (cg->settings.value("MinimizeToTray", false).toBool() && !this->isHidden())
    {
        this->hide();
    }
    else
    {
        this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        this->show();
        this->activateWindow();
        this->raise();
    }
}

void MainWindow::settingsClipboard_toggled(bool)
{
    if (ui.settingsRadioClipboardAlways->isChecked())
    {
        cg->settings.setValue("Clipboard", "always");
    }
    else if (ui.settingsRadioClipboardNever->isChecked())
    {
        cg->settings.setValue("Clipboard", "never");
    }
    else if (ui.settingsRadioClipboardAsk->isChecked())
    {
        cg->settings.setValue("Clipboard", "ask");
    }
}

void MainWindow::disableDownloadUi(bool disable)
{
    ui.downloadComboFormat->setDisabled(disable);
    ui.downloadComboQuality->setDisabled(disable);
    ui.downloadStart->setDisabled(disable);
    ui.label_2->setDisabled(disable);
    ui.label_3->setDisabled(disable);
}

void MainWindow::disableDownloadTreeButtons(bool disable)
{
    ui.downloadOpen->setDisabled(disable);
    ui.downloadCancel->setDisabled(disable);
    ui.downloadPause->setDisabled(disable);
}

void MainWindow::on_downloadTree_currentItemChanged(QTreeWidgetItem* /*current*/, QTreeWidgetItem* /*previous*/)
{
    if (ui.downloadTree->currentIndex().row() == -1)
    {
        disableDownloadTreeButtons();
    }
    else
    {
        disableDownloadTreeButtons(false);
    }
}

void MainWindow::on_settingsNeverAskForPath_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        cg->settings.setValue("NeverAskForPath", true);
    }
    else
    {
        cg->settings.setValue("NeverAskForPath", false);
    }
}

 void MainWindow::closeEvent(QCloseEvent *event)
 {
    if (cg->downloadsRunning() > 0)
    {
        QMessageBox* exitBox;
        exitBox = new QMessageBox(QMessageBox::Question, tr("ClipGrab - Exit confirmation"), tr("There is still at least one download in progress.<br />If you exit the program now, all downloads will be canceled and cannot be recovered later.<br />Do you really want to quit ClipGrab now?"), QMessageBox::Yes|QMessageBox::No);
        if (exitBox->exec() == QMessageBox::Yes)
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
       event->accept();;
    }
 }

 void MainWindow::settingsNotifications_toggled(bool)
 {
    if (ui.settingsRadioNotificationsAlways->isChecked())
    {
        cg->settings.setValue("Notifications", "always");
    }
    else if (ui.settingsRadioNotificationsFinish->isChecked())
    {
        cg->settings.setValue("Notifications", "finish");
    }
    else if (ui.settingsRadioNotificationsNever->isChecked())
    {
        cg->settings.setValue("Notifications", "never");
    }
 }

 void MainWindow::handleFinishedConversion(video* finishedVideo)
 {
     if (cg->settings.value("Notifications", "finish") == "always")
     {
         Notifications::showMessage(tr("Download finished"), tr("Downloading and converting “%title” is now finished.").replace("%title", finishedVideo->title()), &systemTrayIcon);
     }
     else if (cg->downloadsRunning() == 0 && cg->settings.value("Notifications", "finish") == "finish")
     {
         Notifications::showMessage(tr("All downloads finished"), tr("ClipGrab has finished downloading and converting all selected videos."), &systemTrayIcon);
     }

     if (cg->settings.value("RemoveFinishedDownloads", false) == true)
     {
         int finishedItemIndex = ui.downloadTree->indexOfTopLevelItem(finishedVideo->treeItem());
         int item = ui.downloadTree->topLevelItemCount() - finishedItemIndex -1;
         ui.downloadTree->takeTopLevelItem(finishedItemIndex);
         emit itemToCancel(item);
     }
 }

void MainWindow::on_settingsRemoveFinishedDownloads_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        cg->settings.setValue("RemoveFinishedDownloads", true);
    }
    else
    {
        cg->settings.setValue("RemoveFinishedDownloads", false);
    }
}

void MainWindow::on_downloadPause_clicked()
{
    if (ui.downloadTree->currentIndex().row() != -1)
    {
        int item = ui.downloadTree->topLevelItemCount() - ui.downloadTree->currentIndex().row() -1;
        cg->pauseDownload(item);
    }
}

void MainWindow::settingsProxyChanged()
{
    cg->settings.setValue("UseProxy", ui.settingsUseProxy->isChecked());
    cg->settings.setValue("ProxyHost", ui.settingsProxyHost->text());
    cg->settings.setValue("ProxyPort", ui.settingsProxyPort->value());
    cg->settings.setValue("ProxyAuthenticationRequired", ui.settingsProxyAuthenticationRequired->isChecked());
    cg->settings.setValue("ProxyPassword", ui.settingsProxyPassword->text());
    cg->settings.setValue("ProxyUsername", ui.settingsProxyUsername->text());
    cg->settings.setValue("ProxyType", ui.settingsProxyType->currentIndex());
    ui.settingsProxyGroup->setEnabled(ui.settingsUseProxy->isChecked());
    ui.settingsProxyAuthenticationRequired->setEnabled(ui.settingsUseProxy->isChecked());
    if (ui.settingsUseProxy->isChecked())
    {
        if (ui.settingsProxyAuthenticationRequired->isChecked())
        {
            ui.settingsProxyAuthenticationGroup->setEnabled(ui.settingsProxyAuthenticationRequired->isChecked());
        }
    }
    else
    {
        ui.settingsProxyAuthenticationGroup->setEnabled(false);
        ui.settingsProxyAuthenticationRequired->setEnabled(false);
     }
    cg->activateProxySettings();
}

void MainWindow::timerEvent(QTimerEvent *)
{
    QPair<qint64, qint64> downloadProgress = cg->downloadProgress();
    if (downloadProgress.first != 0 && downloadProgress.second != 0)
    {
        #ifdef Q_WS_X11
            systemTrayIcon.setToolTip("<strong style=\"font-size:14px\">" + tr("ClipGrab") + "</strong><br /><span style=\"font-size:13px\">" + QString::number(downloadProgress.first*100/downloadProgress.second) + " %</span><br />" + QString::number((double)downloadProgress.first/1024/1024, QLocale::system().decimalPoint().toAscii(), 1) + tr(" MiB") + "/" + QString::number((double)downloadProgress.second/1024/1024, QLocale::system().decimalPoint().toAscii(), 1) + tr(" MiB"));
        #else
        systemTrayIcon.setToolTip(tr("ClipGrab") + " - " + QString::number(downloadProgress.first*100/downloadProgress.second) + " % - " + QString::number((double)downloadProgress.first/1024/1024, QLocale::system().decimalPoint().toAscii(), 1) + tr(" MiB") + "/" + QString::number((double)downloadProgress.second/1024/1024, QLocale::system().decimalPoint().toAscii(), 1) + tr(" KiB"));
        #endif
        setWindowTitle("ClipGrab - " + QString::number(downloadProgress.first*100/downloadProgress.second) + " %");
    }
    else
    {
        #ifdef Q_WS_X11
            systemTrayIcon.setToolTip("<strong style=\"font-size:14px\">" + tr("ClipGrab") + "</strong><br /><span style=\"font-size:13px\">" + tr("Currently no downloads in progress."));
        #endif
        systemTrayIcon.setToolTip(tr("ClipGrab") + tr("Currently no downloads in progress."));
        setWindowTitle(tr("ClipGrab - Download and Convert Online Videos"));
    }
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (isMinimized() && cg->settings.value("MinimizeToTray", false).toBool())
        {
            QTimer::singleShot(500, this, SLOT(hide()));
            event->ignore();
        }
    }
}

void MainWindow::on_settingsMinimizeToTray_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        cg->settings.setValue("MinimizeToTray", true);
    }
    else
    {
        cg->settings.setValue("MinimizeToTray", false);
    }
}


void MainWindow::on_downloadLineEdit_returnPressed()
{
    if (currentVideo)
    {
        if (!currentVideo->title().isEmpty())
        {
            this->startDownload();
        }
    }
}

void MainWindow::on_label_linkActivated(QString link)
{
    QDesktopServices::openUrl(link);
}

void MainWindow::on_settingsUseMetadata_stateChanged(int state)
{
    if (state == Qt::Checked)
    {
        cg->settings.setValue("UseMetadata", true);
    }
    else
    {
        cg->settings.setValue("UseMetadata", false);
    }
}

void MainWindow::on_searchLineEdit_textChanged(QString keywords)
{
    if (!keywords.isEmpty())
    {
    if (searchReply)
    {
        searchReply->abort();
        searchReply->deleteLater();
    }
        this->searchReply = searchNam->get(QNetworkRequest(QUrl("http://gdata.youtube.com/feeds/mobile/videos?max-results=8&q=" + keywords.replace(QRegExp("[&\\?%\\s]"), "+"))));
        connect(this->searchReply, SIGNAL(finished()), this, SLOT(processSearchReply()));
    }
    else
    {
        this->searchReply = searchNam->get(QNetworkRequest(QUrl("http://gdata.youtube.com/feeds/mobile/standardfeeds/most_popular?max-results=4")));
        connect(this->searchReply, SIGNAL(finished()), this, SLOT(processSearchReply()));
    }
}

void MainWindow::processSearchReply()
{
    QDomDocument* dom = new QDomDocument();
    dom->setContent(searchReply->readAll());
    QDomNodeList entries = dom->elementsByTagName("entry");

    QString searchHtml;
    searchHtml.append("<style>body {margin:0;padding:0;width:100%;left:0px;top:0px;font-family:'Segoe UI', Ubuntu, sans-serif} img{position:relative;top:-22px;left:-15px} .entry{display:block;position:relative;width:50%;float:left;height:100px;} a{color:#1a1a1a;text-decoration:none;} span.title{display:block;font-weight:bold;font-size:14px;position:absolute;left:140px;top:16px;} span.duration{display:block;font-size:11px;position:absolute;left:140px;top:70px;color:#aaa}  span.thumbnail{width:120px;height:68px;background:#00b2de;display:block;overflow:hidden;position:absolute;left:8px;top:16px;} a:hover{background: #00b2de;color:#fff}</style>");
    searchHtml.append("<body>");

    for (int i = 0; i < entries.count(); i++)
    {
        QString duration;
        int seconds = entries.at(i).toElement().elementsByTagName("yt:duration").at(0).toElement().attribute("seconds").toInt();

        duration.append(QString("%1:").arg(QString::number(floor(seconds/3600)), 2, QChar('0')));
        duration.append(QString("%1:").arg(QString::number(floor((seconds%3600)/60)), 2, QChar('0')));
        duration.append(QString("%1").arg(QString::number(seconds%3600%60), 2, QChar('0')));

        searchHtml.append("<a href=\"http://www.youtube.com/watch?v=" + entries.at(i).toElement().elementsByTagName("id").at(0).toElement().text().split("/").last() + "\" class=\"entry\">");
        searchHtml.append("<span class=\"title\">" + entries.at(i).toElement().elementsByTagName("title").at(0).toElement().text() + "</span>");
        searchHtml.append("<span class=\"duration\">" + duration + "</span>");
        searchHtml.append("<span class=\"thumbnail\"><img width=\"125%\" src=\"" + entries.at(i).toElement().elementsByTagName("media:thumbnail").at(0).toElement().attribute("url") + "\"  /></span");



        searchHtml.append("</a>");
    }

    searchHtml.append("</body>");
    this->ui.searchWebView->setHtml(searchHtml);
    ui.searchWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

}

void MainWindow::on_searchWebView_linkClicked(QUrl url)
{
    this->ui.downloadLineEdit->setText(url.toString());;
    this->ui.mainTab->setCurrentIndex(1);
}

void MainWindow::on_downloadComboFormat_currentIndexChanged(int index)
{
    cg->settings.setValue("LastFormat", index);
}

void MainWindow::on_mainTab_currentChanged(int index)
{
    cg->settings.setValue("MainTab", index);
}

void MainWindow::on_downloadTree_doubleClicked(QModelIndex /*index*/)
{
    if (ui.downloadTree->currentIndex().row() != -1)
    {
        if (cg->isDownloadFinished(ui.downloadTree->topLevelItemCount()-1))
        {
            QString targetFile;
            cg->downloadProgress();
            targetFile = cg->getDownloadTargetPath(ui.downloadTree->topLevelItemCount()-1 - ui.downloadTree->currentIndex().row());
            QUrl targetFileUrl = QUrl::fromLocalFile(targetFile);
            qDebug() << targetFileUrl << targetFileUrl.isValid();
            qDebug() << QDesktopServices::openUrl(targetFileUrl);
        }
    }
}

void MainWindow::on_settingsLanguage_currentIndexChanged(int index)
{
    cg->settings.setValue("Language", cg->languages.at(index).code);

}

void MainWindow::on_buttonDonate_clicked()
{
    QDesktopServices::openUrl(QUrl("https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=AS6TDMR667GJL"));
}

void MainWindow::on_settingsUseWebM_toggled(bool checked)
{
    cg->settings.setValue("UseWebM", checked);
}
