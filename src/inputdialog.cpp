#include "inputdialog.h"
#include "ui_inputdialog.h"
#include <iostream>
#include <boundingbox.h>
#include <QPushButton>

InputDialog::InputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InputDialog)
{
    ui->setupUi(this);
}

void clearLayout(QLayout *layout) {
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
        delete item->layout();
        }
        if (item->widget()) {
           delete item->widget();
        }
        delete item;
    }
}


void InputDialog::fillAttributeForm(QString className){
    int attribute_count = 0;
    selections.clear();
    for(auto &attribute : meta[className].attributes){
        ui->gridLayout->addWidget(new QLabel(attribute.first), attribute_count, 0);
        selections.insert(std::make_pair(attribute.first, new QComboBox));
        for(auto &value : meta[className].attributes[attribute.first])
            selections[attribute.first]->addItem(value);
        ui->gridLayout->addWidget(selections[attribute.first], attribute_count, 1);
        attribute_count++;
    }
}

void InputDialog::load(bool selected){
    if (selected){
        clearLayout(ui->gridLayout);
        ui->classComboBox->clear();
        if(!classes.empty())
            for(auto &entry : classes)
                if(entry != "")
                    ui->classComboBox->addItem(entry);
        ui->classComboBox->setCurrentText(box.classname);
        ui->idLineEdit->setText(QString::number(box.id));
        if(!box.attributes.empty()){
            fillAttributeForm(box.classname);
            for(auto &attribute : meta[box.classname].attributes)
                selections[attribute.first]->setCurrentText(box.attributes[attribute.first]);
        }
    }else{
        box.attributes.clear();
        emit getMeta();
        ui->idLineEdit->setText(QString::number(emit getMaxID(ui->classComboBox->currentText()) + 1));
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
    clearLayout(ui->gridLayout);
    if(!classes.empty()){
        for(auto &entry : classes)
            if(entry != "")
                ui->classComboBox->addItem(entry);
    if(meta.find(ui->classComboBox->currentText()) != meta.end())
        fillAttributeForm(ui->classComboBox->currentText());
    }
}

void InputDialog::on_classComboBox_activated(const QString &currentText)
{
    clearLayout(ui->gridLayout);
    ui->idLineEdit->setText(QString::number(emit getMaxID(ui->classComboBox->currentText()) + 1));

    selections.clear();
    if(meta.find(currentText) != meta.end()){
        fillAttributeForm(currentText);
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

void InputDialog::on_idLineEdit_textEdited(const QString &user_id)
{
    bool accepted = emit checkDuplicateId(ui->classComboBox->currentText(), user_id);
    if (!accepted)
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    else
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}
