/****************************************************************************
** Meta object code from reading C++ file 'editor_panel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ui/editor_panel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'editor_panel.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_memu8086__ui__EditorPanel_t {
    uint offsetsAndSizes[28];
    char stringdata0[26];
    char stringdata1[19];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[16];
    char stringdata5[20];
    char stringdata6[23];
    char stringdata7[14];
    char stringdata8[5];
    char stringdata9[3];
    char stringdata10[16];
    char stringdata11[27];
    char stringdata12[18];
    char stringdata13[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_memu8086__ui__EditorPanel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_memu8086__ui__EditorPanel_t qt_meta_stringdata_memu8086__ui__EditorPanel = {
    {
        QT_MOC_LITERAL(0, 25),  // "memu8086::ui::EditorPanel"
        QT_MOC_LITERAL(26, 18),  // "breakpoint_toggled"
        QT_MOC_LITERAL(45, 0),  // ""
        QT_MOC_LITERAL(46, 4),  // "line"
        QT_MOC_LITERAL(51, 15),  // "source_modified"
        QT_MOC_LITERAL(67, 19),  // "update_gutter_width"
        QT_MOC_LITERAL(87, 22),  // "highlight_current_line"
        QT_MOC_LITERAL(110, 13),  // "update_gutter"
        QT_MOC_LITERAL(124, 4),  // "rect"
        QT_MOC_LITERAL(129, 2),  // "dy"
        QT_MOC_LITERAL(132, 15),  // "on_text_changed"
        QT_MOC_LITERAL(148, 26),  // "on_cursor_position_changed"
        QT_MOC_LITERAL(175, 17),  // "show_find_replace"
        QT_MOC_LITERAL(193, 9)   // "find_next"
    },
    "memu8086::ui::EditorPanel",
    "breakpoint_toggled",
    "",
    "line",
    "source_modified",
    "update_gutter_width",
    "highlight_current_line",
    "update_gutter",
    "rect",
    "dy",
    "on_text_changed",
    "on_cursor_position_changed",
    "show_find_replace",
    "find_next"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_memu8086__ui__EditorPanel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   68,    2, 0x06,    1 /* Public */,
       4,    0,   71,    2, 0x06,    3 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       5,    0,   72,    2, 0x08,    4 /* Private */,
       6,    0,   73,    2, 0x08,    5 /* Private */,
       7,    2,   74,    2, 0x08,    6 /* Private */,
      10,    0,   79,    2, 0x08,    9 /* Private */,
      11,    0,   80,    2, 0x08,   10 /* Private */,
      12,    0,   81,    2, 0x08,   11 /* Private */,
      13,    0,   82,    2, 0x08,   12 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,    8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject memu8086::ui::EditorPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_memu8086__ui__EditorPanel.offsetsAndSizes,
    qt_meta_data_memu8086__ui__EditorPanel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_memu8086__ui__EditorPanel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<EditorPanel, std::true_type>,
        // method 'breakpoint_toggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'source_modified'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'update_gutter_width'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'highlight_current_line'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'update_gutter'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_text_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cursor_position_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'show_find_replace'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'find_next'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void memu8086::ui::EditorPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EditorPanel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->breakpoint_toggled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->source_modified(); break;
        case 2: _t->update_gutter_width(); break;
        case 3: _t->highlight_current_line(); break;
        case 4: _t->update_gutter((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 5: _t->on_text_changed(); break;
        case 6: _t->on_cursor_position_changed(); break;
        case 7: _t->show_find_replace(); break;
        case 8: _t->find_next(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EditorPanel::*)(int );
            if (_t _q_method = &EditorPanel::breakpoint_toggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (EditorPanel::*)();
            if (_t _q_method = &EditorPanel::source_modified; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject *memu8086::ui::EditorPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *memu8086::ui::EditorPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_memu8086__ui__EditorPanel.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int memu8086::ui::EditorPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void memu8086::ui::EditorPanel::breakpoint_toggled(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void memu8086::ui::EditorPanel::source_modified()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
