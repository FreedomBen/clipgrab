#ifndef DOWNLOADLISTMODEL_H
#define DOWNLOADLISTMODEL_H

#include <QObject>
#include <QWidget>
#include <QAbstractItemModel>
#include <video.h>


#include <QVariant>
#include <QVector>
#include "clipgrab.h"


class DownloadListModel : public QAbstractItemModel {
    Q_OBJECT;

public:
    explicit DownloadListModel(ClipGrab* cg, QObject *parent = nullptr);
    ~DownloadListModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    video* getVideo(QModelIndex);

private:
    ClipGrab* cg;
    QList<video*> videos;
    QVector<QString> header {tr("Portal"), tr("Title"), tr("Quality"), tr("Format"), tr("Progress")};
};

#endif // DOWNLOADLISTMODEL_H
