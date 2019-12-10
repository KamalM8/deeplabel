#ifndef METAOBJECT_H
#define METAOBJECT_H

#include <map>
#include <QString>

struct MetaObject{
    QString className;
    std::map<QString, std::vector<QString>> attributes;
    MetaObject(){}
    MetaObject(QString className) : className(className) {}
};

#endif // METAOBJECT_H
