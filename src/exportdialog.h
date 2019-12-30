#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QtDebug>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

    QString getOutputFolder(){return output_folder; }
    QString getContributer(){return contributer;}

private slots:
    void setOutputFolder(QString path="");
    void setContributer(QString name="");
private:
    Ui::ExportDialog *ui;
    bool checkOK();

    QSettings *settings;

    QString output_folder = "";
    QString names_file = "";
    QString contributer = "";

};

#endif // EXPORTDIALOG_H
