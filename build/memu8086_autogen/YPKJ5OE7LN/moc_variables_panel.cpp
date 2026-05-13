/****************************************************************************
** Meta object code from reading C++ file 'variables_panel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/variables_panel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'variables_panel.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_memu8086__ui__VariablesPanel_t {
    uint offsetsAndSizes[20];
    char stringdata0[29];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[5];
    char stringdata5[13];
    char stringdata6[13];
    char stringdata7[5];
    char stringdata8[18];
    char stringdata9[4];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_memu8086__ui__VariablesPanel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_memu8086__ui__VariablesPanel_t qt_meta_stringdata_memu8086__ui__VariablesPanel = {
    {
        QT_MOC_LITERAL(0, 28),  // "memu8086::ui::VariablesPanel"
        QT_MOC_LITERAL(29, 14),  // "show_in_memory"
        QT_MOC_LITERAL(44, 0),  // ""
        QT_MOC_LITERAL(45, 8),  // "uint32_t"
        QT_MOC_LITERAL(54, 4),  // "addr"
        QT_MOC_LITERAL(59, 12),  // "on_add_watch"
        QT_MOC_LITERAL(72, 12),  // "remove_watch"
        QT_MOC_LITERAL(85, 4),  // "name"
        QT_MOC_LITERAL(90, 17),  // "show_context_menu"
        QT_MOC_LITERAL(108, 3)   // "pos"
    },
    "memu8086::ui::VariablesPanel",
    "show_in_memory",
    "",
    "uint32_t",
    "addr",
    "on_add_watch",
    "remove_watch",
    "name",
    "show_context_menu",
    "pos"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_memu8086__ui__VariablesPanel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    0,   41,    2, 0x08,    3 /* Private */,
       6,    1,   42,    2, 0x08,    4 /* Private */,
       8,    1,   45,    2, 0x08,    6 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QPoint,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject memu8086::ui::VariablesPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_memu8086__ui__VariablesPanel.offsetsAndSizes,
    qt_meta_data_memu8086__ui__VariablesPanel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_memu8086__ui__VariablesPanel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VariablesPanel, std::true_type>,
        // method 'show_in_memory'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'on_add_watch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'remove_watch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'show_context_menu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>
    >,
    nullptr
} };

void memu8086::ui::VariablesPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VariablesPanel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->show_in_memory((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1]))); break;
        case 1: _t->on_add_watch(); break;
        case 2: _t->remove_watch((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->show_context_menu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VariablesPanel::*)(uint32_t );
            if (_t _q_method = &VariablesPanel::show_in_memory; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *memu8086::ui::VariablesPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *memu8086::ui::VariablesPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_memu8086__ui__VariablesPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int memu8086::ui::VariablesPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void memu8086::ui::VariablesPanel::show_in_memory(uint32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
