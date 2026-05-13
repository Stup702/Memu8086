/****************************************************************************
** Meta object code from reading C++ file 'toolbar.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/toolbar.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'toolbar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_memu8086__ui__Toolbar_t {
    uint offsetsAndSizes[26];
    char stringdata0[22];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[13];
    char stringdata5[13];
    char stringdata6[18];
    char stringdata7[14];
    char stringdata8[14];
    char stringdata9[4];
    char stringdata10[18];
    char stringdata11[6];
    char stringdata12[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_memu8086__ui__Toolbar_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_memu8086__ui__Toolbar_t qt_meta_stringdata_memu8086__ui__Toolbar = {
    {
        QT_MOC_LITERAL(0, 21),  // "memu8086::ui::Toolbar"
        QT_MOC_LITERAL(22, 16),  // "assemble_clicked"
        QT_MOC_LITERAL(39, 0),  // ""
        QT_MOC_LITERAL(40, 11),  // "run_clicked"
        QT_MOC_LITERAL(52, 12),  // "stop_clicked"
        QT_MOC_LITERAL(65, 12),  // "step_clicked"
        QT_MOC_LITERAL(78, 17),  // "step_over_clicked"
        QT_MOC_LITERAL(96, 13),  // "reset_clicked"
        QT_MOC_LITERAL(110, 13),  // "speed_changed"
        QT_MOC_LITERAL(124, 3),  // "ips"
        QT_MOC_LITERAL(128, 17),  // "on_slider_changed"
        QT_MOC_LITERAL(146, 5),  // "value"
        QT_MOC_LITERAL(152, 14)   // "animate_status"
    },
    "memu8086::ui::Toolbar",
    "assemble_clicked",
    "",
    "run_clicked",
    "stop_clicked",
    "step_clicked",
    "step_over_clicked",
    "reset_clicked",
    "speed_changed",
    "ips",
    "on_slider_changed",
    "value",
    "animate_status"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_memu8086__ui__Toolbar[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    1 /* Public */,
       3,    0,   69,    2, 0x06,    2 /* Public */,
       4,    0,   70,    2, 0x06,    3 /* Public */,
       5,    0,   71,    2, 0x06,    4 /* Public */,
       6,    0,   72,    2, 0x06,    5 /* Public */,
       7,    0,   73,    2, 0x06,    6 /* Public */,
       8,    1,   74,    2, 0x06,    7 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    1,   77,    2, 0x08,    9 /* Private */,
      12,    0,   80,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,    9,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   11,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject memu8086::ui::Toolbar::staticMetaObject = { {
    QMetaObject::SuperData::link<QToolBar::staticMetaObject>(),
    qt_meta_stringdata_memu8086__ui__Toolbar.offsetsAndSizes,
    qt_meta_data_memu8086__ui__Toolbar,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_memu8086__ui__Toolbar_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Toolbar, std::true_type>,
        // method 'assemble_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'run_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stop_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'step_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'step_over_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'reset_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'speed_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<float, std::false_type>,
        // method 'on_slider_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'animate_status'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void memu8086::ui::Toolbar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Toolbar *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->assemble_clicked(); break;
        case 1: _t->run_clicked(); break;
        case 2: _t->stop_clicked(); break;
        case 3: _t->step_clicked(); break;
        case 4: _t->step_over_clicked(); break;
        case 5: _t->reset_clicked(); break;
        case 6: _t->speed_changed((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 7: _t->on_slider_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->animate_status(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Toolbar::*)();
            if (_t _q_method = &Toolbar::assemble_clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Toolbar::*)();
            if (_t _q_method = &Toolbar::run_clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Toolbar::*)();
            if (_t _q_method = &Toolbar::stop_clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Toolbar::*)();
            if (_t _q_method = &Toolbar::step_clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Toolbar::*)();
            if (_t _q_method = &Toolbar::step_over_clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Toolbar::*)();
            if (_t _q_method = &Toolbar::reset_clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (Toolbar::*)(float );
            if (_t _q_method = &Toolbar::speed_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *memu8086::ui::Toolbar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *memu8086::ui::Toolbar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_memu8086__ui__Toolbar.stringdata0))
        return static_cast<void*>(this);
    return QToolBar::qt_metacast(_clname);
}

int memu8086::ui::Toolbar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void memu8086::ui::Toolbar::assemble_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void memu8086::ui::Toolbar::run_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void memu8086::ui::Toolbar::stop_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void memu8086::ui::Toolbar::step_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void memu8086::ui::Toolbar::step_over_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void memu8086::ui::Toolbar::reset_clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void memu8086::ui::Toolbar::speed_changed(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
