#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MainWindow)
{
    //il vector viene inizializzato con un evento vuoto per allocare spazio
    //e successivamente rimosso, altrimenti crash
    Event *a = nullptr;
    eventi.push_back(a);
    eventi.pop_back();

    toDos.push_back(a);
    toDos.pop_back();

    createPreviewGroupBox();
    createGeneralOptionsGroupBox();
    createTextFormatsGroupBox();

    //inizializzo ciò che serve
    spaceCalendarButtons=new QGroupBox();
    layButtons = new QGridLayout();
    spaceCalendarButtons->setLayout(layButtons);

    spaceTextEditToDo = new QGroupBox(tr("ToDoList: "));
    spaceTextEditTask = new QGroupBox(tr("TaskList: "));
    layTE = new QGridLayout();
    textEdit = new QTextEdit();
    layout = new QGridLayout;

    layTE->addWidget(textEdit);
    layout->addWidget(spaceTextEditTask,0,1);
    layout->addWidget(spaceCalendarButtons,1,0);
    spaceTextEditTask->setLayout(layTE);

    layout->addWidget(previewGroupBox, 0, 0);
    textEdit->setReadOnly(true);

    QPushButton *button2 = new QPushButton("Calendar Preferences");
    button22 = new QPushButton("Hide Preferences");


    layButtons->addWidget(button2, 2,0);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);

    eventList = NULL;

    setWindowTitle(tr("Calendar Widget"));
    api = API::getInstance("");

    createCalendarButtons();

    api->getCalendars();

    //inserisco le varie connect
    connect(button2, SIGNAL(clicked()), this, SLOT(showPreferences()) );
    connect(button22, SIGNAL(clicked()), this, SLOT(hidePreferences()) );
    connect(api, SIGNAL(signalCalendarList(std::vector<Calendar*>)), this, SLOT(showCalendars(std::vector<Calendar*>)));
    connect(api, SIGNAL(signalEventList(std::vector<Event*>)),this,SLOT(showEvents(std::vector<Event*>)));
    connect(api, SIGNAL(signalToDoList(std::vector<Event*>)), this, SLOT(showToDos(std::vector<Event*>)));
    connect(api, SIGNAL(signalToDo(Event*)), this, SLOT(refreshToDo(Event*)) );
    connect(api, SIGNAL(signalEvent(Event*)), this, SLOT(refreshEvents(Event*)) );
    connect(api, SIGNAL(signalEventDeleted(Event*)), this, SLOT(refreshEvents(Event*)));
    connect(api, SIGNAL(signalToDoDeleted(Event*)), this, SLOT(refreshToDo(Event*)));
    connect(api, SIGNAL(signalCalendar(Calendar*)), this, SLOT(refreshCalendar(Calendar*)));
    connect(api, SIGNAL(signalCalendarDeleted(Calendar*)), this, SLOT(refreshCalendar(Calendar*)));

    //avvio il thread per gestire sincronizzazione tra utenti
    RefreshThread *t=new RefreshThread;
    connect(t,SIGNAL(signalRefresh()),this,SLOT(refreshSlot()),Qt::BlockingQueuedConnection);
    t->start();
}


/*----------------------------FUNZIONI----------------------------------------*/

int MainWindow::checkTime(){
    int sizeI = editDataI->toPlainText().toStdString().size();
    int sizeF = editDataF->toPlainText().toStdString().size();

    if(sizeI!=5 || sizeF!=5){
        return -1;
    }
    if(!std::regex_match(editDataI->toPlainText().toStdString(),std::regex("[0-9][0-9]:[0-9][0-9]")) ||
            !std::regex_match(editDataF->toPlainText().toStdString(),std::regex("[0-9][0-9]:[0-9][0-9]"))){
        return -1;
    }
    if(sizeI == 5 && sizeF == 5){
        //controllare che il formato sia HH:mm
        int hI = std::stoi(editDataI->toPlainText().toStdString().substr(0,2));
        int mI = std::stoi(editDataI->toPlainText().toStdString().substr(3,2));
        int hF = std::stoi(editDataF->toPlainText().toStdString().substr(0,2));
        int mF = std::stoi(editDataF->toPlainText().toStdString().substr(3,2));

        if(hI > hF || hI<0 || hI>24 || hF<0 || hF>24 || mI<0 || mI>59 || mF<0 || mF>59 || (hI==hF && mI>mF || (hI==hF && mI==mF))){
            return -1;
        }
        else{
            return 0;
        }
    }
    return 0;
}

int MainWindow::checkTimeUpdate(){
    int sizeI = oraInizioEdit->toPlainText().toStdString().size();
    int sizeF = oraFineEdit->toPlainText().toStdString().size();

    if(sizeI!=5 || sizeF!=5){
        return -1;
    }
    if(!std::regex_match(oraInizioEdit->toPlainText().toStdString(),std::regex("[0-9][0-9]:[0-9][0-9]")) ||
            !std::regex_match(oraFineEdit->toPlainText().toStdString(),std::regex("[0-9][0-9]:[0-9][0-9]"))){
        return -1;
    }
    if(sizeI == 5 && sizeF == 5){
        //controllare che il formato sia HH:mm
        int hI = std::stoi(oraInizioEdit->toPlainText().toStdString().substr(0,2));
        int mI = std::stoi(oraInizioEdit->toPlainText().toStdString().substr(3,2));
        int hF = std::stoi(oraFineEdit->toPlainText().toStdString().substr(0,2));
        int mF = std::stoi(oraFineEdit->toPlainText().toStdString().substr(3,2));

        if(hI > hF || hI<0 || hI>24 || hF<0 || hF>24 || mI<0 || mI>59 || mF<0 || mF>59 || (hI==hF && mI>mF || (hI==hF && mI==mF))){
            return -1;
        }
        else{
            return 0;
        }
    }
    return 0;
}

void MainWindow::createCalendarButtons(){
    QPushButton *sel = new QPushButton("add/remove/share calendar");
    QMenu *menu = new QMenu(this);
    QAction *ac = new QAction(tr("add Calendar"));
    QAction *rc = new QAction(tr("remove Calendar"));
    QAction *sc = new QAction(tr("share Calendar"));
    menu->addAction(ac);
    menu->addAction(rc);
    menu->addAction(sc);
    sel->setMenu(menu);
    layButtons->addWidget(sel,3,0);

    connect(ac, SIGNAL(triggered()), this, SLOT(addNewCalendarSlot()));
    connect(rc,SIGNAL(triggered()), this, SLOT(removeCalendarSlot()));
    connect(sc,SIGNAL(triggered()), this, SLOT(shareCalendarSlot()));
}

