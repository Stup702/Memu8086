/****************************************************************************
** Meta object code from reading C++ file 'memory_panel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/memory_panel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'memory_panel.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_memu8086__ui__HexView_t {
    uint offsetsAndSizes[14];
    char stringdata0[22];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[5];
    char stringdata5[10];
    char stringdata6[17];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_memu8086__ui__HexView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_memu8086__ui__HexView_t qt_meta_stringdata_memu8086__ui__HexView = {
    {
        QT_MOC_LITERAL(0, 21),  // "memu8086::ui::HexView"
        QT_MOC_LITERAL(22, 14),  // "jump_to_disasm"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 8),  // "uint32_t"
        QT_MOC_LITERAL(47, 4),  // "addr"
        QT_MOC_LITERAL(52, 9),  // "add_watch"
        QT_MOC_LITERAL(62, 16)   // "address_selected"
    },
    "memu8086::ui::HexView",
    "jump_to_disasm",
    "",
    "uint32_t",
    "addr",
    "add_watch",
    "address_selected"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_memu8086__ui__HexView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   32,    2, 0x06,    1 /* Public */,
       5,    1,   35,    2, 0x06,    3 /* Public */,
       6,    1,   38,    2, 0x06,    5 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

Q_CONSTINIT const QMetaObject memu8086::ui::HexView::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractScrollArea::staticMetaObject>(),
    qt_meta_stringdata_memu8086__ui__HexView.offsetsAndSizes,
    qt_meta_data_memu8086__ui__HexView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_memu8086__ui__HexView_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<HexView, std::true_type>,
        // method 'jump_to_disasm'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'add_watch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'address_selected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>
    >,
    nullptr
} };

void memu8086::ui::HexView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HexView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->jump_to_disasm((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1]))); break;
        case 1: _t->add_watch((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1]))); break;
        case 2: _t->address_selected((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (HexView::*)(uint32_t );
            if (_t _q_method = &HexView::jump_to_disasm; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (HexView::*)(uint32_t );
            if (_t _q_method = &HexView::add_watch; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (HexView::*)(uint32_t );
            if (_t _q_method = &HexView::address_selected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *memu8086::ui::HexView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *memu8086::ui::HexView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_memu8086__ui__HexView.stringdata0))
        return static_cast<void*>(this);
    return QAbstractScrollArea::qt_metacast(_clname);
}

int memu8086::ui::HexView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void memu8086::ui::HexView::jump_to_disasm(uint32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void memu8086::ui::HexView::add_watch(uint32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void memu8086::ui::HexView::address_selected(uint32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
namespace {
struct qt_meta_stringdata_memu8086__ui__MemoryPanel_t {
    uint offsetsAndSizes[30];
    char stringdata0[26];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[5];
    char stringdata5[15];
    char stringdata6[10];
    char stringdata7[14];
    char stringdata8[18];
    char stringdata9[15];
    char stringdata10[15];
    char stringdata11[17];
    char stringdata12[8];
    char stringdata13[17];
    char stringdata14[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_memu8086__ui__MemoryPanel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_memu8086__ui__MemoryPanel_t qt_meta_stringdata_memu8086__ui__MemoryPanel = {
    {
        QT_MOC_LITERAL(0, 25),  // "memu8086::ui::MemoryPanel"
        QT_MOC_LITERAL(26, 16),  // "address_selected"
        QT_MOC_LITERAL(43, 0),  // ""
        QT_MOC_LITERAL(44, 8),  // "uint32_t"
        QT_MOC_LITERAL(53, 4),  // "addr"
        QT_MOC_LITERAL(58, 14),  // "jump_to_disasm"
        QT_MOC_LITERAL(73, 9),  // "add_watch"
        QT_MOC_LITERAL(83, 13),  // "on_go_clicked"
        QT_MOC_LITERAL(97, 17),  // "on_search_changed"
        QT_MOC_LITERAL(115, 14),  // "on_search_next"
        QT_MOC_LITERAL(130, 14),  // "on_search_prev"
        QT_MOC_LITERAL(145, 16),  // "on_ascii_toggled"
        QT_MOC_LITERAL(162, 7),  // "checked"
        QT_MOC_LITERAL(170, 16),  // "on_width_changed"
        QT_MOC_LITERAL(187, 5)   // "index"
    },
    "memu8086::ui::MemoryPanel",
    "address_selected",
    "",
    "uint32_t",
    "addr",
    "jump_to_disasm",
    "add_watch",
    "on_go_clicked",
    "on_search_changed",
    "on_search_next",
    "on_search_prev",
    "on_ascii_toggled",
    "checked",
    "on_width_changed",
    "index"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_memu8086__ui__MemoryPanel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   68,    2, 0x06,    1 /* Public */,
       5,    1,   71,    2, 0x06,    3 /* Public */,
       6,    1,   74,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    0,   77,    2, 0x08,    7 /* Private */,
       8,    0,   78,    2, 0x08,    8 /* Private */,
       9,    0,   79,    2, 0x08,    9 /* Private */,
      10,    0,   80,    2, 0x08,   10 /* Private */,
      11,    1,   81,    2, 0x08,   11 /* Private */,
      13,    1,   84,    2, 0x08,   13 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void, QMetaType::Int,   14,

       0        // eod
};

Q_CONSTINIT const QMetaObject memu8086::ui::MemoryPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_memu8086__ui__MemoryPanel.offsetsAndSizes,
    qt_meta_data_memu8086__ui__MemoryPanel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_memu8086__ui__MemoryPanel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MemoryPanel, std::true_type>,
        // method 'address_selected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'jump_to_disasm'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'add_watch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        // method 'on_go_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_search_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_search_next'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_search_prev'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_ascii_toggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'on_width_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void memu8086::ui::MemoryPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MemoryPanel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->address_selected((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1]))); break;
        case 1: _t->jump_to_disasm((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1]))); break;
        case 2: _t->add_watch((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1]))); break;
        case 3: _t->on_go_clicked(); break;
        case 4: _t->on_search_changed(); break;
        case 5: _t->on_search_next(); break;
        case 6: _t->on_search_prev(); break;
        case 7: _t->on_ascii_toggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->on_width_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MemoryPanel::*)(uint32_t );
            if (_t _q_method = &MemoryPanel::address_selected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MemoryPanel::*)(uint32_t );
            if (_t _q_method = &MemoryPanel::jump_to_disasm; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MemoryPanel::*)(uint32_t );
            if (_t _q_method = &MemoryPanel::add_watch; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *memu8086::ui::MemoryPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *memu8086::ui::MemoryPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_memu8086__ui__MemoryPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int memu8086::ui::MemoryPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void memu8086::ui::MemoryPanel::address_selected(uint32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void memu8086::ui::MemoryPanel::jump_to_disasm(uint32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void memu8086::ui::MemoryPanel::add_watch(uint32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
