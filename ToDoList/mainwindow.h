#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDateTime>
#include "ui_mainwindow.h"
#include "api.h"
#include "calendar.h"
#include "event.h"
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLocale>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QPushButton>
#include <iostream>
#include <QButtonGroup>
#include <QMessageBox>
#include <regex>
#include <QThread>

QT_BEGIN_NAMESPACE
class QCalendarWidget;
class QCheckBox;
class QComboBox;
class QDate;
class QDateEdit;
class QGridLayout;
class QGroupBox;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    API *api;                           //istanza singleton che fa richieste

    QGridLayout *layout;                //layout principale
    QGridLayout *l2=nullptr;            //layout per show del task selezionato
    QGridLayout *layButtons;            //layout per bottoni posti al di sotto del calendario
    QGridLayout *layTE;                 //layout per textedit

    std::vector<Calendar*> calendari;   //elenco prelevato da Google
    std::vector<Event*> eventi;
    std::vector<Event*> toDos;
    Calendar *calendarTmp = NULL;              //calendario temporaneo sul quale eseguire ops
    Event *eventTmp=NULL;                    //evento temporaneamente selezionato
    Event *toDoTmp = NULL;                     //toDo temporaneamente selezionato

    QCalendarWidget *calendar;
    QTextEdit *textEdit = NULL;         //area di testo per visione task
    QTextEdit *textEdit2;               //area di testo per visione toDo
    QLabel *titolo;                     //contiene il nome del task da modificare
    QPushButton *button22;              //bottone che permette di nascondere le preferenze del calendario
    std::string timeMin="";                //data usata per fare getCalendarsByDay
    std::string timeMax="";                //variabile = a timeMin per fare getCalendarsByDay

    //blocchi usati per gestire hiding e layout
    QGroupBox *spaceToDoInfo=NULL;              //racchiude le info del toDo
    QGroupBox *spaceToDoModal=NULL;             //racchiude il modal del toDo
    QGroupBox *spaceEventInfo=NULL;             //racchiude le info dell'event
    QGroupBox *spaceEventModal=NULL;            //racchiude il modal dell'event
    QGroupBox *spaceEventAdd=NULL;              //racchiude l'add dell'event
    QGroupBox *spaceToDoAdd=NULL;               //racchiude l'add del toDo
    QGroupBox *spaceCalendarAdd=NULL;           //racchiude l'add di un calendario
    QGroupBox *spaceCalendarShare=NULL;         //racchiude la share di un calendario
    QGroupBox *spaceCalendarDelete=NULL;        //racchiude la delete di un calendario
    QGroupBox *spaceTextEditTask=NULL;
    QGroupBox *spaceTextEditToDo=NULL;
    QGroupBox *spaceCalendarButtons=NULL;

    //campi della modifica task
    QTextEdit *teEdit;          //titolo
    QTextEdit *dEdit;           //data
    QTextEdit *oraInizioEdit;
    QTextEdit *oraFineEdit;

    //campi di aggiunta task
    QTextEdit *editNomeE;       //titolo
    QTextEdit *editDataI;       //ora inizio
    QTextEdit *editDataF;       //ora fine
    QTextEdit *editData;        //YYYY-MM-DD

    //campi aggiunta toDo
    QTextEdit *nomeToDo;

    //campi modifica toDo
    QTextEdit *toDoEditName;

    //campi aggiunta calendario
    QTextEdit *nomeCal;

    //campi share calendario
    QTextEdit *email;

    //campo per distinguere la prima getCalendars dalle altre
    //impostato a true ad ogni delete del calendario
    //cosicchè prenderà il primo se la lista di cal è != 0
    bool inizio=true;

