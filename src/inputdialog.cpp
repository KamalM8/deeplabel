#include "inputdialog.h"
#include "ui_inputdialog.h"
#include <iostream>
#include <boundingbox.h>

InputDialog::InputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputDialog)
{
    ui->setupUi(this);
}

void clearLayout(QLayout* layout, bool deleteWidgets = true)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void InputDialog::load(bool selected){
    // TODO (kamal) refactor this function
    if (selected){
        //emit getMeta();
        selections.clear();
        int attribute_count = 0;
        for(auto &attribute : meta[box.classname].attributes){
            ui->gridLayout->addWidget(new QLabel(attribute.first), attribute_count, 0);
            selections.insert(std::make_pair(attribute.first, new QComboBox));
            for(auto &value : meta[box.classname].attributes[attribute.first])
                selections[attribute.first]->addItem(value);
            ui->gridLayout->addWidget(selections[attribute.first], attribute_count, 1);
            attribute_count++;
        }
        ui->classComboBox->setCurrentText(box.classname);
        ui->idLineEdit->setText(QString::number(box.id));
        for(auto &attribute : meta[ui->classComboBox->currentText()].attributes){
            selections[attribute.first]->setCurrentText(box.attributes[attribute.first]);
        }
    }else{
        box.attributes.clear();
        emit getMeta();
    }
}

InputDialog::~InputDialog()
{
    delete ui;
}

void InputDialog::updateMeta(std::map<QString, MetaObject> newMeta){
    meta = newMeta;
    ui->classComboBox->clear();
    selections.clear();
    int attribute_count = 0;
    // TODO (kamal) this could break if we have classes but no meta?
    if(!meta.empty() || !classes.empty()){
        ui->classComboBox->clear();
        for(auto &entry : classes)
            if(entry != "")
                ui->classComboBox->addItem(entry);
        for(auto &attribute : meta[ui->classComboBox->currentText()].attributes){
            ui->gridLayout->addWidget(new QLabel(attribute.first), attribute_count, 0);
            selections.insert(std::make_pair(attribute.first, new QComboBox));
            for(auto &value : meta[ui->classComboBox->currentText()].attributes[attribute.first])
                selections[attribute.first]->addItem(value);
            ui->gridLayout->addWidget(selections[attribute.first], attribute_count, 1);
            attribute_count++;
        }
    }
}

void InputDialog::on_classComboBox_activated(const QString &currentText)
{
    clearLayout(ui->gridLayout);

    selections.clear();
    int attribute_count = 0;
    if(!meta.empty()){
        for(auto &attribute : meta[currentText].attributes){
            ui->gridLayout->addWidget(new QLabel(attribute.first), attribute_count, 0);
            selections.insert(std::make_pair(attribute.first, new QComboBox));
            for(auto &value : meta[ui->classComboBox->currentText()].attributes[attribute.first])
                selections[attribute.first]->addItem(value);
            ui->gridLayout->addWidget(selections[attribute.first], attribute_count, 1);
            attribute_count++;
        }
    }
}

void InputDialog::on_buttonBox_accepted()
{
    box.attributes.clear();
    box.classname = ui->classComboBox->currentText();

    if (ui->idLineEdit->text() == "")
        box.id = -1;
    else
        box.id = ui->idLineEdit->text().toInt();

    for(auto &selection: selections){
        box.attributes.insert(std::make_pair(selection.first, selection.second->currentText()));
    }
    status = true;
    clearLayout(ui->gridLayout);
}

void InputDialog::getClassList(QList<QString> newClasses)
{
    classes = newClasses;
}

void InputDialog::on_buttonBox_rejected()
{
    status = false;
}
