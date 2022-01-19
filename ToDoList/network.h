#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <iostream>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Network : public QObject {
    Q_OBJECT
public:
    explicit Network(QObject *parent = 0);
    void makeGetRequest(QString endpoint);
    void makePostRequest(QString endpoint, QByteArray postData);
    void makePutRequest(QString endpoint, QByteArray putData);
    void makeDeleteRequest(QString endpoint);

signals:
    void networkResponse(std::string response); //viene emesso per essere preso da API

public slots:
    void manageReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    ~Network();
};

#endif // NETWORK_H
