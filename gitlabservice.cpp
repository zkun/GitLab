#include <QSettings>
#include <QJsonObject>
#include <QtHttpServer>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include "gitlabservice.h"

GitLabService::GitLabService(QObject *parent)
    : QObject{parent}
{
    QSettings setting;
    setting.beginGroup("GitLab");
    m_key = setting.value("key").toString();
    m_token = setting.value("token").toByteArray();
    setting.endGroup();

    if (m_key.isEmpty())
        qDebug() << "GitLab key isEmpty";
    if (m_token.isEmpty())
        qDebug() << "GitLab token isEmpty";

    manager = new QNetworkAccessManager(this);
    manager->setAutoDeleteReplies(true);
}

void GitLabService::listen(QHttpServer *server)
{
    if (server == nullptr)
        return;

    server->route("/gitlab/", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        auto token = request.value("X-Gitlab-Token");
        if (token != m_token)
            return QStringLiteral(u"令牌无效.");

        QJsonObject json = QJsonDocument::fromJson(request.body()).object();
        auto event = json.value("event_name").toString();
        if (event == "push")
            processPush(json);

        return QStringLiteral(u"不支持的事件");
    });
}

void GitLabService::sendWechatMessage(const QByteArray &data)
{
    QNetworkRequest request;
    request.setUrl(QUrl("https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=" + m_key));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.size());

    QNetworkReply *reply = manager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        if (json.value("errcode").toInt())
            qDebug() << json.value("errmsg").toString();
    });
}

void GitLabService::processPush(const QJsonObject &json)
{
    auto msg = json.value("message").toString();
    auto user = json.value("user_name").toString();
    auto project = json.value("project").toObject();
    QJsonArray commits = json.value("commits").toArray();

    auto url = project.value("web_url").toString();
    auto path = project.value("path_with_namespace").toString();

    QJsonArray list;
    for (int index = 0; index < commits.size(); ++index) {
        auto obj = commits.at(index).toObject();
        auto author = obj.value("author").toObject();

        list.append(QJsonObject {
            {"keyname", author.value("name").toString()},
            {"value", obj.value("message").toString()},
        });
    }

    const QJsonObject source {
        {"icon_url", "https://gitlab.com/favicon.ico"},
        {"desc", "GitLab 推送事件"},
    };

    const QJsonObject title {
        {"title", user + " 推送代码到 " + path},
        {"desc", msg},
    };

    const QJsonObject action {
        {"type", 1},
        {"url", url},
    };

    const QJsonObject card {
        {"card_type", "text_notice"},
        {"source", source},
        {"main_title", title},
        {"card_action", action},
        {"horizontal_content_list", list},
    };

    const QJsonObject resp {
        {"msgtype", "template_card"},
        {"template_card", card},
    };

    return sendWechatMessage(QJsonDocument(resp).toJson());
}
