#ifndef MESSAGE_DIALOG_H
#define MESSAGE_DIALOG_H

#include <QDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineProfile>

namespace Ui {
class messageDialog;
}

class MessageDialogWebEnginePage : public QWebEnginePage
{
    Q_OBJECT
public:
    MessageDialogWebEnginePage(QWebEngineProfile* profile, QObject* parent = 0) :  QWebEnginePage(profile, parent)
    {
        this->setAudioMuted(true);
    }
    bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool /*isMainFrame*/)
    {
        if (type == QWebEnginePage::NavigationTypeLinkClicked)
        {
            emit linkClicked(url);
        }
        if (type == QWebEnginePage::NavigationTypeTyped)
        {
            return true;
        }
        return false;
    }
protected:
    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel /*level*/, const QString & /*message*/, int /*lineNumber*/, const QString & /*sourceID*/) {
        //Don't log anything
    }
signals:
    void linkClicked(const QUrl & url);
};

class messageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit messageDialog(QWidget *parent = 0);
    ~messageDialog();
    void setUrl(QUrl url);
    void setLinkPolicy(QString);

private:
    Ui::messageDialog *ui;
    MessageDialogWebEnginePage* page;
    QString linkPolicy;

private slots:
    void handleLink(const QUrl);
};

#endif // MESSAGE_DIALOG_H
