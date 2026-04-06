#pragma once

#include <cstdint>
#include <memory>

/**
 * @brief High-quality audio resampler wrapping SpeexDSP
 *
 * When SpeexDSP is available (TALKLESS_HAS_SPEEXDSP), uses speex_resampler
 * which provides significantly better quality than miniaudio's linear resampler.
 * Falls back to miniaudio linear resampler when SpeexDSP is not linked.
 * Runtime toggle allows A/B comparison when Speex is available.
 *
 * Thread-safety: NOT thread-safe. Each instance should be used from one thread only.
 */
class SpeexResampler
{
public:
    SpeexResampler();
    ~SpeexResampler();

    // Non-copyable
    SpeexResampler(const SpeexResampler&) = delete;
    SpeexResampler& operator=(const SpeexResampler&) = delete;

    // Movable
    SpeexResampler(SpeexResampler&&) noexcept;
    SpeexResampler& operator=(SpeexResampler&&) noexcept;

    /**
     * @brief Initialize the resampler
     * @param channels Number of audio channels (1=mono, 2=stereo)
     * @param inputRate Input sample rate in Hz
     * @param outputRate Output sample rate in Hz
     * @param quality Speex quality level 0-10 (default 5, good for real-time)
     * @return true if initialization succeeded
     */
    bool init(uint32_t channels, uint32_t inputRate, uint32_t outputRate, int quality = 5);

    /**
     * @brief Uninitialize and free resources
     */
    void uninit();

    /**
     * @brief Check if the resampler is initialized
     */
    bool isInitialized() const { return m_initialized; }

    /**
     * @brief Process interleaved float PCM frames
     * @param pIn Input buffer (interleaved float samples)
     * @param pInFrameCount [in/out] Number of input frames available / consumed
     * @param pOut Output buffer (interleaved float samples)
     * @param pOutFrameCount [in/out] Number of output frames available / produced
     * @return true on success
     */
    bool process(const float* pIn, uint64_t* pInFrameCount,
                 float* pOut, uint64_t* pOutFrameCount);

    /**
     * @brief Reset the resampler state (clear internal buffers)
     */
    void reset();

    /**
     * @brief Check if Speex backend is available (compiled in)
     */
    static bool isSpeexAvailable();

    /**
     * @brief Set whether to use Speex (true) or miniaudio fallback (false)
     * Only has effect when Speex is available. Requires re-init to take effect.
     */
    void setUseSpeex(bool enabled);
    bool useSpeex() const { return m_useSpeex; }

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    bool m_initialized = false;
    bool m_useSpeex = true;
    uint32_t m_channels = 0;
    uint32_t m_inputRate = 0;
    uint32_t m_outputRate = 0;
    int m_quality = 5;
};
