/****************************************************************************
** Meta object code from reading C++ file 'DerivationViewer.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/gui/DerivationViewer.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DerivationViewer.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16DerivationViewerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN16DerivationViewerE = QtMocHelpers::stringData(
    "DerivationViewer",
    "onStepBack",
    "",
    "onStepForward",
    "onAutoPlay",
    "onAutoPlayTick",
    "onSpeedChanged",
    "value",
    "onStepSelected",
    "row",
    "onStepModeToggled",
    "enabled",
    "onStepScrubberMoved",
    "onExportRules"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN16DerivationViewerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x08,    1 /* Private */,
       3,    0,   69,    2, 0x08,    2 /* Private */,
       4,    0,   70,    2, 0x08,    3 /* Private */,
       5,    0,   71,    2, 0x08,    4 /* Private */,
       6,    1,   72,    2, 0x08,    5 /* Private */,
       8,    1,   75,    2, 0x08,    7 /* Private */,
      10,    1,   78,    2, 0x08,    9 /* Private */,
      12,    1,   81,    2, 0x08,   11 /* Private */,
      13,    0,   84,    2, 0x08,   13 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject DerivationViewer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZN16DerivationViewerE.offsetsAndSizes,
    qt_meta_data_ZN16DerivationViewerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN16DerivationViewerE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DerivationViewer, std::true_type>,
        // method 'onStepBack'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStepForward'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAutoPlay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAutoPlayTick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSpeedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onStepSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onStepModeToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onStepScrubberMoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onExportRules'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void DerivationViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DerivationViewer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onStepBack(); break;
        case 1: _t->onStepForward(); break;
        case 2: _t->onAutoPlay(); break;
        case 3: _t->onAutoPlayTick(); break;
        case 4: _t->onSpeedChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->onStepSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->onStepModeToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->onStepScrubberMoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onExportRules(); break;
        default: ;
        }
    }
}

const QMetaObject *DerivationViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DerivationViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN16DerivationViewerE.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DerivationViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_WARNING_POP
