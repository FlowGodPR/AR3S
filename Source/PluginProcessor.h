#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SharedMemory.h"
#include "Localization.h"

class SimpleGainAudioProcessor : public juce::AudioProcessor
{
public:
    struct AnalysisSnapshot
    {
        // Pre-processing (input)
        float preRmsDb = -120.0f;
        float prePeakDb = -120.0f;
        float preCrestDb = 0.0f;
        float prePhaseCorrelation = 1.0f;
        
        // Post-processing (output)
        float postRmsDb = -120.0f;
        float postPeakDb = -120.0f;
        float postCrestDb = 0.0f;
        float postPhaseCorrelation = 1.0f;
        
        // Convenience accessors for backwards compatibility
        float rmsDb = -120.0f;
        float peakDb = -120.0f;
        float crestDb = 0.0f;
        float phaseCorrelation = 1.0f;
        
        // Advanced metering
        float stereoWidth = 100.0f;      // 0% = mono, 100% = normal stereo, >100% = wide
        float dynamicRange = 0.0f;       // Peak - RMS (crest factor)
        float headroom = 0.0f;           // dB below 0
        float shortTermLufs = -120.0f;   // Short-term LUFS (3 second window approx)
        float integratedLufs = -120.0f;  // Integrated LUFS
        float truePeak = -120.0f;        // Inter-sample true peak
        int clipCount = 0;               // Number of clips detected
        float lowEnergy = 0.0f;          // Low freq energy (0-1)
        float midEnergy = 0.0f;          // Mid freq energy (0-1)
        float highEnergy = 0.0f;         // High freq energy (0-1)
        bool monoCompatible = true;      // True if safe to sum to mono
    };
    
    // Satellite channel info for UI display
    struct SatelliteInfo
    {
        bool active = false;
        juce::String channelName;
        float rmsDb = -120.0f;
        float peakDb = -120.0f;
        float crestDb = 0.0f;
        float phaseCorrelation = 1.0f;
        float currentGain = 1.0f;
        int sourceType = 0;
        int64_t lastUpdateTime = 0;
        
        // Control values
        float gainDb = 0.0f;
        float targetDb = -18.0f;
        float ceilingDb = 0.0f;
        bool autoEnabled = false;
        float riderAmount = 0.0f;
        bool controlledByMaster = false;
    };
    
    // Satellite control structure
    struct SatelliteControl
    {
        float gainDb = 0.0f;
        float targetDb = -18.0f;
        float ceilingDb = 0.0f;
        bool autoEnabled = false;
        float riderAmount = 0.0f;
        bool controlledByMaster = false;
    };

    SimpleGainAudioProcessor();
    ~SimpleGainAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState();

    void requestAiSuggestion(const juce::String& genre,
                             const juce::String& source,
                             const juce::String& situation);
    void requestChatMessage(const juce::String& userMessage);  // Free-form chat
    void requestOllamaModelList();
    void applyAiRecommendation();
    void autoSetGainFromAnalysis();
    AnalysisSnapshot getAnalysisSnapshot() const;
    juce::String getAiNotes() const;
    juce::String getAiStatus() const;
    juce::String getChatResponse() const;
    juce::StringArray getAvailableModels() const;
    int getAvailableModelsVersion() const;
    int getChatResponseVersion() const;

    // AI Provider management
    enum class AiProvider { Ollama = 0, OpenAI, Anthropic, OpenRouter, MiniMax };
    void setAiProvider(AiProvider provider);
    AiProvider getAiProvider() const { return currentProvider; }
    void setApiKey(const juce::String& key);
    juce::String getApiKey() const;
    void setSelectedModel(const juce::String& model);
    juce::String getSelectedModel() const;
    void refreshModelList();
    
    // Equipment settings
    void setSelectedMic(const juce::String& mic) { const juce::ScopedLock lock(settingsLock); selectedMic = mic; }
    juce::String getSelectedMic() const { const juce::ScopedLock lock(settingsLock); return selectedMic; }
    void setSelectedPreamp(const juce::String& preamp) { const juce::ScopedLock lock(settingsLock); selectedPreamp = preamp; }
    juce::String getSelectedPreamp() const { const juce::ScopedLock lock(settingsLock); return selectedPreamp; }
    void setSelectedInterface(const juce::String& iface) { const juce::ScopedLock lock(settingsLock); selectedInterface = iface; }
    juce::String getSelectedInterface() const { const juce::ScopedLock lock(settingsLock); return selectedInterface; }
    
    // Language settings
    void setLanguage(Language lang) { const juce::ScopedLock lock(settingsLock); currentLanguage = lang; }
    Language getLanguage() const { const juce::ScopedLock lock(settingsLock); return currentLanguage; }
    
    // Theme settings (syncs to satellites via shared memory)
    void setThemeIndex(int index);
    int getThemeIndex() const { const juce::ScopedLock lock(settingsLock); return currentThemeIndex; }
    
    // Knob style settings (syncs to satellites via shared memory)
    void setKnobStyle(int style);
    int getKnobStyle() const { const juce::ScopedLock lock(settingsLock); return currentKnobStyle; }
    