void MainWindow::deleteComponents(){
    if(spaceToDoAdd!=NULL){
        delete spaceToDoAdd;
        spaceToDoAdd=NULL;
    }
    if(spaceCalendarAdd!=NULL){
        delete spaceCalendarAdd;
        spaceCalendarAdd=NULL;
    }
    if(spaceCalendarShare!=NULL){
        delete spaceCalendarShare;
        spaceCalendarShare=NULL;
    }
    if(spaceCalendarDelete!=NULL){
        delete spaceCalendarDelete;
        spaceCalendarDelete=NULL;
    }
    if(spaceEventAdd!=NULL){
        delete spaceEventAdd;
        spaceEventAdd=NULL;
    }
}

void MainWindow::modal(){
    if(spaceEventInfo != NULL){
        spaceEventInfo->setVisible(false);
    }
    spaceEventInfo = new QGroupBox(tr("Task info: "));
    QPushButton *modifyEv = new QPushButton("Edit me");
    QPushButton *hideInfo = new QPushButton("Hide info");
    QPushButton *deleteEv = new QPushButton("Delete me");
    /* if(l2==nullptr){*/
    l2 = new QGridLayout;
    spaceEventInfo->setLayout(l2);
    layout->addWidget(spaceEventInfo,0,2);
    QLabel *nomeE = new QLabel("Task name: ");
    titolo = new QLabel();
    titolo->setText(eventTmp->getSummary().c_str());
    QLabel *dataI = new QLabel("Starting at: ");
    QLabel *dataI2 = new QLabel();
    dataI2->setText(eventTmp->getStartDate().substr(11,5).c_str());
    QLabel *dataF = new QLabel("Ending at: ");
    QLabel *dataF2 = new QLabel();
    dataF2->setText(eventTmp->getEndDate().substr(11,5).c_str());

    l2->addWidget(nomeE);
    l2->addWidget(titolo);
    l2->addWidget(dataI);
    l2->addWidget(dataI2);
    l2->addWidget(dataF);
    l2->addWidget(dataF2);
    l2->addWidget(hideInfo);
    l2->addWidget(modifyEv);
    l2->addWidget(deleteEv);

    connect(modifyEv,SIGNAL(clicked()), this, SLOT(modifyTask()));
    connect(deleteEv, SIGNAL(clicked()), this, SLOT(deleteEvent()));
    connect(hideInfo, SIGNAL(clicked()), this, SLOT(nascondiInfo()));
}

void MainWindow::modalToDo(){
    if(spaceToDoInfo!=NULL){
        spaceToDoInfo->setVisible(false);
    }
    spaceToDoInfo = new QGroupBox(tr("ToDo info"));

    QPushButton *mod = new QPushButton("Edit me");
    QPushButton *hid = new QPushButton("Hide info");
    QPushButton *del = new QPushButton("Delete me");

    QGridLayout *l3 = new QGridLayout;
    spaceToDoInfo->setLayout(l3);
    //layout->addLayout(l3,0,4);
    layout->addWidget(spaceToDoInfo,0,4);
    QLabel *nomeE = new QLabel("ToDo name: ");
    QLabel *titoloToDo = new QLabel();
    titoloToDo->setText(toDoTmp->getSummary().c_str());

    l3->addWidget(nomeE);
    l3->addWidget(titoloToDo);
    l3->addWidget(hid);
    l3->addWidget(mod);
    l3->addWidget(del);

    connect(mod,SIGNAL(clicked()), this, SLOT(modToDo()));
    connect(del, SIGNAL(clicked()), this, SLOT(deleteToDo()));
    connect(hid, SIGNAL(clicked()), this, SLOT(nascondiInfoToDo()));
}


/*----------------------------SLOT--------------------------------------------*/

void MainWindow::addNewCalendarSlot(){
    deleteComponents();

    spaceCalendarAdd = new QGroupBox();
    QPushButton *saveCal = new QPushButton("Save");
    QPushButton *annullaCal = new QPushButton("Cancel");
    QGridLayout *layNew = new QGridLayout();
    QLabel *nomeC = new QLabel("Calendar Name: ");
    nomeCal = new QTextEdit();
    nomeCal->setMaximumHeight(35);
    spaceCalendarAdd->setLayout(layNew);

    layout->addWidget(spaceCalendarAdd,5,0);
    layNew->addWidget(nomeC);
    layNew->addWidget(nomeCal);
    layNew->addWidget(saveCal);
    layNew->addWidget(annullaCal);

    connect(saveCal, SIGNAL(clicked()), this, SLOT(addNewCalendar()));
    connect(annullaCal, SIGNAL(clicked()), this, SLOT(hideNewCalendar()));
}

void MainWindow::addNewCalendar(){
    if(nomeCal->toPlainText().length()!=0 && !std::regex_match(nomeCal->toPlainText().toStdString(),std::regex("\\s+"))){
        spaceCalendarAdd->hide();
        api->insertCalendar(nomeCal->toPlainText().toStdString());
    }
    else{
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("Something gone wrong!");
        msgBox->exec();
    }
}

void MainWindow::addNewToDoSlot(){

    if(spaceEventAdd != NULL){
        //spaceEventAdd->hide();
        delete spaceEventAdd;
        spaceEventAdd=NULL;
    }
    //per gestire più click
    if(spaceToDoAdd!=NULL){
        delete spaceToDoAdd;
        spaceToDoAdd=NULL;
    }
    if(calendari.size()==0){
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("You are going to do something without a calendar..");
        msgBox->exec();
    }
    else{
        spaceToDoAdd = new QGroupBox();
        QPushButton *saveToDo = new QPushButton("Save");
        QPushButton *annullaToDo = new QPushButton("Cancel");
        QGridLayout *layNew = new QGridLayout();
        QLabel *nomeE = new QLabel("ToDo name: ");
        nomeToDo = new QTextEdit();
        nomeToDo->setMaximumHeight(35);
        spaceToDoAdd->setLayout(layNew);

        layout->addWidget(spaceToDoAdd,2,0);
        layNew->addWidget(nomeE,0,0);
        layNew->addWidget(nomeToDo,1,0);
        layNew->addWidget(saveToDo,0,1);
        layNew->addWidget(annullaToDo,1,1);

        connect(saveToDo, SIGNAL(clicked()), this, SLOT(addNewToDo()));
        connect(annullaToDo, SIGNAL(clicked()), this, SLOT(hideNewToDo()));
    }

}

