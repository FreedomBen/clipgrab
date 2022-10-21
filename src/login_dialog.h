#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QDesktopServices>
#include <QtNetwork>
#include <QtWebEngineWidgets>
#include "http_handler.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QString key, QWidget *parent = nullptr);
    ~LoginDialog();
    void setUrl(const QUrl url);
    QList<QNetworkCookie> getCookies();

private:
    Ui::LoginDialog *ui;
    QNetworkCookieJar cookieJar;
    QWebEnginePage* page;
    QString key;
    QUrl url;

private slots:
    void handleCookieAdded(const QNetworkCookie);
    void emitUrlChanged(const QUrl url);
    void handleAccepted();
    void handleRejected();

signals:
    void urlChanged(const QUrl url);
    void cookieAdded(const QUrl url);
};

#endif // LOGIN_DIALOG_H
