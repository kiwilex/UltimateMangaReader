#ifndef DOWNLOADFILEJOB_H
#define DOWNLOADFILEJOB_H

#include <QSslError>
#include <QtCore>
#include <QtNetwork>

class DownloadFileJob : public QObject
{
    Q_OBJECT

protected:
    QNetworkAccessManager *networkManager;
    QFile file;
    QScopedPointer<QNetworkReply> reply;

public:
    DownloadFileJob(QObject *parent, QNetworkAccessManager *networkManager,
                    const QString &url, const QString &localFilePath);
    ~DownloadFileJob();

    QString url;
    QString originalUrl;
    QString filepath;
    bool isCompleted;
    QString errorString;

    bool await(int timeout = 10000);

signals:
    void completed();
    void downloadError();

public slots:
    void start();
    void restart();
    virtual void downloadFileReadyRead();
    virtual void downloadFileFinished();
    virtual void onSslErrors(const QList<QSslError> &errors);
    virtual void onError(QNetworkReply::NetworkError);
};

#endif  // DOWNLOADFILEJOB_H
