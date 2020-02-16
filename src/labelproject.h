#ifndef LABELPROJECT_H
#define LABELPROJECT_H

#include <QObject>
#include <QApplication>
#include <QDir>
#include <QDirIterator>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QProgressDialog>

#include <QDebug>
#include <QMessageBox>
#include <algorithm>
#include <unordered_set>

#include <boundingbox.h>
#include <opencv2/opencv.hpp>
#include <MetaObject.h>

class LabelProject : public QObject
{
public:
    Q_OBJECT

public:
    explicit LabelProject(QObject *parent = nullptr);
    ~LabelProject();
    bool loadDatabase(QString fileName);
    bool createDatabase(QString fileName);
    bool addClass(QString className);
    //bool getClassList(QList<QString> &classes);
    bool removeClass(QString className);
    bool removeClassLabels(QString className);
    bool classInDB(QString classname);
    bool addAsset(QString fileName);
    void addVideo(QString fileName, QString outputFolder);
    bool getImageList(QList<QString> &images);
    bool getLabeledImagesID(QList<int> &images);
    bool getImageBboxes(int id, std::map<int, BoundingBox> &boxMap);
    bool removeImage(QString fileName);
    bool imageInDB(QString fileName);

    bool addLabel(QString fileName, BoundingBox bbox);
    bool getLabels(QString fileName, QList<BoundingBox> &bboxes);
    bool getLabels(int imageId, QList<BoundingBox> &bboxes);
    bool removeLabel(QString fileName, BoundingBox bbox);
    bool removeLabels(QString fileName);
    bool updateLabel(QString fileName, BoundingBox bbox);

    bool setOccluded(QString fileName, BoundingBox bbox, int occluded);

    int getNextUnlabelled(QString fileName);
    int getNextInstance(QString fileName, QString className);

    //QString getClassName(int classId);
    int getImageId(QString fileName);
    int getClassId(QString className);
    QString getClassName(int classID);
    void assignThread(QThread* thread);

    void getMetaInfo(std::map<QString, MetaObject> &metaInfo);
    bool loadMeta();

signals:
    void finished();
    void video_split_finished(QString);
    void load_finished();
    void load_progress(int);
    void updateMeta(std::map<QString, MetaObject> meta);

public slots:
    int sendMaxID(QString className);
    int addImageFolder(QString path);
    void cancelLoad();
    void addFolderRecursive(QString path_filter);
    bool addValue(QString newValue, QString currentAttribute, QString currentClass);
    bool deleteValue(QString value, QString currentAttribute, QString currentClass);
    bool getClassList(QList<QString> &classes);
    void sendMeta();
    bool checkDuplicateId(QString className, QString id);
    bool clearDatabaseLabels();

private:
    QSqlDatabase db;
    QMutex mutex;
    bool checkDatabase();
    bool should_cancel;
    QString connection_name;
    std::map<QString, MetaObject> meta;
};

#endif // LABELPROJECT_H
