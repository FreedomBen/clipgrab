#ifndef HELPER_DOWNLOADER_H
#define HELPER_DOWNLOADER_H

#include <QDialog>

namespace Ui {
class HelperDownloader;
}

class HelperDownloader : public QDialog
{
    Q_OBJECT

public:
    explicit HelperDownloader(QWidget *parent = nullptr);
    ~HelperDownloader();

private:
    Ui::HelperDownloader *ui;
};

#endif // HELPER_DOWNLOAD_H
