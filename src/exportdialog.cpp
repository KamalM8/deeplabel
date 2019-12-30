#include "exportdialog.h"
#include "ui_exportdialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);

    connect(ui->outputFolderLineEdit, SIGNAL(textEdited(QString)), SLOT(setOutputFolder(QString)));
    connect(ui->contributerLineEdit, SIGNAL(textEdited(QString)), SLOT(setContributer(QString)));
    connect(ui->outputFolderPushButton, SIGNAL(clicked()), this, SLOT(setOutputFolder()));

    settings = new QSettings("DeepLabel", "DeepLabel");

    if(settings->contains("output_folder")){
        auto path = settings->value("output_folder").toString();
        if(path != ""){
            setOutputFolder(path);
        }
    }

}

ExportDialog::~ExportDialog()
{
    delete settings;
    delete ui;
}

void ExportDialog::setOutputFolder(QString path){

    if(path == ""){
        QString openDir;
        if(output_folder == ""){
             openDir = QDir::homePath();
        }else{
             openDir = output_folder;
        }

        path = QFileDialog::getExistingDirectory(this, tr("Select output folder"),
                                                        openDir);
    }

    if(path != ""){
        output_folder = path;
        ui->outputFolderLineEdit->setText(output_folder);
        settings->setValue("output_folder", output_folder);
    }

    checkOK();
}

void ExportDialog::setContributer(QString name){
    if(name != "")
        contributer = name;
}

bool ExportDialog::checkOK(){

    // If output folder exists
    if(!QDir(output_folder).exists() || output_folder == ""){
        qDebug() << "Export output folder doesn't exist";
        return false;
    }

    return true;
}
