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



#include "mainwindow.h"

MainWindow::MainWindow(ClipGrab* cg, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    this->cg = cg;
    ui.setupUi(this);
}


MainWindow::~MainWindow()
{
    delete this->searchPage;
}

void MainWindow::init()
{
    DownloadListModel* l = new DownloadListModel(cg, this);
    this->ui.downloadTree->setModel(l);
    connect(ui.downloadTree, &QTreeView::dataChanged,[=] {
        handle_downloadTree_currentChanged(ui.downloadTree->currentIndex(), QModelIndex());
    });
    connect(cg, &ClipGrab::downloadFinished, this, &MainWindow::handleFinishedConversion);

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
    connect(cg, SIGNAL(compatibleUrlFoundInClipboard(QString)), this, SLOT(compatibleUrlFoundInClipBoard(QString)));

    //*
    //* Download Tab
    //*
    connect(ui.downloadStart, SIGNAL(clicked()), this, SLOT(startDownload()));
    connect(ui.downloadLineEdit, &QLineEdit::textChanged, [=](const QString url) {
        if (!url.startsWith("http://") && !url.startsWith("https://")) return;

        ui.downloadLineEdit->setReadOnly(true);
        ui.downloadInfoBox->setText(tr("Please wait while ClipGrab is loading information about the video ..."));
        disableDownloadUi();
        cg->fetchVideoInfo(url);
    });
    connect(cg, &ClipGrab::currentVideoStateChanged, this, &MainWindow::handleCurrentVideoStateChanged);

    connect(ui.downloadTree->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handle_downloadTree_currentChanged);
    ui.downloadTree->header()->setStretchLastSection(false);
    ui.downloadTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui.downloadTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui.downloadTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui.downloadTree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui.downloadTree->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui.downloadLineEdit->setFocus(Qt::OtherFocusReason);

    int lastFormat = cg->settings.value("LastFormat", 0).toInt();
    for (int i = 0; i < this->cg->formats.size(); ++i)
    {
        this->ui.downloadComboFormat->addItem(this->cg->formats.at(i)._name);
    }

    this->ui.downloadComboFormat->setCurrentIndex(lastFormat);


    //*
    //* Search Tab
    //*
    QWebEngineProfile* profile = new QWebEngineProfile;
    this->searchPage = new SearchWebEnginePage(profile);
    ui.searchWebEngineView->setPage(searchPage);
    ui.searchWebEngineView->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    ui.searchWebEngineView->setContextMenuPolicy(Qt::NoContextMenu);
    connect(ui.searchWebEngineView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(handleSearchResultClicked(QUrl)));
    connect(&searchTimer, SIGNAL(timeout()), this, SLOT(searchTimerTimeout()));
    connect(cg, &ClipGrab::youtubeDlDownloadFinished, [=] {
        YoutubeDl::find(true);
        this->updateSearch("");
        this->updateYoutubeDlVersionInfo();
    });
    connect(cg, &ClipGrab::searchFinished, this, &MainWindow::handleSearchResults);
    cg->search();

    //*
    //* Settings Tab
    //*
    connect(this->ui.settingsRadioClipboardAlways, SIGNAL(toggled(bool)), this, SLOT(settingsClipboard_toggled(bool)));
    connect(this->ui.settingsRadioClipboardNever, SIGNAL(toggled(bool)), this, SLOT(settingsClipboard_toggled(bool)));
    connect(this->ui.settingsRadioClipboardAsk, SIGNAL(toggled(bool)), this, SLOT(settingsClipboard_toggled(bool)));
    connect(this->ui.settingsRadioNotificationsAlways, SIGNAL(toggled(bool)), this, SLOT(settingsNotifications_toggled(bool)));
    connect(this->ui.settingsRadioNotificationsFinish, SIGNAL(toggled(bool)), this, SLOT(settingsNotifications_toggled(bool)));
    connect(this->ui.settingsRadioNotificationsNever, SIGNAL(toggled(bool)), this, SLOT(settingsNotifications_toggled(bool)));


    this->ui.settingsSavedPath->setText(cg->settings.value("savedPath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString());
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
      ui.settingsRememberVideoQuality->setChecked(cg->settings.value("rememberVideoQuality", true).toBool());
      ui.settingsRememberLogins->hide();
//    ui.settingsRememberLogins->setChecked(cg->settings.value("rememberLogins", true).toBool());
      ui.settingsRemoveFinishedDownloads->setChecked(cg->settings.value("RemoveFinishedDownloads", false).toBool());
      ui.settingsIgnoreSSLErrors->hide();
//    ui.settingsIgnoreSSLErrors->setChecked(cg->settings.value(("IgnoreSSLErrors"), false).toBool());
      ui.settingsForceIpV4->setChecked(cg->settings.value("forceIpV4", false).toBool());


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

    //*
    //* About Tab
    //*

    #ifdef Q_OS_MAC
        this->ui.downloadOpen->hide();
        this->cg->settings.setValue("Clipboard", "always");
        this->ui.generalSettingsTabWidget->removeTab(2);
        //FIXME this->setStyleSheet("#label_4{padding-top:25px}#label{font-size:10px}#centralWidget{background:#fff};#mainTab{margin-top;-20px};#label_4{padding:10px}#downloadInfoBox, #settingsGeneralInfoBox, #settingsLanguageInfoBox, #aboutInfoBox, #searchInfoBox{color: background: #00B4DE;}");
        // this->ui.label_4->setMinimumHeight(120);
    #endif
    updateYoutubeDlVersionInfo();

    //*
    //* Drag and Drop
    //*
    this->setAcceptDrops(true);
    this->ui.searchWebEngineView->setAcceptDrops(false);

    //*
    //*Keyboard shortcuts
    //*
    QSignalMapper* tabShortcutMapper = new QSignalMapper(this);

    QShortcut* tabShortcutSearch = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_1), this);
    tabShortcutMapper->setMapping(tabShortcutSearch, 0);
    QShortcut* tabShortcutDownload = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_2), this);
    tabShortcutMapper->setMapping(tabShortcutDownload, 1);
    QShortcut* tabShortcutSettings = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_3), this);
    tabShortcutMapper->setMapping(tabShortcutSettings, 2);
    QShortcut* tabShortcutAbout = new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_4), this);
    tabShortcutMapper->setMapping(tabShortcutAbout, 3);

    connect(tabShortcutSearch, SIGNAL(activated()), tabShortcutMapper, SLOT(map()));
    connect(tabShortcutDownload, SIGNAL(activated()), tabShortcutMapper, SLOT(map()));
    connect(tabShortcutSettings, SIGNAL(activated()), tabShortcutMapper, SLOT(map()));
    connect(tabShortcutAbout, SIGNAL(activated()), tabShortcutMapper, SLOT(map()));
    connect(tabShortcutMapper, SIGNAL(mapped(int)), this->ui.mainTab, SLOT(setCurrentIndex(int)));

    //*
    //*Miscellaneous
    //*
    this->ui.mainTab->setCurrentIndex(cg->settings.value("MainTab", 0).toInt());

    //Prevent updating remembered resolution when updating programatically
    this->updatingComboQuality = false;

    #ifdef Q_OS_MACX
    //fixme
    //if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    //{
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
    //}
    #endif
}

