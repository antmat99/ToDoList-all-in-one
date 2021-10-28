#include "notepad.h"
#include "./ui_notepad.h"

#include <QPushButton>
#include <QGridLayout>

Notepad::Notepad(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Notepad)
{
    ui->setupUi(this);

}

Notepad::~Notepad()
{
    delete ui;
}

