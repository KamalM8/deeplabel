#include "importer.h"

Importer::Importer(LabelProject *project, QObject *parent) : QObject(parent)
{
    this->project = project;
    project->getImageList(images);
}

bool Importer::load(QString filename){

    if (filename == "") return false;

    QFile file;
    file.setFileName(filename);
    bool ok = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if(!ok){
        qDebug()<<"Error, Cannot open file";
        return false;
    }

    QJsonDocument  jsonDoc = QJsonDocument::fromJson(file.readAll());
    data = jsonDoc.object();
    file.close();
    ok = loadClasses();
    ok &= loadMeta();
    ok &= loadLabels();
    if (ok)
        return true;
    else
        return false;
}

bool Importer::loadClasses(){
    QJsonArray classes = data.value("classes").toArray();
    bool ok = true;
    for(auto className : classes)
       ok &= project->addClass(className.toObject().value("name").toString());
    if(!ok){
        qDebug()<< "Could not load classes";
        return false;
    }
    else
        return true;
}

bool Importer::loadMeta(){
    QJsonArray meta = data.value("meta").toArray();
    bool status = true;
    QString className;
    for(auto metaObject : meta){
        for(auto& key: metaObject.toObject().keys()){
            // get class name first
            if (key == "class")
                className = metaObject.toObject().value(key).toString();
        }
        for(auto& key: metaObject.toObject().keys()){
            // add values for attributes
            if (key != "class"){
                QString attribute = key;
                for(auto value : metaObject.toObject().value(key).toArray())
                    status &= project->addValue(value.toString(), attribute, className);
            }
        }
    }
    return status;
}

bool Importer::loadLabels(){
    bool status = true;
    QJsonObject labels = data.value("annotation").toObject();
    int i = 0;
    for(auto image_id: labels.keys()){
        for(auto bboxLabel : labels.value(image_id).toArray()){
            BoundingBox bbox;
            bbox.classname = bboxLabel.toObject().value("class").toString();
            bbox.classid = project->getClassId(bbox.classname);
            bbox.id = bboxLabel.toObject().value("id").toInt();
            bbox.rect.setX(bboxLabel.toObject().value("x").toInt());
            bbox.rect.setY(bboxLabel.toObject().value("y").toInt());
            bbox.rect.setWidth(bboxLabel.toObject().value("w").toInt());
            bbox.rect.setHeight(bboxLabel.toObject().value("h").toInt());
            for(auto key: bboxLabel.toObject().value("attributes").toObject().keys()){
                QString value = bboxLabel.toObject().value("attributes").toObject().value(key).toString();
                bbox.attributes.insert(std::make_pair(key, value));
            }
            status &= project->addLabel(images[image_id.toInt() - 1], bbox);
        }
    }
    return status;
}
