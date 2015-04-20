#ifndef BASEENTITY_H
#define BASEENTITY_H

#include <QObject>
#include <QMetaType>
#include <QVariantMap>
#include <QAbstractListModel>
#include <QMetaProperty>
#include "abstractEntity.h"
#include "../qdbmanager_global.h"

#define Q_TRANSIENT(property)       Q_CLASSINFO(EXCLUDE_PROPERTY_INFO,  #property)
#define Q_TABLENAME(tablename)      Q_CLASSINFO(TABLENAME_INFO, #tablename)
#define Q_INDEX(field)              Q_CLASSINFO(INCLUDE_INDEX_INFO, #field)
#define Q_INDEX_MUL(...)            Q_CLASSINFO(INCLUDE_INDEX_INFO, #__VA_ARGS__)

#define Q_FIELD(type,name)              Q_PROPERTY(type name READ get_##name WRITE set_##name NOTIFY changed_##name)
#define Q_TRANSIENT_FIELD(type,name)    Q_PROPERTY(type name READ get_##name WRITE set_##name NOTIFY changed_##name)\
                                        Q_TRANSIENT(name)

class BaseEntity
        : public AbstractEntity //, public QObjectCopyHelper<BaseEntity>
{
    Q_OBJECT
    Q_PROPERTY(qlonglong id         READ getId          WRITE setId         NOTIFY idChanged)
    Q_PROPERTY(QString __TABLENAME  READ tableName                                          )
public:
    explicit BaseEntity(QObject* parent = 0);

    qlonglong getId();
    void setId(qlonglong id);

    virtual QString toString() { return QString("0x%1").arg((quintptr)getId(), QT_POINTER_SIZE * 2, 16, QChar('0')); }

    static QString tableName();

signals:
    void idChanged(int id);

private:
    int id;
};
Q_DECLARE_METATYPE(BaseEntity*)

typedef QList<BaseEntity*>             QEntityList;

#endif // BASEENTITY_H
