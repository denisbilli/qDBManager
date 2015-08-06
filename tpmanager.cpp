#include "tpmanager.h"

TPManager* TPManager::_instance = 0;

TPManager *TPManager::getInstance()
{
    if (_instance == 0) {
        _instance = new TPManager;
    }
    return _instance;
}

QVariant TPManager::getValue(QString key)
{
    return cache.value(key);
}

void TPManager::setValue(QString key, QVariant value)
{
    cache.insert(key, value);
}

TPManager::TPManager(QObject *parent) : QObject(parent)
{

}

TPManager::~TPManager()
{

}

