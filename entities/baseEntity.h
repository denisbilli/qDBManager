#ifndef BASEENTITY_H
#define BASEENTITY_H

#include <QObject>
#include <QMetaType>
#include <QVariantMap>
#include <QAbstractListModel>
#include <QMetaProperty>
#include "abstractEntity.h"
#include <tpmanager.h>

/************************************************************************************* PRIVATE MACROS: THESE SHOULD NOT BE USED OUTSIDE */
#define __Q_PARENT_CLASS(name,table,field,cascadetype)  __Q_PARENT_CLASS_G(name : table ( field ) : cascadetype)
#define __Q_PARENT_CLASS_G(X)                           Q_CLASSINFO(FOREIGN_KEY_INFO, #X)
#define __Q_TRANSIENT(property)                         Q_CLASSINFO(EXCLUDE_PROPERTY_INFO,  #property)
#define __Q_PROPERTY(A)                                 Q_PROPERTY(A)

/****************************************************************************** PUBLIC MACROS: USE THESE FOR YOUR ENTITIES DECLARATIONS */

// Q_TABLENAME -> Defines the name of the DB Table of this Entity. This is NOT an optional field
#define Q_TABLENAME(tablename)                          Q_CLASSINFO(TABLENAME_INFO, #tablename)

#define Q_ATTR_GET(type,name,func)                      type get##name() { return get_##func(); }
#define Q_ATTR_SET(type,name,func)                      void set##name(type v) { set_##func(v); }

#define Q_ATTR_GET_2(type,name)                         type get##name() { return getObjectProperty(#name).value<type>(); }
#define Q_ATTR_SET_2(type,name)                         void set##name(type v) { setObjectProperty(#name, v); emit changed##name(v); }

#define Q_ATTR(type,name,func)                          Q_ATTR_GET(type,name,func)\
                                                        Q_ATTR_SET(type,name,func)

#define Q_ATTR_2(type,name)                             Q_ATTR_GET_2(type,name)\
                                                        Q_ATTR_SET_2(type,name)

// Q_INDEX_* -> Defines one or more INDEXES for the DB Table
#define Q_INDEX(field)                                  Q_CLASSINFO(INCLUDE_INDEX_INFO, #field)
#define Q_INDEX_MUL(...)                                Q_CLASSINFO(INCLUDE_INDEX_INFO, #__VA_ARGS__)

// Q_FIELD -> Defines a Q_OBJECT property available in QML that IS synched into the DB
#define Q_FIELD(type,name)                              __Q_PROPERTY(type name READ get_##name WRITE set_##name NOTIFY changed_##name)

#define Q_FIELD_2(type,bigname,name)                    __Q_PROPERTY(type name READ get##bigname WRITE set##bigname NOTIFY changed##bigname)

//#define Q_FIELD_3(type,bigname,name)                    Q_ATTR(type,bigname,name)\
//                                                        __Q_PROPERTY(type name READ get_##name WRITE set_##name NOTIFY changed_##name)

//#define Q_CREATE_FIELD(_1,_2,_3,NAME,...) NAME
//#define Q_FIELD(...) Q_CREATE_FIELD(__VA_ARGS__, Q_FIELD_3, Q_FIELD_2)(__VA_ARGS__)

// Q_FK_FIELD -> Defines a Q_OBJECT property available in QML that IS synched into the DB and that references another field of another table
#define Q_FK_FIELD(type,name,parent,field,cascadetype)  Q_FIELD(type,name)\
                                                        __Q_PARENT_CLASS(name,parent,field,cascadetype)

#define Q_FK_FIELD_2(type,name,parent,field,cascadetype)  Q_FIELD_2(type,parent,name)\
                                                        __Q_PARENT_CLASS(name,parent,field,cascadetype)

// Q_TRANSIENT_FIELD -> Defines a Q_OBJECT property available in QML that is NOT synched into the DB
#define Q_TRANSIENT_FIELD(type,name)                    __Q_PROPERTY(type name READ get_##name WRITE set_##name NOTIFY changed_##name)\
                                                        __Q_TRANSIENT(name)

