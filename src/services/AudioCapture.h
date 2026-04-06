#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <atomic>
#include <functional>

struct ma_device;
struct ma_context;

/**
 * @brief Simple microphone capture using miniaudio.
 *
 * Captures float32 PCM at 48 kHz mono and forwards frames to a callback.
 * No playback, no loopback — just a mic.
 */
class AudioCapture : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isCapturing READ isCapturing NOTIFY isCapturingChanged)
    Q_PROPERTY(QVariantList inputDevices READ inputDevices NOTIFY inputDevicesChanged)
    Q_PROPERTY(QString activeInputDevice READ activeInputDevice WRITE setActiveInputDevice NOTIFY activeInputDeviceChanged)

public:
    using AudioCallback = std::function<void(const float* samples, uint32_t frameCount)>;

    explicit AudioCapture(QObject* parent = nullptr);
    ~AudioCapture() override;

    [[nodiscard]] bool isCapturing() const { return m_capturing.load(std::memory_order_relaxed); }
    [[nodiscard]] QVariantList inputDevices() const;
    [[nodiscard]] QString activeInputDevice() const { return m_activeDeviceId; }

    void setActiveInputDevice(const QString& deviceId);
    void setAudioCallback(AudioCallback cb) { m_callback = std::move(cb); }

    Q_INVOKABLE void startCapture();
    Q_INVOKABLE void stopCapture();
    Q_INVOKABLE void refreshDevices();

signals:
    void isCapturingChanged();
    void inputDevicesChanged();
    void activeInputDeviceChanged();
    void captureError(const QString& error);

private:
    static void dataCallback(ma_device* device, void* output, const void* input, uint32_t frameCount);

    ma_device* m_device = nullptr;
    ma_context* m_context = nullptr;
    std::atomic<bool> m_capturing{false};
    QString m_activeDeviceId;
    AudioCallback m_callback;
};
