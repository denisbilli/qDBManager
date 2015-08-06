#ifndef CRITERIABUILDER_H
#define CRITERIABUILDER_H

#include <QObject>
#include <QMap>
#include <QVariant>

class CriteriaBuilder : public QObject
{
    Q_OBJECT
public:
    explicit CriteriaBuilder(QObject *parent = 0) : QObject(parent) {}

    QStringList toList();
    void insert(const QString key, const QVariant value);
    void insertLessOrEq(const QString key, const QVariant value);
    void insertGrtOrEq(const QString key, const QVariant value);
    void insertLess(const QString key, const QVariant value);
    void insertGrtr(const QString key, const QVariant value);
signals:

public slots:

private:
    bool isNumeric(QVariant value);
    QStringList m_list;
};

#endif // CRITERIABUILDER_H
