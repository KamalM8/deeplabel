#include "configureclassdialog.h"
#include "ui_configureclassdialog.h"
#include <iostream>

ConfigureClassDialog::ConfigureClassDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureClassDialog)
{
    ui->setupUi(this);
}

void ConfigureClassDialog::load()
{
   ui->comboBox->clear();
   if (!classes.empty())
       for(auto &className : classes)
           if(className != "")
               ui->comboBox->addItem(className);
}

ConfigureClassDialog::~ConfigureClassDialog()
{
    delete ui;
}

void ConfigureClassDialog::on_addClass_clicked()
{
    emit addClass(ui->addClassEdit->text());
    ui->comboBox->clear();

    QString classname;
    foreach(classname, classes){
        if(classname != "")
            ui->comboBox->addItem(classname);
    }
}

void ConfigureClassDialog::on_deleteClass_clicked()
{
    emit deleteClass(ui->comboBox->currentText());
    ui->comboBox->clear();

    QString classname;
    foreach(classname, classes){
        if(classname != "")
            ui->comboBox->addItem(classname);
    }
}

void ConfigureClassDialog::getClassList(QList<QString> newClasses)
{
    classes = newClasses;
}
