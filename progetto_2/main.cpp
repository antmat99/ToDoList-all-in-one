#include "notepad.h"

#include <QApplication>
#include <QPushButton>
#include <QGridLayout>
#include <QCalendarWidget>
#include <QTextEdit>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Notepad w;
    QWidget *window = new QWidget;
    QCalendarWidget *calendar = new QCalendarWidget();
        QPushButton *button1 = new QPushButton("One");
        QPushButton *button2 = new QPushButton("Two");
        QPushButton *button3 = new QPushButton("Three");
        QTextEdit *textEdit = new QTextEdit();

        QGridLayout *layout = new QGridLayout(window);
        layout->addWidget(calendar, 0, 0,3,2);
        layout->addWidget(button1, 0, 2);
        layout->addWidget(button2, 1, 2);
        layout->addWidget(button3, 2, 2);
        layout->addWidget(textEdit, 3, 0,3,3);
        window->show();
    //w.show();
    return a.exec();
}
