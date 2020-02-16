#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QSettings>
#include <QTemporaryDir>
#include <QScrollArea>
#include <QScreen>
#include <QtConcurrent>
#include <QProgressDialog>

#include <opencv2/opencv.hpp>
#include <imagelabel.h>
#include <labelproject.h>
#include "exporter.h"

#include <detection/detectoropencv.h>
#include <detection/detectorsetupdialog.h>
#include <src/configureclassdialog.h>
#include <src/configattributedialog.h>

#include <algorithm>
#include <exportdialog.h>
#include <multitracker.h>
#include <imagedisplay.h>
#include <src/importer.h>

#include <QtAwesome.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    LabelProject *project = new LabelProject;
    ImageLabel *currentImage;
    ExportDialog *export_dialog;
    MultiTracker *multitracker;
    QScrollArea *imageScrollArea;
    ImageDisplay *display;
    DetectorOpenCV detector;
    ConfigAttributeDialog *attrDialog = new ConfigAttributeDialog;
    ConfigureClassDialog *classDialog = new ConfigureClassDialog;
    //InputDialog* inputDialog = new InputDialog;

    // Enable tracking boxes in previous frames
    bool track_previous = false;
    bool refine_on_propagate = false;
    bool wrap_index;
    int current_index;

    void initDisplay();

    QList<QString> images;
    QList<QString> classes;
    QString current_imagepath;
    QString current_class;
    QString projectName;

    int number_images;
    void updateImageList();
    void updateClassList();
    void updateLabels();

    QSettings* settings;


private slots:

    void updateDisplay(void);

    void openProject(QString filename = "");
    void mergeProject(QString filename = "");
    void newProject(void);

    void addClass(QString);
    //void setCurrentClass(QString);
    void removeClass(void);
    void removeClass(QString);

    void addVideo(void);
    void addImages(void);
    void addImageFolder(void);
    void addImageFolders();
    void nextImage(void);
    void previousImage(void);
    void removeImage(void);
    void importLabels();

    void addRemoveClass();
    void addRemoveAttributes();
    void clearLabels();

    void addLabel(BoundingBox bbox);
    void removeLabel(BoundingBox bbox);
    void updateLabel(BoundingBox old_bbox, BoundingBox new_bbox);
    void removeImageLabels(void);
    void removeImageLabelsForward();

    void enableWrap(bool enable);
    void launchExportDialog();
    void handleExportDialog();

    // Tracking
    void initTrackers();
    void updateTrackers();
    void toggleAutoPropagate(bool state);
    void nextUnlabelled();
    void nextInstance();

    void updateImageInfo();
    void updateLabelInfo(BoundingBox);
    void defaultLabelInfo();
    void jumpForward(int n = 10);
    void jumpBackward(int n = 10);

    void updateCurrentIndex(int index);
signals:
    void selectedClass(QString);
    void updateClassList(QList<QString>);
    void clearDatasetLabels();

};

#endif // MAINWINDOW_H
