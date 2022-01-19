#include "api.h"

API* API::instance=nullptr;

bool passThread=false;

API* API::getInstance(std::string t_code){
    if(instance==nullptr)
        instance=new API(t_code);
    return instance;
}//getInstance

API::API(std::string t_code,QObject *parent) : QObject(parent){
    network=new Network();
    parser=new Parser();
    //connect per ottenere le risposte da network
    connect(network,SIGNAL(networkResponse(std::string)),this,SLOT(responseSlot(std::string)));
    //POST per ottenere a_code, exp_in e r_token
    try {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;
        //imposto i parametri
        request.setOpt(new curlpp::options::Url(std::string("https://oauth2.googleapis.com/token?code="+t_code+"&client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&client_secret=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&grant_type=authorization_code&redirect_uri=http://localhost:8080&code_verifier=1111111111111111111111111111111111111111111").c_str()));
        request.setOpt(new curlpp::options::Verbose(true));
        std::list<std::string> header;
        header.push_back("Content-Type: text/plain");
        //imposto header
        request.setOpt(new curlpp::options::HttpHeader(header));
        //specifico che il body è vuoto
        request.setOpt(new curlpp::options::PostFields("\r"));
        request.setOpt(new curlpp::options::PostFieldSize(0));
        //metto da parte la risposta che otterrò
        std::ostringstream body;
        //il body della risposta alla post viene salvato in response
        request.setOpt(new curlpp::options::WriteStream(&body));
        //eseguo post
        request.perform();
        //splitto per prendere i parametri che mi servono
        std::vector<std::string> v = split(body.str(), " ");
        //salvo le var che userò per inserirle nelle varie richieste
        a_code = v[3];      //access_token
        exp_in = v[6];      //expires_in
        r_token = v[9];     //refresh_token
        //elimino "" e , siccome ho splittato il testo da json
        a_code.erase(std::remove(a_code.begin(), a_code.end(), '\"'), a_code.end());
        a_code.erase(std::remove(a_code.begin(), a_code.end(), ','), a_code.end());
        exp_in.erase(std::remove(exp_in.begin(), exp_in.end(), ','), exp_in.end());
        r_token.erase(std::remove(r_token.begin(), r_token.end(), '\"'), r_token.end());
        r_token.erase(std::remove(r_token.begin(), r_token.end(), ','), r_token.end());
    }
    catch ( curlpp::LogicError & e ) {
        std::cout << e.what() << std::endl;
    }
    catch ( curlpp::RuntimeError & e ) {
        std::cout << e.what() << std::endl;
    }
    //avvio il thread per il refresh del token
    APIThread *t=new APIThread;
    t->setM(&m);
    t->setCV(&cv);
    t->setExpIn(&exp_in);
    connect(t,SIGNAL(signalRefresh()),this,SLOT(refreshTokenSlot()));
    t->start();
}//costruttore