void MainWindow::addNewToDo(){
    if(nomeToDo->toPlainText().length()!=0 && !std::regex_match(nomeToDo->toPlainText().toStdString(),std::regex("\\s+"))){
        spaceToDoAdd->hide();
        if(spaceToDoInfo!=NULL){
            spaceToDoInfo->hide();
        }
        api->insertToDo(calendarTmp->getId(), nomeToDo->toPlainText().toStdString());
    }
    else{
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("Something gone wrong!");
        msgBox->exec();
    }

}

void MainWindow::refreshSlot(){
    api->getCalendars();
}

void MainWindow::refreshCalendar(Calendar* c){
    api->getCalendars();
}

void MainWindow::hideNewCalendar(){
    spaceCalendarAdd->hide();
}

void MainWindow::removeCalendarSlot(){
    deleteComponents();
    if(calendari.size()==0){
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("You are going to do something without a calendar..");
        msgBox->exec();
    }
    else{
        spaceCalendarDelete = new QGroupBox();
        QPushButton *deleteCal = new QPushButton("Delete");
        QPushButton *annullaCal = new QPushButton("Cancel");
        QGridLayout *layNew = new QGridLayout();
        QLabel *nomeC = new QLabel("Calendar to delete: ");
        QTextEdit *nomeCDE = new QTextEdit();
        nomeCDE->setText(calendarTmp->getSummary().c_str());
        nomeCDE->setReadOnly(true);
        nomeCDE->setMaximumHeight(35);
        spaceCalendarDelete->setLayout(layNew);
        layout->addWidget(spaceCalendarDelete,5,0);
        layNew->addWidget(nomeC);
        layNew->addWidget(nomeCDE);
        layNew->addWidget(deleteCal);
        layNew->addWidget(annullaCal);

        connect(deleteCal, SIGNAL(clicked()), this, SLOT(removeCalendar()));
        connect(annullaCal, SIGNAL(clicked()), this, SLOT(hideDeleteCalendar()));
    }

}

void MainWindow::removeCalendar(){

    if(calendari.size()==1){
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("You are going to remove your last calendar. Are you sure ? ");
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No );
        int ret = msgBox->exec();
        switch(ret){
        case QMessageBox::Yes:
            spaceCalendarDelete->hide();
            textEdit->clear();
            textEdit2->clear();
            if(eventi.size()!=0){
                delete eventList;
                eventList = NULL;
            }
            if(toDos.size()!=0){
                delete toDoList;
                toDoList = NULL;
            }
            inizio=true;
            api->deleteCalendar(calendarTmp->getId());
            break;
        case QMessageBox::No:
            msgBox->hide();
            break;
        }
    }
    else{

        spaceCalendarDelete->hide();
        if(textEdit != NULL){
            textEdit->clear();
        }

        if(eventi.size()!=0){
            eventList->clear();
        }
        inizio=true;
        api->deleteCalendar(calendarTmp->getId());
    }

}

void MainWindow::hideDeleteCalendar(){
    spaceCalendarDelete->hide();
}

void MainWindow::shareCalendarSlot(){
    deleteComponents();
    if(calendari.size()==0){
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("You are going to do something without a calendar..");
        msgBox->exec();
    }
    else{
        spaceCalendarShare = new QGroupBox();
        QPushButton *shareCal = new QPushButton("Share");
        QPushButton *annullaCal = new QPushButton("Cancel");
        QGridLayout *layNew = new QGridLayout();
        QLabel *nomeC = new QLabel("Email: ");
        email = new QTextEdit();
        email->setMaximumHeight(35);
        spaceCalendarShare->setLayout(layNew);

        //layout->addLayout(layNew,5,0);
        layout->addWidget(spaceCalendarShare,5,0);
        layNew->addWidget(nomeC);
        layNew->addWidget(email);
        layNew->addWidget(shareCal);
        layNew->addWidget(annullaCal);

        connect(shareCal, SIGNAL(clicked()), this, SLOT(shareCalendar()));
        connect(annullaCal, SIGNAL(clicked()), this, SLOT(hideShareCalendar()));
    }

}

void MainWindow::shareCalendar(){
    if(email->toPlainText().length()!=0 && !std::regex_match(email->toPlainText().toStdString(),std::regex("\\s+"))){
        spaceCalendarShare->hide();
        api->shareCalendar(calendarTmp->getId(), email->toPlainText().toStdString());
    }
    else{
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("Something gone wrong!");
        msgBox->exec();
    }
}

void MainWindow::hideShareCalendar(){
    spaceCalendarShare->hide();
}

void MainWindow::refreshEvents(Event *a){
    api->getEventsByCalendarDay(calendarTmp->getId(),timeMin,timeMax);
}

void MainWindow::refreshToDo(Event * a){
    delete toDoList;
    toDoList=NULL;
    api->getToDos(calendarTmp->getId());
}

void MainWindow::getEventsByDay(){

    if(calendari.size()!=0){
        if(spaceEventInfo!=NULL){
            spaceEventInfo->hide();
        }
        QDate data = calendar->selectedDate();
        //costruisco la data che mi serve, con formato YYYY-MM-DDTHH:MM:SSZ
        std::string giorno = std::to_string(data.day());
        std::string mese = std::to_string(data.month());
        std::string anno = std::to_string(data.year());
        timeMin = anno+"-"+mese+"-"+giorno+"T00:00:01Z";
        timeMax = anno+"-"+mese+"-"+giorno+"T23:59:59Z";


        api->getEventsByCalendarDay(calendarTmp->getId(),timeMin,timeMax);
    }
    else{
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("You are going to do something without a calendar..");
        msgBox->exec();
    }

}

void MainWindow::getToDos(){

    api->getToDos(calendarTmp->getId());
}

void MainWindow::hidePreferences(){
    generalOptionsGroupBox->hide();
    button22->hide();
}

void MainWindow::showPreferences(){
    generalOptionsGroupBox->show();
    button22->show();
    layout->addWidget(generalOptionsGroupBox, 0, 5);
    layButtons->addWidget(button22, 2,0);

}

