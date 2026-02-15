#pragma once

#include <juce_core/juce_core.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <vector>

// Shared memory structure for inter-plugin communication
// Max 32 satellite channels
constexpr int MAX_SATELLITES = 32;

// Control data sent FROM master TO satellite
struct SatelliteControlData
{
    std::atomic<float> gainDb { 0.0f };        // Gain in dB (-24 to +12)
    std::atomic<float> targetDb { -18.0f };    // Target dB for auto gain
    std::atomic<float> ceilingDb { 0.0f };     // Ceiling dB (0 = no ceiling)
    std::atomic<bool> autoEnabled { false };   // Auto gain on/off
    std::atomic<float> riderAmount { 0.0f };   // Vocal rider amount (0-1)
    std::atomic<bool> noiseEnabled { false };  // Noise gate on/off
    std::atomic<float> noiseThreshDb { -55.0f }; // Noise gate threshold
    std::atomic<float> noiseReductionDb { 12.0f }; // Noise gate reduction
    std::atomic<bool> controlledByMaster { false }; // Whether master is controlling this satellite (global/master control)
    std::atomic<bool> perSatelliteOverride { false }; // If true, use per-satellite values below
    std::atomic<int64_t> controlUpdateTime { 0 };  // When controls were last updated
};

// Metering data sent FROM satellite TO master
struct SatelliteData
{
    std::atomic<bool> active { false };
    std::atomic<uint64_t> instanceId { 0 };  // Unique ID for this satellite instance
    std::atomic<float> rmsDb { -120.0f };
    std::atomic<float> peakDb { -120.0f };
    std::atomic<float> crestDb { 0.0f };
    std::atomic<float> phaseCorrelation { 1.0f };
    std::atomic<float> currentGain { 1.0f };
    std::atomic<int64_t> lastUpdateTime { 0 };
    char channelName[64] { 0 };
    std::atomic<int> sourceType { 0 };  // 0=Vocals, 1=Drums, etc.
    std::atomic<int> situationType { 0 }; // 0=Tracking, 1=Mixing, etc.
    
    // Control data from master
    SatelliteControlData control;
};

struct SharedPluginData
{
    static constexpr uint32_t MAGIC = 0x41523353; // "AR3S"
    static constexpr uint32_t VERSION = 3;
    
    std::atomic<uint32_t> magic { MAGIC };
    std::atomic<uint32_t> version { VERSION };
    std::atomic<int> activeSatelliteCount { 0 };
    
    // Master plugin data (read by satellites)
    std::atomic<float> masterTargetDb { -18.0f };
    std::atomic<float> masterGainDb { 0.0f };      // Master gain knob value
    std::atomic<float> masterCeilingDb { 0.0f };   // Master ceiling value
    std::atomic<float> masterLufsTarget { -14.0f }; // LUFS target
    std::atomic<bool> masterAutoEnabled { true };
    std::atomic<bool> masterRiderEnabled { false }; // Vocal rider on/off
    std::atomic<float> masterRiderAmount { 0.0f };  // Rider amount (0-1)
    std::atomic<bool> masterLufsEnabled { false };  // LUFS targeting on/off
    std::atomic<int> masterGenre { 0 };
    std::atomic<int> masterSource { 0 };           // Source type
    std::atomic<int> masterSituation { 0 };        // Workflow situation
    std::atomic<int64_t> masterInitTime { 0 }; // When master was initialized
    std::atomic<int> masterThemeIndex { 0 };   // Theme index for uniform appearance
    std::atomic<int> masterKnobStyle { 0 };    // Knob style for uniform appearance
    
    // Master metering data (for AI access from any plugin)
    std::atomic<float> masterRmsDb { -60.0f };
    std::atomic<float> masterPeakDb { -60.0f };
    std::atomic<float> masterCrestDb { 12.0f };
    std::atomic<float> masterPhaseCorrelation { 1.0f };
    std::atomic<float> masterShortTermLufs { -24.0f };
    std::atomic<float> masterIntegratedLufs { -24.0f };
    
