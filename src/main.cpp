#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QStandardPaths>

#include "services/GlobalDictationService.h"
#include "services/WhisperModelManager.h"
#include "services/TextPostProcessor.h"
#include "services/DictationStatsService.h"
#include "services/AudioCapture.h"
#include "helpers/MacWindowHelper.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("TalkScribe"));
    app.setApplicationName(QStringLiteral("TalkScribe"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    // ── Services ──
    WhisperModelManager modelManager;
    GlobalDictationService dictationService;
    TextPostProcessor postProcessor;
    DictationStatsService statsService;
    AudioCapture audioCapture;

#ifdef Q_OS_MACOS
    MacWindowHelper macWindowHelper;
    dictationService.setMacWindowHelper(&macWindowHelper);
#endif

    // Wire up services
    dictationService.setModelManager(&modelManager);
    dictationService.setTextPostProcessor(&postProcessor);
    dictationService.setDictationStatsService(&statsService);
    dictationService.setEnabled(true);

    // Storage path for stats
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    statsService.setStoragePath(dataDir);

    // Audio capture → dictation service
    audioCapture.setAudioCallback([&dictationService](const float* samples, uint32_t frameCount) {
        dictationService.feedAudio(samples, frameCount);
    });

    // Start/stop audio capture when dictation toggles
    QObject::connect(&dictationService, &GlobalDictationService::isActiveChanged, [&]() {
        if (dictationService.isActive())
            audioCapture.startCapture();
        else if (!dictationService.isStopping())
            audioCapture.stopCapture();
    });
    QObject::connect(&dictationService, &GlobalDictationService::isStoppingChanged, [&]() {
        if (!dictationService.isStopping() && !dictationService.isActive())
            audioCapture.stopCapture();
    });

    // ── QML Engine ──
    QQmlApplicationEngine engine;

    auto* ctx = engine.rootContext();
    ctx->setContextProperty(QStringLiteral("globalDictationService"), &dictationService);
    ctx->setContextProperty(QStringLiteral("whisperModelManager"), &modelManager);
    ctx->setContextProperty(QStringLiteral("textPostProcessor"), &postProcessor);
    ctx->setContextProperty(QStringLiteral("dictationStatsService"), &statsService);
    ctx->setContextProperty(QStringLiteral("audioCapture"), &audioCapture);
#ifdef Q_OS_MACOS
    ctx->setContextProperty(QStringLiteral("macWindowHelper"), &macWindowHelper);
#else
    ctx->setContextProperty(QStringLiteral("macWindowHelper"), nullptr);
#endif

    engine.addImportPath(QStringLiteral("qrc:/qml"));

    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
