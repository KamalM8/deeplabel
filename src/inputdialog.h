#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>

#include "boundingbox.h"
#include "MetaObject.h"

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog
{
public:
    Q_OBJECT

public:
    explicit InputDialog(QWidget *parent = nullptr);
    BoundingBox box;
    void load(bool);
    bool status = false;
    ~InputDialog();

signals:
    void getMeta();

private slots:
    void updateMeta(std::map<QString, MetaObject>);

    void on_buttonBox_accepted();

    void on_classComboBox_activated(const QString &);

    void getClassList(QList<QString> classes);

    void on_buttonBox_rejected();

private:
    Ui::InputDialog *ui;
    std::map<QString, MetaObject> meta;
    QLabel* attributeLabel = new QLabel("attribute");
    QLabel* valueLabel = new QLabel("value");
    std::map<QString, QComboBox*> selections;
    QList<QString> classes;
};

#endif // INPUTDIALOG_H
