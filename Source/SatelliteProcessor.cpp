#include "SatelliteProcessor.h"
#include "SatelliteEditor.h"
#include <cmath>

namespace
{
    inline float linearToDb(float linear)
    {
        return linear > 0.0f ? 20.0f * std::log10(linear) : -120.0f;
    }
    
    inline float dbToLinear(float db)
    {
        return std::pow(10.0f, db / 20.0f);
    }
}

SatelliteProcessor::SatelliteProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Generate unique instance ID based on time and random
    instanceId = static_cast<uint64_t>(juce::Time::currentTimeMillis()) ^ 
                 static_cast<uint64_t>(juce::Random::getSystemRandom().nextInt64());
    
    // Listen for source parameter changes to sync to shared memory
    parameters.addParameterListener("source", this);
    connectToSharedMemory();
    
    // Start timer to keep satellite active even when not processing audio
    startTimerHz(10);  // 10 Hz = every 100ms
}

SatelliteProcessor::~SatelliteProcessor()
{
    parameters.removeParameterListener("source", this);
    disconnectFromSharedMemory();
    stopTimer();
}

void SatelliteProcessor::connectToSharedMemory()
{
    if (sharedMemory.openOrCreate())
    {
        auto* data = sharedMemory.getData();
        if (data != nullptr)
        {
            slotIndex = data->findAvailableSlot();
            if (slotIndex >= 0)
            {
                auto& sat = data->satellites[slotIndex];
                sat.active.store(true);
                sat.instanceId.store(instanceId);  // Store our unique ID
                sat.lastUpdateTime.store(juce::Time::currentTimeMillis());
                sat.sourceType.store(sourceType);
                std::strncpy(sat.channelName, channelName.toRawUTF8(), 63);
                sat.channelName[63] = '\0';
                
                DBG("AR3S Satellite [" + juce::String(instanceId) + "]: Connected to slot " + juce::String(slotIndex) + " as '" + channelName + "'");
            }
            else
            {
                DBG("AR3S Satellite: No available slot found!");
            }
        }
    }
    else
    {
        DBG("AR3S Satellite: Failed to open/create shared memory!");
    }
}

void SatelliteProcessor::disconnectFromSharedMemory()
{
    if (sharedMemory.isValid() && slotIndex >= 0 && slotIndex < MAX_SATELLITES)
    {
        auto* data = sharedMemory.getData();
        if (data != nullptr)
        {
            // Only clear if the slot is still ours
            auto& sat = data->satellites[slotIndex];
            if (sat.instanceId.load() == instanceId)
            {
                sat.active.store(false);
                sat.instanceId.store(0);
                sat.lastUpdateTime.store(0);
                DBG("AR3S Satellite [" + juce::String(instanceId) + "]: Disconnected from slot " + juce::String(slotIndex));
            }
        }
    }
    slotIndex = -1;
}

void SatelliteProcessor::setChannelName(const juce::String& name)
{
    channelName = name;
    if (sharedMemory.isValid() && slotIndex >= 0)
    {
        auto* data = sharedMemory.getData();
        std::strncpy(data->satellites[slotIndex].channelName, 
                    name.toRawUTF8(), 63);
    }
}

void SatelliteProcessor::setSourceType(int type)
{
    sourceType = type;
    if (sharedMemory.isValid() && slotIndex >= 0)
    {
        auto* data = sharedMemory.getData();
        data->satellites[slotIndex].sourceType.store(type);
    }
}

