#include "parser.h"

Parser::Parser(){

}

Calendar* Parser::toCalendar(std::string jsonString){
    Calendar *ret = new Calendar();
    QJsonObject json;
    QString qstring = QString::fromStdString(jsonString);
    QJsonDocument doc = QJsonDocument::fromJson(qstring.toUtf8());
    if(!doc.isNull())
        json=doc.object();
    else {
        std::cout<<"Document is null"<<std::endl;
    }
    //costruisco ret
    auto idFuture = std::async(&Parser::getString,this,json,"id");
    auto summaryFuture = std::async(&Parser::getString,this,json,"summary");
    //prendo id
    ret->setId(idFuture.get());
    //prendo summary
    ret->setSummary(summaryFuture.get());
    return ret;
}

std::vector<Calendar*> Parser::toCalendarList(std::string jsonString){
    std::vector<Calendar*> ret;
    QJsonObject json;
    QString qstring = QString::fromStdString(jsonString);
    QJsonDocument doc = QJsonDocument::fromJson(qstring.toUtf8());
    if(!doc.isNull())
        json=doc.object();
    else {
        std::cout<<"Document is null"<<std::endl;
    }
    //prendo items
    if(json.contains("items") && json["items"].isArray()){
        QJsonArray items=json["items"].toArray();
        for(int i=0;i<items.size();i++){
            QJsonValue calendar_as_qjsonvalue = items.at(i);
            QJsonObject calendar_as_object = calendar_as_qjsonvalue.toObject();
            auto document_ptr = std::unique_ptr<QJsonDocument>(new QJsonDocument(calendar_as_object));
            Calendar* calendar = toCalendar(document_ptr->toJson().toStdString());
            ret.push_back(calendar);
        }
    }
    else{
        std::cout<<"Wrong format from calendar list\n"+jsonString<<std::endl;
    }
    return ret;
}

Event* Parser::toEvent(std::string jsonString){
    Event *ret=new Event();
    QJsonObject json;
    QString qstring = QString::fromStdString(jsonString);
    QJsonDocument doc = QJsonDocument::fromJson(qstring.toUtf8());
    if(!doc.isNull())
        json=doc.object();
    else {
        std::cout<<"Document is null"<<std::endl;
    }
    //costruisco ret
    auto idFuture = std::async(&Parser::getString,this,json,"id");
    auto summaryFuture = std::async(&Parser::getString,this,json,"summary");
    auto descriptionFuture = std::async(&Parser::getString,this,json,"description");
    auto startFuture = std::async(&Parser::getFieldFromObject,this,json,"start","dateTime");
    auto endFuture = std::async(&Parser::getFieldFromObject,this,json,"end","dateTime");
    //prendo id
    ret->setId(idFuture.get());
    //prendo summary
    ret->setSummary(summaryFuture.get());
    //prendo description
    ret->setDescription(descriptionFuture.get());
    //prendo start date
    ret->setStartDate(startFuture.get());
    //prendo end date
    ret->setEndDate(endFuture.get());
    return ret;
}

std::string Parser::getString(QJsonObject json, std::string key){
    if(json.contains(key.c_str()) && json[key.c_str()].isString())
        return json[key.c_str()].toString().toStdString();
    return "Error in parser";
}

std::string Parser::getDouble(QJsonObject json, std::string key){
    if(json.contains(key.c_str()) && json[key.c_str()].isDouble())
        return std::to_string(json[key.c_str()].toDouble());
    return "Error in parser";
}

std::string Parser::getFieldFromObject(QJsonObject json, std::string object_key, std::string field_key){
    if(json.contains(object_key.c_str()) && json[object_key.c_str()].isObject()){
        QJsonObject o = json[object_key.c_str()].toObject();
        if(o.contains(field_key.c_str()) && o[field_key.c_str()].isString()){
            std::string date = o[field_key.c_str()].toString().toStdString();
            return date;
        }
    }
    return "Error in parser";
}

std::vector<Event*> Parser::toFullEventList(std::string jsonString){
    //restituisce un vettore con sia event che todo
    std::vector<Event*> ret;
    QJsonObject json;
    QString qstring = QString::fromStdString(jsonString);
    QJsonDocument doc = QJsonDocument::fromJson(qstring.toUtf8());
    if(!doc.isNull())
        json=doc.object();
    else {
        std::cout<<"Document is null"<<std::endl;
    }
    //prendo items
    if(json.contains("items") && json["items"].isArray()){
        QJsonArray items=json["items"].toArray();
        for(int i=0;i<items.size();i++){
            QJsonValue event_as_qjsonvalue = items.at(i);
            QJsonObject event_as_object = event_as_qjsonvalue.toObject();
            auto document_ptr = std::unique_ptr<QJsonDocument>(new QJsonDocument(event_as_object));
            Event* event = toEvent(document_ptr->toJson().toStdString());
            ret.push_back(event);
        }
    }
    else{
        std::cout<<"Wrong format full event list\n"+jsonString<<std::endl;    }
    return ret;
}

std::vector<Event*> Parser::toEventList(std::string jsonString){
    std::vector<Event*> ret = toFullEventList(jsonString);
    //se la data degli eventi è 1 Gennaio 1901 devo filtrarli in modo da rimuovere eventuali "todo"
    if(ret.size()!=0 && ret.at(0)->getStartDate().compare("1901-01-01")==0){
        ret.erase(std::remove_if(ret.begin(),ret.end(),[](Event* x){return x->getDescription().compare("todo")==0;}),ret.end());
    }
    return ret;
}

std::vector<Event*> Parser::toToDoList(std::string jsonString){
    std::vector<Event*> ret=toFullEventList(jsonString);
    //rimuovo eventuali "event" in modo da avere solo "todo"
    ret.erase(std::remove_if(ret.begin(),ret.end(),[](Event* x){return x->getDescription().compare("event")==0;}),ret.end());
    return ret;
}

std::pair<std::string,std::string> Parser::getRefreshParams(std::string jsonString){
    std::pair<std::string,std::string> ret;
    QJsonObject json;
    QString qstring = QString::fromStdString(jsonString);
    QJsonDocument doc = QJsonDocument::fromJson(qstring.toUtf8());
    if(!doc.isNull())
        json=doc.object();
    else {
        std::cout<<"Document is null"<<std::endl;
    }
    auto accessTokenFuture = std::async(&Parser::getString,this,json,"access_token");
    auto expiresInFuture = std::async(&Parser::getDouble,this,json,"expires_in");
    ret.first=accessTokenFuture.get();
    ret.second=expiresInFuture.get();
    return ret;
}