void MainWindow::showCalendars(std::vector<Calendar *> ret){
    //salvo l'elenco di calendari in una variabile
    calendari = ret;

    //GESTIONE MENU A TENDINA
    calendarList = new QComboBox;

    //vedo se il calendario sul quale sto lavorando è tra quelli in ret
    //cosi imposto l'indice
    int indiceCalendarTmp = 0;
    if(calendarTmp != NULL){
        bool trovato = false;
        for( int i = 0;i<calendari.size();i++){
            if(calendari.at(i)->getId().compare(calendarTmp->getId())==0){
                indiceCalendarTmp = i;
                trovato = true;
                break;
            }
        }
        if(trovato==false){
            //calendarTmp è stato eliminato
            if(ret.size()!=0){
                calendarTmp=ret.at(0);
            }
        }
    }
    //li aggiungo tutti
    for(auto el : ret){
        calendarList->addItem(tr(el->getSummary().c_str()));
    }
    //scelgo il calendarTmp
    if(ret.size()!=0 && calendarTmp!=NULL){
        //mostra il calendarTmp
        calendarList->setCurrentIndex(indiceCalendarTmp);
    }
    layButtons->addWidget(calendarList,1,0);
    //attento che se l'array è vuoto, crasha
    if(ret.size()!=0){
        if(inizio){
            inizio = false;
            calendarTmp=ret.at(0);
        }
        api->getToDos(calendarTmp->getId());
    }

    connect(calendarList, &QComboBox::currentIndexChanged, this, &MainWindow::selectCalendarTitle);
}

void MainWindow::showToDos(std::vector<Event *> ret){
    if(spaceTextEditToDo!=NULL){
        delete spaceTextEditToDo;
        spaceTextEditToDo=NULL;
    }
    toDos = ret;
   /* if(toDos.size()!=0){
        toDoTmp = ret.at(0);
    }*/
    spaceTextEditToDo = new QGroupBox(tr("ToDoList:"));
    QGridLayout *layToDo = new QGridLayout();
    textEdit2 = new QTextEdit();
    textEdit2->setReadOnly(true);
    toDoList = new QComboBox;
    toDoList->addItem("-");
    for(auto x : ret){
        textEdit2->append(x->getSummary().c_str());
        toDoList->addItem(tr(x->getSummary().c_str()));
    }
    //al refresh viene fatta la get e se non è stato modificato nulla
    //bisogna lasciare il toDo a quello precedente
    int indiceToDoTmp = 0;
    if(toDoTmp != NULL){
        bool trovato = false;
        for( int i = 0;i<toDos.size();i++){
            if(toDos.at(i)->getId().compare(toDoTmp->getId())==0){
                indiceToDoTmp = i;
                trovato = true;
                break;
            }
        }
        if(trovato==false){
            //toDoTmp è stato eliminato
        }
    }
    if(ret.size()!=0 && toDoTmp!=NULL){

        toDoList->setCurrentIndex(indiceToDoTmp+1);
    }

    if(toDoTmp==NULL){
        toDoList->setCurrentIndex(0);
    }

    layToDo->addWidget(textEdit2);
    layToDo->addWidget(toDoList);
    spaceTextEditToDo->setLayout(layToDo);
    layout->addWidget(spaceTextEditToDo,0,3);
    if(timeMax.compare("")!=0 && timeMin.compare("")!=0){
        api->getEventsByCalendarDay(calendarTmp->getId(),timeMin, timeMax);
    }
    connect(toDoList,SIGNAL(currentIndexChanged(int)), this, SLOT(impostaNomeTodo(int)));
}

void MainWindow::impostaNomeTodo(int index){
    if(index==0){
        toDoTmp=NULL;
        spaceToDoInfo->hide();
    }
    else{
        toDoTmp=toDos.at(index-1);
        modalToDo();
    }

}

void MainWindow::nascondiInfoToDo(){
    //provo a creare un qgroupbox, gli setto il layout, e poi è il groupbox che nascondo o mostro
    spaceToDoInfo->hide();
    toDoList->setCurrentIndex(0);
}

void MainWindow::deleteToDo(){

    spaceToDoInfo->hide();
    api->deleteToDo(calendarTmp->getId(),toDoTmp->getId());
}

void MainWindow::modToDo(){
    if(spaceToDoModal!=NULL){
        delete spaceToDoModal;
        spaceToDoModal = NULL;
    }
    spaceToDoModal = new QGroupBox(tr("Edit toDo:"));
    QGridLayout *layEditToDo = new QGridLayout;
    spaceToDoModal->setLayout(layEditToDo);
    toDoEditName = new QTextEdit();
    QLabel *t = new QLabel();
    t->setText("ToDo name:");
    toDoEditName->setFixedHeight(35);
    toDoEditName->setText(toDoTmp->getSummary().c_str());
    QPushButton *salvaMod = new QPushButton("Save changes");
    QPushButton *annullaMod = new QPushButton("Cancel changes");
    layEditToDo->addWidget(t);
    layEditToDo->addWidget(toDoEditName);
    layout->addWidget(spaceToDoModal,1,3);
    layEditToDo->addWidget(salvaMod,0,1);
    layEditToDo->addWidget(annullaMod,1,1);
    connect(annullaMod, SIGNAL(clicked()), this, SLOT(hideModalToDo()));
    connect(salvaMod, SIGNAL(clicked()), this, SLOT(updateToDo()));

}

void MainWindow::hideModalToDo(){
    spaceToDoModal->hide();
}

void MainWindow::updateToDo(){


    if(toDoEditName->toPlainText().length()!=0 && !std::regex_match(toDoEditName->toPlainText().toStdString(),std::regex("\\s+"))){
        spaceToDoInfo->hide();
        spaceToDoModal->hide();
        api->updateToDo(calendarTmp->getId(), toDoTmp->getId(), toDoEditName->toPlainText().toStdString());
        toDoTmp=NULL;
    }
    else{
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("Something gone wrong!");
        msgBox->exec();
    }

}

