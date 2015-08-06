#ifndef ABSTRACTENTITY_H
#define ABSTRACTENTITY_H

#include <QObject>
#include <QMetaType>
#include <QVariantMap>
#include <QAbstractListModel>
#include <QMetaProperty>
#include <QSignalMapper>

#define TABLENAME_INFO              "__TABLENAME"
#define EXCLUDE_PROPERTY_INFO       "__EXCLUDE_PROP"
#define INCLUDE_PROPERTY_INFO       "__INCLUDE_PROP"
#define INCLUDE_INDEX_INFO          "__INDEX_PROP"
#define FIELD_SIZE_INFO(x)          "__FIELD_SIZE_"##x
#define FOREIGN_KEY_INFO            "__FOREIGN_KEY"

class AbstractEntity
        : public QObject
{
    Q_OBJECT

    friend class BaseEntity;
    friend class QDBManager;

public:
    explicit AbstractEntity(QObject* parent = 0);
    virtual bool equals(QObject* elem) = 0;

private:
    QVariantMap propertyMap;
};
Q_DECLARE_METATYPE(AbstractEntity*)

#endif // ABSTRACTENTITY_H
