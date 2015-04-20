#include "abstractEntity.h"

AbstractEntity::AbstractEntity(QObject *parent)
    : QObject(parent)
{
}

QVariant AbstractEntity::getObjectProperty(QString property)
{
   return propertyMap.value(property, "");
}

void AbstractEntity::setObjectProperty(QString property, QVariant value)
{
    propertyMap.insert(property, value);
}