void MainWindow::startDownload() {
    video* video = cg->getCurrentVideo();
    if (video  == nullptr) return;

    QString targetDirectory = cg->settings.value("savedPath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    QString filename = video->getSafeFilename();

    qDebug() << targetDirectory << filename;

    if (cg->settings.value("NeverAskForPath", false).toBool() == false) {
       targetFileSelected(video, QFileDialog::getSaveFileName(this, tr("Select Target"), targetDirectory +"/" + filename));
    } else {
        targetFileSelected(video, targetDirectory + "/" + filename);
    }
}

void MainWindow::targetFileSelected(video* video, QString target)
{
    if (target.isEmpty()) return;

    QDir targetDir = QFileInfo(target).absoluteDir();
    if (!targetDir.exists()) {
        ui.settingsNeverAskForPath->setChecked(false);
        this->startDownload();
        return;
    }

    if (cg->settings.value("saveLastPath", true).toBool() == true) {
        QString targetDir = target;
        targetDir.remove(targetDir.split("/", QString::SkipEmptyParts).last()).replace(QRegExp("/+$"), "/");
        ui.settingsSavedPath->setText(targetDir);
    }

    if (cg->settings.value("UseMetadata", false).toBool() == true) {
        if (ui.downloadComboFormat->currentIndex() == 4 || ui.downloadComboFormat->currentIndex() == 5) {
            metadataDialog = new QDialog;
            mdui.setupUi(metadataDialog);
            mdui.title->setText(video->getTitle());
            mdui.artist->setText(video->getArtist());
            metadataDialog->setModal(true);
            metadataDialog->exec();

            video->setMetaTitle(mdui.title->text());
            video->setMetaArtist(mdui.artist->text());

            delete metadataDialog;
        }
    }

    video->setQuality(ui.downloadComboQuality->currentIndex());
    video->setConverter(cg->formats.at(ui.downloadComboFormat->currentIndex())._converter, cg->formats.at(ui.downloadComboFormat->currentIndex())._mode);
    video->setTargetFilename(target);
    cg->enqueueDownload(video);
    ui.downloadLineEdit->clear();
    cg->clearCurrentVideo();
}

void MainWindow::handleCurrentVideoStateChanged(video* video) {
    if (video != cg->getCurrentVideo()) return;

    if (video == nullptr) {
        ui.downloadInfoBox->setText(tr("Please enter the link to the video you want to download in the field below."));
        disableDownloadUi(true);
        return;
    }

    if (video->getState() == video::state::error) {
        ui.downloadInfoBox->setText(tr("No downloadable video could be found.<br />Maybe you have entered the wrong link or there is a problem with your connection."));
        ui.downloadLineEdit->setReadOnly(false);
        disableDownloadUi(true);
    }

    if (video->getState() != video::state::fetched) return;

    ui.mainTab->setCurrentIndex(1);
    disableDownloadUi(false);
    ui.downloadLineEdit->setReadOnly(false);
    ui.downloadInfoBox->setText("<strong>" + video->getTitle() + "</strong>");

    this->updatingComboQuality = true;
    ui.downloadComboQuality->clear();
    QList<videoQuality> qualities = video->getQualities();
    for (int i = 0; i < qualities.size(); i++) {
        ui.downloadComboQuality->addItem(qualities.at(i).name, qualities.at(i).resolution);
    }

    if (cg->settings.value("rememberVideoQuality", true).toBool()) {
        int rememberedResolution = cg->settings.value("rememberedVideoQuality", -1).toInt();
        int bestResolutionMatch = 0;
        int bestResolutionMatchPosition = 0;
        for (int i = 0; i < qualities.length(); i++) {
            int resolution = qualities.at(i).resolution;
            if (resolution <= rememberedResolution && resolution > bestResolutionMatch) {
                bestResolutionMatch = resolution;
                bestResolutionMatchPosition = i;
            }
        }
        ui.downloadComboQuality->setCurrentIndex(bestResolutionMatchPosition);
    }
    this->updatingComboQuality = false;
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

void MainWindow::on_downloadCancel_clicked() {
    video* selectedVideo = ((DownloadListModel*) ui.downloadTree->model())->getVideo(ui.downloadTree->currentIndex());
    if (selectedVideo == nullptr) return;

    selectedVideo->cancel();
}

void MainWindow::on_downloadPause_clicked() {
    video* selectedVideo = ((DownloadListModel*) ui.downloadTree->model())->getVideo(ui.downloadTree->currentIndex());
    if (selectedVideo == nullptr) return;

    if (selectedVideo->getState() == video::state::paused) {
        selectedVideo->resume();
        return;
    }
    selectedVideo->pause();
}

void MainWindow::on_downloadOpen_clicked() {
    video* selectedVideo = ((DownloadListModel*) ui.downloadTree->model())->getVideo(ui.downloadTree->currentIndex());
    if (selectedVideo == nullptr) return;
    cg->openTargetFolder(selectedVideo);
}

void MainWindow::on_settingsSaveLastPath_stateChanged(int state)
{
    cg->settings.setValue("saveLastPath", state == Qt::Checked);
}

void MainWindow::compatibleUrlFoundInClipBoard(QString url) {
    if (QApplication::activeModalWidget() == nullptr) {
        if (cg->settings.value("Clipboard", "ask") == "ask") {
            Notifications::showMessage(tr("ClipGrab: Video discovered in your clipboard"), tr("ClipGrab has discovered the address of a compatible video in your clipboard. Click on this message to download it now."), &systemTrayIcon);
        } else if (cg->settings.value("Clipboard", "ask") == "always") {
            this->ui.downloadLineEdit->setText(url);
            this->ui.mainTab->setCurrentIndex(1);
            this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
            this->show();
            this->activateWindow();
            this->raise();
        }
    }
}

void MainWindow::systemTrayMessageClicked() {
    if (QApplication::activeModalWidget() == nullptr) {
        this->ui.downloadLineEdit->setText(cg->clipboardUrl);
        this->ui.mainTab->setCurrentIndex(1);
        this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        this->show();
        this->activateWindow();
        this->raise();
    }
}

void MainWindow::systemTrayIconActivated(QSystemTrayIcon::ActivationReason) {
    if (cg->settings.value("MinimizeToTray", false).toBool() && !this->isHidden()) {
        this->hide();
    } else {
        this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        this->show();
        this->activateWindow();
        this->raise();
    }
}

void MainWindow::settingsClipboard_toggled(bool)
{
    if (ui.settingsRadioClipboardAlways->isChecked()) {
        cg->settings.setValue("Clipboard", "always");
    } else if (ui.settingsRadioClipboardNever->isChecked()) {
        cg->settings.setValue("Clipboard", "never");
    } else if (ui.settingsRadioClipboardAsk->isChecked()) {
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

void MainWindow::handle_downloadTree_currentChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    video* selectedVideo = ((DownloadListModel*) ui.downloadTree->model())->getVideo(current);
    if (selectedVideo == nullptr) {
        disableDownloadTreeButtons();
    } else {
        disableDownloadTreeButtons(false);
        if (selectedVideo->getState() == video::state::finished || selectedVideo->getState() == video::state::canceled) {
            ui.downloadCancel->setDisabled(true);
            ui.downloadPause->setDisabled(true);
        }
    }
}

void MainWindow::on_settingsNeverAskForPath_stateChanged(int state)
{
    cg->settings.setValue("NeverAskForPath", state == Qt::Checked);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (cg->getRunningDownloadsCount() > 0) {
        QMessageBox* exitBox;
        exitBox = new QMessageBox(QMessageBox::Question, tr("ClipGrab - Exit confirmation"), tr("There is still at least one download in progress.<br />If you exit the program now, all downloads will be canceled and cannot be recovered later.<br />Do you really want to quit ClipGrab now?"), QMessageBox::Yes|QMessageBox::No);
        if (exitBox->exec() == QMessageBox::Yes) {
            systemTrayIcon.hide();
            connect(cg, &ClipGrab::allDownloadsCanceled, [event] {
                event->accept();
            });
            cg->cancelAllDownloads();
        } else {
            event->ignore();
        }
    } else {
        systemTrayIcon.hide();
        event->accept();
    }
}

void MainWindow::settingsNotifications_toggled(bool) {
    if (ui.settingsRadioNotificationsAlways->isChecked()) {
        cg->settings.setValue("Notifications", "always");
    } else if (ui.settingsRadioNotificationsFinish->isChecked()) {
        cg->settings.setValue("Notifications", "finish");
    } else if (ui.settingsRadioNotificationsNever->isChecked()) {
        cg->settings.setValue("Notifications", "never");
    }
}

void MainWindow::handleFinishedConversion(video* finishedVideo) {
     if (cg->settings.value("Notifications", "finish") == "always") {
         Notifications::showMessage(tr("Download finished"), tr("Downloading and converting “%title” is now finished.").replace("%title", finishedVideo->getTitle()), &systemTrayIcon);
     } else if (cg->getRunningDownloadsCount() == 0 && cg->settings.value("Notifications", "finish") == "finish") {
         Notifications::showMessage(tr("All downloads finished"), tr("ClipGrab has finished downloading and converting all selected videos."), &systemTrayIcon);
     }
}

void MainWindow::on_settingsRemoveFinishedDownloads_stateChanged(int state) {
    cg->settings.setValue("RemoveFinishedDownloads", state == Qt::Checked);
}

void MainWindow::settingsProxyChanged() {
    cg->settings.setValue("UseProxy", ui.settingsUseProxy->isChecked());
    cg->settings.setValue("ProxyHost", ui.settingsProxyHost->text());
    cg->settings.setValue("ProxyPort", ui.settingsProxyPort->value());
    cg->settings.setValue("ProxyAuthenticationRequired", ui.settingsProxyAuthenticationRequired->isChecked());
    cg->settings.setValue("ProxyPassword", ui.settingsProxyPassword->text());
    cg->settings.setValue("ProxyUsername", ui.settingsProxyUsername->text());
    cg->settings.setValue("ProxyType", ui.settingsProxyType->currentIndex());
    ui.settingsProxyGroup->setEnabled(ui.settingsUseProxy->isChecked());
    ui.settingsProxyAuthenticationRequired->setEnabled(ui.settingsUseProxy->isChecked());

    ui.settingsProxyAuthenticationGroup->setEnabled(ui.settingsUseProxy->isChecked() && ui.settingsProxyAuthenticationRequired->isChecked());
    cg->activateProxySettings();
}

void MainWindow::timerEvent(QTimerEvent *)
{
//    QPair<qint64, qint64> downloadProgress = cg->downloadProgress();
//    if (downloadProgress.first != 0 && downloadProgress.second != 0)
//    {
//        #ifdef Q_WS_X11
//            systemTrayIcon.setToolTip("<strong style=\"font-size:14px\">" + tr("ClipGrab") + "</strong><br /><span style=\"font-size:13px\">" + QString::number(downloadProgress.first*100/downloadProgress.second) + " %</span><br />" + QString::number((double)downloadProgress.first/1024/1024, QLocale::system().decimalPoint().toAscii(), 1) + tr(" MiB") + "/" + QString::number((double)downloadProgress.second/1024/1024, QLocale::system().decimalPoint().toAscii(), 1) + tr(" MiB"));
//        #else
//        systemTrayIcon.setToolTip(tr("ClipGrab") + " - " + QString::number(downloadProgress.first*100/downloadProgress.second) + " % - " + QString::number((double)downloadProgress.first/1024/1024, QLocale::system().decimalPoint().toLatin1(), 1) + tr(" MiB") + "/" + QString::number((double)downloadProgress.second/1024/1024, QLocale::system().decimalPoint().toLatin1(), 1) + tr(" KiB"));
//        #endif
//        setWindowTitle("ClipGrab - " + QString::number(downloadProgress.first*100/downloadProgress.second) + " %");
//    }
//    else
//    {
//        #ifdef Q_WS_X11
//            systemTrayIcon.setToolTip("<strong style=\"font-size:14px\">" + tr("ClipGrab") + "</strong><br /><span style=\"font-size:13px\">" + tr("Currently no downloads in progress."));
//        #endif
//        systemTrayIcon.setToolTip(tr("ClipGrab") + tr("Currently no downloads in progress."));
//        setWindowTitle(tr("ClipGrab - Download and Convert Online Videos"));
//    }
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && cg->settings.value("MinimizeToTray", false).toBool()) {
            QTimer::singleShot(500, this, SLOT(hide()));
            event->ignore();
        }
    }
}

void MainWindow::on_settingsMinimizeToTray_stateChanged(int state) {
    cg->settings.setValue("MinimizeToTray", state == Qt::Checked);
}


void MainWindow::on_downloadLineEdit_returnPressed() {
    startDownload();
}

void MainWindow::on_label_linkActivated(QString link)
{
    QDesktopServices::openUrl(link);
}

void MainWindow::on_settingsUseMetadata_stateChanged(int state)
{
    cg->settings.setValue("UseMetadata", state == Qt::Checked);
}

void MainWindow::on_searchLineEdit_textChanged(QString keywords)
{
    if (keywords.startsWith("https://") || keywords.startsWith("http://")) {
        ui.mainTab->setCurrentIndex(1);
        ui.downloadLineEdit->setText(keywords);
        return;
    }

    bool isTimerActive = searchTimer.isActive();
    searchTimer.stop();
    searchTimer.setSingleShot(true);
    searchTimer.start(1500);

    if (isTimerActive) return;
    this->ui.searchWebEngineView->page()->load(QUrl("qrc:///search/loading-progress.html"));
}

void MainWindow::updateSearch(QString keywords) {
    if (this->ui.searchWebEngineView->page()->url().toString() != "qrc:///search/loading-progress.html") {
        this->ui.searchWebEngineView->page()->load(QUrl("qrc:///search/loading-progress.html"));
    }
    cg->search(keywords);
}

void MainWindow::handleSearchResults(video* searchPlaylist)
{
    QList<video*> videos = searchPlaylist->getPlaylistVideos();

    QString searchHtml;
    #ifdef Q_OS_MAC
        QString fontFamily = "Helvetica Neue";
    #else
        QFontDatabase fontDatabase;
        QString font = fontDatabase.systemFont(QFontDatabase::GeneralFont).family();
        QString fontFamily = "'" + font + "',  sans-serif";
    #endif
    searchHtml.append("<!doctype html>");
    searchHtml.append("<html>");
    searchHtml.append("<head>");
    searchHtml.append("<style>body {font-family: " + fontFamily + "}</style>");
    searchHtml.append("<link rel=\"stylesheet\" href=\"qrc:///search/search-styles.css\"></link>");
    searchHtml.append("</head>");
    searchHtml.append("<body>");

    for (int i = 0; i < videos.length(); i++) {
        QString link = videos.at(i)->getUrl();
        QString title = videos.at(i)->getTitle();
        QString thumbnail = videos.at(i)->getThumbnail();
        QString duration = cg->humanizeSeconds(videos.at(i)->getDuration());
        searchHtml.append("<a href=\"" + link + "\" class=\"entry\">");
        searchHtml.append("<span class=\"title\">" + title + "</span>");
        searchHtml.append("<span class=\"thumbnail\" style=\"background-image: url('" + thumbnail + "')\"</span>");
        if (!duration.isEmpty()) {
            searchHtml.append("<span class=\"duration\">" + duration + "</span>");
        }
        searchHtml.append("</a>");
    }

    searchHtml.append("</body>");
    searchHtml.append("</html>");
    if (videos.length() == 0) {
        QFile loadingFailedHTML(":/search/loading-failed.html");
        loadingFailedHTML.open(QFile::ReadOnly);
        searchHtml = QString(loadingFailedHTML.readAll()).replace("%NORESULTS%", tr("No results found."));
    }
    this->ui.searchWebEngineView->page()->setHtml(searchHtml);
}

void MainWindow::handleSearchResultClicked(const QUrl & url)
{
    this->ui.downloadLineEdit->setText(url.toString());
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

void MainWindow::on_settingsIgnoreSSLErrors_toggled(bool checked)
{
    cg->settings.setValue("IgnoreSSLErrors", checked);
}

void MainWindow::on_downloadTree_customContextMenuRequested(const QPoint &point)
{
    QModelIndex i = ui.downloadTree->indexAt(point);
    if (!i.isValid()) return;

    video* selectedVideo = ((DownloadListModel*) ui.downloadTree->model())->getVideo(i);
    if (selectedVideo == nullptr) return;

    QMenu contextMenu;
    QAction* openDownload = contextMenu.addAction(tr("&Open downloaded file"));
    QAction* openFolder = contextMenu.addAction(tr("Open &target folder"));
    contextMenu.addSeparator();
    QAction* pauseDownload = contextMenu.addAction(tr("&Pause download"));
    QAction* restartDownload = contextMenu.addAction(tr("&Restart download"));
    QAction* cancelDownload = contextMenu.addAction(tr("&Cancel download"));
    contextMenu.addSeparator();
    QAction* copyLink = contextMenu.addAction(tr("Copy &video link"));
    QAction* openLink = contextMenu.addAction(tr("Open video link in &browser"));

    if (selectedVideo->getState() == video::state::paused) {
        pauseDownload->setText(tr("Resume download"));
    }

    if (selectedVideo->getState() == video::state::canceled) {
        contextMenu.removeAction(pauseDownload);
        contextMenu.removeAction(cancelDownload);
    }

    if (selectedVideo->getState() == video::state::finished) {
        contextMenu.removeAction(pauseDownload);
        contextMenu.removeAction(restartDownload);
        contextMenu.removeAction(cancelDownload);
        #ifdef Q_OS_MAC
            openFolder->setText(tr("Show in &Finder"));
        #endif
    } else {
        contextMenu.removeAction(openDownload);
    }


    QAction* selectedAction = contextMenu.exec(ui.downloadTree->mapToGlobal(point));
    if (selectedAction == restartDownload) {
        selectedVideo->restart();
    } else if (selectedAction == cancelDownload) {
        selectedVideo->cancel();
    } else if (selectedAction == pauseDownload) {
        if (selectedVideo->getState() == video::state::paused) {
            selectedVideo->resume();
        } else {
            selectedVideo->pause();
        }
    } else if (selectedAction == openFolder) {
        cg->openTargetFolder(selectedVideo);
    } else if (selectedAction == openDownload) {
        cg->openDownload(selectedVideo);
    } else if (selectedAction == openLink) {
        QString link = selectedVideo->getUrl();
        QDesktopServices::openUrl(QUrl(link));
    } else if (selectedAction == copyLink) {
        QApplication::clipboard()->setText(selectedVideo->getUrl());
    }

    contextMenu.deleteLater();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    ui.downloadLineEdit->setText(event->mimeData()->text());
    ui.mainTab->setCurrentIndex(1);
}

void MainWindow::updateYoutubeDlVersionInfo() {
    QString youtubeDlVersion = YoutubeDl::getVersion();
    QString pythonVersion = YoutubeDl::getPythonVersion();
    QString youtubeDlPath = YoutubeDl::find();
    QString pythonPath = YoutubeDl::findPython();
    QString label = tr("youtube-dl: %1 (%2)\nPython: %3 (%4)")
            .arg(youtubeDlPath)
            .arg(youtubeDlVersion)
            .arg(pythonPath)
            .arg(pythonVersion);
    ui.labelYoutubeDlVersionInfo->setText(label);
};

void MainWindow::on_settingsRememberLogins_toggled(bool /*checked*/)
{
//    cg->settings.setValue("rememberLogins", checked);
//    cg->settings.setValue("youtubeRememberLogin", checked);
//    cg->settings.setValue("facebookRememberLogin", checked);
//    cg->settings.setValue("vimeoRememberLogin", checked);
//    if (!checked)
//    {
//        cg->settings.remove("youtubeCookies");
//        cg->settings.remove("facebookCookies");
//        cg->settings.remove("vimeoCookies");
//    }
}

void MainWindow::on_settingsRememberVideoQuality_toggled(bool checked)
{
    cg->settings.setValue("rememberVideoQuality", checked);
}

void MainWindow::on_downloadComboQuality_currentIndexChanged(int index)
{
    if (this->updatingComboQuality || !cg->settings.value("rememberVideoQuality", true).toBool() || cg->getCurrentVideo() == nullptr) {
        return;
    }

    cg->settings.setValue("rememberedVideoQuality", ui.downloadComboQuality->itemData(index, Qt::UserRole).toInt());
}

void MainWindow::searchTimerTimeout()
{
    updateSearch(this->ui.searchLineEdit->text());
}

void MainWindow::on_downloadTree_doubleClicked(const QModelIndex &index)
{
    video* video = ((DownloadListModel*) ui.downloadTree->model())->getVideo(index);
    if (video == nullptr) return;
    cg->openDownload(video);
}


void MainWindow::on_settingsForceIpV4_toggled(bool checked) {
    cg->settings.setValue("forceIpV4", checked);
}
