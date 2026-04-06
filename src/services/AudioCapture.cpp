#include "AudioCapture.h"
#include <miniaudio.h>

#include <QDebug>

static constexpr uint32_t SAMPLE_RATE = 48000;
static constexpr uint32_t CHANNELS = 1;

AudioCapture::AudioCapture(QObject* parent)
    : QObject(parent)
{
    m_context = new ma_context;
    if (ma_context_init(nullptr, 0, nullptr, m_context) != MA_SUCCESS) {
        qWarning() << "AudioCapture: Failed to initialize audio context";
        delete m_context;
        m_context = nullptr;
    }
}

AudioCapture::~AudioCapture()
{
    stopCapture();
    if (m_context) {
        ma_context_uninit(m_context);
        delete m_context;
    }
}

void AudioCapture::dataCallback(ma_device* device, void* /*output*/, const void* input, uint32_t frameCount)
{
    auto* self = static_cast<AudioCapture*>(device->pUserData);
    if (self && self->m_callback && input)
        self->m_callback(static_cast<const float*>(input), frameCount);
}

QVariantList AudioCapture::inputDevices() const
{
    QVariantList result;
    if (!m_context) return result;

    ma_device_info* infos;
    ma_uint32 count;
    if (ma_context_get_devices(m_context, nullptr, nullptr, &infos, &count) != MA_SUCCESS)
        return result;

    for (ma_uint32 i = 0; i < count; ++i) {
        QVariantMap dev;
        dev["id"] = QString::fromUtf8(infos[i].name);
        dev["name"] = QString::fromUtf8(infos[i].name);
        dev["isDefault"] = infos[i].isDefault;
        result.append(dev);
    }
    return result;
}

void AudioCapture::setActiveInputDevice(const QString& deviceId)
{
    if (m_activeDeviceId == deviceId) return;
    m_activeDeviceId = deviceId;
    emit activeInputDeviceChanged();

    if (m_capturing.load()) {
        stopCapture();
        startCapture();
    }
}

void AudioCapture::startCapture()
{
    if (m_capturing.load() || !m_context) return;

    m_device = new ma_device;

    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_f32;
    config.capture.channels = CHANNELS;
    config.sampleRate = SAMPLE_RATE;
    config.dataCallback = dataCallback;
    config.pUserData = this;
    config.performanceProfile = ma_performance_profile_low_latency;

    if (ma_device_init(m_context, &config, m_device) != MA_SUCCESS) {
        qWarning() << "AudioCapture: Failed to init capture device";
        delete m_device;
        m_device = nullptr;
        emit captureError(tr("Failed to initialize microphone"));
        return;
    }

    if (ma_device_start(m_device) != MA_SUCCESS) {
        qWarning() << "AudioCapture: Failed to start capture";
        ma_device_uninit(m_device);
        delete m_device;
        m_device = nullptr;
        emit captureError(tr("Failed to start microphone"));
        return;
    }

    m_capturing.store(true);
    emit isCapturingChanged();
    qDebug() << "AudioCapture: Started at" << SAMPLE_RATE << "Hz mono";
}

void AudioCapture::stopCapture()
{
    if (!m_device) return;

    ma_device_stop(m_device);
    ma_device_uninit(m_device);
    delete m_device;
    m_device = nullptr;

    m_capturing.store(false);
    emit isCapturingChanged();
    qDebug() << "AudioCapture: Stopped";
}

void AudioCapture::refreshDevices()
{
    emit inputDevicesChanged();
}
