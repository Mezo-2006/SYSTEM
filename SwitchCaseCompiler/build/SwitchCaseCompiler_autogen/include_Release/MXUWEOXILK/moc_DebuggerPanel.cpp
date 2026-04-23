/****************************************************************************
** Meta object code from reading C++ file 'DebuggerPanel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/gui/DebuggerPanel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DebuggerPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN13DebuggerPanelE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN13DebuggerPanelE = QtMocHelpers::stringData(
    "DebuggerPanel",
    "stepChanged",
    "",
    "newIndex",
    "onPlayPause",
    "onReverse",
    "onStepForward",
    "onStepBackward",
    "onSliderMoved",
    "value",
    "onSpeedChanged",
    "updateCurrentStep",
    "onRuleDoubleClicked",
    "QTreeWidgetItem*",
    "item",
    "column"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN13DebuggerPanelE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   68,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,   71,    2, 0x08,    3 /* Private */,
       5,    0,   72,    2, 0x08,    4 /* Private */,
       6,    0,   73,    2, 0x08,    5 /* Private */,
       7,    0,   74,    2, 0x08,    6 /* Private */,
       8,    1,   75,    2, 0x08,    7 /* Private */,
      10,    1,   78,    2, 0x08,    9 /* Private */,
      11,    0,   81,    2, 0x08,   11 /* Private */,
      12,    2,   82,    2, 0x08,   12 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 13, QMetaType::Int,   14,   15,

       0        // eod
};

Q_CONSTINIT const QMetaObject DebuggerPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN13DebuggerPanelE.offsetsAndSizes,
    qt_meta_data_ZN13DebuggerPanelE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN13DebuggerPanelE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DebuggerPanel, std::true_type>,
        // method 'stepChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onPlayPause'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onReverse'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStepForward'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStepBackward'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSliderMoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSpeedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'updateCurrentStep'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRuleDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QTreeWidgetItem *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void DebuggerPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DebuggerPanel *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stepChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->onPlayPause(); break;
        case 2: _t->onReverse(); break;
        case 3: _t->onStepForward(); break;
        case 4: _t->onStepBackward(); break;
        case 5: _t->onSliderMoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->onSpeedChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->updateCurrentStep(); break;
        case 8: _t->onRuleDoubleClicked((*reinterpret_cast< std::add_pointer_t<QTreeWidgetItem*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (DebuggerPanel::*)(int );
            if (_q_method_type _q_method = &DebuggerPanel::stepChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *DebuggerPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DebuggerPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN13DebuggerPanelE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DebuggerPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void DebuggerPanel::stepChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