    // Satellite data array
    SatelliteData satellites[MAX_SATELLITES];
    
    // Clear all satellite slots (call when master initializes)
    void clearAllSatellites()
    {
        for (int i = 0; i < MAX_SATELLITES; ++i)
        {
            satellites[i].active.store(false);
            satellites[i].lastUpdateTime.store(0);
        }
        activeSatelliteCount.store(0);
    }
    
    // Find an available slot for a new satellite
    // Also clears stale satellites that haven't updated in 5+ seconds
    int findAvailableSlot() 
    {
        auto currentTime = juce::Time::currentTimeMillis();
        
        // First pass: find completely inactive slot
        for (int i = 0; i < MAX_SATELLITES; ++i)
        {
            if (!satellites[i].active.load())
                return i;
        }
        
        // Second pass: find stale slot (active but not updated in 5+ seconds)
        for (int i = 0; i < MAX_SATELLITES; ++i)
        {
            auto lastUpdate = satellites[i].lastUpdateTime.load();
            if (currentTime - lastUpdate > 5000)
            {
                // Clear the stale slot
                satellites[i].active.store(false);
                satellites[i].lastUpdateTime.store(0);
                return i;
            }
        }
        
        return -1;
    }
};

class SharedMemoryManager
{
public:
    SharedMemoryManager() = default;
    
    ~SharedMemoryManager()
    {
        close();
    }
    
    bool openOrCreate()
    {
        // Use a file in /tmp for cross-plugin communication (works on macOS without sandboxing issues)
        juce::File shmFile("/tmp/ar3s_shared_memory.bin");
        filePath = shmFile.getFullPathName();
        
        bool needsInit = false;
        
        // Create file if it doesn't exist
        if (!shmFile.existsAsFile())
        {
            // Create and size the file
            juce::FileOutputStream fos(shmFile);
            if (!fos.openedOk())
            {
                DBG("AR3S SharedMemory: Failed to create file");
                return false;
            }
            
            // Write zeros to initialize the file
            std::vector<char> zeros(sizeof(SharedPluginData), 0);
            fos.write(zeros.data(), zeros.size());
            fos.flush();
            needsInit = true;
            isCreator = true;
            DBG("AR3S SharedMemory: Created new file");
        }
        
        // Open the file for memory mapping
        fd = ::open(filePath.toRawUTF8(), O_RDWR);
        if (fd < 0)
        {
            DBG("AR3S SharedMemory: Failed to open file, errno=" + juce::String(errno));
            return false;
        }
        
        // Ensure file is correct size
        struct stat st;
        if (fstat(fd, &st) == 0 && st.st_size < (off_t)sizeof(SharedPluginData))
        {
            ftruncate(fd, sizeof(SharedPluginData));
            needsInit = true;
        }
        
        // Map memory
        data = static_cast<SharedPluginData*>(
            mmap(nullptr, sizeof(SharedPluginData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        
        if (data == MAP_FAILED)
        {
            DBG("AR3S SharedMemory: mmap failed, errno=" + juce::String(errno));
            data = nullptr;
            ::close(fd);
            fd = -1;
            return false;
        }
        
        // Initialize if we created it or it's new
        if (needsInit)
        {
            new (data) SharedPluginData();
            DBG("AR3S SharedMemory: Initialized new shared data");
        }
        
        DBG("AR3S SharedMemory: Successfully mapped, data=" + juce::String::toHexString((int64_t)data));
        return true;
    }
    
    void close()
    {
        if (data != nullptr)
        {
            munmap(data, sizeof(SharedPluginData));
            data = nullptr;
        }
        
        if (fd >= 0)
        {
            ::close(fd);
            fd = -1;
        }
    }
    
    SharedPluginData* getData() { return data; }
    const SharedPluginData* getData() const { return data; }
    bool isValid() const { return data != nullptr; }
    
private:
    int fd = -1;
    SharedPluginData* data = nullptr;
    bool isCreator = false;
    juce::String filePath;
};
