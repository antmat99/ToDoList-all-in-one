#ifndef CALENDAR_H
#define CALENDAR_H

#include <iostream>

class Calendar
{
public:
    Calendar();
    Calendar(std::string id, std::string summary);
    std::string getId();
    std::string getSummary();
    void setId(std::string id);
    void setSummary(std::string summary);
    std::string toString();

private:
    std::string id;
    std::string summary; //titolo del calendario

};

#endif // CALENDAR_H