void MainWindow::showEvents(std::vector<Event *> ret){
    eventi = ret;
    //textEdit->clear();
    if(spaceTextEditTask!=NULL){
        delete spaceTextEditTask;
        spaceTextEditTask=NULL;
    }
    spaceTextEditTask = new QGroupBox(tr("TaskList: "));
    layTE = new QGridLayout();
    textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    eventList = new QComboBox;
    eventList->addItem("-");
    //impostare il primo elemento del menu a tendina come vuoto o primo task dell'array
    for( int i = 0;i<ret.size();i++){
        eventList->addItem(tr(ret.at(i)->getSummary().c_str()));
        textEdit->append("Task title: ");
        textEdit->append( ret.at(i)->getSummary().c_str() );
        textEdit->append("\n");

    }

    //al refresh viene fatta la get e se non è stato modificato nulla
    //bisogna lasciare il task a quello precedente
    int indiceTaskTmp = 0;
    if(eventTmp != NULL){
        bool trovato = false;
        for( int i = 0;i<eventi.size();i++){
            if(eventi.at(i)->getId().compare(eventTmp->getId())==0){
                indiceTaskTmp = i;
                trovato = true;
                break;
            }
        }
        if(trovato==false){
            //eventTmp è stato eliminato
            //eventList->setCurrentIndex(0);
        }
    }
    if(ret.size()!=0 && eventTmp!=NULL){
        eventList->setCurrentIndex(indiceTaskTmp+1);
    }
    if(eventTmp==NULL){
        eventList->setCurrentIndex(0);
    }

    layTE->addWidget(textEdit);
    layTE->addWidget(eventList);
    spaceTextEditTask->setLayout(layTE);
    layout->addWidget(spaceTextEditTask,0,1);
    connect(eventList,SIGNAL(currentIndexChanged(int)), this, SLOT(selectEventTitle(int)));

    QPushButton *sel = new QPushButton("Add task/todo");
    QMenu *menu = new QMenu(this);
    QAction *at = new QAction(tr("Add task"));
    QAction *atd = new QAction(tr("Add todo"));
    menu->addAction(at);
    menu->addAction(atd);
    sel->setMenu(menu);
    layButtons->addWidget(sel,4,0);

    connect(at, SIGNAL(triggered()), this, SLOT(addNewTaskSlot()));
    connect(atd,SIGNAL(triggered()), this, SLOT(addNewToDoSlot()));
}

void MainWindow::hideNewToDo(){
    spaceToDoAdd->hide();
}

void MainWindow::addNewTaskSlot(){
    if(spaceToDoAdd != NULL){
        spaceToDoAdd->hide();
        delete spaceToDoAdd;
        spaceToDoAdd=NULL;
    }
    if(spaceEventAdd != NULL){
        delete spaceEventAdd;
        spaceEventAdd=NULL;
    }
    if(calendari.size()==0){
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("You are going to do something without a calendar..");
        msgBox->exec();
    }
    else{
        spaceEventAdd = new QGroupBox(tr("Add task: "));
        QPushButton *saveNew = new QPushButton("Save");
        QPushButton *annullaNew = new QPushButton("Cancel");
        QGridLayout *layNew = new QGridLayout();
        spaceEventAdd->setLayout(layNew);
        //layout->addLayout(layNew,5,0);
        layout->addWidget(spaceEventAdd,2,0);
        QLabel *nomeE = new QLabel("Task name: ");
        QLabel *dataI = new QLabel("Starting at: ");
        QLabel *dataF = new QLabel("Ending at: ");
        QLabel *data = new QLabel("Date: ");
        QDate dataa = calendar->selectedDate();
        std::string giorno = std::to_string(dataa.day());
        std::string mese = std::to_string(dataa.month());
        std::string anno = std::to_string(dataa.year());
        std::string pippoBaudo = anno+"-"+mese+"-"+giorno;
        editNomeE = new QTextEdit(); editNomeE->setMaximumHeight(35);
        editDataI = new QTextEdit(); editDataI->setMaximumHeight(35);
        editDataF = new QTextEdit(); editDataF->setMaximumHeight(35);
        editData = new QTextEdit(pippoBaudo.c_str()); editData ->setMaximumHeight(35); editData->setReadOnly(true);

        layNew->addWidget(nomeE,0,0);
        layNew->addWidget(editNomeE,1,0);
        layNew->addWidget(dataI,2,0);
        layNew->addWidget(editDataI,3,0);
        layNew->addWidget(dataF,2,1);
        layNew->addWidget(editDataF,3,1);
        layNew->addWidget(data,0,1);
        layNew->addWidget(editData,1,1);
        layNew->addWidget(saveNew,1,2);
        layNew->addWidget(annullaNew,3,2);

        connect(saveNew, SIGNAL(clicked()), this, SLOT(addNewTask()));
        connect(annullaNew, SIGNAL(clicked()), this, SLOT(hideAddTask()));
    }

}

void MainWindow::hideAddTask(){
    spaceEventAdd->hide();
}

void MainWindow::addNewTask(){
    int ret = checkTime();
    if(ret==0 && editNomeE->toPlainText().length()!=0 && !std::regex_match(editNomeE->toPlainText().toStdString(),std::regex("\\s+"))){
        spaceEventAdd->hide();
        if(spaceEventInfo!=NULL){
            spaceEventInfo->hide();
        }

        timeMin = editData->toPlainText().toStdString()+"T00:00:01Z";
        timeMax = editData->toPlainText().toStdString()+"T23:59:59Z";

        //data nel formato YYYY-MM-DDTHH:MM:SS+01:00
        std::string startd = editData->toPlainText().toStdString()+"T"+editDataI->toPlainText().toStdString()+":00+01:00";
        std::string endd = editData->toPlainText().toStdString()+"T"+editDataF->toPlainText().toStdString()+":00+01:00";
        api->insertEvent(calendarTmp->getId(), editNomeE->toPlainText().toStdString(), startd, endd);
    }
    else{
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("Something gone wrong! \n Tips: Time format is HH:mm");
        msgBox->exec();
    }
}

void MainWindow::selectEventTitle(int index){
    if(index==0){
        eventTmp=NULL;
        spaceEventInfo->hide();
    }
    else{
        eventTmp = eventi.at(index-1);
        modal();
    }



}

void MainWindow::nascondiInfo(){
    spaceEventInfo->hide();
    eventList->setCurrentIndex(0);

}

