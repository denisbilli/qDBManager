#ifndef TPMANAGER_H
#define TPMANAGER_H

#include <QObject>
#include <QVariantMap>

class TPManager : public QObject
{
    Q_OBJECT

protected:
    explicit TPManager(QObject *parent = 0);
    ~TPManager();

private:
    static TPManager* _instance;
    QVariantMap cache;

public:
    /* Singleton */
    static TPManager *getInstance();

    QVariant getValue(QString key);
    void setValue(QString key, QVariant value);
};

#endif // TPMANAGER_H
