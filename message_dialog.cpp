#include "message_dialog.h"
#include "ui_message_dialog.h"

messageDialog::messageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::messageDialog)
{
    ui->setupUi(this);
    page = new MessageDialogWebEnginePage(new QWebEngineProfile);
    connect(page, SIGNAL(linkClicked(QUrl)), this, SLOT(handleLink(QUrl)));
    ui->webEngineView->setPage(page);
    ui->webEngineView->setAcceptDrops(false);
    ui->webEngineView->setContextMenuPolicy(Qt::NoContextMenu);
    linkPolicy = "open";
}

messageDialog::~messageDialog()
{
    delete ui;
}

void messageDialog::setUrl(QUrl url)
{
    ui->webEngineView->page()->setUrl(url);
}

void messageDialog::setLinkPolicy(QString policy) {
    linkPolicy = policy;
}


void messageDialog::handleLink(QUrl url)
{
    if ((linkPolicy == "open-external" && url.host() == page->url().host()) || linkPolicy == "follow")
    {
        page->setUrl(url);
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
}
