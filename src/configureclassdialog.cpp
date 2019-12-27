#include "configureclassdialog.h"
#include "ui_configureclassdialog.h"
#include <iostream>

ConfigureClassDialog::ConfigureClassDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureClassDialog)
{
    ui->setupUi(this);
    ui->addClass->setEnabled(false);
    ui->deleteClass->setEnabled(false);
    ui->comboBox->setEnabled(false);

}

void ConfigureClassDialog::load(){
    // update class dialog with existing classes
    ui->comboBox->clear();
    if (!classes.empty())
       for(auto &className : classes)
           if(className != "")
               ui->comboBox->addItem(className);

    if(ui->comboBox->count() != 0){
        ui->comboBox->setEnabled(true);
        ui->deleteClass->setEnabled(true);
    }else{
        ui->comboBox->setEnabled(false);
        ui->deleteClass->setEnabled(false);
    }
}

ConfigureClassDialog::~ConfigureClassDialog()
{
    delete ui;
}

void ConfigureClassDialog::on_addClass_clicked()
{
    // add class to dialog and database (classes table)
    emit addClass(ui->addClassEdit->text());
    ui->comboBox->clear();
    ui->addClassEdit->clear();

    QString classname;
    std::reverse(classes.begin(), classes.end());
    foreach(classname, classes){
        if(classname != "")
            ui->comboBox->addItem(classname);
    }

    if(ui->comboBox->count() != 0){
        ui->comboBox->setEnabled(true);
        ui->deleteClass->setEnabled(true);
    }else{
        ui->comboBox->setEnabled(false);
        ui->deleteClass->setEnabled(false);
    }
}

void ConfigureClassDialog::on_deleteClass_clicked()
{
    // delete class from dialog and database (classes table)
    emit deleteClass(ui->comboBox->currentText());
    ui->comboBox->clear();

    QString classname;
    foreach(classname, classes){
        if(classname != "")
            ui->comboBox->addItem(classname);
    }
    if(ui->comboBox->count() != 0){
        ui->comboBox->setEnabled(true);
        ui->deleteClass->setEnabled(true);
    }else{
        ui->comboBox->setEnabled(false);
        ui->deleteClass->setEnabled(false);
    }
}

void ConfigureClassDialog::getClassList(QList<QString> newClasses)
{
    // get new classes from database interface
    classes = newClasses;
}

void ConfigureClassDialog::on_addClassEdit_textEdited(const QString &className)
{
    // disable button if field is empty
    if (className != "")
        ui->addClass->setEnabled(true);
    else
        ui->addClass->setEnabled(false);
}
