#include "calendar.h"

Calendar::Calendar()
{

}

Calendar::Calendar(std::string id, std::string summary){
    this->id=id;
    this->summary=summary;
}

std::string Calendar::getId(){
    return id;
}

std::string Calendar::getSummary(){
    return summary;
}

void Calendar::setId(std::string id){
    this->id=id;
}

void Calendar::setSummary(std::string summary){
    this->summary=summary;
}

std::string Calendar::toString(){
    return "Calendar id: "+id+", title: "+summary;
}

