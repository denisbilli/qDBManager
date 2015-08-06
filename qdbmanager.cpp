#include "QDBManager.h"

#include <QDir>
#include <QVariant>
#include <QUrl>
#include <QMetaProperty>
#include <QSqlQuery>

#include "interfaces/qsqlite.h"

//QDBManager* QDBManager::_instance = 0;

QDBManager::QDBManager(QObject *parent) : QObject(parent)
{
    m_hasForeignKeys = false;
}

QList<QString> QDBManager::getListOfTables()
{
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return QStringList();
        }
        alreadyOpenedDB = false;
    }

    QSqlQuery result = db.exec(list_of_tables_query());
    while(result.next()) {
        QString tableName = result.value(0).toString().replace("\"","");
        if(!listOfTables.contains(tableName))
            listOfTables << tableName;
    }

    result.clear();
    result.finish();

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return listOfTables;
}

BaseEntity* QDBManager::findById(QString table, QString entityName, int id)
{
    if(id<0) {
        return NULL;
    }

    BaseEntity* ref = 0;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        QString sqlQuery = QString(find_one_by_id_query()).arg(table).arg(id);
        QSqlQuery result;
        result.exec(sqlQuery+";");

        ref = newObject<BaseEntity>(entityName);

        if(ref != 0) {
            QMapIterator<QString, QVariant> i(ref->propertyMap);
            result.first();
            while (i.hasNext()) {
                i.next();
                ref->setObjectProperty(i.key(), result.value(i.key()));
            }
        }
        ref->resetDirty();

        result.clear();
        result.finish();
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return ref;
}

QList<BaseEntity *> QDBManager::find(QString table, QString entityName, QStringList criteria, QString orderBy, bool desc)
{
    QEntityList refList;

    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return refList;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        QString strCriteria;
        int cCount = 0;
        foreach (QString elCriteria, criteria) {
            strCriteria.append(elCriteria);
            if(cCount++ < criteria.length()-1)
                strCriteria.append(" AND ");
        }
        QString sqlQuery;
        if(orderBy.isEmpty()) {
            sqlQuery = QString(criteria_query()).arg(table).arg(strCriteria);
        } else {
            sqlQuery = QString(criteria_orderby_query()).arg(table).arg(strCriteria).arg(orderBy);
            if(desc) sqlQuery += " DESC";
        }
        QSqlQuery result;
        result.exec(sqlQuery+";");

        while(result.next()) {
            BaseEntity* ref = newObject<BaseEntity>(entityName);
            if(ref == 0) {
                continue;
            }
            QMapIterator<QString, QVariant> i(ref->propertyMap);
            while (i.hasNext()) {
                i.next();
                ref->setObjectProperty(i.key(), result.value(i.key()));
            }
            ref->resetDirty();
            refList<<ref;
        }

        result.clear();
        result.finish();
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return refList;
}

QList<BaseEntity *> QDBManager::entityComplexQuery(QString entityName, QString sqlQuery)
{
    QList<BaseEntity *> refList;

    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return refList;
        }
        alreadyOpenedDB = false;
    }

    QSqlQuery result;
    result.exec(sqlQuery.endsWith(";")?sqlQuery:sqlQuery+";");

    while(result.next()) {
        BaseEntity* ref = newObject<BaseEntity>(entityName);
        if(ref == 0) {
            continue;
        }
        QMapIterator<QString, QVariant> i(ref->propertyMap);
        while (i.hasNext()) {
            i.next();
            ref->setObjectProperty(i.key(), result.value(i.key()));
        }
        ref->resetDirty();
        refList<<ref;
    }

    result.clear();
    result.finish();

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return refList;
}

QList<BaseEntity *> QDBManager::internal_listAll(QString table, QString entityName, QString orderBy, bool desc)
{
    QList<BaseEntity *> refList;

    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return refList;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        QString sqlQuery;
        if(orderBy.isEmpty()) {
            sqlQuery = QString(find_all_query()).arg(table);
        } else {
            sqlQuery = QString(find_all_orderby_query()).arg(table).arg(orderBy);
            if(desc) sqlQuery += " DESC";
        }
        QSqlQuery result;
        result.exec(sqlQuery+";");

        while(result.next()) {
            BaseEntity* ref = newObject<BaseEntity>(entityName);
            if(ref == 0) {
                continue;
            }
            QMapIterator<QString, QVariant> i(ref->propertyMap);
            while (i.hasNext()) {
                i.next();
                QVariant value;
                int idx = ref->staticMetaObject.indexOfProperty(i.key().toStdString().c_str());
                if(QString(ref->staticMetaObject.property(idx).typeName()) == "bool") {
                    QVariant tmpVal = result.value(i.key());
                    value = tmpVal.toInt() == 1 ? true : false;
                } else
                    value = result.value(i.key());

                ref->setObjectProperty(i.key(), value);
            }
            ref->resetDirty();
            refList<<ref;
        }

        result.clear();
        result.finish();
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return refList;
}

