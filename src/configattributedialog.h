#ifndef CONFIGATTRIBUTEDIALOG_H
#define CONFIGATTRIBUTEDIALOG_H

#include <QDialog>
#include <QInputDialog>

#include "labelproject.h"
#include "MetaObject.h"

namespace Ui {
class ConfigAttributeDialog;
}

class ConfigAttributeDialog : public QDialog
{
public:
    Q_OBJECT
public:
    explicit ConfigAttributeDialog(QWidget *parent = nullptr);
    ~ConfigAttributeDialog();
    void load();

signals:
    void deleteAttribute(QString attribute, QString currentClass);
    void addValue(QString newValue, QString currentAttribute, QString currentClass);
    void deleteValue(QString value, QString currentAttribute, QString currentClass);

private slots:
    void updateMeta(std::map<QString, MetaObject>);

    void getClassList(QList<QString>);

    void on_addAttributeButton_clicked();

    void on_deleteAttributeButton_clicked();

    void on_addValueButton_clicked();

    void on_deleteValueButton_clicked();

    void on_classComboBox_currentIndexChanged(QString currentText);

    void on_attrComboBox_currentIndexChanged(QString attribute);

private:
    Ui::ConfigAttributeDialog *ui;
    QList<QString> classes;
    std::map<QString, MetaObject> meta;
};

#endif // CONFIGATTRIBUTEDIALOG_H
