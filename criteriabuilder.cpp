#include "criteriabuilder.h"
#include <QStringList>

QStringList CriteriaBuilder::toList()
{
    return m_list;
}

void CriteriaBuilder::insert(const QString key, const QVariant value) {
    QString typeName = value.typeName();
    QString format;
    bool isInt;
    value.toInt(&isInt);
    if(typeName.compare("QString")==0 && !isInt)
        format = "%1='%2'";
    else
        format = "%1=%2";
    m_list << QString(format).arg(key).arg(value.toString());
}

void CriteriaBuilder::insertLessOrEq(const QString key, const QVariant value) {
    if(!isNumeric(value)) return;
    m_list << QString("%1<=%2").arg(key).arg(value.toString());
}

void CriteriaBuilder::insertGrtOrEq(const QString key, const QVariant value) {
    if(!isNumeric(value)) return;
    m_list << QString("%1>=%2").arg(key).arg(value.toString());
}

void CriteriaBuilder::insertLess(const QString key, const QVariant value) {
    if(!isNumeric(value)) return;
    m_list << QString("%1<%2").arg(key).arg(value.toString());
}

void CriteriaBuilder::insertGrtr(const QString key, const QVariant value) {
    if(!isNumeric(value)) return;
    m_list << QString("%1>%2").arg(key).arg(value.toString());
}

bool CriteriaBuilder::isNumeric(QVariant value)
{
    QString typeName = QString(value.typeName());
    bool isInt      = (typeName == "int");
    bool isDouble   = (typeName == "double");
    bool isFloat    = (typeName == "float");
    bool isLong     = (typeName == "long");
    return isInt || isDouble || isFloat || isLong;
}