void MainWindow::selectCalendarTitle(int index){
    /*int c = 0;
    for(auto x : calendari){
        if(c==index){
            //ho trovato quello giusto e lo metto da parte in una variabile
            calendarTmp=x;
            break;
        }
        c++;
    }*/
    calendarTmp = calendari.at(index);
    if(textEdit!=NULL){
        textEdit->clear();
    }

    if(spaceEventInfo!=NULL){
        delete spaceEventInfo;
        spaceEventInfo=NULL;
    }
    if(spaceToDoInfo!=NULL){
        delete spaceToDoInfo;
        spaceToDoInfo=NULL;
    }
    if(eventList!=NULL){
        delete eventList;
        eventList=NULL;
    }
    timeMin=timeMax="";
    api->getToDos(calendarTmp->getId());
}

void MainWindow::modifyTask(){
    if(spaceEventModal!=NULL){
        delete spaceEventModal;
        spaceEventModal=NULL;
    }
    spaceEventModal = new QGroupBox(tr("Edit task:"));
    QGridLayout *layEdit = new QGridLayout;
    spaceEventModal->setLayout(layEdit);
    teEdit = new QTextEdit();
    dEdit= new QTextEdit();
    oraInizioEdit = new QTextEdit();
    oraFineEdit = new QTextEdit();
    QLabel *t = new QLabel();
    t->setText("Task title:");
    QLabel *d = new QLabel();
    d->setText("Date:");
    dEdit->setFixedHeight(35);
    dEdit->setText(eventTmp->getStartDate().substr(0,10).c_str());
    dEdit->setReadOnly(true);
    teEdit->setText(eventTmp->getSummary().c_str());
    teEdit->setFixedHeight(35);
    QLabel *oi = new QLabel();
    oi->setText("Starting at:");
    QLabel *of = new QLabel();
    of->setText("Ending at:");

    oraInizioEdit->setFixedHeight(35);
    oraFineEdit->setFixedHeight(35);
    oraInizioEdit->setText(eventTmp->getStartDate().substr(11,5).c_str());
    oraFineEdit->setText(eventTmp->getEndDate().substr(11,5).c_str());
    QPushButton *salvaMod = new QPushButton("Save changes");
    QPushButton *annullaMod = new QPushButton("Cancel changes");
    layEdit->addWidget(t);
    layEdit->addWidget(teEdit);
    layEdit->addWidget(d);
    layEdit->addWidget(dEdit);
    layEdit->addWidget(oi);
    layEdit->addWidget(oraInizioEdit);
    layEdit->addWidget(of);
    layEdit->addWidget(oraFineEdit);
    layout->addWidget(spaceEventModal,1,1);
    layEdit->addWidget(salvaMod,0,1);
    layEdit->addWidget(annullaMod,1,1);
    connect(salvaMod, SIGNAL(clicked()), this, SLOT(updateTask()));
    connect(annullaMod,SIGNAL(clicked()), this, SLOT(hideModalEvent()));

}

void MainWindow::hideModalEvent(){
    spaceEventModal->hide();
}

void MainWindow::updateTask(){
    int ret = checkTimeUpdate();
    if(ret==0 && teEdit->toPlainText().length()!=0 && !std::regex_match(teEdit->toPlainText().toStdString(),std::regex("\\s+"))){
        spaceEventModal->hide();
        spaceEventInfo->hide();
        /*if(eventList!=NULL){
            eventList->setCurrentIndex(0);
        }*/
        //costruisco startDate e endDate del formato YYYY-MM-DDTHH:MM:SS+01:00
        std::string startd = eventTmp->getStartDate().substr(0,10)+"T"+oraInizioEdit->toPlainText().toStdString()+":00+01:00";
        std::string endd = eventTmp->getStartDate().substr(0,10)+"T"+oraFineEdit->toPlainText().toStdString()+":00+01:00";
        api->updateEvent(calendarTmp->getId(), eventTmp->getId(), teEdit->toPlainText().toStdString(), startd, endd);
        eventTmp=NULL;
    }
    else{
        QMessageBox *msgBox = new QMessageBox();
        msgBox->setText("Something gone wrong! \n Tips: Time format is HH:mm");
        msgBox->exec();
    }

}

void MainWindow::deleteEvent(){
    spaceEventInfo->hide();
    api->deleteEvent(calendarTmp->getId(),eventTmp->getId());

}

/**************************************** Thread ************************************************/

void RefreshThread::run(){
    while(true){
        int waitingTimeInSeconds = 10;
        std::this_thread::sleep_for(std::chrono::seconds(waitingTimeInSeconds));
        emit signalRefresh();      //refresh
    }
}

/*---------------------------------FUNZIONI E SLOT CALENDARIO----------------------------------------*/

void MainWindow::localeChanged(int index)
{
    const QLocale newLocale(localeCombo->itemData(index).toLocale());
    calendar->setLocale(newLocale);
    int newLocaleFirstDayIndex = firstDayCombo->findData(newLocale.firstDayOfWeek());
    firstDayCombo->setCurrentIndex(newLocaleFirstDayIndex);
}

void MainWindow::firstDayChanged(int index)
{
    calendar->setFirstDayOfWeek(Qt::DayOfWeek(
                                    firstDayCombo->itemData(index).toInt()));
}

void MainWindow::selectionModeChanged(int index)
{
    calendar->setSelectionMode(QCalendarWidget::SelectionMode(
                                   selectionModeCombo->itemData(index).toInt()));
}

void MainWindow::horizontalHeaderChanged(int index)
{
    calendar->setHorizontalHeaderFormat(QCalendarWidget::HorizontalHeaderFormat(
                                            horizontalHeaderCombo->itemData(index).toInt()));
}

void MainWindow::verticalHeaderChanged(int index)
{
    calendar->setVerticalHeaderFormat(QCalendarWidget::VerticalHeaderFormat(
                                          verticalHeaderCombo->itemData(index).toInt()));
}

void MainWindow::selectedDateChanged() {
    currentDateEdit->setDate(calendar->selectedDate());
}

void MainWindow::minimumDateChanged(QDate date)
{
    calendar->setMinimumDate(date);
    maximumDateEdit->setDate(calendar->maximumDate());
}

void MainWindow::maximumDateChanged(QDate date)
{
    calendar->setMaximumDate(date);
    minimumDateEdit->setDate(calendar->minimumDate());
}