void SatelliteProcessor::updateSharedMemory()
{
    if (!sharedMemory.isValid())
        return;
    
    // Rate limit to 20 times per second (every 50ms) to avoid performance issues
    const auto currentTime = juce::Time::currentTimeMillis();
    if (currentTime - lastSharedMemoryUpdateTime < 50)
        return;
    
    auto* data = sharedMemory.getData();
    
    // Check if our slot was cleared by master (e.g. master restarted) or if we don't have a valid slot
    // If so, we need to re-register
    bool needsReconnect = (slotIndex < 0);
    if (!needsReconnect && slotIndex >= 0 && slotIndex < MAX_SATELLITES)
    {
        // Check if our slot is still ours using instance ID
        auto& sat = data->satellites[slotIndex];
        needsReconnect = !sat.active.load() || (sat.instanceId.load() != instanceId);
    }
    
    if (needsReconnect)
    {
        // First, clear our old slot if we had one and it was ours
        if (slotIndex >= 0 && slotIndex < MAX_SATELLITES)
        {
            auto& oldSat = data->satellites[slotIndex];
            if (oldSat.instanceId.load() == instanceId)
            {
                oldSat.active.store(false);
                oldSat.instanceId.store(0);
                oldSat.lastUpdateTime.store(0);
            }
        }
        
        // Find a new slot
        slotIndex = data->findAvailableSlot();
        if (slotIndex >= 0)
        {
            auto& sat = data->satellites[slotIndex];
            sat.active.store(true);
            sat.instanceId.store(instanceId);  // Claim with our unique ID
            sat.lastUpdateTime.store(currentTime);
            sat.sourceType.store(sourceType);
            std::strncpy(sat.channelName, channelName.toRawUTF8(), 63);
            sat.channelName[63] = '\0';
            DBG("AR3S Satellite [" + juce::String(instanceId) + "]: Re-connected to slot " + juce::String(slotIndex));
        }
        else
        {
            lastSharedMemoryUpdateTime = currentTime;
            return; // No slot available
        }
    }
    
    auto& sat = data->satellites[slotIndex];
    
    // Send post-processing levels to shared memory (what the master sees)
    sat.rmsDb.store(postRmsDb.load());
    sat.peakDb.store(postPeakDb.load());
    sat.crestDb.store(postCrestDb.load());
    sat.phaseCorrelation.store(postPhaseCorrelation.load());
    sat.currentGain.store(currentAppliedGain.load());
    sat.lastUpdateTime.store(currentTime);
    sat.active.store(true);  // Keep confirming we're active
    
    lastSharedMemoryUpdateTime = currentTime;
}

void SatelliteProcessor::readMasterControls()
{
    if (!sharedMemory.isValid() || slotIndex < 0)
        return;
    
    auto* data = sharedMemory.getData();
    auto& sat = data->satellites[slotIndex];
    
    // Check if per-satellite override is active
    bool perSatelliteOverride = sat.control.perSatelliteOverride.load();
    bool masterControl = sat.control.controlledByMaster.load();
    auto controlTime = sat.control.controlUpdateTime.load();
    auto currentTime = juce::Time::currentTimeMillis();

    // Master control valid if flag is set AND control was updated recently (within 5 seconds)
    bool validMasterControl = masterControl && (currentTime - controlTime < 5000);
    controlledByMaster.store(validMasterControl);

    // If local override is enabled, user has manual control - don't apply master or per-satellite settings
    if (localOverride.load())
        return;

    // If per-satellite override is active, use those values
    if (perSatelliteOverride)
    {
        float satGainDb = sat.control.gainDb.load();
        satGainDb = juce::jlimit(-24.0f, 12.0f, satGainDb);
        if (auto* gainParam = parameters.getParameter("gain"))
        {
            float normalized = (satGainDb - (-24.0f)) / (12.0f - (-24.0f));
            gainParam->setValueNotifyingHost(normalized);
        }

        float satTargetDb = sat.control.targetDb.load();
        if (auto* targetParam = parameters.getParameter("target_db"))
        {
            float normTarget = (satTargetDb - (-48.0f)) / (0.0f - (-48.0f));
            targetParam->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, normTarget));
        }

        bool satAuto = sat.control.autoEnabled.load();
        if (auto* autoParam = parameters.getParameter("auto_enabled"))
        {
            autoParam->setValueNotifyingHost(satAuto ? 1.0f : 0.0f);
        }

        float satRider = sat.control.riderAmount.load();
        if (auto* riderParam = parameters.getParameter("rider_amount"))
        {
            riderParam->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, satRider));
        }
        return;
    }

    // If master is controlling us, apply the master setting
    if (validMasterControl)
    {
        float masterGainDb = sat.control.gainDb.load();
        masterGainDb = juce::jlimit(-24.0f, 12.0f, masterGainDb);
        if (auto* gainParam = parameters.getParameter("gain"))
        {
            float normalized = (masterGainDb - (-24.0f)) / (12.0f - (-24.0f));
            gainParam->setValueNotifyingHost(normalized);
        }

        float masterTargetDb = sat.control.targetDb.load();
        if (auto* targetParam = parameters.getParameter("target_db"))
        {
            float normTarget = (masterTargetDb - (-48.0f)) / (0.0f - (-48.0f));
            targetParam->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, normTarget));
        }

        bool masterAuto = sat.control.autoEnabled.load();
        if (auto* autoParam = parameters.getParameter("auto_enabled"))
        {
            autoParam->setValueNotifyingHost(masterAuto ? 1.0f : 0.0f);
        }

        float masterRider = sat.control.riderAmount.load();
        if (auto* riderParam = parameters.getParameter("rider_amount"))
        {
            riderParam->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, masterRider));
        }
    }
}

void SatelliteProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    
    // Professional timing coefficients
    // Auto-gain: 300ms smoothing for transparent level adjustment
    autoSmoothCoeff = std::exp(-1.0 / (sampleRate * 0.3));
    // Vocal rider: 80ms attack, 300ms release (matches main plugin for consistency)
    riderAttackCoeff = std::exp(-1.0 / (sampleRate * 0.08));
    riderReleaseCoeff = std::exp(-1.0 / (sampleRate * 0.3));
    
    // Reconnect if needed
    if (!sharedMemory.isValid())
        connectToSharedMemory();
}

void SatelliteProcessor::releaseResources()
{
}

bool SatelliteProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void SatelliteProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Read controls from master if needed
    readMasterControls();
    
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    
    // Get parameters - gain is now in dB
    const float gainDb = *parameters.getRawParameterValue("gain");
    const float manualGain = dbToLinear(gainDb);  // Convert dB to linear
    const float targetDb = *parameters.getRawParameterValue("target_db");
    const float ceilingDb = *parameters.getRawParameterValue("ceiling");
    const bool autoEnabled = *parameters.getRawParameterValue("auto_enabled") > 0.5f;
    const float riderAmount = *parameters.getRawParameterValue("rider_amount");
    
    // ============ PRE-PROCESSING METERING ============
    float preSumSquaresL = 0.0f, preSumSquaresR = 0.0f;
    float prePeak = 0.0f;
    float preSumLR = 0.0f;
    
    if (numChannels >= 2 && numSamples > 0)
    {
        const auto* leftData = buffer.getReadPointer(0);
        const auto* rightData = buffer.getReadPointer(1);
        
        for (int i = 0; i < numSamples; ++i)
        {
            const auto L = leftData[i];
            const auto R = rightData[i];
            preSumSquaresL += L * L;
            preSumSquaresR += R * R;
            preSumLR += L * R;
            prePeak = std::max(prePeak, std::max(std::abs(L), std::abs(R)));
        }
        
        const float denom = std::sqrt(preSumSquaresL * preSumSquaresR);
        if (denom > 1.0e-10f)
        {
            float correlation = preSumLR / denom;
            float currentPhase = prePhaseCorrelation.load();
            prePhaseCorrelation.store(0.7f * currentPhase + 0.3f * juce::jlimit(-1.0f, 1.0f, correlation));
        }
    }
    else if (numChannels == 1)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const auto sample = buffer.getSample(0, i);
            preSumSquaresL += sample * sample;
            prePeak = std::max(prePeak, std::abs(sample));
        }
        prePhaseCorrelation.store(0.0f);  // Mono has no phase relationship
    }
    
    const float preTotalSumSquares = preSumSquaresL + preSumSquaresR;
    const auto preRms = std::sqrt(preTotalSumSquares / std::max(1, numChannels * numSamples));
    const auto preRmsDbVal = linearToDb(preRms);
    const auto prePeakDbVal = linearToDb(prePeak);
    
    preRmsDb.store(preRmsDbVal);
    prePeakDb.store(prePeakDbVal);
    preCrestDb.store(prePeakDbVal - preRmsDbVal);
    
    // ============ PROCESSING ============
    float totalGain = manualGain;
    
    // Auto gain
    if (autoEnabled && preRmsDbVal > -80.0f)
    {
        const float correction = targetDb - preRmsDbVal;
        const float targetGain = dbToLinear(correction);
        autoSmoothedGain = autoSmoothCoeff * autoSmoothedGain + (1.0f - autoSmoothCoeff) * targetGain;
        totalGain *= juce::jlimit(0.125f, 4.0f, autoSmoothedGain);
    }
    else
    {
        autoSmoothedGain = 1.0f;
    }
    
    // Vocal rider - professional gain riding with transparent operation
    if (riderAmount > 0.01f && preRmsDbVal > -60.0f)
    {
        const float deviation = targetDb - preRmsDbVal;
        // Limit rider to ±6 dB for transparency, then scale by amount
        const float limitedDeviation = juce::jlimit(-6.0f, 6.0f, deviation);
        const float riderGainTarget = dbToLinear(limitedDeviation * riderAmount);
        
        if (riderGainTarget > riderSmoothedGain)
            riderSmoothedGain = riderAttackCoeff * riderSmoothedGain + (1.0f - riderAttackCoeff) * riderGainTarget;
        else
            riderSmoothedGain = riderReleaseCoeff * riderSmoothedGain + (1.0f - riderReleaseCoeff) * riderGainTarget;
        
        // Final clamp for safety
        totalGain *= juce::jlimit(0.5f, 2.0f, riderSmoothedGain);
    }
    else
    {
        riderSmoothedGain = 1.0f;
    }
    
    currentAppliedGain.store(totalGain);
    
    // Apply gain
    buffer.applyGain(totalGain);
    
    // ============ CEILING / MAX PEAK LIMITER ============
    // Transparent peak limiting with smooth gain reduction to avoid pumping
    if (ceilingDb < -0.1f)  // Only apply if ceiling is below 0 dB
    {
        float ceilingLinear = dbToLinear(ceilingDb);
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        
        // Find the absolute peak in the entire buffer
        float maxPeak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float* data = buffer.getReadPointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                float absSample = std::abs(data[i]);
                if (absSample > maxPeak)
                    maxPeak = absSample;
            }
        }
        
        // Apply limiting if needed
        if (maxPeak > ceilingLinear && maxPeak > 0.0001f)
        {
            float targetGainReduction = ceilingLinear / maxPeak;
            
            // Smooth the gain reduction to avoid pumping
            const float attackCoeff = 0.1f;  // Fast attack for peaks
            const float releaseCoeff = 0.999f;  // Slow release
            
            float coeff = (targetGainReduction < ceilingSmoothedGain) ? attackCoeff : releaseCoeff;
            ceilingSmoothedGain = coeff * ceilingSmoothedGain + (1.0f - coeff) * targetGainReduction;
            
            // Apply the limiting
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float* data = buffer.getWritePointer(ch);
                for (int i = 0; i < numSamples; ++i)
                {
                    data[i] *= ceilingSmoothedGain;
                }
            }
        }
        else
        {
            // No limiting needed, slowly return to unity
            ceilingSmoothedGain = 0.999f * ceilingSmoothedGain + 0.001f * 1.0f;
        }
    }
    else
    {
        ceilingSmoothedGain = 1.0f;
    }
    
    // ============ POST-PROCESSING METERING ============
    float postSumSquaresL = 0.0f, postSumSquaresR = 0.0f;
    float postPeak = 0.0f;
    float postSumLR = 0.0f;
    
    if (numChannels >= 2 && numSamples > 0)
    {
        const auto* leftData = buffer.getReadPointer(0);
        const auto* rightData = buffer.getReadPointer(1);
        
        for (int i = 0; i < numSamples; ++i)
        {
            const auto L = leftData[i];
            const auto R = rightData[i];
            postSumSquaresL += L * L;
            postSumSquaresR += R * R;
            postSumLR += L * R;
            postPeak = std::max(postPeak, std::max(std::abs(L), std::abs(R)));
        }
        
        const float denom = std::sqrt(postSumSquaresL * postSumSquaresR);
        if (denom > 1.0e-10f)
        {
            float correlation = postSumLR / denom;
            float currentPhase = postPhaseCorrelation.load();
            postPhaseCorrelation.store(0.7f * currentPhase + 0.3f * juce::jlimit(-1.0f, 1.0f, correlation));
        }
    }
    else if (numChannels == 1)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const auto sample = buffer.getSample(0, i);
            postSumSquaresL += sample * sample;
            postPeak = std::max(postPeak, std::abs(sample));
        }
        postPhaseCorrelation.store(1.0f);
    }
    
    const float postTotalSumSquares = postSumSquaresL + postSumSquaresR;
    const auto postRms = std::sqrt(postTotalSumSquares / std::max(1, numChannels * numSamples));
    const auto postRmsDbVal = linearToDb(postRms);
    const auto postPeakDbVal = linearToDb(postPeak);
    
    postRmsDb.store(postRmsDbVal);
    postPeakDb.store(postPeakDbVal);
    postCrestDb.store(postPeakDbVal - postRmsDbVal);
    
    // Update shared memory with latest data
    updateSharedMemory();
}