    // AI suggestion accessor for editor
    float getAiSuggestedTargetDb() const { return aiTargetDb; }
    
    // FFT spectrum data accessor (returns magnitudes in dB for spectrum display)
    std::vector<float> getSpectrumData() const;
    int getFFTSize() const { return fftSize; }
    double getSampleRateValue() const { return currentSampleRate; }
    
    // Settings persistence
    void saveSettings();
    void loadSettings();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    class AiClient;
    friend class AiClient;

    juce::AudioProcessorValueTreeState parameters;

    std::unique_ptr<AiClient> aiClient;

    // Pre-processing (input) levels
    std::atomic<float> preRmsDb { -120.0f };
    std::atomic<float> prePeakDb { -120.0f };
    std::atomic<float> preCrestDb { 0.0f };
    std::atomic<float> prePhaseCorrelation { 1.0f };
    
    // Post-processing (output) levels
    std::atomic<float> postRmsDb { -120.0f };
    std::atomic<float> postPeakDb { -120.0f };
    std::atomic<float> postCrestDb { 0.0f };
    std::atomic<float> postPhaseCorrelation { 1.0f };
    
    // Advanced metering
    std::atomic<float> stereoWidth { 100.0f };
    std::atomic<float> shortTermLufs { -120.0f };
    std::atomic<float> integratedLufs { -120.0f };
    std::atomic<float> truePeak { -120.0f };
    std::atomic<int> clipCount { 0 };
    std::atomic<float> lowEnergy { 0.0f };
    std::atomic<float> midEnergy { 0.0f };
    std::atomic<float> highEnergy { 0.0f };
    
    // LUFS integration accumulators
    double lufsSum = 0.0;
    int64_t lufsSampleCount = 0;
    static constexpr int LUFS_WINDOW_SAMPLES = 132300;  // ~3 seconds at 44.1kHz
    std::vector<float> lufsWindow;
    int lufsWindowIndex = 0;
    
    // FFT for frequency analysis (512-point FFT = order 9)
    static constexpr int fftOrder = 9;
    static constexpr int fftSize = 1 << fftOrder;  // 512 samples
    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> fftWindow { fftSize, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, fftSize * 2> fftData;  // Complex output
    std::array<float, fftSize> fftInputBuffer;  // Accumulate samples
    int fftInputPos = 0;  // Current position in input buffer

    juce::CriticalSection aiLock;
    juce::String aiNotes;
    juce::String aiStatus;
    juce::String chatResponse;
    std::atomic<int> chatResponseVersion { 0 };
    juce::StringArray availableModels;
    std::atomic<int> availableModelsVersion { 0 };
    float aiTargetDb = -18.0f;
    float aiRiderAmount = 0.5f;
    float aiGainDb = 0.0f;
    bool aiHasRecommendation = false;
    
    // Provider settings
    AiProvider currentProvider { AiProvider::Ollama };
    juce::String apiKey;
    juce::String selectedModel { "llama3" };
    juce::CriticalSection settingsLock;
    
    // Equipment settings
    juce::String selectedMic { "" };
    juce::String selectedPreamp { "" };
    juce::String selectedInterface { "" };
    
    // Language setting
    Language currentLanguage { Language::English };
    
    // Theme setting (synced to satellites)
    int currentThemeIndex { 1 };  // Modern Dark default
    int currentKnobStyle { 0 };   // Modern Arc default
    
    // Shared memory for satellite communication
    SharedMemoryManager sharedMemory;
    bool sharedMemoryConnected = false;
    int64_t lastSatelliteControlPushTime = 0;
    float lastPushedTargetDb = -999.0f;
    bool lastPushedAutoEnabled = false;
    float lastPushedRiderAmount = -1.0f;

    
public:
    // Satellite data access
    void initializeSharedMemory();
    int getActiveSatelliteCount() const;
    SatelliteInfo getSatelliteInfo(int index) const;
    void setSatelliteControl(int index, const SatelliteControl& control);
    void releaseSatelliteControl(int index);
    juce::String getSatellitesSummary() const;
    juce::String getSatellitesJsonContext() const;
    void setAllSatellitesGain(float gainDb);
    void setAllSatellitesTarget(float targetDb);
    void setAllSatellitesCeiling(float ceilingDb);
    void autoGainAllSatellites(float targetDb);
    
private:
    double currentSampleRate = 44100.0;
    float autoSmoothedGain = 1.0f;
    float riderSmoothedGain = 1.0f;
    float lufsSmoothedGain = 1.0f;       // For LUFS auto-gain
    float ceilingSmoothedGain = 1.0f;    // For smooth ceiling limiting
    float autoSmoothCoeff = 0.9f;
    float riderAttackCoeff = 0.9f;
    float riderReleaseCoeff = 0.95f;



    void applyAiResponseText(const juce::String& response);
    void setAiNotesMessage(const juce::String& message);
    void setAiStatusMessage(const juce::String& message);
    void setChatResponseMessage(const juce::String& message);
    void setAvailableModels(const juce::StringArray& models);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleGainAudioProcessor)
};
