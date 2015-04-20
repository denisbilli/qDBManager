#include "baseEntity.h"

BaseEntity::BaseEntity(QObject *parent)
    : AbstractEntity(parent)//, QObjectCopyHelper(this)
{
    //inizializzo l'id a -1 per far capire al motore DB che l'indice deve essere generato
    propertyMap.insert("ID", -1);
}

qlonglong BaseEntity::getId()
{
   return propertyMap.value("ID", -1).toLongLong();
}

void BaseEntity::setId(qlonglong id)
{
   propertyMap.insert("ID", id);
   emit idChanged(id);
}

QString BaseEntity::tableName()
{
    int tableIndex = staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
    return staticMetaObject.classInfo(tableIndex).value();
}
