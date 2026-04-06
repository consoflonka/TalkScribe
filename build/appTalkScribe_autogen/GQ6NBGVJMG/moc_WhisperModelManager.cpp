/****************************************************************************
** Meta object code from reading C++ file 'WhisperModelManager.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/services/WhisperModelManager.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WhisperModelManager.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN19WhisperModelManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto WhisperModelManager::qt_create_metaobjectdata<qt_meta_tag_ZN19WhisperModelManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "WhisperModelManager",
        "availableModelsChanged",
        "",
        "activeModelChanged",
        "isDownloadingChanged",
        "downloadProgressChanged",
        "downloadErrorChanged",
        "modelReady",
        "modelId",
        "modelDeleted",
        "threadCountChanged",
        "autoDetectLanguageChanged",
        "selectedLanguageChanged",
        "globalModeChanged",
        "benchmarkResult",
        "msPerChunk",
        "isModelAvailable",
        "downloadModel",
        "cancelDownload",
        "deleteModel",
        "setActiveModel",
        "totalDiskUsage",
        "setThreadCount",
        "count",
        "setAutoDetectLanguage",
        "enabled",
        "setSelectedLanguage",
        "lang",
        "setGlobalMode",
        "mode",
        "benchmarkModel",
        "deviceCanHandleLocal",
        "recommendedMode",
        "availableModels",
        "QVariantList",
        "activeModelId",
        "isDownloading",
        "downloadProgress",
        "downloadError",
        "gpuAccelerated",
        "gpuBackend",
        "threadCount",
        "autoDetectLanguage",
        "onnxAvailable",
        "globalMode",
        "selectedLanguage"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'availableModelsChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'activeModelChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'isDownloadingChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'downloadProgressChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'downloadErrorChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'modelReady'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Signal 'modelDeleted'
        QtMocHelpers::SignalData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Signal 'threadCountChanged'
        QtMocHelpers::SignalData<void()>(10, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'autoDetectLanguageChanged'
        QtMocHelpers::SignalData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'selectedLanguageChanged'
        QtMocHelpers::SignalData<void()>(12, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'globalModeChanged'
        QtMocHelpers::SignalData<void()>(13, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'benchmarkResult'
        QtMocHelpers::SignalData<void(const QString &, int)>(14, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 }, { QMetaType::Int, 15 },
        }}),
        // Method 'isModelAvailable'
        QtMocHelpers::MethodData<bool(const QString &) const>(16, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 8 },
        }}),
        // Method 'downloadModel'
        QtMocHelpers::MethodData<void(const QString &)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Method 'cancelDownload'
        QtMocHelpers::MethodData<void()>(18, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'deleteModel'
        QtMocHelpers::MethodData<void(const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Method 'setActiveModel'
        QtMocHelpers::MethodData<void(const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Method 'totalDiskUsage'
        QtMocHelpers::MethodData<qint64() const>(21, 2, QMC::AccessPublic, QMetaType::LongLong),
        // Method 'setThreadCount'
        QtMocHelpers::MethodData<void(int)>(22, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 23 },
        }}),
        // Method 'setAutoDetectLanguage'
        QtMocHelpers::MethodData<void(bool)>(24, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 25 },
        }}),
        // Method 'setSelectedLanguage'
        QtMocHelpers::MethodData<void(const QString &)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 27 },
        }}),
        // Method 'setGlobalMode'
        QtMocHelpers::MethodData<void(const QString &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 29 },
        }}),
        // Method 'benchmarkModel'
        QtMocHelpers::MethodData<void(const QString &)>(30, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Method 'deviceCanHandleLocal'
        QtMocHelpers::MethodData<bool() const>(31, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'recommendedMode'
        QtMocHelpers::MethodData<QString() const>(32, 2, QMC::AccessPublic, QMetaType::QString),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'availableModels'
        QtMocHelpers::PropertyData<QVariantList>(33, 0x80000000 | 34, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'activeModelId'
        QtMocHelpers::PropertyData<QString>(35, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable, 1),
        // property 'isDownloading'
        QtMocHelpers::PropertyData<bool>(36, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'downloadProgress'
        QtMocHelpers::PropertyData<double>(37, QMetaType::Double, QMC::DefaultPropertyFlags, 3),
        // property 'downloadError'
        QtMocHelpers::PropertyData<QString>(38, QMetaType::QString, QMC::DefaultPropertyFlags, 4),
        // property 'gpuAccelerated'
        QtMocHelpers::PropertyData<bool>(39, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'gpuBackend'
        QtMocHelpers::PropertyData<QString>(40, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'threadCount'
        QtMocHelpers::PropertyData<int>(41, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 7),
        // property 'autoDetectLanguage'
        QtMocHelpers::PropertyData<bool>(42, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 8),
        // property 'onnxAvailable'
        QtMocHelpers::PropertyData<bool>(43, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Constant),
        // property 'globalMode'
        QtMocHelpers::PropertyData<QString>(44, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 10),
        // property 'selectedLanguage'
        QtMocHelpers::PropertyData<QString>(45, QMetaType::QString, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 9),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<WhisperModelManager, qt_meta_tag_ZN19WhisperModelManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject WhisperModelManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19WhisperModelManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19WhisperModelManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN19WhisperModelManagerE_t>.metaTypes,
    nullptr
} };

void WhisperModelManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<WhisperModelManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->availableModelsChanged(); break;
        case 1: _t->activeModelChanged(); break;
        case 2: _t->isDownloadingChanged(); break;
        case 3: _t->downloadProgressChanged(); break;
        case 4: _t->downloadErrorChanged(); break;
        case 5: _t->modelReady((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->modelDeleted((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->threadCountChanged(); break;
        case 8: _t->autoDetectLanguageChanged(); break;
        case 9: _t->selectedLanguageChanged(); break;
        case 10: _t->globalModeChanged(); break;
        case 11: _t->benchmarkResult((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2]))); break;
        case 12: { bool _r = _t->isModelAvailable((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 13: _t->downloadModel((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->cancelDownload(); break;
        case 15: _t->deleteModel((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->setActiveModel((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: { qint64 _r = _t->totalDiskUsage();
            if (_a[0]) *reinterpret_cast<qint64*>(_a[0]) = std::move(_r); }  break;
        case 18: _t->setThreadCount((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->setAutoDetectLanguage((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->setSelectedLanguage((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 21: _t->setGlobalMode((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->benchmarkModel((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 23: { bool _r = _t->deviceCanHandleLocal();
            if (_a[0]) *reinterpret_cast<bool*>(_a[0]) = std::move(_r); }  break;
        case 24: { QString _r = _t->recommendedMode();
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::availableModelsChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::activeModelChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::isDownloadingChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::downloadProgressChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::downloadErrorChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)(const QString & )>(_a, &WhisperModelManager::modelReady, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)(const QString & )>(_a, &WhisperModelManager::modelDeleted, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::threadCountChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::autoDetectLanguageChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::selectedLanguageChanged, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)()>(_a, &WhisperModelManager::globalModeChanged, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (WhisperModelManager::*)(const QString & , int )>(_a, &WhisperModelManager::benchmarkResult, 11))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariantList*>(_v) = _t->availableModels(); break;
        case 1: *reinterpret_cast<QString*>(_v) = _t->activeModelId(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->isDownloading(); break;
        case 3: *reinterpret_cast<double*>(_v) = _t->downloadProgress(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->downloadError(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->gpuAccelerated(); break;
        case 6: *reinterpret_cast<QString*>(_v) = _t->gpuBackend(); break;
        case 7: *reinterpret_cast<int*>(_v) = _t->threadCount(); break;
        case 8: *reinterpret_cast<bool*>(_v) = _t->autoDetectLanguage(); break;
        case 9: *reinterpret_cast<bool*>(_v) = _t->onnxAvailable(); break;
        case 10: *reinterpret_cast<QString*>(_v) = _t->globalMode(); break;
        case 11: *reinterpret_cast<QString*>(_v) = _t->selectedLanguage(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setActiveModel(*reinterpret_cast<QString*>(_v)); break;
        case 7: _t->setThreadCount(*reinterpret_cast<int*>(_v)); break;
        case 8: _t->setAutoDetectLanguage(*reinterpret_cast<bool*>(_v)); break;
        case 10: _t->setGlobalMode(*reinterpret_cast<QString*>(_v)); break;
        case 11: _t->setSelectedLanguage(*reinterpret_cast<QString*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *WhisperModelManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WhisperModelManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN19WhisperModelManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int WhisperModelManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 25)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 25;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 25)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 25;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void WhisperModelManager::availableModelsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void WhisperModelManager::activeModelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void WhisperModelManager::isDownloadingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void WhisperModelManager::downloadProgressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void WhisperModelManager::downloadErrorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void WhisperModelManager::modelReady(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void WhisperModelManager::modelDeleted(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1);
}

// SIGNAL 7
void WhisperModelManager::threadCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void WhisperModelManager::autoDetectLanguageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void WhisperModelManager::selectedLanguageChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void WhisperModelManager::globalModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void WhisperModelManager::benchmarkResult(const QString & _t1, int _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 11, nullptr, _t1, _t2);
}
QT_WARNING_POP
