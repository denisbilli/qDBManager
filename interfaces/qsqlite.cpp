#include "qsqlite.h"

qSqlite::qSqlite(QObject* parent) : QDBManager(parent)
{
    m_defaultFileName = "my.db.sqlite";
    m_sqlDriver = "QSQLITE";
}

bool qSqlite::openDB(QString connectionString)
{
    // Find QSLite driver
    db = QSqlDatabase::addDatabase("QSQLITE");

    if(!connectionString.isEmpty())
        db.setDatabaseName(connectionString);
    else {
#ifdef Q_OS_ANDROID
        QString dbFileName = QString("./%1").arg(m_defaultFileName);
        QFile dbFile(dbFileName);
        if (!dbFile.exists()) {
            QFile originalDbFile(QString("assets:/%1").arg(m_defaultFileName));
            originalDbFile.copy(dbFileName);
            QFile::setPermissions(dbFileName, QFile::WriteOwner | QFile::ReadOwner);
        }
        db.setDatabaseName(dbFileName);
#else

#ifdef Q_OS_LINUX
        // NOTE: We have to store database file into user home folder in Linux
        QString path(QDir::home().path());
        path.append(QDir::separator()).append(m_defaultFileName);
        path = QDir::toNativeSeparators(path);
        db.setDatabaseName(path);
#else
        // NOTE: File exists in the application private folder, in Symbian Qt implementation
        db.setDatabaseName(m_defaultFileName);
#endif

#endif
    }

    if (!db.isOpen()) {
        bool open = db.open();
        if(open) {
            //carico la lista delle tabelle
            getListOfTables();
        } else {
            emit error(DB_ERROR_OPEN, QT_TR_NOOP("Si è verificato un errore nell'apertura del Database"));
        }
        // Open database
        return open;
    } else {
        return true;
    }
    emit error(DB_ERROR_OPEN, QT_TR_NOOP("Si è verificato un errore nell'apertura del Database"));
    return false;
}