bool SatelliteProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* SatelliteProcessor::createEditor()
{
    return new SatelliteEditor(*this);
}

const juce::String SatelliteProcessor::getName() const { return "AR3S Satellite"; }
bool SatelliteProcessor::acceptsMidi() const { return false; }
bool SatelliteProcessor::producesMidi() const { return false; }
bool SatelliteProcessor::isMidiEffect() const { return false; }
double SatelliteProcessor::getTailLengthSeconds() const { return 0.0; }
int SatelliteProcessor::getNumPrograms() { return 1; }
int SatelliteProcessor::getCurrentProgram() { return 0; }
void SatelliteProcessor::setCurrentProgram(int) {}
const juce::String SatelliteProcessor::getProgramName(int) { return {}; }
void SatelliteProcessor::changeProgramName(int, const juce::String&) {}

void SatelliteProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::XmlElement xml("SatelliteState");
    xml.setAttribute("channelName", channelName);
    xml.setAttribute("sourceType", sourceType);
    
    auto state = parameters.copyState();
    xml.addChildElement(state.createXml().release());
    
    copyXmlToBinary(xml, destData);
}

void SatelliteProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xmlState = getXmlFromBinary(data, sizeInBytes);
    if (xmlState != nullptr)
    {
        channelName = xmlState->getStringAttribute("channelName", "Channel");
        sourceType = xmlState->getIntAttribute("sourceType", 0);
        
        if (auto* paramsXml = xmlState->getChildByName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*paramsXml));
        
        // Update shared memory
        if (sharedMemory.isValid() && slotIndex >= 0)
        {
            setChannelName(channelName);
            setSourceType(sourceType);
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SatelliteProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Gain in dB (-24 to +12dB, like a normal gain plugin)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain dB",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f),
        0.0f));  // Default 0 dB (unity)
    
    // Target dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "target_db", "Target dB",
        juce::NormalisableRange<float>(-48.0f, 0.0f, 0.1f),
        -18.0f));
    
    // Ceiling dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ceiling", "Max Peak",
        juce::NormalisableRange<float>(-24.0f, 0.0f, 0.1f),
        0.0f));  // Default 0 dB (no limiting)
    
    // Auto gain enabled
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "auto_enabled", "Auto Gain", false));
    
    // Vocal rider amount
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "rider_amount", "Rider Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    // Source type
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "source", "Source",
        juce::StringArray { "Lead Vocal", "BG Vocal", "Kick", "Snare", "Hi-Hat", 
                           "Drums", "Bass", "E.Guitar", "A.Guitar", "Keys", 
                           "Synth", "Strings", "Other" },
        0));
    
    return { params.begin(), params.end() };
}

int SatelliteProcessor::getMasterThemeIndex() const
{
    if (!sharedMemory.isValid())
        return 1;  // Default to Modern Dark
    
    auto* data = sharedMemory.getData();
    if (data == nullptr)
        return 1;
    
    return data->masterThemeIndex.load();
}

int SatelliteProcessor::getMasterKnobStyle() const
{
    if (!sharedMemory.isValid())
        return 0;  // Default to first knob style
    
    auto* data = sharedMemory.getData();
    if (data == nullptr)
        return 0;
    
    return data->masterKnobStyle.load();
}

void SatelliteProcessor::timerCallback()
{
    // Keep satellite active in shared memory even when not processing audio
    updateSharedMemory();
}

void SatelliteProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "source")
    {
        int newSourceType = static_cast<int>(newValue);
        setSourceType(newSourceType);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SatelliteProcessor();
}
