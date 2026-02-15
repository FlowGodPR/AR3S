#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SharedMemory.h"

class SatelliteProcessor : public juce::AudioProcessor,
                          private juce::AudioProcessorValueTreeState::Listener,
                          private juce::Timer
{
public:
    SatelliteProcessor();
    ~SatelliteProcessor() override;

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

    // Metering accessors - Pre (input)
    float getPreRmsDb() const { return preRmsDb.load(); }
    float getPrePeakDb() const { return prePeakDb.load(); }
    float getPreCrestDb() const { return preCrestDb.load(); }
    float getPrePhaseCorrelation() const { return prePhaseCorrelation.load(); }
    
    // Metering accessors - Post (output)
    float getPostRmsDb() const { return postRmsDb.load(); }
    float getPostPeakDb() const { return postPeakDb.load(); }
    float getPostCrestDb() const { return postCrestDb.load(); }
    float getPostPhaseCorrelation() const { return postPhaseCorrelation.load(); }
    
    // Legacy accessors (return post levels)
    float getRmsDb() const { return postRmsDb.load(); }
    float getPeakDb() const { return postPeakDb.load(); }
    float getCrestDb() const { return postCrestDb.load(); }
    float getPhaseCorrelation() const { return postPhaseCorrelation.load(); }
    float getCurrentGain() const { return currentAppliedGain.load(); }
    
    // Channel info
    juce::String getChannelName() const { return channelName; }
    void setChannelName(const juce::String& name);
    int getSourceType() const { return sourceType; }
    void setSourceType(int type);
    
    // Connection status
    bool isConnected() const { return sharedMemory.isValid() && slotIndex >= 0; }
    bool isControlledByMaster() const { return controlledByMaster.load(); }
    int getSlotIndex() const { return slotIndex; }
    int getMasterThemeIndex() const;  // Get theme from master via shared memory
    int getMasterKnobStyle() const;   // Get knob style from master via shared memory
    
    // Local override control - allows user to manually adjust gain
    bool isLocalOverride() const { return localOverride.load(); }
    void setLocalOverride(bool override) { localOverride.store(override); }
    void enableMasterControl() { localOverride.store(false); }  // Re-engage master control
    
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    SharedMemoryManager sharedMemory;
    int slotIndex = -1;
    
    // Metering - Pre (input) and Post (output)
    std::atomic<float> preRmsDb { -120.0f };
    std::atomic<float> prePeakDb { -120.0f };
    std::atomic<float> preCrestDb { 0.0f };
    std::atomic<float> prePhaseCorrelation { 1.0f };
    
    std::atomic<float> postRmsDb { -120.0f };
    std::atomic<float> postPeakDb { -120.0f };
    std::atomic<float> postCrestDb { 0.0f };
    std::atomic<float> postPhaseCorrelation { 1.0f };
    
    std::atomic<float> currentAppliedGain { 1.0f };
    
    // Channel info
    juce::String channelName { "Channel" };
    int sourceType = 0;
    
    // Unique instance ID (generated on construction)
    uint64_t instanceId = 0;
    
    // Control state
    std::atomic<bool> controlledByMaster { false };
    std::atomic<bool> localOverride { false };  // User has taken manual control
    
    // Processing state
    double currentSampleRate = 44100.0;
    float autoSmoothedGain = 1.0f;
    float riderSmoothedGain = 1.0f;
    float ceilingSmoothedGain = 1.0f;
    float autoSmoothCoeff = 0.9f;
    float riderAttackCoeff = 0.9f;
    float riderReleaseCoeff = 0.95f;
    
    juce::AudioProcessorValueTreeState parameters;
    
    // Rate limiting for shared memory updates
    int64_t lastSharedMemoryUpdateTime = 0;
    
    void connectToSharedMemory();
    void disconnectFromSharedMemory();
    void updateSharedMemory();
    void readMasterControls();
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SatelliteProcessor)
};
