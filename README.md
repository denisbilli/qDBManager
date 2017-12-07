# qDBManager
This is a simple DBManager for SQLite on Qt Framework. It has been created since I needed an easy way to manage my SQLite database in my C++/Qt applications. I always worked with an Entity-like approach, and I wasn't able to find a similar approach that was a "pure" C++ solution (there is **QxOrm** but it uses boost, and I didn't want. Anyway you can find it here: http://www.qxorm.com/qxorm_en/home.html). Eventually I decided to do it myself.

# How it works
As already said, it is based on a pure Qt approach. The only thing you need to do is to include *qdbmanager.pri* inside your *.pro* file, like this:
```c++
include(../qdbmanager/qdbmanager.pri)
```

**Let's create an entity**

Now you have to create an Entity for our test. Let's call it *TestEntity*. I created a snippet in QtCreator in order to be very fast in this phase.
```c++
#ifndef TESTENTITY_H
#define TESTENTITY_H

#include "entities/BaseEntity.h"

class TestEntity : public BaseEntity
{
    Q_OBJECT
    Q_TABLENAME("TestEntity")
    
    /* TODO PROPERTIES */
    
public:
    explicit TestEntity(QObject* parent=0);
    
    /* TODO ACCESSORS */
    
    virtual QString toString() { 
        //TODO
        return QString(""); 
    }
    
    virtual bool equals(QObject *elem) { 
        //TODO
        return false;
    }
    
signals:
    /* TODO SIGNALS */
    
private:
    /* TODO STATUS VARIABLES */
};
Q_DECLARE_METATYPE(TestEntity*)

typedef QList<TestEntity*>          QListTestEntity;
typedef QMap<int,QListTestEntity>   QMapTestEntity;

#endif // TESTENTITY_H
```

Now, you have several possibilities to get it work. The first is to use my personal shortcuts:

* Q_FIELD([FIELD_TYPE],[FIELD_NAME]) => Creates a mapped field of that Type inside the Entity
* Q_TRANSIENT_FIELD([FIELD_TYPE],[FIELD_NAME]) => Creates an unmapped field of that Type inside the Entity (the same as Hibernate)
* Q_INDEX([FIELD_NAME]) => Creates an index with that field
* Q_INDEX_MUL([FIELD_NAME_1],[FIELD_NAME_2]) => Creates an index with those two fields

Let's say that you need two fields called *Name* and *Surname* mapped and indexed in the Entity, and a transient *Connected* field, you would do something like:

```c++
#ifndef TESTENTITY_H
#define TESTENTITY_H

#include "entities/BaseEntity.h"

class TestEntity : public BaseEntity
{
    Q_OBJECT
    Q_TABLENAME("TestEntity")

    Q_FIELD(QString,name)
    Q_FIELD(QString,surname)

    Q_TRANSIENT_FIELD(bool,connected)

    Q_INDEX(name)
    Q_INDEX(surname)

public:
    explicit TestEntity(QObject* parent=0);

    QString get_name();
    void set_name(QString name);

    QString get_surname();
    void set_surname(QString surname);

    bool get_connected();
    void set_connected(bool connected);

    virtual QString toString() { return get_name() + " " + get_surname(); }

    virtual bool equals(QObject *elem) { return qobject_cast<TestEntity*>(elem)->toString() == toString(); }

signals:
    void changed_name(QString name);
    void changed_surname(QString surname);
    void changed_connected(bool connected);

private:
    QString name;
    QString surname;
    bool connected;
};
Q_DECLARE_METATYPE(TestEntity*)

typedef QList<TestEntity*>          QListTestEntity;
typedef QMap<int,QListTestEntity>   QMapTestEntity;

#endif // TESTENTITY_H
```

```c++
#include "testentity.h"

TestEntity::TestEntity(QObject* parent) : BaseEntity(parent)
{
    propertyMap.insert("Name", "");
    propertyMap.insert("Surname", "");
    connected = false;
}

QString TestEntity::get_name()
{
    return propertyMap.value("Name").toString();
}

void TestEntity::set_name(QString name)
{
    propertyMap.insert("Name", name);
    emit changed_name(name);
}

QString TestEntity::get_surname()
{
    return propertyMap.value("Surname").toString();
}

void TestEntity::set_surname(QString surname)
{
    propertyMap.insert("Surname", surname);
    emit changed_surname(surname);
}

bool TestEntity::get_connected()
{
    return this->connected;
}

void TestEntity::set_connected(bool connected)
{
    this->connected = connected;
}
```

Now your *main.cpp* looks like this:

```c++
#include <QCoreApplication>

#include "qdbmanager.h"
#include "testentity.h"
#include <QDebug>

#include "interfaces/qsqlite.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDBManager *dbm = QDBManager::create("test", "QSQLITE");
    dbm->register_entity<TestEntity>();

    dbm->openDB("testdb.sqlite");
    dbm->createTable<TestEntity>();

    qDebug()<<"WRITE TEST";
    for (int i = 0; i < 10; ++i) {
        int id = i+1;
        TestEntity* testEntity = new TestEntity();
        testEntity->set_name("Leonard");
        testEntity->set_surname("Hofstadter");
        testEntity->set_connected(true);

        qDebug()<<"ID "<<id<<"\trows affected "<<dbm->insertOrUpdate<TestEntity>(testEntity);
    }

    QListTestEntity testEntities = dbm->listAll<TestEntity>();
    qDebug()<<testEntities.count();
    foreach (TestEntity* testEntity, testEntities) {
        qDebug()<<testEntity->toString()<<testEntity->get_connected();
    }

    dbm->closeDB();

    return a.exec();
}
```

As you can see, you must register your entity if you want to use it in your program. You must use the method 

```c++
dbm->register_entity<[ENTITY_CLASS_NAME]>();
```

before reading the Entity from the database, otherwise you will obtain a NULL object.

# Other functionalities
This version is fully capable of the basic CRUD operations, and also of retrieving entities from the database performing basic searches (using the Criteria Builder object), but also full queries. I didn't have the time to write a full Help, you may be able to find it yourself. If you are curious you can still write me anyway.

# TODO
- Only SQLite is supported for the moment, but I'm willing to support other RDBMS soon

You can write your own adapter extending the class **QDBManager** (have a look to the file interfaces/qsqlite.h).

# Conclusions
Hope to help someone with this lines. Every suggestion is appreciated.