#define Q_TRANSIENT_FIELD_2(type,bigname,name)          Q_FIELD_2(type,bigname,name)\
                                                        __Q_TRANSIENT(name)

// Q_READONLY_TRANSIENT_FIELD -> Defines a readonly Q_OBJECT property available in QML that is NOT synched into the DB
#define Q_READONLY_TRANSIENT_FIELD(type,name)           __Q_PROPERTY(type name READ get_##name                  NOTIFY changed_##name)\
                                                        __Q_TRANSIENT(name)

#define Q_READONLY_TRANSIENT_FIELD_2(type,bigname,name)  __Q_PROPERTY(type name READ get##bigname               NOTIFY changed##bigname)\
                                                        __Q_TRANSIENT(name)

#ifdef TOP_DOWN
#define Q_CHILDREN_LIST(X,type,name,field)              QList<type*> get_##name(QDBManager* dbm) {\
                                                            CriteriaBuilder criterias;\
                                                            criterias.insert(#field,this->getId());\
                                                            return dbm->find<type>(criterias.toList()); \
                                                        }\
                                                        QList<type*> get_##name() {\
                                                            QDBManager* dbm = QDBManager::create();\
                                                            return get_##name(dbm); \
                                                        }\

#define Q_FOREIGN_KEY_OBJ(table,entityName,name,func)   BaseEntity* get_obj_##name(QDBManager* dbm) {\
                                                            return dbm->findById(#table,#entityName,func()); \
                                                        }\
                                                        BaseEntity* get_obj_##name() {\
                                                            QDBManager* dbm = QDBManager::create();\
                                                            return get_obj_##name(dbm); \
                                                        }\

#endif

#ifdef BOTTOM_UP
#define Q_CHILDREN_LIST(table,entityName,name,field)    QEntityList get_##name(QDBManager* dbm) {\
                                                            CriteriaBuilder criterias;\
                                                            criterias.insert(#field,this->getId());\
                                                            return dbm->find(#table, #entityName, criterias.toList()); \
                                                        }\
                                                        QEntityList get_##name() {\
                                                            QDBManager* dbm = QDBManager::create();\
                                                            return get_##name(dbm); \
                                                        }\

#define Q_FOREIGN_KEY_OBJ(X,type,name,func)             type* get_obj_##name(QDBManager* dbm) {\
                                                            return dbm->findById<type>(func()); \
                                                        }\
                                                        type* get_obj_##name() {\
                                                            QDBManager* dbm = QDBManager::create();\
                                                            return get_obj_##name(dbm); \
                                                        }\

#endif

// CASCADE MODES for the FOREIGN KEYS
#define Q_CASCADE_ALL                                   "__CASCADE_ALL__"
#define Q_CASCADE_UPD                                   "__CASCADE_UPD__"
#define Q_CASCADE_DEL                                   "__CASCADE_DEL__"
#define Q_CASCADE_UPD_RESTRICT_DEL                      "__RESTRICT_DEL_CASCADE_UPD__"

class BaseEntity : public AbstractEntity
{
    Q_OBJECT

    friend class QDBManager;

    Q_READONLY_TRANSIENT_FIELD_2(QString,Guid,guid)
    Q_FIELD_2(qlonglong,Id,id)
public:
    explicit BaseEntity(QObject* parent = 0);

    bool isDirty() { return m_dirty; }

    qlonglong getId();
    void setId(qlonglong id);

    QString getGuid();

    virtual QString toString() { return QString("0x%1").arg((quintptr)getId(), QT_POINTER_SIZE * 2, 16, QChar('0')); }

signals:
    void changedId(int id);
    void changedGuid();

protected:
    QStringList m_exclusionList;
    QStringList calcExclusionList();
    void resetDirty() { m_dirty = false; old_id = -1; }

    QVariant getObjectProperty(QString property);
    void setObjectProperty(QString property, QVariant value);

private:

    qlonglong old_id;
    bool m_dirty;
};
Q_DECLARE_METATYPE(BaseEntity*)

typedef QList<BaseEntity*>             QEntityList;
typedef QMap<QString, BaseEntity*>     QMapEntity;

#endif // BASEENTITY_H
