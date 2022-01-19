#ifndef API_H
#define API_H

#include <curl/curl.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <sstream>
#include <vector>
#include "network.h"
#include "parser.h"
#include <QObject>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <QThread>

using namespace curlpp::options;

class API : public QObject {
    Q_OBJECT
public:
    static API* getInstance(std::string t_code="");
    /* Funzioni sui calendari */
    void getCalendars();
    void insertCalendar(std::string summary);
    void deleteCalendar(std::string calendar_id);
    void shareCalendar(std::string calendar_id, std::string user_email);
    /* Funzioni sugli eventi */
    void getEventsByCalendarDay(std::string calendar_id, std::string dateMin, std::string dateMax);
    void insertEvent(std::string calendar_id, std::string summary, std::string begin_date, std::string end_date);
    void updateEvent(std::string calendar_id, std::string event_id, std::string new_summary, std::string start_date, std::string end_date);
    void deleteEvent(std::string calendar_id, std::string event_id);
    /* Funzioni sui ToDo */
    void getToDos(std::string calendar_id);
    void insertToDo(std::string calendar_id, std::string summary);
    void updateToDo(std::string calendar_id, std::string event_id, std::string new_summary);
    void deleteToDo(std::string calendar_id, std::string event_id);

    void getToDoEvents(std::string calendar_id,std::string dateMin,std::string dateMax);

public slots:
    void responseSlot(std::string response);
    void refreshTokenSlot();

signals:
    void signalCalendar(Calendar* calendar);
    void signalCalendarList(std::vector<Calendar*> calendarList);
    void signalEvent(Event* event);
    void signalEventList(std::vector<Event*> eventList);
    void signalToDoList(std::vector<Event*> eventList);
    void signalToDo(Event* event);
    void signalToDoListEventList(std::pair<std::vector<Event*>,std::vector<Event*>> ); //pair.first=todos, pair.second=events
    void signalEventDeleted(Event* event);      //parametri vuoti
    void signalToDoDeleted(Event* event);       //stesso di sopra
    void signalCalendarDeleted(Calendar* c);    //per triggerare getCalendar() dopo l'eliminazione


private:
    static API *instance;
    enum ResultType {CALENDAR_LIST, CALENDAR, EVENT_LIST, EVENT, TODO_LIST, TODO, EMPTY,
                     DELETE_EVENT, DELETE_TODO, DELETE_CALENDAR, REFRESH, TODOEVENTS_PARTIAL, TODOEVENTS_FINAL};
    ResultType resultType;
    std::string a_code;
    std::string r_token;
    std::string exp_in;
    Network *network;
    Parser *parser;
    std::vector<Event*> todos;
    std::string dateMin;
    std::string dateMax;
    std::string calendar_id;
    std::mutex m; //lock su exp_in per gestire il refresh
    std::condition_variable cv; //usata dal thread per attendere finché non avrò ottenuto il nuovo exp_in
    /* Metodi privati */
    API(std::string t_code,QObject *parent = 0); //costruttore privato
    std::vector<std::string> split(std::string s, std::string delimiter);
    void refreshToken(API* api);
    void returnCalendarList(std::string response);
    void returnCalendar(std::string response);
    void returnEventList(std::string response);
    void returnEvent(std::string response);
    void returnToDoList(std::string response);
    void returnToDo(std::string response);
    void refreshParams(std::string response);
    void returnEventDeleted();
    void returnToDoDeleted();
    void returnCalendarDeleted();
    void insertGenericEvent(std::string calendar_id,std::string summary,std::string description,std::string begin_date,std::string end_date);
    void returnToDoListEventListPartial(std::string response);
    void returnToDoListEventListFinal(std::string response);
};

//Thread per la gestione del refresh del token
class APIThread : public QThread {
    Q_OBJECT
public:
    void setCV(std::condition_variable *cv);
    void setM(std::mutex *m);
    void setExpIn(std::string *exp_in);
signals:
    void signalRefresh();
private:
    void run();
    std::mutex *m;
    std::condition_variable *cv;
    std::string *exp_in_thread;
};


#endif // API_H
