#ifndef GITLABSERVICE_H
#define GITLABSERVICE_H

#include <QObject>

class QHttpServer;
class QNetworkAccessManager;
class GitLabService : public QObject
{
    Q_OBJECT
public:
    explicit GitLabService(QObject *parent = nullptr);

public slots:
    void listen(QHttpServer *server = nullptr);

signals:

private slots:
    void sendWechatMessage(const QByteArray &data);
    void processPush(const QJsonObject &json);

private:
    QString m_key;
    QByteArray m_token;
    QNetworkAccessManager *manager;
};

#endif // GITLABSERVICE_H
