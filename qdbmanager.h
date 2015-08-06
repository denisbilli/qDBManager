#ifndef QDBManager_H
#define QDBManager_H

//#include "qdbmanager_global.h"

#include <QMutex>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

#include "entities/baseEntity.h"
#include "criteriabuilder.h"
#include "entityfactory.h"

#ifndef CONTEXT_NAME
#define CONTEXT_NAME "default"
#endif

#ifndef CONTEXT_DB_DEF
#define CONTEXT_DB_DEF "QSQLITE"
#endif

#if defined(TOP_DOWN) && defined(BOTTOM_UP)
    #error Impossibile compilare con due versione di codice contrastanti
#endif

class QDBManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(Errors)

protected:
//    static QDBManager* _instance;

    bool m_hasForeignKeys;
    QString m_sqlDriver;
    QList<QString> listOfTables;

    /************************************************************************************
    * SQL FUNCTIONS
    ************************************************************************************/
    virtual QString list_of_tables_query() = 0;
    virtual QString criteria_query() = 0;
    virtual QString criteria_orderby_query() = 0;
    virtual QString find_one_query() = 0;
    virtual QString find_one_by_id_query() = 0;
    virtual QString delete_one_by_id_query() = 0;
    virtual QString find_all_query() = 0;
    virtual QString find_all_orderby_query() = 0;
    virtual QString insert_one_query() = 0;
    virtual QString count_by_id_query() = 0;
    virtual QString update_one_query() = 0;
    virtual QString update_one_by_id_query() = 0;
    virtual QString new_id_query() = 0;
    virtual QString create_index() = 0;

    virtual QString cascade_all() = 0;
    virtual QString cascade_upd() = 0;
    virtual QString cascade_del() = 0;
    virtual QString cascade_upd_restrict_del() = 0;

    virtual QString begin_transaction() = 0;
    virtual QString commit_transaction() = 0;
    virtual QString rollback_transaction() = 0;

    virtual QString begin_transaction(QString) = 0;
    virtual QString commit_transaction(QString) = 0;
    virtual QString rollback_transaction(QString) = 0;

    /************************************************************************************
    * PROTECTED METHODS
    ************************************************************************************/
    explicit QDBManager(QObject* parent = 0);
    explicit QDBManager(const QDBManager &);

    virtual QString getSqlDriver() { return m_sqlDriver; }

    /************************************************************************************/
private:
    QDBManager& operator=(const QDBManager &);

    /************************************************************************************/
public:
    QSqlDatabase db;
    static QDBManager *create(QString name="", QString sqlDriver="");

    /* 0-99 Errori */
    enum Errors {
        DB_ERROR_GENERIC        = 0,

        DB_ERROR_CREATE         = 1,
        DB_ERROR_OPEN           = 2,
        DB_ERROR_CLOSE          = 3,

        DB_ERROR_CREATE_TABLE   = 10,
        DB_ERROR_QUERY          = 11,
        DB_ERROR_INSERT_TABLE   = 12,
        DB_ERROR_DELETE_TABLE   = 13,
        DB_ERROR_UPDATE_TABLE   = 14,
        DB_ERROR_SQL_COMMAND    = 15
    };

    /*************************************************************************************
     * createTable - crea la tabella se non esiste. ritorna true in caso positivo
     * deleteTable - svuota ed elimina la tabella se esiste. ritorna true in caso positivo
     * createAllTables - crea tutte le tabelle del sistema (solo se non esistono)
     *
     * Tutte queste funzioni invocano automaticamente openDB nel caso la connessione
     * non sia aperta.
     ************************************************************************************/
    bool createTable(QMetaObject meta);
    bool syncEntityTable(QMetaObject meta);
    bool deleteTable(QMetaObject meta);
    bool createAllTables();
    QList<QString> getListOfTables();

    /*************************************************************************************
     * Usage functions. These have been set as private in date 09/01/2015 since the
     * user/developer must use templated version ones instead.
     ************************************************************************************/
    QList<BaseEntity *> find(QString table, QString entityName, QStringList criteria,
                             QString orderBy="", bool desc=false);
    QList<BaseEntity *> entityComplexQuery(QString entityName, QString query);
    BaseEntity* findById(QString table, QString entityName, int id);
    QList<BaseEntity *> internal_listAll(QString table, QString entityName, QString orderBy="",
                                         bool desc=false);
    QVariantList one_column_query(QString query);

    bool beginTransaction();
    bool beginTransaction(QString name);
    bool commitTransaction();
    bool commitTransaction(QString name);
    bool rollbackTransaction();
    bool rollbackTransaction(QString name);

    bool directCommand(QString command);

    bool containsTable(QMetaObject meta) {
        int idx = meta.indexOfClassInfo(TABLENAME_INFO);
        QString table = meta.classInfo(idx).value();
        return getListOfTables().contains(table);
    }

