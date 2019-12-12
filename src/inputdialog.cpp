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


InputDialog::~InputDialog()
{
    delete ui;
}

void InputDialog::updateMeta(std::map<QString, MetaObject> newMeta){
    meta = newMeta;
    ui->classComboBox->clear();
    int attribute_count = 0;
    if(!meta.empty()){
        ui->classComboBox->clear();
        for(auto &object: meta)
            if(object.second.className != "")
                ui->classComboBox->addItem(object.second.className);
        for(auto &attribute : meta[ui->classComboBox->currentText()].attributes){
            ui->gridLayout->addWidget(new QLabel(attribute.first), attribute_count, 0);
            selection.insert(std::make_pair(attribute.first, new QComboBox));
            for(auto &value : meta[ui->classComboBox->currentText()].attributes[attribute.first])
                selection[attribute.first]->addItem(value);
            ui->gridLayout->addWidget(selection[attribute.first], attribute_count, 1);
            attribute_count++;
        }
    }
}

void InputDialog::on_classComboBox_currentIndexChanged(QString currentText)
{
    clearLayout(ui->gridLayout);

    selection.clear();
    int attribute_count = 0;
    if(!meta.empty()){
        for(auto &attribute : meta[currentText].attributes){
            ui->gridLayout->addWidget(new QLabel(attribute.first), attribute_count, 0);
            selection.insert(std::make_pair(attribute.first, new QComboBox));
            for(auto &value : meta[ui->classComboBox->currentText()].attributes[attribute.first])
                selection[attribute.first]->addItem(value);
            ui->gridLayout->addWidget(selection[attribute.first], attribute_count, 1);
            attribute_count++;
        }
    }
}
