/****************************************************************************
** Meta object code from reading C++ file 'subprocess.h'
**
** Created:
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "subprocess.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'subprocess.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SubProcess[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      73,   11,   11,   11, 0x08,
     103,   91,   11,   11, 0x08,
     151,   11,   11,   11, 0x08,
     180,  174,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SubProcess[] = {
    "SubProcess\0\0name,reply,errorCode\0"
    "processFinished(QString,QByteArray,int)\0"
    "handleReadyRead()\0code,status\0"
    "handleProcessFinished(int,QProcess::ExitStatus)\0"
    "handleProcessStarted()\0error\0"
    "handleProcessError(QProcess::ProcessError)\0"
};

void SubProcess::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SubProcess *_t = static_cast<SubProcess *>(_o);
        switch (_id) {
        case 0: _t->processFinished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->handleReadyRead(); break;
        case 2: _t->handleProcessFinished((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        case 3: _t->handleProcessStarted(); break;
        case 4: _t->handleProcessError((*reinterpret_cast< QProcess::ProcessError(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SubProcess::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SubProcess::staticMetaObject = {
    { &QProcess::staticMetaObject, qt_meta_stringdata_SubProcess,
      qt_meta_data_SubProcess, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SubProcess::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SubProcess::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SubProcess::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SubProcess))
        return static_cast<void*>(const_cast< SubProcess*>(this));
    return QProcess::qt_metacast(_clname);
}

int SubProcess::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QProcess::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void SubProcess::processFinished(const QString & _t1, const QByteArray & _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
