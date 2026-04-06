/****************************************************************************
** Meta object code from reading C++ file 'GlobalDictationService.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/services/GlobalDictationService.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GlobalDictationService.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
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
struct qt_meta_tag_ZN22GlobalDictationServiceE_t {};
} // unnamed namespace

template <> constexpr inline auto GlobalDictationService::qt_create_metaobjectdata<qt_meta_tag_ZN22GlobalDictationServiceE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "GlobalDictationService",
        "isActiveChanged",
        "",
        "isStoppingChanged",
        "isLoadingChanged",
        "enabledChanged",
        "autoPasteChanged",
        "overlayVisibleChanged",
        "silenceTimeoutChanged",
        "textUpdated",
        "dictationComplete",
        "textPasted",
        "errorOccurred",
        "error",
        "audioLevelChanged",
        "overlayPositionResetRequested",
        "onWhisperTextReady",
        "text",
        "timestampMs",
        "onParakeetTextReady",
        "toggle",
        "start",
        "stop",
        "preloadModel",
        "copyToClipboard",
        "clearText",
        "requestOverlayPositionReset",
        "isActive",
        "isStopping",
        "isLoading",
        "enabled",
        "autoPaste",
        "overlayVisible",
        "silenceTimeoutSecs",
        "currentText",
        "finalText",
        "audioLevel"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'isActiveChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isStoppingChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isLoadingChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'enabledChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'autoPasteChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'overlayVisibleChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'silenceTimeoutChanged'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'textUpdated'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'dictationComplete'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'textPasted'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'errorOccurred'
        QtMocHelpers::SignalData<void(const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 },
        }}),
        // Signal 'audioLevelChanged'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'overlayPositionResetRequested'
        QtMocHelpers::SignalData<void()>(15, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onWhisperTextReady'
        QtMocHelpers::SlotData<void(const QString &, qint64)>(16, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 17 }, { QMetaType::LongLong, 18 },
        }}),
        // Slot 'onParakeetTextReady'
        QtMocHelpers::SlotData<void(const QString &, qint64)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 17 }, { QMetaType::LongLong, 18 },
        }}),
        // Method 'toggle'
        QtMocHelpers::MethodData<void()>(20, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'start'
        QtMocHelpers::MethodData<void()>(21, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'stop'
        QtMocHelpers::MethodData<void()>(22, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'preloadModel'
        QtMocHelpers::MethodData<void()>(23, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'copyToClipboard'
        QtMocHelpers::MethodData<void()>(24, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'clearText'
        QtMocHelpers::MethodData<void()>(25, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'requestOverlayPositionReset'
        QtMocHelpers::MethodData<void()>(26, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'isActive'
        QtMocHelpers::PropertyData<bool>(27, QMetaType::Bool, QMC::DefaultPropertyFlags, 0),
        // property 'isStopping'
        QtMocHelpers::PropertyData<bool>(28, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
        // property 'isLoading'
        QtMocHelpers::PropertyData<bool>(29, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'enabled'
        QtMocHelpers::PropertyData<bool>(30, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'autoPaste'
        QtMocHelpers::PropertyData<bool>(31, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'overlayVisible'
        QtMocHelpers::PropertyData<bool>(32, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
        // property 'silenceTimeoutSecs'
        QtMocHelpers::PropertyData<int>(33, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 6),
        // property 'currentText'
        QtMocHelpers::PropertyData<QString>(34, QMetaType::QString, QMC::DefaultPropertyFlags, 7),
        // property 'finalText'
        QtMocHelpers::PropertyData<QString>(35, QMetaType::QString, QMC::DefaultPropertyFlags, 8),
        // property 'audioLevel'
        QtMocHelpers::PropertyData<qreal>(36, QMetaType::QReal, QMC::DefaultPropertyFlags, 11),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<GlobalDictationService, qt_meta_tag_ZN22GlobalDictationServiceE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject GlobalDictationService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22GlobalDictationServiceE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22GlobalDictationServiceE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN22GlobalDictationServiceE_t>.metaTypes,
    nullptr
} };

void GlobalDictationService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<GlobalDictationService *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->isActiveChanged(); break;
        case 1: _t->isStoppingChanged(); break;
        case 2: _t->isLoadingChanged(); break;
        case 3: _t->enabledChanged(); break;
        case 4: _t->autoPasteChanged(); break;
        case 5: _t->overlayVisibleChanged(); break;
        case 6: _t->silenceTimeoutChanged(); break;
        case 7: _t->textUpdated(); break;
        case 8: _t->dictationComplete(); break;
        case 9: _t->textPasted(); break;
        case 10: _t->errorOccurred((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->audioLevelChanged(); break;
        case 12: _t->overlayPositionResetRequested(); break;
        case 13: _t->onWhisperTextReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2]))); break;
        case 14: _t->onParakeetTextReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<qint64>>(_a[2]))); break;
        case 15: _t->toggle(); break;
        case 16: _t->start(); break;
        case 17: _t->stop(); break;
        case 18: _t->preloadModel(); break;
        case 19: _t->copyToClipboard(); break;
        case 20: _t->clearText(); break;
        case 21: _t->requestOverlayPositionReset(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::isActiveChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::isStoppingChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::isLoadingChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::enabledChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::autoPasteChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::overlayVisibleChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::silenceTimeoutChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::textUpdated, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::dictationComplete, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::textPasted, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)(const QString & )>(_a, &GlobalDictationService::errorOccurred, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::audioLevelChanged, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (GlobalDictationService::*)()>(_a, &GlobalDictationService::overlayPositionResetRequested, 12))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<bool*>(_v) = _t->isActive(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->isStopping(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->isLoading(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->isEnabled(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->autoPaste(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->overlayVisible(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->silenceTimeoutSecs(); break;
        case 7: *reinterpret_cast<QString*>(_v) = _t->currentText(); break;
        case 8: *reinterpret_cast<QString*>(_v) = _t->finalText(); break;
        case 9: *reinterpret_cast<qreal*>(_v) = _t->audioLevel(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 3: _t->setEnabled(*reinterpret_cast<bool*>(_v)); break;
        case 4: _t->setAutoPaste(*reinterpret_cast<bool*>(_v)); break;
        case 5: _t->setOverlayVisible(*reinterpret_cast<bool*>(_v)); break;
        case 6: _t->setSilenceTimeoutSecs(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *GlobalDictationService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GlobalDictationService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN22GlobalDictationServiceE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int GlobalDictationService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void GlobalDictationService::isActiveChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void GlobalDictationService::isStoppingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void GlobalDictationService::isLoadingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void GlobalDictationService::enabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void GlobalDictationService::autoPasteChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void GlobalDictationService::overlayVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void GlobalDictationService::silenceTimeoutChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void GlobalDictationService::textUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void GlobalDictationService::dictationComplete()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void GlobalDictationService::textPasted()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void GlobalDictationService::errorOccurred(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 10, nullptr, _t1);
}

// SIGNAL 11
void GlobalDictationService::audioLevelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void GlobalDictationService::overlayPositionResetRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}
QT_WARNING_POP
