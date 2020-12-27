#ifndef WEB_ENGINE_VIEW_H
#define WEB_ENGINE_VIEW_H

#include <QObject>
#include <QWidget>
#include <QtWebEngineWidgets>

class CGWebEngineView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit CGWebEngineView(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // WEB_ENGINE_VIEW_H
