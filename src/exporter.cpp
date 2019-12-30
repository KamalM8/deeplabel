#include "exporter.h"

Exporter::Exporter(LabelProject *project, QObject *parent) : QObject(parent)
{

    this->project = project;
    project->getImageList(images);
}


bool Exporter::setOutputFolder(const QString folder){

    if(folder == "") return false;

    output_folder = folder;

    //Make output folder if it doesn't exist
    if (!QDir(output_folder).exists()){
        qDebug() << "Making output folder" << output_folder;
        QDir().mkpath(output_folder);
    }

    return true;

}

bool Exporter::saveImage(cv::Mat &image, const QString output, const double scale_x, const double scale_y){

    if(image.rows == 0 || image.cols == 0){
        qDebug() << "Empty image ";
        return false;
    }

    if(scale_x > 0 && scale_y > 0)
        cv::resize(image, image, cv::Size(), scale_x, scale_y);

    std::vector<int> compression_params;

    // Png compression - maximum is super slow
    // TODO: add support to adjust this
    if(output.split(".").last().toLower() == "png"){
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(6);
    }

    return cv::imwrite(output.toStdString(), image, compression_params);
}

bool Exporter::export_labels(){

    // create .json label file with following name format
    // labels_dataName_contributer_timestamp.json
    auto now = QDateTime::currentDateTime();
    QString timestamp = now.date().toString(Qt::ISODate);
    QString label_filename = "label_"+QFileInfo(dataName).baseName()+"_"+contributer+"_"+timestamp+".json";
    QString label_filepath = QString("%1/%2").arg(output_folder).arg(label_filename);

    QJsonObject labelFile;
    QJsonObject labelInfo;

    // Save label info
    labelInfo["year"] = now.date().year();
    labelInfo["datasetName"] = QFileInfo(dataName).baseName();
    labelInfo["dateCreated"] = now.date().toString(Qt::ISODate);
    labelInfo["contributer"] = contributer;

    labelFile["info"] = labelInfo;

    // Save classes and ids
    QJsonArray classes;
    QList<QString> classnames;
    project->getClassList(classnames);

    for(auto &classname : classnames){
        QJsonObject category;
        category["id"] = project->getClassId(classname);
        category["name"] = classname;

        classes.append(category);
    }
    labelFile["classes"] = classes;

    // Save label meta information
    QJsonArray metaInfo;
    std::map<QString, MetaObject> meta;
    project->getMetaInfo(meta);
    for(auto& metaObject : meta){
        QJsonObject entry;
        if (metaObject.first != ""){
            entry["class"] = metaObject.first;
            for(auto& attribute: metaObject.second.attributes){
                QJsonArray values;
                for(auto& value: attribute.second)
                    values.append(value);
                entry[attribute.first] = values;
            }
            metaInfo.append(entry);
        }
    }

    labelFile["meta"] = metaInfo;

    // get distinct labeled images's ID in database
    QList<int> imagesID;
    project->getLabeledImagesID(imagesID);

    int imageID;
    QJsonObject image;
    //int i = 0;

    foreach(imageID, imagesID){
        std::map<int, BoundingBox> bboxes;
        project->getImageBboxes(imageID, bboxes);
        BoundingBox bbox;
        QJsonArray bboxArray;
        QJsonObject bboxInfo;
        for (auto& bbox: bboxes){
            bboxInfo["boxId"] = bbox.first;
            bboxInfo["x"] = bbox.second.rect.x();
            bboxInfo["y"] = bbox.second.rect.y();
            bboxInfo["w"] = bbox.second.rect.width();
            bboxInfo["h"] = bbox.second.rect.height();
            bboxInfo["id"] = bbox.second.id;
            bboxInfo["class"] = bbox.second.classname;
            QJsonObject attributeMap;
            for(auto& attribute: bbox.second.attributes){
                attributeMap[attribute.first] = attribute.second;
            }
            bboxInfo["attributes"] = attributeMap;
            bboxArray.append(bboxInfo);
        }
        image[QString::number(imageID)] = bboxArray;
        // TODO (kamal) this signal does nothing in original code
        //emit export_progress((100 * i++)/imagesID.size());
    }
    labelFile["annotation"] = image;

    QJsonDocument json_output(labelFile);
    QFile f(label_filepath);

    f.open(QIODevice::WriteOnly | QIODevice::Truncate);

    if(f.isOpen()){
        f.write(json_output.toJson());
    }

    f.close();

    return true;
}
