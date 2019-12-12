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
    std::map<QString, QComboBox*> selection;
    ~InputDialog();

private slots:
    void updateMeta(std::map<QString, MetaObject>);

    void on_classComboBox_currentIndexChanged(QString currentText);
    
private:
    Ui::InputDialog *ui;
    std::map<QString, MetaObject> meta;
    QLabel* attributeLabel = new QLabel("attribute");
    QLabel* valueLabel = new QLabel("value");
};

#endif // INPUTDIALOG_H