void MainWindow::weekdayFormatChanged()
{
    QTextCharFormat format;

    format.setForeground(qvariant_cast<QColor>(
                             weekdayColorCombo->itemData(weekdayColorCombo->currentIndex())));
    calendar->setWeekdayTextFormat(Qt::Monday, format);
    calendar->setWeekdayTextFormat(Qt::Tuesday, format);
    calendar->setWeekdayTextFormat(Qt::Wednesday, format);
    calendar->setWeekdayTextFormat(Qt::Thursday, format);
    calendar->setWeekdayTextFormat(Qt::Friday, format);
}

void MainWindow::weekendFormatChanged()
{
    QTextCharFormat format;

    format.setForeground(qvariant_cast<QColor>(
                             weekendColorCombo->itemData(weekendColorCombo->currentIndex())));
    calendar->setWeekdayTextFormat(Qt::Saturday, format);
    calendar->setWeekdayTextFormat(Qt::Sunday, format);
}

void MainWindow::reformatHeaders()
{
    QString text = headerTextFormatCombo->currentText();
    QTextCharFormat format;

    if (text == tr("Bold"))
        format.setFontWeight(QFont::Bold);
    else if (text == tr("Italic"))
        format.setFontItalic(true);
    else if (text == tr("Green"))
        format.setForeground(Qt::green);
    calendar->setHeaderTextFormat(format);
}

void MainWindow::reformatCalendarPage()
{
    QTextCharFormat mayFirstFormat;
    const QDate mayFirst(calendar->yearShown(), 5, 1);

    QTextCharFormat firstFridayFormat;
    QDate firstFriday(calendar->yearShown(), calendar->monthShown(), 1);
    while (firstFriday.dayOfWeek() != Qt::Friday)
        firstFriday = firstFriday.addDays(1);

    if (firstFridayCheckBox->isChecked()) {
        firstFridayFormat.setForeground(Qt::blue);
    } else { // Revert to regular colour for this day of the week.
        Qt::DayOfWeek dayOfWeek(static_cast<Qt::DayOfWeek>(firstFriday.dayOfWeek()));
        firstFridayFormat.setForeground(calendar->weekdayTextFormat(dayOfWeek).foreground());
    }

    calendar->setDateTextFormat(firstFriday, firstFridayFormat);

    // When it is checked, "May First in Red" always takes precedence over "First Friday in Blue".
    if (mayFirstCheckBox->isChecked()) {
        mayFirstFormat.setForeground(Qt::red);
    } else if (!firstFridayCheckBox->isChecked() || firstFriday != mayFirst) {
        // We can now be certain we won't be resetting "May First in Red" when we restore
        // may 1st's regular colour for this day of the week.
        Qt::DayOfWeek dayOfWeek(static_cast<Qt::DayOfWeek>(mayFirst.dayOfWeek()));
        calendar->setDateTextFormat(mayFirst, calendar->weekdayTextFormat(dayOfWeek));
    }

    calendar->setDateTextFormat(mayFirst, mayFirstFormat);
}

//Qui avviene la creazione del calendario
void MainWindow::createPreviewGroupBox()
{
    previewGroupBox = new QGroupBox(tr("Calendar"));
    calendar = new QCalendarWidget;
    calendar->setMinimumDate(QDate(1900, 1, 1));
    calendar->setMaximumDate(QDate(3000, 1, 1));
    calendar->setGridVisible(true);

    connect(calendar, &QCalendarWidget::currentPageChanged,
            this, &MainWindow::reformatCalendarPage);
    //Questa connect ti permette di prendere l'evento dato il giorno cliccato
    connect(calendar, &QCalendarWidget::clicked, this, &MainWindow::getEventsByDay  );
    previewLayout = new QGridLayout;
    previewLayout->addWidget(calendar, 0, 0, Qt::AlignCenter);
    previewGroupBox->setLayout(previewLayout);
}

