#ifndef PARSER_H
#define PARSER_H

#include "calendar.h"
#include "event.h"
#include <vector>
#include <iostream>
#include <QJsonObject>
#include <QString>
#include <QJsonDocument>
#include <QJsonArray>
#include <future>

class Parser
{
public:
    Parser();
    std::vector<Calendar*> toCalendarList(std::string jsonString);
    Calendar* toCalendar(std::string jsonString);
    std::vector<Event*> toEventList(std::string jsonString);
    Event* toEvent(std::string jsonString);
    std::vector<Event*> toToDoList(std::string jsonString);
    std::pair<std::string,std::string> getRefreshParams(std::string jsonString);
private:
    std::vector<Event*> toFullEventList(std::string jsonString);
    std::string getString(QJsonObject json, std::string key);
    std::string getDouble(QJsonObject json, std::string key);
    std::string getFieldFromObject(QJsonObject json, std::string object_key, std::string field_key);
};

#endif // PARSER_H
