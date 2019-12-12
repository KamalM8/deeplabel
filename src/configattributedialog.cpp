#include "configattributedialog.h"
#include "ui_configattributedialog.h"
#include <iostream>
#include <algorithm>

ConfigAttributeDialog::ConfigAttributeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigAttributeDialog)
{
    ui->setupUi(this);
    ui->valueComboBox->setEnabled(false);
    ui->addValueButton->setEnabled(false);
    ui->deleteValueButton->setEnabled(false);
}

void ConfigAttributeDialog::load(){
    if(!meta.empty()){
        ui->attrComboBox->clear();
        for(auto &attribute : meta[ui->classComboBox->currentText()].attributes)
            if(attribute.first != "")
                ui->attrComboBox->addItem(attribute.first);
        ui->valueComboBox->clear();
        for(auto &value : meta[ui->classComboBox->currentText()].attributes[ui->attrComboBox->currentText()])
            if(value != "")
                ui->valueComboBox->addItem(value);
        ui->valueComboBox->setEnabled(true);
        ui->addValueButton->setEnabled(true);
        ui->deleteValueButton->setEnabled(true);
    }
}

void ConfigAttributeDialog::getClassList(QList<QString> new_classes){
    classes = new_classes;
    ui->classComboBox->clear();

    QString classname;
    foreach(classname, classes){
        if(classname != ""){
            ui->classComboBox->addItem(classname);
        }
    }
}

ConfigAttributeDialog::~ConfigAttributeDialog()
{
    delete ui;
}

void ConfigAttributeDialog::on_addAttributeButton_clicked()
{
    bool ok;
    QString newAttribute = QInputDialog::getText(0, "Add new attribute", "New attribute: ",
                     QLineEdit::Normal, "", &ok);
    MetaObject* classInfo = &meta[ui->classComboBox->currentText()];
    classInfo->attributes[newAttribute];
    ui->attrComboBox->clear();

    std::map<QString,std::vector<QString>>::iterator iter;
    for(iter = classInfo->attributes.begin(); iter != classInfo->attributes.end(); ++iter){
        if(iter->first != "")
            ui->attrComboBox->addItem(iter->first);
    }
    // TODO (kamal): show the latest addition on UI

    if(!classInfo->attributes.empty())
    {
        ui->valueComboBox->setEnabled(true);
        ui->addValueButton->setEnabled(true);
        ui->deleteValueButton->setEnabled(true);
    }
}

void ConfigAttributeDialog::on_deleteAttributeButton_clicked()
{
    //it values exist, delete all entries of that attribute
    emit deleteAttribute(ui->attrComboBox->currentText(), ui->classComboBox->currentText());
    MetaObject* classInfo = &meta[ui->classComboBox->currentText()];
    classInfo->attributes.erase(ui->attrComboBox->currentText());
    ui->attrComboBox->clear();

    std::map<QString,std::vector<QString>>::iterator iter;
    for(iter = classInfo->attributes.begin(); iter!= classInfo->attributes.end(); ++iter){
        if(iter->first != "")
            ui->attrComboBox->addItem(iter->first);
    }
    if (classInfo->attributes.empty())
    {
        ui->valueComboBox->setEnabled(false);
        ui->addValueButton->setEnabled(false);
        ui->deleteValueButton->setEnabled(false);
    }
}

void ConfigAttributeDialog::on_addValueButton_clicked()
{
    bool ok;
    QString newValue= QInputDialog::getText(0, "Add new value", "New value: ",
         QLineEdit::Normal, "", &ok);
    emit addValue(newValue, ui->attrComboBox->currentText(), ui->classComboBox->currentText());

    ui->valueComboBox->clear();
    for(auto &value : meta[ui->classComboBox->currentText()].attributes[ui->attrComboBox->currentText()]){
        if(value != "")
            ui->valueComboBox->addItem(value);
    }
    // TODO (kamal): show the latest addition on UI
}


void ConfigAttributeDialog::on_deleteValueButton_clicked()
{
    emit deleteValue(ui->valueComboBox->currentText(), ui->attrComboBox->currentText(), ui->classComboBox->currentText());
    ui->valueComboBox->clear();

    for(auto &value : meta[ui->classComboBox->currentText()].attributes[ui->attrComboBox->currentText()]){
        if(value != "")
            ui->valueComboBox->addItem(value);
    }
}

void ConfigAttributeDialog::on_classComboBox_currentIndexChanged(QString currentText)
{
    ui->attrComboBox->clear();
    if (currentText != ""){

        for(auto& attribute: meta[currentText].attributes){
        if(attribute.first != "")
            ui->attrComboBox->addItem(attribute.first);
        }

        ui->attrComboBox->setCurrentIndex(0);
    }
}

void ConfigAttributeDialog::on_attrComboBox_currentIndexChanged(QString attribute)
{
    ui->valueComboBox->clear();

    for(auto &value : meta[ui->classComboBox->currentText()].attributes[attribute]){
        if(value != "")
            ui->valueComboBox->addItem(value);
    }

    ui->valueComboBox->setCurrentIndex(0);
}

void ConfigAttributeDialog::updateMeta(std::map<QString, MetaObject> new_meta){
    meta = new_meta;
}