public slots:
            //slot gestione calendario
    void addNewCalendar();                                 //effettua la chiamata alla post
    void addNewCalendarSlot();                             //risposta al trigger del bottone che prepara all'op
    void hideNewCalendar();                                //nasconde il layout
    void hideShareCalendar();                              //   ""    ""   ""
    void hideDeleteCalendar();                             //   ""    ""   ""
    void removeCalendar();                                 //effettua chiamata alla delete
    void removeCalendarSlot();                             //risposta al trigger del bottone che prepara all'op
    void shareCalendar();                                  //effettua la chiamata alla share
    void shareCalendarSlot();                              //risposta al trigger del bottone che prepara all'op
    void refreshCalendar(Calendar*);                       //refresha i calendari dopo un evento
    void showPreferences();                                //mostra le preferenze
    void hidePreferences();                                //nasconde le preferenze
    void showCalendars(std::vector<Calendar *> ret);       //mostra tutti i calendari
    void selectCalendarTitle(int index);                   //selezione del calendario da menu a tendina

            //slot gestione Task
    void hideAddTask();                                    //nasconde il layout
    void hideModalEvent();                                 //   ""    ""   ""
    void refreshEvents(Event*);                            //refresh dei task dopo qualche op
    void addNewTask();                                     //effettua chiamata alla post
    void addNewTaskSlot();                                 //risposta al trigger del bottone che prepara all'op
    void updateTask();                                     //effettua chiamata alla update
    void deleteEvent();                                    //effettua chiamata alla delete
    void showEvents(std::vector<Event *> ret);             //mostra tutti i tasks
    void selectEventTitle(int index);                      //selezione del task da menu a tendina
    void nascondiInfo();                                   //nasconde le info del task selezionato
    void modifyTask();                                     //crea il modal per la modifica di un task
    void getEventsByDay();                                 //preleva i task in base alla data selezionata


            //slot gestione ToDo
    void hideNewToDo();                                    //nasconde il layout
    void hideModalToDo();                                  //nasconde il layout
    void refreshToDo(Event*);                              //chiama la refresh dei toDo
    void modToDo();                                        //modifica di un toDo
    void addNewToDo();                                     //aggiunge un toDo
    void addNewToDoSlot();                                 //risposta al trigger del bottone
    void nascondiInfoToDo();                               //nasconde info del toDo selezionato
    void updateToDo();                                     //aggiorna un toDo
    void deleteToDo();                                     //elimina un toDO
    void getToDos();                                       //preleva i toDo in base alla data selezionata
    void showToDos(std::vector<Event *> ret);              //mostra i toDo
    void impostaNomeTodo(int index);                       //selezione del toDo da menu a tendina

            //slot gestione thread
    void refreshSlot();


private slots:
            //slot privati usati per gestione preferenze calendario
    void localeChanged(int index);
    void firstDayChanged(int index);
    void selectionModeChanged(int index);
    void horizontalHeaderChanged(int index);
    void verticalHeaderChanged(int index);
    void selectedDateChanged();
    void minimumDateChanged(QDate date);
    void maximumDateChanged(QDate date);
    void weekdayFormatChanged();
    void weekendFormatChanged();
    void reformatHeaders();
    void reformatCalendarPage();

private:

     Ui::MainWindow *ui;

    int checkTime();                                //funzione che controlla l'integrità della data durante il new_task
    int checkTimeUpdate();                          //idem ma durante il modal che gestisce l'update di un task

    void deleteComponents();                         //gestisce la distruzione di un componente
    void modal();                                   //mostra le info di un task
    void modalToDo();                               //mostra le info di un toDo

    //creazione dei vari componenti
    void createCalendarButtons();
    void createPreviewGroupBox();
    void createTextFormatsGroupBox();
    void createGeneralOptionsGroupBox();
    QComboBox *createColorComboBox();

    QGroupBox *previewGroupBox;                     //groupBox sul qual poggia il layout del calendario
    QGridLayout *previewLayout;                     //layout sul quale poggia il calendario
    QDateEdit *currentDateEdit;                     //data impostata al click


    //utilizzati per gestione menù a tendina
    QComboBox *calendarList;
    QComboBox *eventList;
    QComboBox *toDoList;

    //campi utilizzati per gestione preferenze calendario
    QLabel *localeLabel;
    QLabel *firstDayLabel;
    QLabel *selectionModeLabel;
    QLabel *horizontalHeaderLabel;
    QLabel *verticalHeaderLabel;
    QComboBox *localeCombo;
    QComboBox *firstDayCombo;
    QComboBox *selectionModeCombo;
    QCheckBox *gridCheckBox;
    QCheckBox *navigationCheckBox;
    QComboBox *horizontalHeaderCombo;
    QComboBox *verticalHeaderCombo;
    QGroupBox *generalOptionsGroupBox;
    QGroupBox *datesGroupBox;
    QLabel *currentDateLabel;
    QLabel *minimumDateLabel;
    QLabel *maximumDateLabel;
    QDateEdit *minimumDateEdit;
    QDateEdit *maximumDateEdit;
    QGroupBox *textFormatsGroupBox;
    QLabel *weekdayColorLabel;
    QLabel *weekendColorLabel;
    QLabel *headerTextFormatLabel;
    QComboBox *weekdayColorCombo;
    QComboBox *weekendColorCombo;
    QComboBox *headerTextFormatCombo;
    QCheckBox *firstFridayCheckBox;
    QCheckBox *mayFirstCheckBox;
};

//Thread per la gestione del refresh di Calendar,Tasks e ToDos
//in caso di modifiche parallele
class RefreshThread : public QThread {
    Q_OBJECT
signals:
    void signalRefresh();
private:
    void run();
};

#endif
