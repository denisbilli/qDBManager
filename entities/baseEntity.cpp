#include "baseEntity.h"
#include <tpmanager.h>

BaseEntity::BaseEntity(QObject *parent)
    : AbstractEntity(parent)
{
    //inizializzo l'id a -1 per far capire al motore DB che l'indice deve essere generato
    setObjectProperty("ID", -1);
    m_dirty = true;
    old_id = -1;
}

QStringList BaseEntity::calcExclusionList()
{
    if(m_exclusionList.count() <= 1) {
        for(int i = 0; i < this->metaObject()->classInfoCount(); ++i) {
            QString name = QString(this->metaObject()->classInfo(i).name());
            QString value = QString(this->metaObject()->classInfo(i).value()).toLower();
            if(name == EXCLUDE_PROPERTY_INFO) { if (!m_exclusionList.contains(value)) m_exclusionList<<value; }
        }
    }
    return m_exclusionList;
}

QVariant BaseEntity::getObjectProperty(QString property)
{
    if(calcExclusionList().contains(property.toLower()))
        return TPManager::getInstance()->getValue(QString("%1_%2").arg(getGuid()).arg(property));
    else
        return propertyMap.value(property);
}

qlonglong BaseEntity::getId()
{
   return propertyMap.value("ID", -1).toLongLong();
}

void BaseEntity::setId(qlonglong id)
{
    old_id = getId();
    propertyMap.insert("ID", id);
    emit changedId(id);
    emit changedGuid();
}

QString BaseEntity::getGuid()
{
    int tableIndex = this->metaObject()->indexOfClassInfo(TABLENAME_INFO);
    QString tableName = this->metaObject()->classInfo(tableIndex).value();
    return QString("%1_%2").arg(tableName).arg(getId());
}

void BaseEntity::setObjectProperty(QString property, QVariant value)
{
    if(calcExclusionList().contains(property.toLower()))
        TPManager::getInstance()->setValue(QString("%1_%2").arg(getGuid()).arg(property), value);
    else
        propertyMap.insert(property, value);
    m_dirty = true;
}
