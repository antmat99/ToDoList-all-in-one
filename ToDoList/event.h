#ifndef EVENT_H
#define EVENT_H

#include <iostream>

/*
Mancano i campi relativi alla condivisione dell'evento tra più utenti
*/

class Event {
public:
    Event();
    Event(std::string id, std::string summary, std::string description, std::string startDate, std::string endDate);
    std::string getId();
    std::string getSummary();
    std::string getDescription(); //Event o Todo: possiamo usare questo campo per distinguerli
    std::string getStartDate();
    std::string getEndDate();
    void setId(std::string id);
    void setSummary(std::string summary);
    void setDescription(std::string description);
    void setStartDate(std::string startDate);
    void setEndDate(std::string endDate);
    std::string toString();
private:
    std::string id;
    std::string summary; //titolo dell'evento
    std::string description; //Event o Todo: possiamo usare questo campo per distinguerli
    std::string startDate;
    std::string endDate;

};

#endif // EVENT_H