void MainWindow::createGeneralOptionsGroupBox(){

    generalOptionsGroupBox = new QGroupBox(tr("Calendar Preferences"));

    localeCombo = new QComboBox;
    int curLocaleIndex = -1;
    int index = 0;
    for (int _lang = QLocale::C; _lang <= QLocale::LastLanguage; ++_lang) {
        QLocale::Language lang = static_cast<QLocale::Language>(_lang);
        const auto locales =
                QLocale::matchingLocales(lang, QLocale::AnyScript, QLocale::AnyTerritory);
        for (auto loc : locales) {
            QString label = QLocale::languageToString(lang);
            auto territory = loc.territory();
            label += QLatin1Char('/');
            label += QLocale::territoryToString(territory);
            if (locale().language() == lang && locale().territory() == territory)
                curLocaleIndex = index;
            localeCombo->addItem(label, loc);
            ++index;
        }
    }
    if (curLocaleIndex != -1)
        localeCombo->setCurrentIndex(curLocaleIndex);
    localeLabel = new QLabel(tr("&Locale"));
    localeLabel->setBuddy(localeCombo);

    firstDayCombo = new QComboBox;
    firstDayCombo->addItem(tr("Sunday"), Qt::Sunday);
    firstDayCombo->addItem(tr("Monday"), Qt::Monday);
    firstDayCombo->addItem(tr("Tuesday"), Qt::Tuesday);
    firstDayCombo->addItem(tr("Wednesday"), Qt::Wednesday);
    firstDayCombo->addItem(tr("Thursday"), Qt::Thursday);
    firstDayCombo->addItem(tr("Friday"), Qt::Friday);
    firstDayCombo->addItem(tr("Saturday"), Qt::Saturday);

    firstDayLabel = new QLabel(tr("Wee&k starts on:"));
    firstDayLabel->setBuddy(firstDayCombo);

    selectionModeCombo = new QComboBox;
    selectionModeCombo->addItem(tr("Single selection"),
                                QCalendarWidget::SingleSelection);
    selectionModeCombo->addItem(tr("None"), QCalendarWidget::NoSelection);

    selectionModeLabel = new QLabel(tr("&Selection mode:"));
    selectionModeLabel->setBuddy(selectionModeCombo);

    //CARINO PER LA GRAFICA: NASCONDE LA GRIGLIA
    gridCheckBox = new QCheckBox(tr("&Grid"));
    gridCheckBox->setChecked(calendar->isGridVisible());
    //LO STESSO DI SOPRA
    navigationCheckBox = new QCheckBox(tr("&Navigation bar"));
    navigationCheckBox->setChecked(true);

    horizontalHeaderCombo = new QComboBox;
    horizontalHeaderCombo->addItem(tr("Single letter day names"),
                                   QCalendarWidget::SingleLetterDayNames);
    horizontalHeaderCombo->addItem(tr("Short day names"),
                                   QCalendarWidget::ShortDayNames);
    horizontalHeaderCombo->addItem(tr("None"),
                                   QCalendarWidget::NoHorizontalHeader);
    horizontalHeaderCombo->setCurrentIndex(1);

    horizontalHeaderLabel = new QLabel(tr("&Horizontal header:"));
    horizontalHeaderLabel->setBuddy(horizontalHeaderCombo);

    verticalHeaderCombo = new QComboBox;
    verticalHeaderCombo->addItem(tr("ISO week numbers"),
                                 QCalendarWidget::ISOWeekNumbers);
    verticalHeaderCombo->addItem(tr("None"), QCalendarWidget::NoVerticalHeader);

    verticalHeaderLabel = new QLabel(tr("&Vertical header:"));
    verticalHeaderLabel->setBuddy(verticalHeaderCombo);

    connect(localeCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::localeChanged);
    connect(firstDayCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::firstDayChanged);
    connect(selectionModeCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::selectionModeChanged);
    connect(gridCheckBox, &QCheckBox::toggled,
            calendar, &QCalendarWidget::setGridVisible);
    connect(navigationCheckBox, &QCheckBox::toggled,
            calendar, &QCalendarWidget::setNavigationBarVisible);
    connect(horizontalHeaderCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::horizontalHeaderChanged);
    connect(verticalHeaderCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::verticalHeaderChanged);

    QHBoxLayout *checkBoxLayout = new QHBoxLayout;
    checkBoxLayout->addWidget(gridCheckBox);
    checkBoxLayout->addStretch();
    checkBoxLayout->addWidget(navigationCheckBox);

    QGridLayout *outerLayout = new QGridLayout;
    outerLayout->addWidget(localeLabel, 0, 0);
    outerLayout->addWidget(localeCombo, 0, 1);
    outerLayout->addWidget(firstDayLabel, 1, 0);
    outerLayout->addWidget(firstDayCombo, 1, 1);
    outerLayout->addWidget(selectionModeLabel, 2, 0);
    outerLayout->addWidget(selectionModeCombo, 2, 1);
    outerLayout->addLayout(checkBoxLayout, 3, 0, 1, 2);
    outerLayout->addWidget(horizontalHeaderLabel, 4, 0);
    outerLayout->addWidget(horizontalHeaderCombo, 4, 1);
    outerLayout->addWidget(verticalHeaderLabel, 5, 0);
    outerLayout->addWidget(verticalHeaderCombo, 5, 1);
    generalOptionsGroupBox->setLayout(outerLayout);

    firstDayChanged(firstDayCombo->currentIndex());
    selectionModeChanged(selectionModeCombo->currentIndex());
    horizontalHeaderChanged(horizontalHeaderCombo->currentIndex());
    verticalHeaderChanged(verticalHeaderCombo->currentIndex());


}

void MainWindow::createTextFormatsGroupBox()
{
    textFormatsGroupBox = new QGroupBox(tr("Text Formats"));

    weekdayColorCombo = createColorComboBox();
    weekdayColorCombo->setCurrentIndex(
                weekdayColorCombo->findText(tr("Black")));

    weekdayColorLabel = new QLabel(tr("&Weekday color:"));
    weekdayColorLabel->setBuddy(weekdayColorCombo);

    weekendColorCombo = createColorComboBox();
    weekendColorCombo->setCurrentIndex(
                weekendColorCombo->findText(tr("Red")));

    weekendColorLabel = new QLabel(tr("Week&end color:"));
    weekendColorLabel->setBuddy(weekendColorCombo);

    headerTextFormatCombo = new QComboBox;
    headerTextFormatCombo->addItem(tr("Bold"));
    headerTextFormatCombo->addItem(tr("Italic"));
    headerTextFormatCombo->addItem(tr("Plain"));

    headerTextFormatLabel = new QLabel(tr("&Header text:"));
    headerTextFormatLabel->setBuddy(headerTextFormatCombo);

    firstFridayCheckBox = new QCheckBox(tr("&First Friday in blue"));

    mayFirstCheckBox = new QCheckBox(tr("May &1 in red"));

    connect(weekdayColorCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::weekdayFormatChanged);
    connect(weekdayColorCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::reformatCalendarPage);
    connect(weekendColorCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::weekendFormatChanged);
    connect(weekendColorCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::reformatCalendarPage);
    connect(headerTextFormatCombo, &QComboBox::currentIndexChanged,
            this, &MainWindow::reformatHeaders);
    connect(firstFridayCheckBox, &QCheckBox::toggled,
            this, &MainWindow::reformatCalendarPage);
    connect(mayFirstCheckBox, &QCheckBox::toggled,
            this, &MainWindow::reformatCalendarPage);

    QHBoxLayout *checkBoxLayout = new QHBoxLayout;
    checkBoxLayout->addWidget(firstFridayCheckBox);
    checkBoxLayout->addStretch();
    checkBoxLayout->addWidget(mayFirstCheckBox);

    QGridLayout *outerLayout = new QGridLayout;
    outerLayout->addWidget(weekdayColorLabel, 0, 0);
    outerLayout->addWidget(weekdayColorCombo, 0, 1);
    outerLayout->addWidget(weekendColorLabel, 1, 0);
    outerLayout->addWidget(weekendColorCombo, 1, 1);
    outerLayout->addWidget(headerTextFormatLabel, 2, 0);
    outerLayout->addWidget(headerTextFormatCombo, 2, 1);
    outerLayout->addLayout(checkBoxLayout, 3, 0, 1, 2);
    textFormatsGroupBox->setLayout(outerLayout);

    weekdayFormatChanged();
    weekendFormatChanged();
    reformatHeaders();
    reformatCalendarPage();
}

QComboBox *MainWindow::createColorComboBox()
{
    QComboBox *comboBox = new QComboBox;
    comboBox->addItem(tr("Red"), QColor(Qt::red));
    comboBox->addItem(tr("Blue"), QColor(Qt::blue));
    comboBox->addItem(tr("Black"), QColor(Qt::black))
            ;
    comboBox->addItem(tr("Magenta"), QColor(Qt::magenta));
    return comboBox;
}
