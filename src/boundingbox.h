#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include<QString>
#include<QRect>
#include<map>

typedef struct{
    QRect rect = QRect(0,0,0,0);
    QString classname = "";
    int occluded = 0;
    bool truncated = false;
    int classid = 0;
    int id = 0;
    std::map<std::string, int> attributes;
    double confidence = 0;
} BoundingBox;

#endif // BOUNDINGBOX_H
