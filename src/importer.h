#ifndef IMPORTER_H
#define IMPORTER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "labelproject.h"

class Importer : public QObject
{
    Q_OBJECT
public:
    explicit Importer(LabelProject *project, QObject *parent = nullptr);
    bool load(QString filename);
    bool loadClasses();
    bool loadMeta();
    bool loadLabels();

protected:
    LabelProject* project;
    QList<QString> images;
    QJsonObject data;


};

#endif // IMPORTER_H
