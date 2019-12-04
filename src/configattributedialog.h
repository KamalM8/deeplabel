#ifndef CONFIGATTRIBUTEDIALOG_H
#define CONFIGATTRIBUTEDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigAttributeDialog;
}

class ConfigAttributeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigAttributeDialog(QWidget *parent = nullptr);
    ~ConfigAttributeDialog();

private slots:
    void getClassList(QList<QString>);
private:
    Ui::ConfigAttributeDialog *ui;
    QList<QString> classes;
};

#endif // CONFIGATTRIBUTEDIALOG_H
