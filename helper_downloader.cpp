#include "helper_downloader.h"
#include "ui_helper_downloader.h"

HelperDownloader::HelperDownloader(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelperDownloader)
{
    ui->setupUi(this);
}

HelperDownloader::~HelperDownloader()
{
    delete ui;
}
