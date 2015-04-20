#ifndef QSQLITE_H
#define QSQLITE_H

#include "../QDBManager.h"


class qSqlite : public QDBManager
{
    Q_OBJECT
private:
    QString m_defaultFileName;

protected:
    virtual QString list_of_tables_query()      { return "SELECT name FROM sqlite_master WHERE type='table'"; }
    virtual QString criteria_query()            { return "SELECT * FROM %1 WHERE %2"; }
    virtual QString criteria_orderby_query()    { return "SELECT * FROM %1 WHERE %2 ORDER BY %3"; }
    virtual QString find_one_query()            { return "SELECT * FROM %1 WHERE %1=%2 LIMIT 1"; }
    virtual QString find_one_by_id_query()      { return "SELECT * FROM %1 WHERE ID=%2 LIMIT 1"; }
    virtual QString delete_one_by_id_query()    { return "DELETE FROM %1 WHERE ID=%2;";  }
    virtual QString find_all_query()            { return "SELECT * FROM %1"; }
    virtual QString find_all_orderby_query()    { return "SELECT * FROM %1 ORDER BY %2";  }
    virtual QString insert_one_query()          { return "INSERT INTO %1 (%2) VALUES(%3)"; }
    virtual QString count_by_id_query()         { return "SELECT Count(*) FROM %1 WHERE ID=%2";  }
    virtual QString update_one_query()          { return "UPDATE %1 SET %2 WHERE %3"; }
    virtual QString update_one_by_id_query()    { return "UPDATE %1 SET %2 WHERE ID=%3" ; }
    virtual QString new_id_query()              { return "SELECT seq FROM sqlite_sequence WHERE name='%1'"; }
    virtual QString create_index()              { return "CREATE INDEX IF NOT EXISTS INDEX_%1 ON %1 (%2)"; }

public:
    explicit qSqlite(QObject* parent = 0);
    virtual bool openDB(QString connectionString);
};

#endif // QSQLITE_H