//    int insertOrUpdate(BaseEntity* entity) {
//        int idx = entity->staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
//        QString table = entity->staticMetaObject.classInfo(idx).value();
//        return insert(table, entity->staticMetaObject.className(), entity, true);
//    }

    bool existsId(QString tableName, int id);
    bool existsEqualEntity(QString table, QString entityName, BaseEntity* entity);

    int newId(QString table);

    int insert(QString table, QString entityName, BaseEntity *ref, bool verifyUpdate=false);
    int remove(QString table, QString entityName, BaseEntity *entity);
    int update(QString table, BaseEntity *ref);

    /*************************************************************************************
    * openDB - crea il DB se non esiste e apre la connessione. ritorna true in caso positivo
    * closeDB - chiude la connessione
    * deleteDB - cancella il DB se esiste. ritorna true in caso positivo
    ************************************************************************************/
    virtual bool openDB(QString connectionString = "") = 0;
    void closeDB();
    bool deleteDB();

    bool isOpen() { return db.isOpen(); }

    /*************************************************************************************
    * TEMPLATED FUNCTIONS
    ************************************************************************************/
    template<typename T>
    static void register_entity() { Register<T, BaseEntity>(T::staticMetaObject.className()); }

    template<typename T>
    static void register_entity(QString entityName) { Register<T, BaseEntity>(entityName); }

    template<typename T>
    bool createTable() { return createTable(T::staticMetaObject); }

    template<typename T>
    bool deleteTable() { return deleteTable(T::staticMetaObject); }

    template<typename T>
    bool sync() { return syncEntityTable(T::staticMetaObject); }

    template<typename T>
    bool containsTable() {
        return containsTable(T::staticMetaObject);
    }

    template<typename T>
    T* newEntity(QObject* parent, bool *result) {
        T* newObj = new T(parent);
        int val = insertOrUpdate<T>(newObj);
        *result = (val==1);

        return newObj;
    }

    template<typename T>
    QList<T*> find(QStringList criteria, QString orderBy="", bool desc=false) {
        int idx = T::staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
        QString table = T::staticMetaObject.classInfo(idx).value();
        QList<T*> list;
        QEntityList outList = find(table, T::staticMetaObject.className(), criteria, orderBy, desc);
        foreach (BaseEntity* entity, outList) {
            list << qobject_cast<T*>(entity);
        }
        return list;
    }

    template<typename T>
    QList<T*> find(QString criteria, QString orderBy="", bool desc=false) {
        QStringList criterias = criteria.split(",", QString::SkipEmptyParts);
        CriteriaBuilder criteriaBuilder;
        foreach (QString criteria, criterias) {
            QStringList keyVal = criteria.split("=", QString::SkipEmptyParts);
            if(keyVal.count() == 2 && !(keyVal[0].isEmpty() || keyVal[1].isEmpty()))
                criteriaBuilder.insert(keyVal[0].trimmed(),keyVal[1].trimmed());
        }
        return find<T>(criteriaBuilder.toList(), orderBy, desc);
    }

    template<typename T>
    QList<T*> query(QString query) {
        QList<T*> list;
        foreach (BaseEntity* entity, entityComplexQuery(T::staticMetaObject.className(), query)) {
            list << qobject_cast<T*>(entity);
        }
        return list;
    }

    template<typename T>
    QList<T*> type_query(QString query) {
        QList<T*> list;


        return list;
    }

    template<typename T>
    T* findById(int id) {
        int idx = T::staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
        QString table = T::staticMetaObject.classInfo(idx).value();
        return qobject_cast<T*>(findById(table, T::staticMetaObject.className(), id));
    }

    template<typename T>
    QList<T*> listAll(QString orderBy="", bool desc=false) {
        int idx = T::staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
        QString table = T::staticMetaObject.classInfo(idx).value();
        QList<T*> list;
        QList<BaseEntity*> baseList = internal_listAll(table, T::staticMetaObject.className(), orderBy, desc);
        foreach (BaseEntity* entity, baseList) {
            list << qobject_cast<T*>(entity);
        }
        return list;
    }

    template<typename T>
    int insert(BaseEntity* entity) {
        int idx = entity->metaObject()->indexOfClassInfo(TABLENAME_INFO);
        QString table = entity->metaObject()->classInfo(idx).value();
        return insert(table, entity->metaObject()->className(), entity, false);
    }

    template<typename T>
    int insertOrUpdate(BaseEntity* entity) {
        int idx = entity->metaObject()->indexOfClassInfo(TABLENAME_INFO);
        QString table = entity->metaObject()->classInfo(idx).value();
        return insert(table, entity->metaObject()->className(), entity, true);
    }

    template<typename T>
    int remove(BaseEntity* entity) {
        int idx = entity->metaObject()->indexOfClassInfo(TABLENAME_INFO);
        QString table = entity->metaObject()->classInfo(idx).value();
        return remove(table, entity->metaObject()->className(), entity);
    }

    template<typename T>
    int exists(BaseEntity* entity) {
        int idx = entity->metaObject()->indexOfClassInfo(TABLENAME_INFO);
        QString table = entity->metaObject()->classInfo(idx).value();
        return existsEqualEntity(table, entity->metaObject()->className(), entity);
    }

    template<typename T>
    bool existsId(int id) {
        int idx = T::staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
        QString table = T::staticMetaObject.classInfo(idx).value();
        return qobject_cast<T*>(existsId(table, id));
    }

    //questa funzione
    template<typename T>
    bool existsEqualEntity(int id) {
        int idx = T::staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
        QString table = T::staticMetaObject.classInfo(idx).value();
        return qobject_cast<T*>(existsEqualEntity(table, id));
    }

    template<typename T>
    int newId() {
        int idx = T::staticMetaObject.indexOfClassInfo(TABLENAME_INFO);
        QString table = T::staticMetaObject.classInfo(idx).value();
        return newId(table);
    }

    /*************************************************************************************
    * QUERY FUNCTIONS
    ************************************************************************************/
    int insert(QString sqlQuery);
    QSqlError lastError();
signals:
    void error(int numErr, QString description);

public slots:
};

#endif // QDBManager_H
