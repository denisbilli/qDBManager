#ifndef FACTORY_H
#define FACTORY_H

#include <QMap>
#include <QString>

template<class Base> Base* newObject(const QString &);

template<class Base> class EntityFactory { // singleton class, one each base class
    EntityFactory() {}
    EntityFactory(const EntityFactory &);
    EntityFactory & operator=(const EntityFactory &);
    ~EntityFactory() { qDeleteAll(creators); }

    Base* create(const QString &nm) const {
        return (creators.contains(nm) ? creators[nm]->create() : NULL);
    }

    friend Base* newObject<>(const QString &);

public:
    class Creator { public:
                    virtual ~Creator() {}
                    virtual Base* create() const = 0;
                  };

    static EntityFactory& instance() {
        static EntityFactory pInstance;
        return pInstance;
    }

    void unregisterCreator(const QString &nm) {
        if (creators.contains(nm)) {
            delete creators[nm];
            creators.remove(nm);
        }
    }

    void registerCreator(const QString &nm, const Creator* c) {
        unregisterCreator(nm);
        creators.insert(nm, c);
    }

private:
    QMap<QString, const Creator*> creators;
};

template<class Base> Base* newObject(const QString &nm) {
    return EntityFactory<Base>::instance().create(nm);
}

template<class Derived, class Base> class Register {
    typedef Derived* (*CreateFn) ();
    class Creator : public EntityFactory<Base>::Creator
    {
        public:
            Creator(CreateFn fn) : createFn(fn) {}
            virtual Derived* create() const {
                return (createFn ? createFn() : new Derived(NULL));
            }
            CreateFn createFn;
  };

public:
    Register(const QString &nm, CreateFn fn = NULL) {
        EntityFactory<Base>::instance().registerCreator(nm, new Creator(fn));
    }
};

#endif
