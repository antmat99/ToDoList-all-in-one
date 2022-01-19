#include "event.h"

Event::Event()
{

}

Event::Event(std::string id, std::string summary, std::string description, std::string startDate, std::string endDate){
    this->id=id;
    this->summary=summary;
    this->description=description;
    this->startDate=startDate;
    this->endDate=endDate;
}

std::string Event::getId(){
    return id;
}

std::string Event::getSummary(){
    return summary;
}

std::string Event::getDescription(){
    return description;
}

std::string Event::getStartDate(){
    return startDate;
}

std::string Event::getEndDate(){
    return endDate;
}

void Event::setId(std::string id){
    this->id=id;
}

void Event::setSummary(std::string summary){
    this->summary=summary;
}

void Event::setDescription(std::string description){
    this->description=description;
}

void Event::setStartDate(std::string startDate){
    this->startDate=startDate;
}

void Event::setEndDate(std::string endDate){
    this->endDate=endDate;
}

std::string Event::toString(){
    return "Event id: "+id+", title: "+summary+", description: "+description+", startDate: "+startDate+", endDate: "+endDate;
}