QVariantList QDBManager::one_column_query(QString query)
{
    bool alreadyOpenedDB = true;
    QVariantList one_column_result;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return one_column_result;
        }
        alreadyOpenedDB = false;
    }

    QSqlQuery* result = new QSqlQuery(db);
    if(result->exec(query.endsWith(";")?query:query+";")) {
    } else {
    }

    while(result->next()) {
        one_column_result << result->value(0);
    }

    result->clear();
    result->finish();

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return one_column_result;
}

bool QDBManager::beginTransaction()
{
    return directCommand(begin_transaction());
}

bool QDBManager::beginTransaction(QString name)
{
    return directCommand(begin_transaction(name));
}

bool QDBManager::commitTransaction()
{
    return directCommand(commit_transaction());
}

bool QDBManager::commitTransaction(QString name)
{
    return directCommand(commit_transaction(name));
}

bool QDBManager::rollbackTransaction()
{
    return directCommand(rollback_transaction());
}

bool QDBManager::rollbackTransaction(QString name)
{
    return directCommand(rollback_transaction(name));
}

bool QDBManager::directCommand(QString command)
{
    bool success=true;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    QSqlQuery* result = new QSqlQuery(db);
    QString sql = command.endsWith(";")?command:command+";";
    success = result->exec(sql);

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return success;
}

void QDBManager::closeDB()
{
    QString connName = db.connectionName();
    db.close();
    db = QSqlDatabase();

#ifdef Q_OS_ANDROID

#endif

    db.removeDatabase(connName);
}

QSqlError QDBManager::lastError()
{
    // If opening database has failed user can ask
    // error description by QSqlError::text()
    return db.lastError();
}

bool QDBManager::deleteDB()
{
    // Close database
    db.close();

#ifdef Q_OS_LINUX
    // NOTE: We have to store database file into user home folder in Linux
    QString path(QDir::home().path());
    path.append(QDir::separator()).append("my.db.sqlite");
    path = QDir::toNativeSeparators(path);
    return QFile::remove(path);
#else

    // Remove created database binary file
    return QFile::remove("my.db.sqlite");
#endif
}

