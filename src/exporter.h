#ifndef EXPORTER_H
#define EXPORTER_H

#include <QObject>
#include <QDateTime>
#include <QImageReader>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>

#include <opencv2/opencv.hpp>
#include <random>

#include <labelproject.h>
#include <boundingbox.h>

class Exporter : public QObject
{
    Q_OBJECT
public:
    explicit Exporter(LabelProject *project, QObject *parent = nullptr);

signals:
    void export_progress(int);

public slots:
    bool setOutputFolder(QString folder);
    void setExportUnlabelled(bool res){export_unlabelled = res;}
    void setDataName(QString name){dataName = name;}
    void setContributer(QString name){contributer = name;}
    bool export_labels();
    //void process();

protected:
    LabelProject *project;
    QList<QString> images;
    QString contributer;
    QString dataName;

    QString output_folder;

    std::map<QString, int> id_map;

    int image_id;
    int label_id;

    bool export_unlabelled = false;
    bool saveImage(cv::Mat &image, const QString output, const double scale_x = -1.0, const double scale_y = -1.0);
};

#endif // EXPORTER_H
