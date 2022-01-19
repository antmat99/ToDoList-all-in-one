#include "network.h"

Network::Network(QObject *parent) : QObject(parent)
{
    manager=new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(manageReply(QNetworkReply*)));
}//costruttore

Network::~Network(){
    delete manager;
    manager=nullptr;
}//distruttore

void Network::makeGetRequest(QString endpoint){
    QNetworkRequest request;
    request.setUrl(endpoint);
    manager->get(request);
}//makeGetRequest

void Network::makePostRequest(QString endpoint, QByteArray postData){
    QNetworkRequest request;
    request.setUrl(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"text/plain");
    manager->post(QNetworkRequest(QUrl(endpoint)),postData);
}//makePostRequest

void Network::makePutRequest(QString endpoint, QByteArray putData){
    QNetworkRequest request;
    request.setUrl(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"text/plain");
    manager->put(QNetworkRequest(QUrl(endpoint)),putData);
}//makeUpdateRequest

void Network::makeDeleteRequest(QString endpoint){
    QNetworkRequest request;
    request.setUrl(endpoint);
    manager->deleteResource(request);
}//makeDeleteRequest

/* Slot */

void Network::manageReply(QNetworkReply *reply){
    QByteArray myData=reply->readAll();
    std::string result=myData.toStdString();
    reply->close();
    reply->deleteLater();
    emit networkResponse(result); //verrà preso da API
}//slot manageReply


