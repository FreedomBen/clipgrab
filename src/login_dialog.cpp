#include "login_dialog.h"
#include "ui_login_dialog.h"

LoginDialog::LoginDialog(QString key, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    disconnect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(handleRejected()));
    connect(this, SIGNAL(accepted()), this, SLOT(handleAccepted()));
/*
    //Use off-the-record profile
    page = new QWebEnginePage(new QWebEngineProfile);
    page->profile()->setHttpUserAgent("Mozilla/5.0 (Windows NT 10.0; rv:68.0) Gecko/20100101 Firefox/68.0");
    page->setAudioMuted(true);
    ui->loginDialogWebEngineView->setPage(page);
    connect(page->profile()->cookieStore(), SIGNAL(cookieAdded(QNetworkCookie)), this, SLOT(handleCookieAdded(QNetworkCookie)));
    connect(ui->loginDialogWebEngineView, SIGNAL(urlChanged(QUrl)), this, SLOT(emitUrlChanged(QUrl)));
    ui->loginDialogWebEngineView->setFocus();

    QSettings settings;
    ui->rememberLogin->setChecked(settings.value(key + "RememberLogin", true).toBool());
    this->key = key;*/
}

LoginDialog::~LoginDialog()
{
    delete page;
    delete ui;
}

QList<QNetworkCookie> LoginDialog::getCookies()
{
    return this->cookieJar.cookiesForUrl(ui->loginDialogWebEngineView->page()->url());
}

void LoginDialog::setUrl(const QUrl url)
{
    this->url = url;
    ui->loginDialogWebEngineView->page()->setUrl(url);
}

void LoginDialog::handleCookieAdded(const QNetworkCookie cookie)
{
    this->cookieJar.insertCookie(cookie);
    emit cookieAdded(ui->loginDialogWebEngineView->page()->url());
}

void LoginDialog::emitUrlChanged(const QUrl url)
{
    emit urlChanged(url);
}

void LoginDialog::handleAccepted() {

    QSettings settings;
    bool rememberLogin = ui->rememberLogin->isChecked();
    settings.setValue(key + "RememberLogin", rememberLogin);
    if (ui->rememberLogin->isChecked()) {
        QUrl url = ui->loginDialogWebEngineView->page()->url() ;
        settings.setValue(key + "Cookies", http_handler::serializeCookies(cookieJar.cookiesForUrl(ui->loginDialogWebEngineView->page()->url())));
    }
}

void LoginDialog::handleRejected() {
    //Don't reject dialog when Return key is pressed in WebEngineView
    if (ui->loginDialogWebEngineView->isAncestorOf(QApplication::focusWidget())) {
        return;
    }
    this->reject();
}
