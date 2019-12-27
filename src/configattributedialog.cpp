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
    // update dialog attributes and values if meta information exists
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
    // update dialog class list if list exists
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
    // add new attribute to form only
    bool ok;
    QString newAttribute = QInputDialog::getText(0, "Add new attribute", "New attribute: ",
                     QLineEdit::Normal, "", &ok);
    MetaObject* classInfo = &meta[ui->classComboBox->currentText()];
    if(ok && !newAttribute.isEmpty())
        classInfo->attributes[newAttribute];
    else
        return
    ui->attrComboBox->clear();

    for(auto iter = classInfo->attributes.begin(); iter != classInfo->attributes.end(); ++iter){
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
    //if value exists, delete all entries of that attribute
    emit deleteAttribute(ui->attrComboBox->currentText(), ui->classComboBox->currentText());
    MetaObject* classInfo = &meta[ui->classComboBox->currentText()];
    classInfo->attributes.erase(ui->attrComboBox->currentText());
    ui->attrComboBox->clear();

    for(auto iter = classInfo->attributes.rbegin(); iter!= classInfo->attributes.rend(); ++iter){
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
    // add value in both form and database (meta table)
    bool ok;
    QString newValue= QInputDialog::getText(0, "Add new value", "New value: ",
         QLineEdit::Normal, "", &ok);

    if(ok && !newValue.isEmpty())
        emit addValue(newValue, ui->attrComboBox->currentText(), ui->classComboBox->currentText());
    else
        return;

    ui->valueComboBox->clear();

    auto iterStart = meta[ui->classComboBox->currentText()].attributes[ui->attrComboBox->currentText()].rbegin();
    auto iterEnd= meta[ui->classComboBox->currentText()].attributes[ui->attrComboBox->currentText()].rend();
    for(; iterStart!=iterEnd; ++iterStart){
        if(*iterStart != "")
            ui->valueComboBox->addItem(*iterStart);
    }
}


void ConfigAttributeDialog::on_deleteValueButton_clicked()
{
    // delete value in both form and database (meta table)
    emit deleteValue(ui->valueComboBox->currentText(), ui->attrComboBox->currentText(), ui->classComboBox->currentText());
    ui->valueComboBox->clear();

    for(auto &value : meta[ui->classComboBox->currentText()].attributes[ui->attrComboBox->currentText()]){
        if(value != "")
            ui->valueComboBox->addItem(value);
    }
}

void ConfigAttributeDialog::on_classComboBox_currentIndexChanged(QString currentText)
{
    // update form to show attributes and values of current class
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
    // update form to show values of selected attribute
    ui->valueComboBox->clear();

    for(auto &value : meta[ui->classComboBox->currentText()].attributes[attribute]){
        if(value != "")
            ui->valueComboBox->addItem(value);
    }

    ui->valueComboBox->setCurrentIndex(0);
}

void ConfigAttributeDialog::updateMeta(std::map<QString, MetaObject> new_meta){
    // get updated meta from database interface
    meta = new_meta;
}