std::vector<std::string> API::split (std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;
    while ((pos_end = s.find (delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }
    res.push_back (s.substr (pos_start));
    return res;
}//split

void API::getCalendars(){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/users/me/calendarList?access_token="+a_code);
    resultType=ResultType::CALENDAR_LIST;
    network->makeGetRequest(url.c_str());
}//getCalendars

void API::insertCalendar(std::string summary){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars?access_token="+a_code);
    std::string requestBody = std::string("{\"summary\":\""+summary+"\",\"timeZone\":\"Europe/Rome\"}");
    resultType=ResultType::CALENDAR;
    network->makePostRequest(url.c_str(),QByteArray(requestBody.c_str()));
}//insertCalendar

void API::deleteCalendar(std::string calendar_id){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"?access_token="+a_code);
    resultType=ResultType::DELETE_CALENDAR;
    network->makeDeleteRequest(url.c_str());
}//deleteCalendar

void API::shareCalendar(std::string calendar_id, std::string user_email){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/acl?access_token="+a_code);
    std::string requestBody = std::string("{\"role\":\"owner\",\"scope\":{\"type\":\"user\",\"value\":\""+user_email+"\"}}");
    resultType=ResultType::EMPTY;
    network->makePostRequest(url.c_str(),QByteArray(requestBody.c_str()));
}//shareCalendar

void API::getEventsByCalendarDay(std::string calendar_id, std::string dateMin, std::string dateMax){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events?timeMin="+dateMin+"&timeMax="+dateMax+"&access_token="+a_code);
    resultType=ResultType::EVENT_LIST;
    network->makeGetRequest(url.c_str());
}//getEventsByCalendarDay

void API::insertEvent(std::string calendar_id, std::string summary, std::string begin_date, std::string end_date){

    resultType=ResultType::EVENT;
    insertGenericEvent(calendar_id,summary,"event",begin_date,end_date);
}//insertEvent

void API::deleteEvent(std::string calendar_id, std::string event_id){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events/"+event_id+"?access_token="+a_code);
    resultType = ResultType::DELETE_EVENT;
    network->makeDeleteRequest(url.c_str());
}//deleteEvent

void API::updateEvent(std::string calendar_id, std::string event_id, std::string new_summary, std::string start_date, std::string end_date){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events/"+event_id+"?access_token="+a_code);
    std::string requestBody = std::string("{\"summary\":\""+new_summary+"\",\"description\":\"event\",\"start\":{\"dateTime\":\""+start_date+"\"},\"end\":{\"dateTime\":\""+end_date+"\"}}");
    resultType=ResultType::EVENT;
    network->makePutRequest(url.c_str(),QByteArray(requestBody.c_str()));
}//updateEvent

void API::getToDos(std::string calendar_id){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events?timeMin=1901-01-01T00:00:01Z&timeMax=1901-01-01T23:59:59Z&access_token="+a_code);
    resultType=ResultType::TODO_LIST;
    network->makeGetRequest(url.c_str());
}//getToDos

void API::insertToDo(std::string calendar_id, std::string summary){
    resultType=ResultType::TODO;
    //crea un nuovo evento, metti il campo description = todo e inseriscilo nella data scelta
    insertGenericEvent(calendar_id, summary,"todo","1901-01-01T00:00:01+01:00","1901-01-01T23:59:59+01:00");
}//insertToDo

void API::updateToDo(std::string calendar_id, std::string event_id, std::string new_summary){
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events/"+event_id+"?access_token="+a_code);
    std::string requestBody = std::string("{\"summary\":\""+new_summary+"\",\"description\":\"todo\",\"start\":{\"dateTime\":\"1901-01-01T00:00:01+01:00\"},\"end\":{\"dateTime\":\"1901-01-01T23:59:59+01:00\"}}");
    resultType=ResultType::TODO;
    network->makePutRequest(url.c_str(),QByteArray(requestBody.c_str()));
}//updateTodo

void API::deleteToDo(std::string calendar_id, std::string event_id){
    resultType=ResultType::DELETE_TODO;
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events/"+event_id+"?access_token="+a_code);
    network->makeDeleteRequest(url.c_str());
}//deleteToDo

void API::insertGenericEvent(std::string calendar_id,std::string summary,std::string description,std::string begin_date,std::string end_date){
    //resultType viene impostato in insertEvent o insertToDo
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events?access_token="+a_code);
    std::string requestBody = std::string("{\"summary\":\""+summary+"\",\"description\":\""+description+"\",\"start\":{\"dateTime\":\""+begin_date+"\"},\"end\":{\"dateTime\":\""+end_date+"\"}}");
    network->makePostRequest(url.c_str(),QByteArray(requestBody.c_str()));
}//insertGenericEvent

void API::getToDoEvents(std::string calendar_id,std::string dateMin,std::string dateMax){
    //salvo le date per quando dovrò chiedere gli eventi
    this->dateMax=dateMax;
    this->dateMin=dateMin;
    this->calendar_id=calendar_id;
    //inizio a prendere i todo
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events?timeMin=1901-01-01T00:00:01Z&timeMax=1901-01-01T23:59:59Z&access_token="+a_code);
    resultType=ResultType::TODOEVENTS_PARTIAL;
    network->makeGetRequest(url.c_str());
}

/**************************** Slot e funzioni correlate *********************************/

void API::refreshTokenSlot(){
    std::string url = "https://oauth2.googleapis.com/token?client_id=1034786735866-o2d6ot6i3mvvrvt2rck9qent1a32odnb.apps.googleusercontent.com&client_secret=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&grant_type=refresh_token&refresh_token="+r_token;
    std::string requestBody = std::string("{\"fakeKey\":\"fakeValue\"}"); //altrimenti restituisce bad request
    resultType=ResultType::REFRESH;
    network->makePostRequest(url.c_str(),QByteArray(requestBody.c_str()));
}

void API::responseSlot(std::string response){
    switch(resultType){
    case ResultType::CALENDAR_LIST: returnCalendarList(response); break;
    case ResultType::CALENDAR: returnCalendar(response); break;
    case ResultType::EVENT_LIST: returnEventList(response); break;
    case ResultType::EVENT: returnEvent(response); break;
    case ResultType::TODO_LIST: returnToDoList(response); break;
    case ResultType::TODO: returnToDo(response); break;
    case ResultType::DELETE_EVENT: returnEventDeleted(); break;
    case ResultType::DELETE_TODO: returnToDoDeleted(); break;
    case ResultType::DELETE_CALENDAR: returnCalendarDeleted(); break;
    case ResultType::REFRESH: refreshParams(response); break;
    case ResultType::TODOEVENTS_PARTIAL: returnToDoListEventListPartial(response); break;
    case ResultType::TODOEVENTS_FINAL: returnToDoListEventListFinal(response); break;
    default: break;
    }
}//responseSlot

void API::returnCalendarList(std::string response){
    //parsing di response, conversione a lista di Calendar
    std::vector<Calendar*> ret = parser->toCalendarList(response);
    ret.erase(ret.begin()); ret.erase(ret.begin()); ret.erase(ret.begin());
    //emit di un segnale apposito
    emit signalCalendarList(ret);
}//returnCalendarList

void API::returnCalendar(std::string response){
    //parsing di response, conversione a Calendar
    Calendar* ret = parser->toCalendar(response);
    //emit di un segnale apposito
    emit signalCalendar(ret);
}//returnCalendar

void API::returnEventList(std::string response){
    //parsing di response, conversione a lista di Event
    std::vector<Event*> ret = parser->toEventList(response);
    //emit di un segnale apposito
    emit signalEventList(ret);
}//returnEventList

void API::returnEvent(std::string response){
    //parsing di response, conversione a Event
    Event* ret = parser->toEvent(response);
    //emit di un segnale apposito
    emit signalEvent(ret);
}//returnEvent

void API::returnToDoList(std::string response){
    //parsing di response, conversione a lista di Event
    std::vector<Event*> ret = parser->toToDoList(response);
    //emit di un segnale apposito
    emit signalToDoList(ret);
}//returnToDoList

void API::returnToDo(std::string response){
    //parsing di response, conversione a Event
    Event* ret = parser->toEvent(response);
    //emit di un segnale apposito
    emit signalToDo(ret);
}//returnToDo

void API::returnCalendarDeleted(){
   Calendar *c = nullptr;
   emit signalCalendarDeleted(c);
}

void API::returnToDoDeleted(){
   Event *e = nullptr;
   emit signalToDoDeleted(e);
}

void API::returnEventDeleted(){
   Event *e = nullptr;
   emit signalEventDeleted(e);
}

void API::refreshParams(std::string response){
    std::unique_lock<std::mutex> l(m);
    std::pair<std::string,std::string> params = parser->getRefreshParams(response);
    a_code = params.first;
    exp_in = params.second;
    //sveglia il thread che si occupa dell'attesa
    passThread=true;
    cv.notify_one();
}

void API::returnToDoListEventListPartial(std::string response){
    //salvo i todo
    todos = parser->toToDoList(response);
    //chiedo gli eventi
    std::string url = std::string("https://www.googleapis.com/calendar/v3/calendars/"+calendar_id+"/events?timeMin="+dateMin+"&timeMax="+dateMax+"&access_token="+a_code);
    resultType=ResultType::TODOEVENTS_FINAL;
    network->makeGetRequest(url.c_str());
}

void API::returnToDoListEventListFinal(std::string response){
    //salvo gli eventi
    std::vector<Event*> events = parser->toEventList(response);
    //restituisco todo ed eventi
    std::pair<std::vector<Event*>,std::vector<Event*>> ret = std::make_pair(todos,events);
    emit signalToDoListEventList(ret);
}

/**************************************** Thread ************************************************/

void APIThread::run(){
    while(true){
        std::unique_lock<std::mutex> l(*m);
        //refresh del token a metà di exp_in
        int waitingTimeInSeconds = std::stoi(*exp_in_thread)/2;
        m->unlock();
        std::this_thread::sleep_for(std::chrono::seconds(waitingTimeInSeconds));
        emit signalRefresh();
        m->lock();
        passThread=false;
        cv->wait(l,[](){return passThread;});
    }
}

void APIThread::setCV(std::condition_variable *cv){
    this->cv=cv;
}

void APIThread::setM(std::mutex *m){
    this->m=m;
}

void APIThread::setExpIn(std::string *exp_in){
    this->exp_in_thread=exp_in;
}
