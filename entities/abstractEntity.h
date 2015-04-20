#ifndef ABSTRACTENTITY_H
#define ABSTRACTENTITY_H

#include <QObject>
#include <QMetaType>
#include <QVariantMap>
#include <QAbstractListModel>
#include <QMetaProperty>

#define TABLENAME_INFO              "__TABLENAME"
#define EXCLUDE_PROPERTY_INFO       "__EXCLUDE_PROP"
#define INCLUDE_PROPERTY_INFO       "__INCLUDE_PROP"
#define INCLUDE_INDEX_INFO          "__INDEX_PROP"
#define FIELD_SIZE_INFO(x)          "__FIELD_SIZE_"##x

class AbstractEntity
        : public QObject
{
    Q_OBJECT
public:
    explicit AbstractEntity(QObject* parent = 0);
    QVariantMap propertyMap;

    virtual bool equals(QObject* elem) = 0;

private:
    QVariant getObjectProperty(QString property);
    void setObjectProperty(QString property, QVariant value);
};
Q_DECLARE_METATYPE(AbstractEntity*)

#endif // ABSTRACTENTITY_H
