#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>

#include "MetaObject.h"

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InputDialog(QWidget *parent = nullptr);
    ~InputDialog();

private slots:
    void updateMeta(std::map<int, MetaObject>);

    void on_classComboBox_currentIndexChanged(int index);
    
private:
    Ui::InputDialog *ui;
    std::map<int, MetaObject> meta;
    QLabel* attributeLabel = new QLabel("attribute");
    QLabel* valueLabel = new QLabel("value");

};

#endif // INPUTDIALOG_H
