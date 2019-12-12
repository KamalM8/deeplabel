#ifndef CONFIGURECLASSDIALOG_H
#define CONFIGURECLASSDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigureClassDialog;
}

class ConfigureClassDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureClassDialog(QWidget *parent = nullptr);
    ~ConfigureClassDialog();
    void load();

signals:
    void addClass(QString);
    void deleteClass(QString);
    void loadClasses();

private slots:
    void on_addClass_clicked();

    void on_deleteClass_clicked();

    void getClassList(QList<QString>);

private:
    Ui::ConfigureClassDialog *ui;
    QList<QString> classes;
};

#endif // CONFIGURECLASSDIALOG_H
