#include "speexResampler.h"

#include <cstring>
#include <iostream>

// miniaudio for fallback resampler (declarations only, no IMPLEMENTATION)
#include "miniaudio.h"

#if TALKLESS_HAS_SPEEXDSP
#include <speex/speex_resampler.h>
#endif

// ── Unified Impl: holds both Speex and miniaudio state ─────────

struct SpeexResampler::Impl
{
#if TALKLESS_HAS_SPEEXDSP
    SpeexResamplerState* speexState = nullptr;
#endif
    ma_resampler maResampler{};
    bool maResamplerInit = false;

    ~Impl() { destroy(); }

    void destroy()
    {
#if TALKLESS_HAS_SPEEXDSP
        if (speexState) {
            speex_resampler_destroy(speexState);
            speexState = nullptr;
        }
#endif
        if (maResamplerInit) {
            ma_resampler_uninit(&maResampler, nullptr);
            maResamplerInit = false;
        }
    }
};

SpeexResampler::SpeexResampler() : m_impl(std::make_unique<Impl>()) {}
SpeexResampler::~SpeexResampler() { uninit(); }

SpeexResampler::SpeexResampler(SpeexResampler&&) noexcept = default;
SpeexResampler& SpeexResampler::operator=(SpeexResampler&&) noexcept = default;

bool SpeexResampler::isSpeexAvailable()
{
#if TALKLESS_HAS_SPEEXDSP
    return true;
#else
    return false;
#endif
}

void SpeexResampler::setUseSpeex(bool enabled)
{
    if (m_useSpeex == enabled)
        return;
    m_useSpeex = enabled;
    // Re-init if already initialized
    if (m_initialized) {
        uint32_t ch = m_channels, inR = m_inputRate, outR = m_outputRate;
        int q = m_quality;
        uninit();
        init(ch, inR, outR, q);
    }
}

bool SpeexResampler::init(uint32_t channels, uint32_t inputRate, uint32_t outputRate, int quality)
{
    uninit();
    if (inputRate == outputRate)
        return false;

    m_channels = channels;
    m_inputRate = inputRate;
    m_outputRate = outputRate;
    m_quality = quality;

#if TALKLESS_HAS_SPEEXDSP
    if (m_useSpeex) {
        int err = 0;
        m_impl->speexState = speex_resampler_init(channels, inputRate, outputRate, quality, &err);
        if (!m_impl->speexState || err != RESAMPLER_ERR_SUCCESS) {
            std::cerr << "[SpeexResampler] speex_resampler_init failed, err=" << err << "\n";
            m_impl->speexState = nullptr;
            return false;
        }
        speex_resampler_skip_zeros(m_impl->speexState);
        m_initialized = true;
        std::cout << "[SpeexResampler] Initialized (Speex): " << inputRate << " -> " << outputRate
                  << " Hz, " << channels << "ch, quality=" << quality << "\n";
        return true;
    }
#endif

    // miniaudio fallback
    ma_resampler_config cfg = ma_resampler_config_init(
        ma_format_f32, channels, inputRate, outputRate, ma_resample_algorithm_linear);
    cfg.linear.lpfOrder = MA_MAX_FILTER_ORDER;

    if (ma_resampler_init(&cfg, nullptr, &m_impl->maResampler) != MA_SUCCESS) {
        std::cerr << "[SpeexResampler] ma_resampler_init failed\n";
        return false;
    }
    m_impl->maResamplerInit = true;
    m_initialized = true;
    std::cout << "[SpeexResampler] Initialized (miniaudio linear): " << inputRate << " -> "
              << outputRate << " Hz, " << channels << "ch\n";
    return true;
}

void SpeexResampler::uninit()
{
    if (m_impl) {
        m_impl->destroy();
    }
    m_initialized = false;
}

bool SpeexResampler::process(const float* pIn, uint64_t* pInFrameCount,
                             float* pOut, uint64_t* pOutFrameCount)
{
    if (!m_initialized)
        return false;

#if TALKLESS_HAS_SPEEXDSP
    if (m_impl->speexState) {
        if (m_channels == 1) {
            spx_uint32_t inLen = static_cast<spx_uint32_t>(*pInFrameCount);
            spx_uint32_t outLen = static_cast<spx_uint32_t>(*pOutFrameCount);
            int err = speex_resampler_process_float(m_impl->speexState, 0,
                                                    pIn, &inLen, pOut, &outLen);
            *pInFrameCount = inLen;
            *pOutFrameCount = outLen;
            return err == RESAMPLER_ERR_SUCCESS;
        } else {
            spx_uint32_t inLen = static_cast<spx_uint32_t>(*pInFrameCount);
            spx_uint32_t outLen = static_cast<spx_uint32_t>(*pOutFrameCount);
            int err = speex_resampler_process_interleaved_float(m_impl->speexState,
                                                                pIn, &inLen,
                                                                pOut, &outLen);
            *pInFrameCount = inLen;
            *pOutFrameCount = outLen;
            return err == RESAMPLER_ERR_SUCCESS;
        }
    }
#endif

    if (m_impl->maResamplerInit) {
        ma_uint64 framesIn = static_cast<ma_uint64>(*pInFrameCount);
        ma_uint64 framesOut = static_cast<ma_uint64>(*pOutFrameCount);
        ma_result result = ma_resampler_process_pcm_frames(
            &m_impl->maResampler, pIn, &framesIn, pOut, &framesOut);
        *pInFrameCount = framesIn;
        *pOutFrameCount = framesOut;
        return result == MA_SUCCESS;
    }

    return false;
}

void SpeexResampler::reset()
{
#if TALKLESS_HAS_SPEEXDSP
    if (m_impl->speexState) {
        speex_resampler_reset_mem(m_impl->speexState);
        return;
    }
#endif
    // miniaudio linear resampler has no explicit reset
}

