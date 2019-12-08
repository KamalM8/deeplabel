#include "configattributedialog.h"
#include "ui_configattributedialog.h"

ConfigAttributeDialog::ConfigAttributeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigAttributeDialog)
{
    ui->setupUi(this);
}

void ConfigAttributeDialog::getClassList(QList<QString> new_classes){
    classes = new_classes;
    ui->classComboBox->clear();

    QString classname;
    foreach(classname, classes){
        if(classname != "")
            ui->classComboBox->addItem(classname);
    }
}

ConfigAttributeDialog::~ConfigAttributeDialog()
{
    delete ui;
}

void ConfigAttributeDialog::on_addAttributeButton_clicked()
{

}

void ConfigAttributeDialog::on_deleteAttributeButton_clicked()
{

}

void ConfigAttributeDialog::on_addValueButton_clicked()
{

}

void ConfigAttributeDialog::on_deleteValueButton_clicked()
{

}