bool QDBManager::createTable(QMetaObject meta)
{
    int tableIndex = meta.indexOfClassInfo(TABLENAME_INFO);
    QString tableName = meta.classInfo(tableIndex).value();

    //verifico se la tabella esiste già nel DB e se sì entro in modalità edit (sync entity->DB)
    if(listOfTables.contains(QString(tableName)))
        return true;

    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    QStringList excludeList;
    QStringList listOfIndexColumns;
    QMap<QString,QString> foreignKeys;
    for(int i = 0; i < meta.classInfoCount(); ++i) {
        QString name = QString(meta.classInfo(i).name());
        QString value = QString(meta.classInfo(i).value());
        if(name == EXCLUDE_PROPERTY_INFO) { excludeList<<value; }
        if(name == INCLUDE_INDEX_INFO) { listOfIndexColumns<<value.toUpper(); }
        if(name == FOREIGN_KEY_INFO) {
            QStringList values = value.split(":", QString::SkipEmptyParts);
            values[2].replace("\"","");
            if(values[2] == Q_CASCADE_ALL) values[2] = cascade_all();
            if(values[2] == Q_CASCADE_DEL) values[2] = cascade_del();
            if(values[2] == Q_CASCADE_UPD) values[2] = cascade_upd();
            if(values[2] == Q_CASCADE_UPD_RESTRICT_DEL) values[2] = cascade_upd_restrict_del();
            foreignKeys.insert(values[0].toUpper(),QString("%1:%2").arg(values[1].toUpper()).arg(values[2].toUpper()));
            m_hasForeignKeys = true;

            QSqlQuery foreignKeyPragma(db);
            bool result = foreignKeyPragma.exec("PRAGMA foreign_keys = \"1\";");
        }
    }

    QStringList listOfProps;
    for(int i = 0; i < meta.propertyCount(); ++i) {
        QString name = QString(meta.property(i).name());
        QString type = QString(meta.property(i).typeName());

        if(name == "objectName" || name.startsWith("__") || excludeList.contains(name))
            continue;

        QString foreignKey = "";
        if(foreignKeys.contains(name.toUpper())) {
            QStringList values = foreignKeys.value(name.toUpper()).split(":", QString::SkipEmptyParts);
            foreignKey=QString("REFERENCES %1 %2").arg(values[0]).arg(values[1]);
        }

        if(type == "int" || type == "bool" || type == "qlonglong") {
            if(name.toLower() != "id") {
                listOfProps << QString("%1 INTEGER NULL %2").arg(name.toUpper()).arg(foreignKey).trimmed();
            } else {
                listOfProps << QString("id INTEGER PRIMARY KEY AUTOINCREMENT");
            }
        } else if(type == "QString") {
            listOfProps << QString("%1 TEXT NULL %2").arg(name.toUpper()).arg(foreignKey).trimmed();
        }
    }

    QString sqlTable = QString("CREATE TABLE %1 (").arg(tableName);
    for (int i = 0; i < listOfProps.size(); ++i) {
        sqlTable += listOfProps.at(i) + (i<listOfProps.size()-1?",":"");
    }
    sqlTable += ");";
    QSqlQuery query;
    bool ret = query.exec(sqlTable);

    // Get database given autoincrement value
    if (ret) {
        getListOfTables();

        if(listOfIndexColumns.count() > 0) {
            //creo l'indice, se richiesto
            //            QString sqlIndex;
            for (int i = 0; i < listOfIndexColumns.size(); ++i) {
                //                sqlIndex += listOfIndexColumns.at(i) + (i<listOfIndexColumns.size()-1?",":"");
                QString sqlIndexQuery = QString(create_index()).arg(tableName).arg(listOfIndexColumns.at(i));

                QSqlQuery query;
                query.exec(sqlIndexQuery);
            }
        }
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return ret;
}

/**
 * @brief QDBManager::syncEntityTable synchronizes the Entity with the DB's Table. The Entity always wins
 * @param table the table name
 * @param ref the Entity pointer
 * @return true if succeded
 */
bool QDBManager::syncEntityTable(QMetaObject meta) {
    bool ret = false;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    int tableIndex = meta.indexOfClassInfo(TABLENAME_INFO);
    QString tableName = meta.classInfo(tableIndex).value();

    if(getListOfTables().contains(tableName)) {
        QSqlQuery tableInfo(db);
        bool result = tableInfo.exec(QString("PRAGMA table_info( '%1' );").arg(tableName));
        if(!result) { return false; }

        QMap<QString, QString> dbFields;
        while(tableInfo.next()) {
            QString columnName = tableInfo.value(1).toString();
            QString columnType = tableInfo.value(2).toString();
            dbFields.insert(columnName, columnType);
        }

        //verifico se la tabella esiste già nel DB; se non esiste, chiudo
        if(!listOfTables.contains(QString(tableName)))
            return false;

        QStringList excludeList;
        for(int i = 0; i < meta.classInfoCount(); ++i) {
            QString name = QString(meta.classInfo(i).name());
            QString value = QString(meta.classInfo(i).value());
            if(name == EXCLUDE_PROPERTY_INFO) {
                excludeList<<value;
            }
        }

        QStringList listOfIndexColumns;
        for(int i = 0; i < meta.classInfoCount(); ++i) {
            QString name = QString(meta.classInfo(i).name());
            QString value = QString(meta.classInfo(i).value());
            if(name == INCLUDE_INDEX_INFO) {
                listOfIndexColumns<<value.toUpper();
            }
        }

        QStringList listOfProps;
        for(int i = 0; i < meta.propertyCount(); ++i) {
            QString name = QString(meta.property(i).name());
            QString type = QString(meta.property(i).typeName());

            if(name == "objectName" || name.startsWith("__") || excludeList.contains(name) || name.toLower() == "id")
                continue;

            if(!dbFields.contains(name.toUpper())) {
                //la colonna non esiste nel DB
                if(type == "int" || type == "bool" || type == "qlonglong") {
                    listOfProps << QString("%1 INTEGER NULL").arg(name.toUpper());
                } else if(type == "QString") {
                    listOfProps << QString("%1 TEXT NULL").arg(name.toUpper());
                }
            }
        }

        if(listOfProps.count() > 0) {
            for (int i = 0; i < listOfProps.size(); ++i) {
                QString sqlTable = QString("ALTER TABLE %1 ADD COLUMN %2;").arg(tableName).arg(listOfProps.at(i));
                QSqlQuery query;
                ret &= query.exec(sqlTable);
            }
        } else {
            ret = true;
        }
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return ret;
}

bool QDBManager::deleteTable(QMetaObject meta)
{
    bool ret = false;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    int tableIndex = meta.indexOfClassInfo(TABLENAME_INFO);
    QString tableName = meta.classInfo(tableIndex).value();

    if(getListOfTables().contains(tableName)) {
        QString sqlTable = QString("DROP TABLE %1").arg(tableName);
        QSqlQuery query;
        ret = query.exec(sqlTable);

        if(ret)
            listOfTables.removeOne(tableName);
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return ret;
}

int QDBManager::insert(QString sqlQuery)
{
    int newId = -1;
    bool ret = false;

    if (db.isOpen()) {
        QSqlQuery query;
        ret = query.exec(sqlQuery);

        // Get database given autoincrement value
        if (ret) {
            // http://www.sqlite.org/c3ref/last_insert_rowid.html
            newId = query.lastInsertId().toInt();
        }

    }
    return newId;
}

bool QDBManager::existsId(QString tableName, int id) {
    int count = 0;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(tableName)) {
        QSqlQuery result = db.exec(QString(count_by_id_query()).arg(tableName).arg(id));
        if(result.first()) {
            count = result.value(0).toInt();
        }
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return (count == 1);
}

bool QDBManager::existsEqualEntity(QString table, QString entityName, BaseEntity* entity)
{
    bool retValue = false;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        QList<BaseEntity*> list = internal_listAll(table, entityName);
        foreach (BaseEntity* curElement, list) {
            if(curElement->equals(entity)) {
                retValue = true;
                break;
            }
        }
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return retValue;
}

int QDBManager::newId(QString table)
{
    int retValue = -1;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        QString sqlQuery = QString(new_id_query()).arg(table);

        QSqlQuery result = db.exec(sqlQuery);

        if(result.next()) {
            retValue = result.value(0).toInt();
        } else {
            retValue = 0;
        }

        result.clear();
        result.finish();
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return retValue+1;
}

int QDBManager::update(QString table, BaseEntity *ref)
{
    int numRowsAffected = 0;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        const QMetaObject* meta = ref->metaObject();

        QStringList excludeList;
        for(int i = 0; i < meta->classInfoCount(); ++i) {
            QString name = QString(meta->classInfo(i).name());
            QString value = QString(meta->classInfo(i).value());
            if(name == "__EXCLUDE_PROP") {
                excludeList<<value;
            }
        }

        QStringList listOfProps;
        QStringList listOfPropNames;
        for(int i = 0; i < meta->propertyCount(); ++i) {
            QString name = QString(meta->property(i).name());
            QString type = QString(meta->property(i).typeName());

            if(name == "objectName" || name.startsWith("__") || excludeList.contains(name))
                continue;

            if(type == "int" || type == "qlonglong") {
                QVariant value = meta->property(i).read(ref);
                listOfPropNames << name;
                listOfProps << QString("%1").arg(value.toString());
            } else if(type == "QString") {
                QVariant value = meta->property(i).read(ref);
                QString strValue = QString(value.toString()).replace("'","''");
                listOfPropNames << name;
                listOfProps << QString("'%1'").arg(strValue);
            } else if(type == "bool") {
                QVariant value = meta->property(i).read(ref);
                listOfPropNames << name;
                listOfProps << QString("%1").arg(value.toBool() ? "1" : "0");
            }
        }

        QString args;
        for (int i = 0; i < listOfProps.size(); ++i) {
            args += listOfPropNames.at(i) + "=" + listOfProps.at(i) + (i<listOfPropNames.size()-1?",":"");
        }

        QString sql = QString(update_one_by_id_query()).arg(table).arg(args).arg(ref->old_id>=0 ? ref->old_id : ref->getId());
        QSqlQuery result = db.exec(sql);
        numRowsAffected = result.numRowsAffected();
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return numRowsAffected;
}

int QDBManager::insert(QString table, QString entityName, BaseEntity *ref, bool verifyUpdate)
{
    Q_UNUSED(entityName)

    int numRowsAffected = 0;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        const QMetaObject* meta = ref->metaObject();

        //Se l'ID esiste già nel DB, faccio l'update.
        //ATTENZIONE: per migliorare le performance l'update viene fatto in ogni caso. Se non torna nulla, allora faccio l'insert
        if(verifyUpdate || ref->old_id >= 1) {
            int retUpdate = update(table, ref);
            if(retUpdate >= 1) {
                return retUpdate;
            }
        }

        QStringList excludeList;
        for(int i = 0; i < meta->classInfoCount(); ++i) {
            QString name = QString(meta->classInfo(i).name());
            QString value = QString(meta->classInfo(i).value());
            if(name == EXCLUDE_PROPERTY_INFO) {
                excludeList<<value;
            }
        }

        QStringList listOfProps;
        QStringList listOfPropNames;
        for(int i = 0; i < meta->propertyCount(); ++i) {
            QString name = QString(meta->property(i).name());
            QString type = QString(meta->property(i).typeName());

            if(name == "objectName" || name.startsWith("__") || excludeList.contains(name))
                continue;

            if(type == "int" || type == "qlonglong") {
                QVariant value = meta->property(i).read(ref);
                if(name.toLower() == "id") {
                    listOfPropNames << name;
                    listOfProps << QString("NULL");
                } else {
                    listOfPropNames << name;
                    listOfProps << QString("%1").arg(value.toString());
                }
            } else if(type == "QString") {
                QVariant value = meta->property(i).read(ref);
                QString strValue = QString(value.toString()).replace("'","''");
                listOfPropNames << name;
                listOfProps << QString("'%1'").arg(strValue);
            } else if(type == "bool") {
                QVariant value = meta->property(i).read(ref);
                listOfPropNames << name;
                listOfProps << QString("%1").arg(value.toBool() ? "1" : "0");
            }
        }

        QString listOfArgs, listOfArgNames;
        for (int i = 0; i < listOfProps.size(); ++i) {
            listOfArgNames += listOfPropNames.at(i) + (i<listOfPropNames.size()-1?",":"");
            listOfArgs += listOfProps.at(i) + (i<listOfProps.size()-1?",":"");
        }

        QString sql = QString(insert_one_query()).arg(table).arg(listOfArgNames).arg(listOfArgs);
        if(!sql.endsWith(";")) sql += ";";

        QSqlQuery result;
        bool ret = result.exec(sql);
        numRowsAffected = result.numRowsAffected();

        if(ret && numRowsAffected == 1) ref->resetDirty();
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    return numRowsAffected;
}

int QDBManager::remove(QString table, QString entityName, BaseEntity *entity)
{
    int idx = -1;
    int numRowsAffected = 0;
    bool alreadyOpenedDB = true;
    //apro il DB se è chiuso
    if (!db.isOpen()) {
        bool openResult = openDB();
        if(!openResult) {
            return false;
        }
        alreadyOpenedDB = false;
    }

    if(getListOfTables().contains(table)) {
        QList<BaseEntity*> list = internal_listAll(table, entityName);
        foreach (BaseEntity* curElement, list) {
            if(curElement->equals(entity)) {
                idx = curElement->getId();
                break;
            }
        }

        QSqlQuery result;

        if(idx > 0) {
            QString sql = QString(delete_one_by_id_query()).arg(table).arg(idx);
            result = db.exec(sql);
            numRowsAffected = result.numRowsAffected();
        }
    }

    if(!alreadyOpenedDB) {
        //se al momento della chiamata il DB era chiuso, lo richiudo
        closeDB();
    }

    if(idx > 0)
        return numRowsAffected;
    else
        return 0;
}

/* FACTORY */
QDBManager *QDBManager::create(QString name, QString sqlDriver)
{
    static QMap<QString, QDBManager*> dbs;

    if(name.isEmpty()) name = CONTEXT_NAME;
    if(sqlDriver.isEmpty()) sqlDriver = CONTEXT_DB_DEF;

    if(dbs.contains(name)) {
        if(dbs[name]->getSqlDriver() == sqlDriver) {
            return dbs[name];
        }
    }
    if(sqlDriver == "QSQLITE") {
        dbs.insert(name, new qSqlite());
        return dbs[name];
    }
    return NULL;
}

