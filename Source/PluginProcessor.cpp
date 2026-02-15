
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

// Returns a JSON string with all satellite/track data for AI context
juce::String SimpleGainAudioProcessor::getSatellitesJsonContext() const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    juce::Array<juce::var> satellitesArray;
    for (int i = 0; i < MAX_SATELLITES; ++i)
    {
        auto info = getSatelliteInfo(i);
        if (info.active)
        {
            juce::DynamicObject::Ptr sat = new juce::DynamicObject();
            sat->setProperty("index", i);
            sat->setProperty("name", info.channelName.isEmpty() ? juce::String("Track ") + juce::String(i+1) : info.channelName);
            sat->setProperty("sourceType", info.sourceType);
            sat->setProperty("rmsDb", info.rmsDb);
            sat->setProperty("peakDb", info.peakDb);
            sat->setProperty("crestDb", info.crestDb);
            sat->setProperty("phaseCorrelation", info.phaseCorrelation);
            sat->setProperty("currentGain", info.currentGain);
            sat->setProperty("gainDb", info.gainDb);
            sat->setProperty("targetDb", info.targetDb);
            sat->setProperty("ceilingDb", info.ceilingDb);
            sat->setProperty("autoEnabled", info.autoEnabled);
            sat->setProperty("riderAmount", info.riderAmount);
            sat->setProperty("controlledByMaster", info.controlledByMaster);
            satellitesArray.add(juce::var(sat.get()));
        }
    }
    root->setProperty("satellites", juce::var(satellitesArray));
    return juce::JSON::toString(juce::var(root.get()));
}

namespace
{
    float dbToLinear(float db)
    {
        return std::pow(10.0f, db / 20.0f);
    }

    float linearToDb(float value)
    {
        return 20.0f * std::log10(std::max(value, 1.0e-9f));
    }
    
    juce::File getSettingsFile()
    {
        return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile("ARES")
            .getChildFile("settings.xml");
    }
}

class SimpleGainAudioProcessor::AiClient : public juce::Thread
{
public:
    explicit AiClient(SimpleGainAudioProcessor& owner)
        : juce::Thread("AiClient"), processor(owner)
    {
        startThread();
    }

    ~AiClient() override
    {
        signalThreadShouldExit();
        requestEvent.signal();
        waitForThreadToExit(2000);
    }

    void request(const juce::String& promptToUse, bool isChatMode = false)
    {
        const juce::ScopedLock lock(requestLock);
        pendingPrompt = promptToUse;
        pendingIsChatMode = isChatMode;
        hasRequest = true;
        requestEvent.signal();
    }

    void requestModelList()
    {
        const juce::ScopedLock lock(requestLock);
        wantsModelList = true;
        requestEvent.signal();
    }

    void run() override
    {
        while (! threadShouldExit())
        {
            requestEvent.wait(2000);
            if (threadShouldExit())
                break;

            juce::String prompt;
            bool fetchModels = false;
            bool isChatMode = false;
            {
                const juce::ScopedLock lock(requestLock);
                if (wantsModelList)
                {
                    fetchModels = true;
                    wantsModelList = false;
                }
                if (! hasRequest && ! fetchModels)
                    continue;
                prompt = pendingPrompt;
                isChatMode = pendingIsChatMode;
                hasRequest = false;
            }

            auto provider = processor.getAiProvider();
            auto apiKey = processor.getApiKey();
            auto model = processor.getSelectedModel();

            if (fetchModels)
            {
                fetchModelsForProvider(provider, apiKey);
                if (prompt.isEmpty())
                    continue;
            }

            if (! prompt.isEmpty())
            {
                sendRequestToProvider(provider, apiKey, model, prompt, isChatMode);
            }
        }
    }

private:
    void fetchModelsForProvider(SimpleGainAudioProcessor::AiProvider provider, const juce::String& apiKey)
    {
        processor.setAiStatusMessage("Fetching models...");

        if (provider == SimpleGainAudioProcessor::AiProvider::Ollama)
        {
            // Support local Ollama (default) and Ollama cloud via OLLAMA_API_URL
            const char* envUrl = std::getenv("OLLAMA_API_URL");
            juce::String baseUrl = envUrl ? juce::String(envUrl) : "http://127.0.0.1:11434";
            juce::URL tagsUrl(baseUrl + "/api/tags");

            int statusCode = 0;
            juce::String headers;
            const char* envKey = std::getenv("OLLAMA_API_KEY");
            if (envKey != nullptr && envKey[0] != '\0')
                headers = "Authorization: Bearer " + juce::String(envKey);

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders(headers)
                .withConnectionTimeoutMs(10000)
                .withStatusCode(&statusCode);

            std::unique_ptr<juce::InputStream> stream(tagsUrl.createInputStream(options));

            if (stream == nullptr)
            {
                processor.setAiStatusMessage(envUrl ? "Ollama (cloud) not reachable" : "Ollama not reachable");
                processor.setAiNotesMessage(envUrl ? "Check OLLAMA_API_URL and OLLAMA_API_KEY" : "Start Ollama with `ollama serve` and retry.");
                processor.setAvailableModels({ "llama3" });
                return;
            }

            auto responseText = stream->readEntireStreamAsString();
            if (statusCode != 200)
            {
                processor.setAiStatusMessage("Ollama error (HTTP " + juce::String(statusCode) + ")");
                processor.setAvailableModels({ "llama3" });
                return;
            }

            auto parsed = juce::JSON::parse(responseText);
            juce::StringArray models;

            if (parsed.isObject())
            {
                auto* obj = parsed.getDynamicObject();
                auto modelsVar = obj->getProperty("models");
                if (modelsVar.isArray())
                {
                    for (const auto& item : *modelsVar.getArray())
                    {
                        if (item.isObject())
                        {
                            auto nameVar = item.getDynamicObject()->getProperty("name");
                            if (nameVar.isString())
                                models.addIfNotAlreadyThere(nameVar.toString());
                        }
                    }
                }
            }

            if (models.isEmpty())
                models.add("llama3");

            processor.setAvailableModels(models);
            processor.setAiStatusMessage("Models updated (" + juce::String(models.size()) + " available) [" + (envUrl ? "Ollama Cloud" : "Local Ollama") + "]");
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::OpenAI)
        {
            if (apiKey.isEmpty())
            {
                processor.setAiStatusMessage("API key required");
                processor.setAvailableModels({ "gpt-4o", "gpt-4o-mini", "gpt-4-turbo", "gpt-3.5-turbo" });
                return;
            }

            juce::URL url("https://api.openai.com/v1/models");
            int statusCode = 0;
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders("Authorization: Bearer " + apiKey)
                .withConnectionTimeoutMs(15000)
                .withStatusCode(&statusCode);

            auto stream = url.createInputStream(options);
            if (stream == nullptr || statusCode != 200)
            {
                processor.setAiStatusMessage(statusCode == 401 ? "Invalid API key" : "OpenAI connection failed");
                processor.setAvailableModels({ "gpt-4o", "gpt-4o-mini", "gpt-4-turbo", "gpt-3.5-turbo" });
                return;
            }

            auto responseText = stream->readEntireStreamAsString();
            auto parsed = juce::JSON::parse(responseText);
            juce::StringArray models;

            if (parsed.isObject())
            {
                auto* obj = parsed.getDynamicObject();
                auto dataVar = obj->getProperty("data");
                if (dataVar.isArray())
                {
                    for (const auto& item : *dataVar.getArray())
                    {
                        if (item.isObject())
                        {
                            auto idVar = item.getDynamicObject()->getProperty("id");
                            if (idVar.isString())
                            {
                                auto id = idVar.toString();
                                if (id.startsWith("gpt-"))
                                    models.addIfNotAlreadyThere(id);
                            }
                        }
                    }
                }
            }

            if (models.isEmpty())
                models = { "gpt-4o", "gpt-4o-mini", "gpt-4-turbo", "gpt-3.5-turbo" };
            
            models.sort(true);
            processor.setAvailableModels(models);
            processor.setAiStatusMessage("OpenAI connected (" + juce::String(models.size()) + " models)");
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::Anthropic)
        {
            // Anthropic doesn't have a public models endpoint, use known models
            juce::StringArray models = { 
                "claude-sonnet-4-20250514", 
                "claude-3-5-sonnet-20241022", 
                "claude-3-5-haiku-20241022", 
                "claude-3-opus-20240229",
                "claude-3-haiku-20240307"
            };
            processor.setAvailableModels(models);
            
            if (apiKey.isEmpty())
                processor.setAiStatusMessage("API key required for Anthropic");
            else
                processor.setAiStatusMessage("Anthropic ready (" + juce::String(models.size()) + " models)");
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::OpenRouter)
        {
            if (apiKey.isEmpty())
            {
                processor.setAiStatusMessage("API key required for OpenRouter");
                processor.setAvailableModels({ 
                    "openai/gpt-4o", "anthropic/claude-sonnet-4-20250514", "anthropic/claude-3.5-sonnet",
                    "google/gemini-pro-1.5", "meta-llama/llama-3.1-70b-instruct"
                });
                return;
            }

            // Fetch models from OpenRouter API
            juce::URL url("https://openrouter.ai/api/v1/models");
            int statusCode = 0;
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders("Authorization: Bearer " + apiKey)
                .withConnectionTimeoutMs(15000)
                .withStatusCode(&statusCode);

            auto stream = url.createInputStream(options);
            if (stream == nullptr || statusCode != 200)
            {
                processor.setAiStatusMessage(statusCode == 401 ? "Invalid API key" : "OpenRouter connection failed");
                processor.setAvailableModels({ 
                    "openai/gpt-4o", "anthropic/claude-sonnet-4-20250514", "anthropic/claude-3.5-sonnet",
                    "google/gemini-pro-1.5", "meta-llama/llama-3.1-70b-instruct"
                });
                return;
            }

            auto responseText = stream->readEntireStreamAsString();
            auto parsed = juce::JSON::parse(responseText);
            juce::StringArray models;

            if (parsed.isObject())
            {
                auto* obj = parsed.getDynamicObject();
                auto dataVar = obj->getProperty("data");
                if (dataVar.isArray())
                {
                    for (const auto& item : *dataVar.getArray())
                    {
                        if (item.isObject())
                        {
                            auto idVar = item.getDynamicObject()->getProperty("id");
                            if (idVar.isString())
                            {
                                auto id = idVar.toString();
                                // Filter for popular/useful models
                                if (id.contains("gpt-4") || id.contains("claude") || 
                                    id.contains("gemini") || id.contains("llama") ||
                                    id.contains("mixtral") || id.contains("mistral"))
                                    models.addIfNotAlreadyThere(id);
                            }
                        }
                    }
                }
            }

            if (models.isEmpty())
            {
                models = { 
                    "openai/gpt-4o", "anthropic/claude-sonnet-4-20250514", "anthropic/claude-3.5-sonnet",
                    "google/gemini-pro-1.5", "meta-llama/llama-3.1-70b-instruct"
                };
            }
            
            models.sort(true);
            processor.setAvailableModels(models);
            processor.setAiStatusMessage("OpenRouter connected (" + juce::String(models.size()) + " models)");
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::MiniMax)
        {
            // MiniMax known models
            juce::StringArray models = { "abab6.5s-chat", "abab6.5-chat", "abab5.5-chat", "abab5.5s-chat" };
            processor.setAvailableModels(models);
            
            if (apiKey.isEmpty())
                processor.setAiStatusMessage("API key required for MiniMax");
            else
                processor.setAiStatusMessage("MiniMax ready (" + juce::String(models.size()) + " models)");
        }
    }

    void sendRequestToProvider(SimpleGainAudioProcessor::AiProvider provider, 
                                const juce::String& apiKey, 
                                const juce::String& model, 
                                const juce::String& prompt,
                                bool isChatMode = false)
    {
        processor.setAiStatusMessage("Querying " + model + "...");

        if (provider == SimpleGainAudioProcessor::AiProvider::Ollama)
        {
            sendOllamaRequest(model, prompt, isChatMode);
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::OpenAI)
        {
            if (apiKey.isEmpty())
            {
                processor.setAiStatusMessage("OpenAI API key required");
                processor.setAiNotesMessage("Add your OpenAI API key in Settings to use this feature.");
                return;
            }
            sendOpenAIRequest(apiKey, model, prompt, isChatMode);
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::Anthropic)
        {
            if (apiKey.isEmpty())
            {
                processor.setAiStatusMessage("Anthropic API key required");
                processor.setAiNotesMessage("Add your Anthropic API key in Settings to use this feature.");
                return;
            }
            sendAnthropicRequest(apiKey, model, prompt, isChatMode);
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::OpenRouter)
        {
            if (apiKey.isEmpty())
            {
                processor.setAiStatusMessage("OpenRouter API key required");
                processor.setAiNotesMessage("Add your OpenRouter API key in Settings to use this feature.");
                return;
            }
            sendOpenRouterRequest(apiKey, model, prompt, isChatMode);
        }
        else if (provider == SimpleGainAudioProcessor::AiProvider::MiniMax)
        {
            if (apiKey.isEmpty())
            {
                processor.setAiStatusMessage("MiniMax API key required");
                processor.setAiNotesMessage("Add your MiniMax API key in Settings to use this feature.");
                return;
            }
            sendMiniMaxRequest(apiKey, model, prompt, isChatMode);
        }
    }

    void sendOllamaRequest(const juce::String& model, const juce::String& prompt, bool isChatMode = false)
    {
        auto requestObject = juce::DynamicObject::Ptr(new juce::DynamicObject());
        requestObject->setProperty("model", model);
        requestObject->setProperty("prompt", prompt);
        requestObject->setProperty("stream", false);

        auto jsonBody = juce::JSON::toString(juce::var(requestObject.get()));
        
        // Use OLLAMA_API_URL for cloud or default to local server
        const char* envUrl = std::getenv("OLLAMA_API_URL");
        juce::String baseUrl = envUrl ? juce::String(envUrl) : "http://127.0.0.1:11434";
        juce::URL url(baseUrl + "/api/generate");
        url = url.withPOSTData(jsonBody);

        int statusCode = 0;
        juce::String headers = "Content-Type: application/json";
        const char* envKey = std::getenv("OLLAMA_API_KEY");
        if (envKey != nullptr && envKey[0] != '\0')
            headers += "\r\nAuthorization: Bearer " + juce::String(envKey);

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders(headers)
            .withConnectionTimeoutMs(120000)
            .withStatusCode(&statusCode);

        auto stream = url.createInputStream(options);
        
        if (stream == nullptr)
        {
            processor.setAiStatusMessage(envUrl ? "Ollama (cloud) connection failed" : "Ollama connection failed");
            if (isChatMode)
                processor.setChatResponseMessage("Could not connect to Ollama. Check your OLLAMA_API_URL / OLLAMA_API_KEY or start local Ollama with `ollama serve`.");
            else
                processor.setAiNotesMessage("Could not connect to Ollama. Check your OLLAMA_API_URL / OLLAMA_API_KEY or start local Ollama with `ollama serve`.");
            return;
        }

        auto responseText = stream->readEntireStreamAsString();

        if (statusCode != 200)
        {
            processor.setAiStatusMessage("Ollama error (HTTP " + juce::String(statusCode) + ")");
            if (isChatMode)
                processor.setChatResponseMessage(responseText.isNotEmpty() ? responseText : "No response body");
            else
                processor.applyAiResponseText(responseText.isNotEmpty() ? responseText : "No response body");
            return;
        }

        auto parsed = juce::JSON::parse(responseText);
        if (parsed.isObject())
        {
            auto* obj = parsed.getDynamicObject();
            auto responseVar = obj->getProperty("response");
            if (responseVar.isString())
            {
                if (isChatMode)
                    processor.setChatResponseMessage(responseVar.toString());
                else
                    processor.applyAiResponseText(responseVar.toString());
                return;
            }
        }
        if (isChatMode)
            processor.setChatResponseMessage(responseText);
        else
            processor.applyAiResponseText(responseText);
    }

    void sendOpenAIRequest(const juce::String& apiKey, const juce::String& model, const juce::String& prompt, bool isChatMode = false)
    {
        auto messagesArray = juce::Array<juce::var>();
        auto messageObj = juce::DynamicObject::Ptr(new juce::DynamicObject());
        messageObj->setProperty("role", "user");
        messageObj->setProperty("content", prompt);
        messagesArray.add(juce::var(messageObj.get()));

        auto requestObject = juce::DynamicObject::Ptr(new juce::DynamicObject());
        requestObject->setProperty("model", model);
        requestObject->setProperty("messages", messagesArray);
        requestObject->setProperty("max_tokens", 1000);

        auto jsonBody = juce::JSON::toString(juce::var(requestObject.get()));
        
        juce::URL url("https://api.openai.com/v1/chat/completions");
        url = url.withPOSTData(jsonBody);

        int statusCode = 0;
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders("Content-Type: application/json\r\nAuthorization: Bearer " + apiKey)
            .withConnectionTimeoutMs(60000)
            .withStatusCode(&statusCode);

        auto stream = url.createInputStream(options);
        
        if (stream == nullptr)
        {
            processor.setAiStatusMessage("OpenAI connection failed");
            auto errMsg = "Could not connect to OpenAI API. Check your internet connection.";
            if (isChatMode) processor.setChatResponseMessage(errMsg);
            else processor.setAiNotesMessage(errMsg);
            return;
        }

        auto responseText = stream->readEntireStreamAsString();

        if (statusCode == 401)
        {
            processor.setAiStatusMessage("Invalid OpenAI API key");
            auto errMsg = "Please check your API key in Settings.";
            if (isChatMode) processor.setChatResponseMessage(errMsg);
            else processor.setAiNotesMessage(errMsg);
            return;
        }

        if (statusCode != 200)
        {
            processor.setAiStatusMessage("OpenAI error (HTTP " + juce::String(statusCode) + ")");
            if (isChatMode) processor.setChatResponseMessage(responseText);
            else processor.applyAiResponseText(responseText);
            return;
        }

        auto parsed = juce::JSON::parse(responseText);
        if (parsed.isObject())
        {
            auto* obj = parsed.getDynamicObject();
            auto choicesVar = obj->getProperty("choices");
            if (choicesVar.isArray() && choicesVar.getArray()->size() > 0)
            {
                auto firstChoice = choicesVar.getArray()->getFirst();
                if (firstChoice.isObject())
                {
                    auto messageVar = firstChoice.getDynamicObject()->getProperty("message");
                    if (messageVar.isObject())
                    {
                        auto contentVar = messageVar.getDynamicObject()->getProperty("content");
                        if (contentVar.isString())
                        {
                            if (isChatMode) processor.setChatResponseMessage(contentVar.toString());
                            else processor.applyAiResponseText(contentVar.toString());
                            return;
                        }
                    }
                }
            }
        }
        if (isChatMode) processor.setChatResponseMessage(responseText);
        else processor.applyAiResponseText(responseText);
    }

    void sendAnthropicRequest(const juce::String& apiKey, const juce::String& model, const juce::String& prompt, bool isChatMode = false)
    {
        auto messagesArray = juce::Array<juce::var>();
        auto messageObj = juce::DynamicObject::Ptr(new juce::DynamicObject());
        messageObj->setProperty("role", "user");
        messageObj->setProperty("content", prompt);
        messagesArray.add(juce::var(messageObj.get()));

        auto requestObject = juce::DynamicObject::Ptr(new juce::DynamicObject());
        requestObject->setProperty("model", model);
        requestObject->setProperty("messages", messagesArray);
        requestObject->setProperty("max_tokens", 1000);

        auto jsonBody = juce::JSON::toString(juce::var(requestObject.get()));
        
        juce::URL url("https://api.anthropic.com/v1/messages");
        url = url.withPOSTData(jsonBody);

        int statusCode = 0;
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders("Content-Type: application/json\r\nx-api-key: " + apiKey + "\r\nanthropic-version: 2023-06-01")
            .withConnectionTimeoutMs(60000)
            .withStatusCode(&statusCode);

        auto stream = url.createInputStream(options);
        
        if (stream == nullptr)
        {
            processor.setAiStatusMessage("Anthropic connection failed");
            auto errMsg = "Could not connect to Anthropic API. Check your internet connection.";
            if (isChatMode) processor.setChatResponseMessage(errMsg);
            else processor.setAiNotesMessage(errMsg);
            return;
        }

        auto responseText = stream->readEntireStreamAsString();

        if (statusCode == 401)
        {
            processor.setAiStatusMessage("Invalid Anthropic API key");
            auto errMsg = "Please check your API key in Settings.";
            if (isChatMode) processor.setChatResponseMessage(errMsg);
            else processor.setAiNotesMessage(errMsg);
            return;
        }

        if (statusCode != 200)
        {
            processor.setAiStatusMessage("Anthropic error (HTTP " + juce::String(statusCode) + ")");
            if (isChatMode) processor.setChatResponseMessage(responseText);
            else processor.applyAiResponseText(responseText);
            return;
        }

        auto parsed = juce::JSON::parse(responseText);
        if (parsed.isObject())
        {
            auto* obj = parsed.getDynamicObject();
            auto contentVar = obj->getProperty("content");
            if (contentVar.isArray() && contentVar.getArray()->size() > 0)
            {
                auto firstContent = contentVar.getArray()->getFirst();
                if (firstContent.isObject())
                {
                    auto textVar = firstContent.getDynamicObject()->getProperty("text");
                    if (textVar.isString())
                    {
                        if (isChatMode) processor.setChatResponseMessage(textVar.toString());
                        else processor.applyAiResponseText(textVar.toString());
                        return;
                    }
                }
            }
        }
        if (isChatMode) processor.setChatResponseMessage(responseText);
        else processor.applyAiResponseText(responseText);
    }

    void sendOpenRouterRequest(const juce::String& apiKey, const juce::String& model, const juce::String& prompt, bool isChatMode = false)
    {
        auto messagesArray = juce::Array<juce::var>();
        auto messageObj = juce::DynamicObject::Ptr(new juce::DynamicObject());
        messageObj->setProperty("role", "user");
        messageObj->setProperty("content", prompt);
        messagesArray.add(juce::var(messageObj.get()));

        auto requestObject = juce::DynamicObject::Ptr(new juce::DynamicObject());
        requestObject->setProperty("model", model);
        requestObject->setProperty("messages", messagesArray);

        auto jsonBody = juce::JSON::toString(juce::var(requestObject.get()));
        
        juce::URL url("https://openrouter.ai/api/v1/chat/completions");
        url = url.withPOSTData(jsonBody);

        int statusCode = 0;
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders("Content-Type: application/json\r\nAuthorization: Bearer " + apiKey)
            .withConnectionTimeoutMs(60000)
            .withStatusCode(&statusCode);

        auto stream = url.createInputStream(options);
        
        if (stream == nullptr)
        {
            processor.setAiStatusMessage("OpenRouter connection failed");
            if (isChatMode) processor.setChatResponseMessage("Connection failed");
            return;
        }

        auto responseText = stream->readEntireStreamAsString();

        if (statusCode != 200)
        {
            processor.setAiStatusMessage("OpenRouter error (HTTP " + juce::String(statusCode) + ")");
            if (isChatMode) processor.setChatResponseMessage(responseText);
            else processor.applyAiResponseText(responseText);
            return;
        }

        auto parsed = juce::JSON::parse(responseText);
        if (parsed.isObject())
        {
            auto* obj = parsed.getDynamicObject();
            auto choicesVar = obj->getProperty("choices");
            if (choicesVar.isArray() && choicesVar.getArray()->size() > 0)
            {
                auto firstChoice = choicesVar.getArray()->getFirst();
                if (firstChoice.isObject())
                {
                    auto messageVar = firstChoice.getDynamicObject()->getProperty("message");
                    if (messageVar.isObject())
                    {
                        auto contentVar = messageVar.getDynamicObject()->getProperty("content");
                        if (contentVar.isString())
                        {
                            if (isChatMode) processor.setChatResponseMessage(contentVar.toString());
                            else processor.applyAiResponseText(contentVar.toString());
                            return;
                        }
                    }
                }
            }
        }
        if (isChatMode) processor.setChatResponseMessage(responseText);
        else processor.applyAiResponseText(responseText);
    }

    void sendMiniMaxRequest(const juce::String& apiKey, const juce::String& model, const juce::String& prompt, bool isChatMode = false)
    {
        auto messagesArray = juce::Array<juce::var>();
        auto messageObj = juce::DynamicObject::Ptr(new juce::DynamicObject());
        messageObj->setProperty("sender_type", "USER");
        messageObj->setProperty("text", prompt);
        messagesArray.add(juce::var(messageObj.get()));

        auto requestObject = juce::DynamicObject::Ptr(new juce::DynamicObject());
        requestObject->setProperty("model", model);
        requestObject->setProperty("messages", messagesArray);

        auto jsonBody = juce::JSON::toString(juce::var(requestObject.get()));
        
        juce::URL url("https://api.minimax.chat/v1/text/chatcompletion_v2");
        url = url.withPOSTData(jsonBody);

        int statusCode = 0;
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
            .withExtraHeaders("Content-Type: application/json\r\nAuthorization: Bearer " + apiKey)
            .withConnectionTimeoutMs(60000)
            .withStatusCode(&statusCode);

        auto stream = url.createInputStream(options);
        
        if (stream == nullptr)
        {
            processor.setAiStatusMessage("MiniMax connection failed");
            if (isChatMode) processor.setChatResponseMessage("Connection failed");
            return;
        }

        auto responseText = stream->readEntireStreamAsString();

        if (statusCode != 200)
        {
            processor.setAiStatusMessage("MiniMax error (HTTP " + juce::String(statusCode) + ")");
            if (isChatMode) processor.setChatResponseMessage(responseText);
            else processor.applyAiResponseText(responseText);
            return;
        }

        auto parsed = juce::JSON::parse(responseText);
        if (parsed.isObject())
        {
            auto* obj = parsed.getDynamicObject();
            auto choicesVar = obj->getProperty("choices");
            if (choicesVar.isArray() && choicesVar.getArray()->size() > 0)
            {
                auto firstChoice = choicesVar.getArray()->getFirst();
                if (firstChoice.isObject())
                {
                    auto messageVar = firstChoice.getDynamicObject()->getProperty("message");
                    if (messageVar.isObject())
                    {
                        auto contentVar = messageVar.getDynamicObject()->getProperty("content");
                        if (contentVar.isString())
                        {
                            if (isChatMode) processor.setChatResponseMessage(contentVar.toString());
                            else processor.applyAiResponseText(contentVar.toString());
                            return;
                        }
                    }
                }
            }
        }
        if (isChatMode) processor.setChatResponseMessage(responseText);
        else processor.applyAiResponseText(responseText);
    }

    SimpleGainAudioProcessor& processor;
    juce::WaitableEvent requestEvent;
    juce::CriticalSection requestLock;
    juce::String pendingPrompt;
    bool pendingIsChatMode = false;
    bool hasRequest = false;
    bool wantsModelList = false;
};

SimpleGainAudioProcessor::SimpleGainAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    aiClient = std::make_unique<AiClient>(*this);
    availableModels = { "llama3" };
    loadSettings();
    
    // Initialize shared memory for satellite communication
    initializeSharedMemory();
}

SimpleGainAudioProcessor::~SimpleGainAudioProcessor()
{
    sharedMemory.close();
}

const juce::String SimpleGainAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleGainAudioProcessor::acceptsMidi() const
{
    return false;
}

bool SimpleGainAudioProcessor::producesMidi() const
{
    return false;
}

bool SimpleGainAudioProcessor::isMidiEffect() const
{
    return false;
}

double SimpleGainAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleGainAudioProcessor::getNumPrograms()
{
    return 1;
}

int SimpleGainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleGainAudioProcessor::setCurrentProgram(int)
{
}

const juce::String SimpleGainAudioProcessor::getProgramName(int)
{
    return {};
}

void SimpleGainAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void SimpleGainAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;

    // Auto-gain smoothing: ~300ms time constant for smooth, transparent level adjustment
    // Matches satellite processor for consistent behavior
    autoSmoothCoeff = std::exp(-1.0f / (static_cast<float>(currentSampleRate) * 0.3f));
    
    // Vocal rider: professional timing
    // Attack: 80ms - fast enough to catch transients but not cause pumping
    // Release: 300ms - smooth return to avoid audible gain riding
    riderAttackCoeff = std::exp(-1.0f / (0.001f * 80.0f * static_cast<float>(currentSampleRate)));
    riderReleaseCoeff = std::exp(-1.0f / (0.001f * 300.0f * static_cast<float>(currentSampleRate)));
    
    // Reset FFT buffers
    fftData.fill(0.0f);
    fftInputBuffer.fill(0.0f);
    fftInputPos = 0;
}

void SimpleGainAudioProcessor::releaseResources()
{
}

bool SimpleGainAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void SimpleGainAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    // ============ PRE-PROCESSING METERING ============
    float preSumSquares = 0.0f;
    float prePeak = 0.0f;
    float preSumLR = 0.0f, preSumL2 = 0.0f, preSumR2 = 0.0f;

    if (numChannels >= 2)
    {
        const auto* leftData = buffer.getReadPointer(0);
        const auto* rightData = buffer.getReadPointer(1);
        
        for (int i = 0; i < numSamples; ++i)
        {
            const auto L = leftData[i];
            const auto R = rightData[i];
            preSumLR += L * R;
            preSumL2 += L * L;
            preSumR2 += R * R;
            preSumSquares += L * L + R * R;
            prePeak = std::max(prePeak, std::max(std::abs(L), std::abs(R)));
        }
        
        const float denom = std::sqrt(preSumL2 * preSumR2);
        float correlation = (denom > 1.0e-10f) ? (preSumLR / denom) : 0.0f;
        prePhaseCorrelation.store(juce::jlimit(-1.0f, 1.0f, correlation));
    }
    else
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            const auto* data = buffer.getReadPointer(channel);
            for (int i = 0; i < numSamples; ++i)
            {
                const auto sample = data[i];
                preSumSquares += sample * sample;
                prePeak = std::max(prePeak, std::abs(sample));
            }
        }
        prePhaseCorrelation.store(0.0f);  // Mono has no phase relationship
    }

    const auto preRms = std::sqrt(preSumSquares / std::max(1, numChannels * numSamples));
    const auto preRmsDbVal = linearToDb(preRms);
    const auto prePeakDbVal = linearToDb(prePeak);
    preRmsDb.store(preRmsDbVal);
    prePeakDb.store(prePeakDbVal);
    preCrestDb.store(prePeakDbVal - preRmsDbVal);

    // ============ PROCESSING ============
    const auto manualGain = parameters.getRawParameterValue("gain");
    const auto autoEnabled = parameters.getRawParameterValue("auto_enabled");
    const auto autoTargetDb = parameters.getRawParameterValue("auto_target_db");
    const auto riderEnabled = parameters.getRawParameterValue("rider_enabled");
    const auto riderAmount = parameters.getRawParameterValue("rider_amount");

    float autoGainLinear = 1.0f;
    
    // Auto-gain: when enabled, calculates gain needed to hit TARGET level
    // This works TOGETHER with the manual GAIN knob
    if (autoEnabled->load() > 0.5f)
    {
        const auto targetDb = autoTargetDb->load();
        
        // Only adjust if we have meaningful signal
        if (preRmsDbVal > -60.0f)
        {
            // Calculate gain needed to reach target RMS
            auto desiredGainDb = targetDb - preRmsDbVal;
            
            // Clamp to safe professional range:
            // -24 dB = 1/16 reduction (very quiet)
            // +12 dB = 4x boost (maximum safe boost without severe noise amplification)
            desiredGainDb = juce::jlimit(-24.0f, 12.0f, desiredGainDb);
            
            autoGainLinear = dbToLinear(desiredGainDb);
            
            // Smooth the gain changes
            autoSmoothedGain = autoSmoothCoeff * autoSmoothedGain + (1.0f - autoSmoothCoeff) * autoGainLinear;
            autoGainLinear = autoSmoothedGain;
        }
    }

    float riderGainLinear = 1.0f;
    if (riderEnabled->load() > 0.5f)
    {
        const auto targetDb = autoTargetDb->load();
        
        if (preRmsDbVal > -60.0f)
        {
            const auto desiredGainDb = targetDb - preRmsDbVal;
            // Professional vocal rider range: ±6 dB maximum for transparent operation
            // Larger ranges cause audible pumping and unnatural dynamics
            riderGainLinear = dbToLinear(juce::jlimit(-6.0f, 6.0f, desiredGainDb));

            const auto coeff = (riderGainLinear < riderSmoothedGain) ? riderAttackCoeff : riderReleaseCoeff;
            riderSmoothedGain = coeff * riderSmoothedGain + (1.0f - coeff) * riderGainLinear;
            // Blend between unity and smoothed gain based on rider amount
            riderGainLinear = juce::jmap(riderAmount->load(), 1.0f, riderSmoothedGain);
        }
    }
    
    // LUFS-based auto-gain: adjusts to hit target LUFS (separate from RMS-based target)
    float lufsGainLinear = 1.0f;
    const auto lufsEnabled = parameters.getRawParameterValue("lufs_enabled");
    const auto lufsTargetParam = parameters.getRawParameterValue("lufs_target");
    
    if (lufsEnabled && lufsEnabled->load() > 0.5f && lufsTargetParam)
    {
        const float lufsTarget = lufsTargetParam->load();
        const float currentLufs = shortTermLufs.load();
        
        // Only adjust if we have valid LUFS reading (not silence)
        if (currentLufs > -60.0f)
        {
            // Calculate gain needed to reach target LUFS
            float desiredLufsGainDb = lufsTarget - currentLufs;
            
            // Clamp to reasonable range for transparency
            desiredLufsGainDb = juce::jlimit(-12.0f, 12.0f, desiredLufsGainDb);
            
            float targetLufsGain = dbToLinear(desiredLufsGainDb);
            
            // Very slow, smooth adjustment for transparent LUFS matching
            // Use 0.999 for ~100ms integration time at 44.1kHz
            lufsSmoothedGain = 0.9995f * lufsSmoothedGain + 0.0005f * targetLufsGain;
            lufsGainLinear = lufsSmoothedGain;
        }
    }

    // Manual GAIN (in dB) always applies, auto-gain and LUFS gain multiplied on top when enabled
    const auto gainDb = manualGain->load();
    const auto manualGainLinear = dbToLinear(gainDb);
    const auto totalGain = manualGainLinear * autoGainLinear * riderGainLinear * lufsGainLinear;
    buffer.applyGain(totalGain);
    
    // === CEILING / MAX PEAK LIMITER ===
    // Transparent peak limiting with smooth gain reduction to avoid pumping
    // NO saturation, NO compression curves - pure gain reduction only
    if (auto* ceilingParam = parameters.getRawParameterValue("ceiling"))
    {
        float ceilingDb = ceilingParam->load();
        // Always apply if ceiling is below 0 dB
        if (ceilingDb < -0.1f)
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
            
            // Calculate target gain reduction
            float targetGainReduction = 1.0f;
            if (maxPeak > ceilingLinear && maxPeak > 0.0001f)
            {
                targetGainReduction = ceilingLinear / maxPeak;
            }
            
            // Smooth gain reduction to avoid pumping artifacts
            // Fast attack (1ms) to catch peaks, slow release (50ms) for transparency
            float attackCoeff = std::exp(-1.0f / (currentSampleRate * 0.001f));   // 1ms attack
            float releaseCoeff = std::exp(-1.0f / (currentSampleRate * 0.050f));  // 50ms release
            
            for (int i = 0; i < numSamples; ++i)
            {
                // Per-sample envelope following
                float coeff = (targetGainReduction < ceilingSmoothedGain) ? attackCoeff : releaseCoeff;
                ceilingSmoothedGain = coeff * ceilingSmoothedGain + (1.0f - coeff) * targetGainReduction;
                
                // Apply smoothed gain reduction
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float* data = buffer.getWritePointer(ch);
                    data[i] *= ceilingSmoothedGain;
                }
            }
        }
    }
    
    // Push control values to satellites - rate limited to avoid performance issues
    // Only push when values change OR every 50ms (20 times per second max)
    if (sharedMemoryConnected)
    {
        const auto currentTime = juce::Time::currentTimeMillis();
        const auto targetDb = autoTargetDb->load();
        const auto autoEn = autoEnabled->load() > 0.5f;
        const auto riderAmt = riderAmount->load();
        const auto riderEn = parameters.getRawParameterValue("rider_enabled")->load() > 0.5f;
        const auto lufsEn = parameters.getRawParameterValue("lufs_enabled")->load() > 0.5f;
        const auto gainDb = manualGain->load();
        
        // Check if any value changed or enough time passed (50ms)
        const bool valuesChanged = (targetDb != lastPushedTargetDb ||
                                    autoEn != lastPushedAutoEnabled ||
                                    std::abs(riderAmt - lastPushedRiderAmount) > 0.01f);
        const bool timeExpired = (currentTime - lastSatelliteControlPushTime) >= 50;
        
        if (valuesChanged || timeExpired)
        {
            auto* memData = sharedMemory.getData();
            if (memData != nullptr)
            {
                // Update master global state (readable by all satellites and for AI)
                memData->masterTargetDb.store(targetDb);
                memData->masterGainDb.store(gainDb);
                memData->masterAutoEnabled.store(autoEn);
                memData->masterRiderEnabled.store(riderEn);
                memData->masterRiderAmount.store(riderAmt);
                memData->masterLufsEnabled.store(lufsEn);
                if (auto* ceilingParam = parameters.getRawParameterValue("ceiling"))
                    memData->masterCeilingDb.store(ceilingParam->load());
                if (auto* lufsTarget = parameters.getRawParameterValue("lufs_target"))
                    memData->masterLufsTarget.store(lufsTarget->load());
                if (auto* genreParam = parameters.getRawParameterValue("genre"))
                    memData->masterGenre.store(static_cast<int>(genreParam->load()));
                if (auto* sourceParam = parameters.getRawParameterValue("source"))
                    memData->masterSource.store(static_cast<int>(sourceParam->load()));
                if (auto* sitParam = parameters.getRawParameterValue("situation"))
                    memData->masterSituation.store(static_cast<int>(sitParam->load()));
                
                // Update master metering data
                memData->masterRmsDb.store(preRmsDb.load());
                memData->masterPeakDb.store(prePeakDb.load());
                memData->masterCrestDb.store(preCrestDb.load());
                memData->masterPhaseCorrelation.store(prePhaseCorrelation.load());
                memData->masterShortTermLufs.store(shortTermLufs.load());
                memData->masterIntegratedLufs.store(integratedLufs.load());
                
                for (int i = 0; i < MAX_SATELLITES; ++i)
                {
                    auto& sat = memData->satellites[i];
                    // Push to active satellites (updated within 2 seconds)
                    if (currentTime - sat.lastUpdateTime.load() < 2000)
                    {
                        sat.control.targetDb.store(targetDb);
                        sat.control.autoEnabled.store(autoEn);
                        sat.control.riderAmount.store(riderAmt);
                        sat.control.controlledByMaster.store(true);
                        sat.control.controlUpdateTime.store(currentTime);
                    }
                }
                
                // Update cache
                lastPushedTargetDb = targetDb;
                lastPushedAutoEnabled = autoEn;
                lastPushedRiderAmount = riderAmt;
                lastSatelliteControlPushTime = currentTime;
            }
        }
    }

    // ============ POST-PROCESSING METERING ============
    float postSumSquares = 0.0f;
    float postPeak = 0.0f;
    float postSumLR = 0.0f, postSumL2 = 0.0f, postSumR2 = 0.0f;

    if (numChannels >= 2)
    {
        const auto* leftData = buffer.getReadPointer(0);
        const auto* rightData = buffer.getReadPointer(1);
        
        for (int i = 0; i < numSamples; ++i)
        {
            const auto L = leftData[i];
            const auto R = rightData[i];
            postSumLR += L * R;
            postSumL2 += L * L;
            postSumR2 += R * R;
            postSumSquares += L * L + R * R;
            postPeak = std::max(postPeak, std::max(std::abs(L), std::abs(R)));
        }
        
        const float denom = std::sqrt(postSumL2 * postSumR2);
        float correlation = (denom > 1.0e-10f) ? (postSumLR / denom) : 0.0f;
        postPhaseCorrelation.store(juce::jlimit(-1.0f, 1.0f, correlation));
    }
    else
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            const auto* data = buffer.getReadPointer(channel);
            for (int i = 0; i < numSamples; ++i)
            {
                const auto sample = data[i];
                postSumSquares += sample * sample;
                postPeak = std::max(postPeak, std::abs(sample));
            }
        }
        postPhaseCorrelation.store(0.0f);  // Mono has no phase relationship
    }

    const auto postRms = std::sqrt(postSumSquares / std::max(1, numChannels * numSamples));
    const auto postRmsDbVal = linearToDb(postRms);
    const auto postPeakDbVal = linearToDb(postPeak);
    postRmsDb.store(postRmsDbVal);
    postPeakDb.store(postPeakDbVal);
    postCrestDb.store(postPeakDbVal - postRmsDbVal);
    
    // ============ ADVANCED METERING ============
    
    // Stereo Width: 0% = mono, 100% = normal stereo, >100% = out of phase/wide
    // Based on correlation: 1.0 = 0% width (mono), 0.0 = 100% width, -1.0 = 200% (out of phase)
    float correlation = postPhaseCorrelation.load();
    float width = (1.0f - correlation) * 100.0f;
    stereoWidth.store(juce::jlimit(0.0f, 200.0f, width));
    
    // True Peak detection (simplified - true intersample peak requires oversampling)
    // For now we track the maximum peak seen
    float currentTruePeak = truePeak.load();
    if (postPeakDbVal > currentTruePeak)
        truePeak.store(postPeakDbVal);
    
    // Clip counting
    if (postPeak >= 1.0f)
        clipCount.store(clipCount.load() + 1);
    
    // Short-term LUFS approximation (simplified K-weighting approximation)
    // Real LUFS uses K-weighting filter, but RMS is a reasonable approximation
    float monoSum = 0.0f;
    if (numChannels >= 2)
    {
        const auto* leftData = buffer.getReadPointer(0);
        const auto* rightData = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
        {
            float mono = (leftData[i] + rightData[i]) * 0.5f;
            monoSum += mono * mono;
        }
    }
    else if (numChannels == 1)
    {
        const auto* data = buffer.getReadPointer(0);
        for (int i = 0; i < numSamples; ++i)
            monoSum += data[i] * data[i];
    }
    
    // Update LUFS accumulator
    lufsSum += monoSum;
    lufsSampleCount += numSamples;
    
    // Calculate short-term LUFS every ~3 seconds worth of samples
    if (lufsSampleCount >= static_cast<int64_t>(currentSampleRate * 3.0))
    {
        float meanSquare = static_cast<float>(lufsSum / static_cast<double>(lufsSampleCount));
        float lufsVal = -0.691f + 10.0f * std::log10(std::max(1e-10f, meanSquare));
        shortTermLufs.store(lufsVal);
        integratedLufs.store(lufsVal);  // Simplified - real integrated is lifetime average
        
        // Reset for next window
        lufsSum = 0.0;
        lufsSampleCount = 0;
    }
    
    // Frequency balance using FFT
    // Accumulate samples into FFT buffer and analyze when full
    if (numChannels >= 1 && numSamples > 0)
    {
        const auto* data = buffer.getReadPointer(0);
        
        for (int i = 0; i < numSamples; ++i)
        {
            fftInputBuffer[fftInputPos] = data[i];
            fftInputPos++;
            
            // When buffer is full, perform FFT analysis
            if (fftInputPos >= fftSize)
            {
                fftInputPos = 0;
                
                // Copy to FFT data and apply window
                std::copy(fftInputBuffer.begin(), fftInputBuffer.end(), fftData.begin());
                std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);  // Zero imaginary part
                fftWindow.multiplyWithWindowingTable(fftData.data(), fftSize);
                
                // Perform FFT
                fft.performFrequencyOnlyForwardTransform(fftData.data());
                
                // Calculate band energies
                // Frequency resolution = sampleRate / fftSize
                // For 44100Hz and 512 samples: ~86Hz per bin
                // Low: 0-300Hz (bins 0-3), Mid: 300-4000Hz (bins 4-46), High: 4000Hz+ (bins 47+)
                float freqPerBin = static_cast<float>(currentSampleRate) / static_cast<float>(fftSize);
                
                int lowEndBin = static_cast<int>(300.0f / freqPerBin);
                int midEndBin = static_cast<int>(4000.0f / freqPerBin);
                int nyquistBin = fftSize / 2;
                
                float low = 0.0f, mid = 0.0f, high = 0.0f;
                
                // Sum magnitude for each band
                for (int bin = 1; bin <= lowEndBin && bin < nyquistBin; ++bin)
                    low += fftData[bin];
                    
                for (int bin = lowEndBin + 1; bin <= midEndBin && bin < nyquistBin; ++bin)
                    mid += fftData[bin];
                    
                for (int bin = midEndBin + 1; bin < nyquistBin; ++bin)
                    high += fftData[bin];
                
                // Normalize to get proportional energy (0-1 each)
                float total = low + mid + high + 1e-10f;
                lowEnergy.store(low / total);
                midEnergy.store(mid / total);
                highEnergy.store(high / total);
            }
        }
    }
}

bool SimpleGainAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SimpleGainAudioProcessor::createEditor()
{
    return new SimpleGainAudioProcessorEditor(*this);
}

void SimpleGainAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SimpleGainAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorValueTreeState& SimpleGainAudioProcessor::getValueTreeState()
{
    return parameters;
}

void SimpleGainAudioProcessor::requestAiSuggestion(const juce::String& genre,
                                                   const juce::String& source,
                                                   const juce::String& situation)
{
    const auto analysis = getAnalysisSnapshot();
    
    // Get equipment settings
    juce::String mic, preamp, iface;
    {
        const juce::ScopedLock lock(settingsLock);
        mic = selectedMic;
        preamp = selectedPreamp;
        iface = selectedInterface;
    }

    juce::String prompt;
    prompt << "You are a gain staging assistant for audio production. "
              "Return strict JSON only in this format: {\"gain_db\":0,\"target_db\":-18,\"rider_amount\":0.5,\"notes\":\"...\",\"satellite_gains\":[]}. "
              "gain_db is the gain adjustment (-24 to +12 dB) to apply NOW to the master. "
              "satellite_gains is an optional array of {\"track\":1,\"gain_db\":0} for individual track adjustments. "
              "Use no extra text.\n";
    prompt << "The AI must only suggest parameters and notes, not control audio processing.\n";
    prompt << "Genre: " << genre << "\n";
    prompt << "Source: " << source << "\n";
    prompt << "Situation: " << situation << "\n";

    // Respect user's language selection: instruct AI to reply using the selected language
    auto langNames = Localization::getLanguageNames();
    auto langIdx = static_cast<int>(getLanguage());
    juce::String langName = (langIdx >= 0 && langIdx < langNames.size()) ? langNames[langIdx] : "English";
    prompt << "Language: " << langName << ". Respond in this language and output only the JSON as specified.\n";
    
    // Include satellite information
    juce::String satelliteSummary = getSatellitesSummary();
    if (!satelliteSummary.contains("No satellite"))
    {
        prompt << "\nSatellite Tracks (summary):\n" << satelliteSummary << "\n";
        prompt << "\nSatellite Tracks (json):\n" << getSatellitesJsonContext() << "\n";
    }
    
    // Include equipment context if specified
    if (mic.isNotEmpty() && mic != "Not Specified")
        prompt << "Microphone: " << mic << "\n";
    if (preamp.isNotEmpty() && preamp != "Not Specified")
        prompt << "Preamp: " << preamp << "\n";
    if (iface.isNotEmpty() && iface != "Not Specified")
        prompt << "Audio Interface: " << iface << "\n";
    
    prompt << "Analysis: RMS " << juce::String(analysis.rmsDb, 1) << " dB, Peak "
           << juce::String(analysis.peakDb, 1) << " dB, Crest " << juce::String(analysis.crestDb, 1) << " dB, "
           << "Short-Term LUFS " << juce::String(shortTermLufs.load(), 1) << ", Phase " << juce::String(analysis.phaseCorrelation, 2) << ".\n";
    
    // Include current plugin settings
    if (auto* gainParam = parameters.getRawParameterValue("gain"))
        prompt << "Current Gain: " << juce::String(gainParam->load(), 1) << " dB. ";
    if (auto* ceilingParam = parameters.getRawParameterValue("ceiling"))
        prompt << "Ceiling: " << juce::String(ceilingParam->load(), 1) << " dB.\n";
    
    prompt << "Calculate gain_db = target_db - current_rms_db (clamped to -24 to +12). "
              "Consider similar current songs for this context. Target gain staging with headroom and stable vocal levels.";

    if (aiClient != nullptr)
        aiClient->request(prompt, false);
}

void SimpleGainAudioProcessor::requestChatMessage(const juce::String& userMessage)
{
    const auto analysis = getAnalysisSnapshot();
    
    // Get equipment and context settings
    juce::String mic, preamp, iface;
    {
        const juce::ScopedLock lock(settingsLock);
        mic = selectedMic;
        preamp = selectedPreamp;
        iface = selectedInterface;
    }
    
    // Get genre and source context
    juce::String genre, source, situation;
    if (auto* p = parameters.getRawParameterValue("genre"))
    {
        const char* genres[] = {"Pop", "Rock", "Hip-Hop", "R&B", "Trap", "Reggaeton", "EDM", "Jazz", "Classical", "Podcast", "Lo-Fi", "Metal", "Country", "Other"};
        int idx = static_cast<int>(p->load());
        if (idx >= 0 && idx < 14) genre = genres[idx];
    }
    if (auto* p = parameters.getRawParameterValue("source"))
    {
        const char* sources[] = {"Lead Vocal", "Background Vocal", "Kick", "Snare", "Hi-Hat", "Full Drums", "Bass", "Electric Guitar", "Acoustic Guitar", "Keys/Piano", "Synth", "Strings", "Mix Bus", "Master"};
        int idx = static_cast<int>(p->load());
        if (idx >= 0 && idx < 14) source = sources[idx];
    }
    if (auto* p = parameters.getRawParameterValue("situation"))
    {
        const char* situations[] = {"Tracking", "Editing", "Mixing", "Mastering"};
        int idx = static_cast<int>(p->load());
        if (idx >= 0 && idx < 4) situation = situations[idx];
    }

    juce::String prompt;
    prompt << "You are ARES, an expert audio engineer, mixer, producer, and mastering engineer. "
              "You are a comprehensive mixing assistant that helps with ALL aspects of music production:\n"
              "- Gain staging and level optimization\n"
              "- Plugin recommendations (EQ, compression, reverb, delay, saturation, etc.)\n"
              "- Specific plugin settings and parameters\n"
              "- Signal chain order and routing\n"
              "- Mixing techniques for each instrument/element\n"
              "- Frequency balance and EQ curves\n"
              "- Compression ratios, attack, release for different sources\n"
              "- Reverb and delay settings\n"
              "- Stereo imaging and panning\n"
              "- Mastering chain suggestions\n"
              "- Reference track analysis\n"
              "- Genre-specific production techniques\n\n";
    
    // Context data for AI awareness (internal use only - don't recite back to user)
    prompt << "[INTERNAL CONTEXT - Use this data to inform your answers but do NOT list it back to the user]\n";

    // Respect user's language selection
    auto langNames = Localization::getLanguageNames();
    auto langIdx = static_cast<int>(getLanguage());
    juce::String langName = (langIdx >= 0 && langIdx < langNames.size()) ? langNames[langIdx] : "English";
    prompt << "Language: " << langName << ". Respond in this language when replying.\n";
    prompt << "Levels: RMS=" << juce::String(analysis.rmsDb, 1) << "dB, Peak=" << juce::String(analysis.peakDb, 1) << "dB, ";
    prompt << "Crest=" << juce::String(analysis.crestDb, 1) << "dB, Phase=" << juce::String(analysis.phaseCorrelation, 2) << ", ";
    prompt << "LUFS=" << juce::String(shortTermLufs.load(), 1) << "/" << juce::String(integratedLufs.load(), 1) << ". ";
    
    if (auto* gainParam = parameters.getRawParameterValue("gain"))
        prompt << "Gain=" << juce::String(gainParam->load(), 1) << "dB, ";
    if (auto* targetParam = parameters.getRawParameterValue("auto_target_db"))
        prompt << "Target=" << juce::String(targetParam->load(), 1) << "dB, ";
    if (auto* ceilingParam = parameters.getRawParameterValue("ceiling"))
        prompt << "Ceiling=" << juce::String(ceilingParam->load(), 1) << "dB. ";
    
    if (genre.isNotEmpty()) prompt << "Genre=" << genre << ", ";
    if (source.isNotEmpty()) prompt << "Source=" << source << ", ";
    if (situation.isNotEmpty()) prompt << "Stage=" << situation << ". ";
    if (mic.isNotEmpty() && mic != "Not Specified") prompt << "Mic=" << mic << ", ";
    if (preamp.isNotEmpty() && preamp != "Not Specified") prompt << "Pre=" << preamp << ", ";
    if (iface.isNotEmpty() && iface != "Not Specified") prompt << "Interface=" << iface << ". ";
    
    // Include satellite tracks information
    juce::String satelliteSummary = getSatellitesSummary();
    if (!satelliteSummary.contains("No satellite"))
        prompt << "Satellites: " << satelliteSummary << " ";
    
    prompt << "[END INTERNAL CONTEXT]\n\n";
    
    prompt << "User: " << userMessage << "\n\n";
    prompt << "Answer directly and concisely. Only mention the user's levels/settings if directly relevant to their question. "
              "Don't recite back all the context data - just use it to give informed answers.";

    if (aiClient != nullptr)
        aiClient->request(prompt, true);
}

void SimpleGainAudioProcessor::requestOllamaModelList()
{
    refreshModelList();
}

void SimpleGainAudioProcessor::refreshModelList()
{
    if (aiClient != nullptr)
        aiClient->requestModelList();
}

void SimpleGainAudioProcessor::applyAiRecommendation()
{
    juce::ScopedLock lock(aiLock);
    if (! aiHasRecommendation)
        return;

    // Apply the AI-recommended gain directly to the gain knob
    if (auto* gainParam = parameters.getParameter("gain"))
        gainParam->setValueNotifyingHost(gainParam->convertTo0to1(aiGainDb));

    // Also set target and rider if provided
    if (auto* targetParam = parameters.getParameter("auto_target_db"))
        targetParam->setValueNotifyingHost(targetParam->convertTo0to1(aiTargetDb));

    if (auto* riderParam = parameters.getParameter("rider_amount"))
        riderParam->setValueNotifyingHost(riderParam->convertTo0to1(aiRiderAmount));
}

void SimpleGainAudioProcessor::autoSetGainFromAnalysis()
{
    float suggestedTarget = -18.0f;
    
    // Get source and situation
    const auto sourceParam = parameters.getRawParameterValue("source");
    const auto situationParam = parameters.getRawParameterValue("situation");
    const auto genreParam = parameters.getRawParameterValue("genre");
    
    const int sourceIndex = static_cast<int>(sourceParam->load());
    const int situationIndex = static_cast<int>(situationParam->load());
    const int genreIndex = static_cast<int>(genreParam->load());
    
    // Base target by source type (professional gain staging standards)
    // Sources: Lead Vocal, BG Vocal, Kick, Snare, Hi-Hat, Full Drums, Bass, E.Guitar, A.Guitar, Keys, Synth, Strings, Mix Bus, Master
    const float sourceTargets[] = {
        -18.0f,  // Lead Vocal - standard vocal level
        -22.0f,  // Background Vocal - sits below lead
        -12.0f,  // Kick - punchy, prominent
        -14.0f,  // Snare - punchy
        -20.0f,  // Hi-Hat - controlled
        -14.0f,  // Full Drums - headroom for transients
        -14.0f,  // Bass - solid foundation
        -18.0f,  // Electric Guitar
        -20.0f,  // Acoustic Guitar
        -20.0f,  // Keys/Piano
        -16.0f,  // Synth - depends on type
        -22.0f,  // Strings - dynamic
        -18.0f,  // Mix Bus
        -14.0f   // Master - pre-limiter level
    };
    
    if (sourceIndex >= 0 && sourceIndex < 14)
        suggestedTarget = sourceTargets[sourceIndex];
    
    // Adjust for genre (Trap/Hip-Hop want louder kicks/bass, EDM wants more headroom)
    // Genres: Pop, Rock, Hip-Hop, R&B, Trap, Reggaeton, EDM, Jazz, Classical, Podcast, Lo-Fi, Metal, Country, Other
    bool isTrap = (genreIndex == 4);
    bool isHipHop = (genreIndex == 2);
    bool isReggaeton = (genreIndex == 5);
    bool isEDM = (genreIndex == 6);
    bool isMetal = (genreIndex == 11);
    
    // Trap/Hip-Hop: louder kick and 808 bass
    if ((isTrap || isHipHop || isReggaeton) && (sourceIndex == 2 || sourceIndex == 6)) // Kick or Bass
        suggestedTarget += 3.0f; // Louder for punch
    
    // Trap vocals often a bit lower for that vibe
    if (isTrap && sourceIndex == 0)
        suggestedTarget = -16.0f;
    
    // EDM needs more headroom
    if (isEDM)
        suggestedTarget -= 2.0f;
    
    // Metal: aggressive levels
    if (isMetal && (sourceIndex == 2 || sourceIndex == 3 || sourceIndex == 5)) // Kick, Snare, Full Drums
        suggestedTarget += 2.0f;
    
    // Adjust for situation/stage
    // Tracking: more headroom, Mixing: standard, Mastering: tighter
    if (situationIndex == 0) // Tracking
        suggestedTarget -= 3.0f;
    else if (situationIndex == 3) // Mastering
        suggestedTarget += 2.0f;
    
    // Clamp to valid range
    suggestedTarget = juce::jlimit(-36.0f, -6.0f, suggestedTarget);

    // If AI has a recommendation, prefer it
    {
        juce::ScopedLock lock(aiLock);
        if (aiHasRecommendation)
            suggestedTarget = aiTargetDb;
    }

    // Set the target - don't force-enable auto gain, it's a separate feature the user controls
    if (auto* targetParam = parameters.getParameter("auto_target_db"))
        targetParam->setValueNotifyingHost(targetParam->convertTo0to1(suggestedTarget));
}

SimpleGainAudioProcessor::AnalysisSnapshot SimpleGainAudioProcessor::getAnalysisSnapshot() const
{
    AnalysisSnapshot snapshot;
    
    // Pre-processing levels
    snapshot.preRmsDb = preRmsDb.load();
    snapshot.prePeakDb = prePeakDb.load();
    snapshot.preCrestDb = preCrestDb.load();
    snapshot.prePhaseCorrelation = prePhaseCorrelation.load();
    
    // Post-processing levels
    snapshot.postRmsDb = postRmsDb.load();
    snapshot.postPeakDb = postPeakDb.load();
    snapshot.postCrestDb = postCrestDb.load();
    snapshot.postPhaseCorrelation = postPhaseCorrelation.load();
    
    // Backwards compatibility - default to post
    snapshot.rmsDb = snapshot.postRmsDb;
    snapshot.peakDb = snapshot.postPeakDb;
    snapshot.crestDb = snapshot.postCrestDb;
    snapshot.phaseCorrelation = snapshot.postPhaseCorrelation;
    
    // Advanced metering
    snapshot.stereoWidth = stereoWidth.load();
    snapshot.dynamicRange = snapshot.postCrestDb;  // Same as crest factor
    snapshot.headroom = 0.0f - snapshot.postPeakDb;  // How far below 0 dBFS
    snapshot.shortTermLufs = shortTermLufs.load();
    snapshot.integratedLufs = integratedLufs.load();
    snapshot.truePeak = truePeak.load();
    snapshot.clipCount = clipCount.load();
    snapshot.lowEnergy = lowEnergy.load();
    snapshot.midEnergy = midEnergy.load();
    snapshot.highEnergy = highEnergy.load();
    snapshot.monoCompatible = snapshot.postPhaseCorrelation > 0.0f;  // Phase > 0 = mono safe
    
    return snapshot;
}

juce::String SimpleGainAudioProcessor::getAiNotes() const
{
    const juce::ScopedLock lock(aiLock);
    return aiNotes;
}

juce::String SimpleGainAudioProcessor::getAiStatus() const
{
    const juce::ScopedLock lock(aiLock);
    return aiStatus;
}

juce::StringArray SimpleGainAudioProcessor::getAvailableModels() const
{
    const juce::ScopedLock lock(aiLock);
    return availableModels;
}

int SimpleGainAudioProcessor::getAvailableModelsVersion() const
{
    return availableModelsVersion.load();
}

void SimpleGainAudioProcessor::applyAiResponseText(const juce::String& response)
{
    juce::ScopedLock lock(aiLock);
    aiNotes = response;

    const auto parsed = juce::JSON::parse(response);
    if (parsed.isObject())
    {
        auto* obj = parsed.getDynamicObject();
        const auto gainVar = obj->getProperty("gain_db");
        const auto targetVar = obj->getProperty("target_db");
        const auto riderVar = obj->getProperty("rider_amount");
        const auto notesVar = obj->getProperty("notes");
        const auto satGainsVar = obj->getProperty("satellite_gains");

        // Parse gain_db - this is the main value AI should recommend
        if (gainVar.isDouble() || gainVar.isInt())
            aiGainDb = juce::jlimit(-24.0f, 12.0f, static_cast<float>(gainVar));
        else
            aiGainDb = 0.0f; // Default if not provided

        if (targetVar.isDouble() || targetVar.isInt())
            aiTargetDb = static_cast<float>(targetVar);

        if (riderVar.isDouble() || riderVar.isInt())
            aiRiderAmount = juce::jlimit(0.0f, 1.0f, static_cast<float>(riderVar));

        if (notesVar.isString())
            aiNotes = notesVar.toString();
        
        // Parse and apply satellite gains if provided
        if (satGainsVar.isArray())
        {
            auto* satArray = satGainsVar.getArray();
            for (int i = 0; i < satArray->size(); ++i)
            {
                auto satObj = satArray->getReference(i);
                if (satObj.isObject())
                {
                    auto* satData = satObj.getDynamicObject();
                    int track = static_cast<int>(satData->getProperty("track")) - 1; // Convert to 0-indexed
                    float satGain = juce::jlimit(-24.0f, 12.0f, static_cast<float>(satData->getProperty("gain_db")));
                    
                    if (track >= 0 && track < MAX_SATELLITES)
                    {
                        SatelliteControl ctrl;
                        ctrl.gainDb = satGain;
                        ctrl.controlledByMaster = true;
                        setSatelliteControl(track, ctrl);
                    }
                }
            }
        }

        aiHasRecommendation = true;
        aiStatus = "AI suggestion ready (" + juce::String(aiGainDb, 1) + " dB)";
        return;
    }

    aiHasRecommendation = false;
    aiStatus = "AI response received (unstructured)";
}

void SimpleGainAudioProcessor::setAiNotesMessage(const juce::String& message)
{
    const juce::ScopedLock lock(aiLock);
    aiNotes = message;
}

void SimpleGainAudioProcessor::setAiStatusMessage(const juce::String& message)
{
    const juce::ScopedLock lock(aiLock);
    aiStatus = message;
}

juce::String SimpleGainAudioProcessor::getChatResponse() const
{
    const juce::ScopedLock lock(aiLock);
    return chatResponse;
}

int SimpleGainAudioProcessor::getChatResponseVersion() const
{
    return chatResponseVersion.load();
}

void SimpleGainAudioProcessor::setChatResponseMessage(const juce::String& message)
{
    const juce::ScopedLock lock(aiLock);
    chatResponse = message;
    chatResponseVersion.fetch_add(1);
}

void SimpleGainAudioProcessor::setAvailableModels(const juce::StringArray& models)
{
    const juce::ScopedLock lock(aiLock);
    availableModels = models;
    availableModelsVersion.fetch_add(1);
}

void SimpleGainAudioProcessor::setAiProvider(AiProvider provider)
{
    {
        const juce::ScopedLock lock(settingsLock);
        currentProvider = provider;
    }
    refreshModelList();
}

void SimpleGainAudioProcessor::setApiKey(const juce::String& key)
{
    const juce::ScopedLock lock(settingsLock);
    apiKey = key;
}

juce::String SimpleGainAudioProcessor::getApiKey() const
{
    const juce::ScopedLock lock(settingsLock);
    return apiKey;
}

void SimpleGainAudioProcessor::setSelectedModel(const juce::String& model)
{
    const juce::ScopedLock lock(settingsLock);
    selectedModel = model;
}

juce::String SimpleGainAudioProcessor::getSelectedModel() const
{
    const juce::ScopedLock lock(settingsLock);
    return selectedModel;
}

void SimpleGainAudioProcessor::saveSettings()
{
    auto settingsFile = getSettingsFile();
    settingsFile.getParentDirectory().createDirectory();
    
    auto xml = std::make_unique<juce::XmlElement>("AresSettings");
    
    {
        const juce::ScopedLock lock(settingsLock);
        xml->setAttribute("provider", static_cast<int>(currentProvider));
        xml->setAttribute("apiKey", apiKey);
        xml->setAttribute("model", selectedModel);
        xml->setAttribute("mic", selectedMic);
        xml->setAttribute("preamp", selectedPreamp);
        xml->setAttribute("interface", selectedInterface);
        xml->setAttribute("language", static_cast<int>(currentLanguage));
        xml->setAttribute("theme", currentThemeIndex);
    }
    
    xml->writeTo(settingsFile);
    
    setAiStatusMessage("Settings saved!");
}

std::vector<float> SimpleGainAudioProcessor::getSpectrumData() const
{
    std::vector<float> magnitudes(fftSize / 2);
    
    // performFrequencyOnlyForwardTransform outputs magnitude values directly
    // in the first fftSize/2 bins - NOT interleaved complex data
    for (int i = 0; i < fftSize / 2; ++i)
    {
        float mag = fftData[i];
        
        // Convert magnitude to dB (normalize by FFT size for proper scaling)
        float normalizedMag = mag / static_cast<float>(fftSize);
        float db = 20.0f * std::log10(std::max(normalizedMag, 1e-10f));
        magnitudes[i] = juce::jlimit(-100.0f, 0.0f, db);
    }
    
    return magnitudes;
}

void SimpleGainAudioProcessor::loadSettings()
{
    auto settingsFile = getSettingsFile();
    
    if (!settingsFile.existsAsFile())
        return;
    
    auto xml = juce::XmlDocument::parse(settingsFile);
    
    if (xml == nullptr || !xml->hasTagName("AresSettings"))
        return;
    
    {
        const juce::ScopedLock lock(settingsLock);
        currentProvider = static_cast<AiProvider>(xml->getIntAttribute("provider", 0));
        apiKey = xml->getStringAttribute("apiKey", "");
        selectedModel = xml->getStringAttribute("model", "llama3");
        selectedMic = xml->getStringAttribute("mic", "");
        selectedPreamp = xml->getStringAttribute("preamp", "");
        selectedInterface = xml->getStringAttribute("interface", "");
        currentLanguage = static_cast<Language>(xml->getIntAttribute("language", 0));
        currentThemeIndex = xml->getIntAttribute("theme", 1);  // Default to Modern Dark
        Localization::getInstance().setLanguage(currentLanguage);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleGainAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Gain in dB: -24 to +12 (matches satellite for consistency)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gain",
        "Gain",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "auto_enabled",
        "Auto Enabled",
        true));

    // Target dB: wider range for more flexibility
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "auto_target_db",
        "Auto Target dB",
        juce::NormalisableRange<float>(-48.0f, 0.0f, 0.1f),
        -18.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "rider_enabled",
        "Vocal Rider Enabled",
        false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "rider_amount",
        "Vocal Rider Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "genre",
        "Genre",
        juce::StringArray { "Pop", "Rock", "Hip-Hop", "R&B", "Trap", "Reggaeton", "EDM", "Jazz", "Classical", "Podcast", "Lo-Fi", "Metal", "Country", "Other" },
        0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "source",
        "Source",
        juce::StringArray { "Lead Vocal", "Background Vocal", "Kick", "Snare", "Hi-Hat", "Full Drums", "Bass", "Electric Guitar", "Acoustic Guitar", "Keys/Piano", "Synth", "Strings", "Mix Bus", "Master" },
        0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "situation",
        "Situation",
        juce::StringArray { "Tracking", "Editing", "Mixing", "Mastering" },
        2));  // Default to Mixing

    // Ceiling/Max Peak - hard limiter ceiling
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ceiling",
        "Max Peak",
        juce::NormalisableRange<float>(-24.0f, 0.0f, 0.1f),
        0.0f));  // Default 0 dB (no limiting)
    
    // LUFS Target for loudness normalization (separate from RMS target)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lufs_target",
        "LUFS Target",
        juce::NormalisableRange<float>(-24.0f, -6.0f, 0.1f),
        -14.0f));  // Default -14 LUFS (Spotify/YouTube)
    
    // Enable LUFS auto-gain (adjusts gain to match LUFS target)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "lufs_enabled",
        "LUFS Mode Enabled",
        false));

    return { params.begin(), params.end() };
}

void SimpleGainAudioProcessor::setThemeIndex(int index)
{
    {
        const juce::ScopedLock lock(settingsLock);
        currentThemeIndex = index;
    }
    
    // Sync to shared memory for satellites
    if (sharedMemoryConnected)
    {
        auto* data = sharedMemory.getData();
        if (data != nullptr)
        {
            data->masterThemeIndex.store(index);
        }
    }
}

void SimpleGainAudioProcessor::setKnobStyle(int style)
{
    {
        const juce::ScopedLock lock(settingsLock);
        currentKnobStyle = style;
    }
    
    // Sync to shared memory for satellites
    if (sharedMemoryConnected)
    {
        auto* data = sharedMemory.getData();
        if (data != nullptr)
        {
            data->masterKnobStyle.store(style);
        }
    }
}

void SimpleGainAudioProcessor::initializeSharedMemory()
{
    sharedMemoryConnected = sharedMemory.openOrCreate();
    if (sharedMemoryConnected)
    {
        auto* data = sharedMemory.getData();
        if (data != nullptr)
        {
            // Clear all stale satellite data from previous sessions
            data->clearAllSatellites();
            data->masterInitTime.store(juce::Time::currentTimeMillis());
            DBG("AR3S Master: Shared memory initialized and cleared");
        }
    }
    else
    {
        DBG("AR3S Master: Failed to initialize shared memory");
    }
}

int SimpleGainAudioProcessor::getActiveSatelliteCount() const
{
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr)
        return 0;
    
    int count = 0;
    auto currentTime = juce::Time::currentTimeMillis();
    
    for (int i = 0; i < MAX_SATELLITES; ++i)
    {
        // Check BOTH active flag AND recent update time
        bool isActive = memData->satellites[i].active.load();
        auto lastUpdate = memData->satellites[i].lastUpdateTime.load();
        // Consider satellite active if marked active AND updated within last 3 seconds
        if (isActive && (currentTime - lastUpdate < 3000))
            ++count;
    }
    
    return count;
}

SimpleGainAudioProcessor::SatelliteInfo SimpleGainAudioProcessor::getSatelliteInfo(int index) const
{
    SatelliteInfo info;
    
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr || index < 0 || index >= MAX_SATELLITES)
        return info;
    
    const auto& sat = memData->satellites[index];
    auto currentTime = juce::Time::currentTimeMillis();
    auto lastUpdate = sat.lastUpdateTime.load();
    bool isActive = sat.active.load();
    
    // Check BOTH active flag AND recent update time (within 3 seconds)
    info.active = isActive && (currentTime - lastUpdate < 3000);
    
    if (info.active)
    {
        info.channelName = juce::String(sat.channelName);
        info.rmsDb = sat.rmsDb.load();
        info.peakDb = sat.peakDb.load();
        info.crestDb = sat.crestDb.load();
        info.phaseCorrelation = sat.phaseCorrelation.load();
        info.currentGain = sat.currentGain.load();
        info.sourceType = sat.sourceType.load();
        info.lastUpdateTime = lastUpdate;
        
        // Read control values
        info.gainDb = sat.control.gainDb.load();
        info.targetDb = sat.control.targetDb.load();
        info.ceilingDb = sat.control.ceilingDb.load();
        info.autoEnabled = sat.control.autoEnabled.load();
        info.riderAmount = sat.control.riderAmount.load();
        info.controlledByMaster = sat.control.controlledByMaster.load();
    }
    
    return info;
}

void SimpleGainAudioProcessor::setSatelliteControl(int index, const SatelliteControl& control)
{
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr || index < 0 || index >= MAX_SATELLITES)
        return;

    auto& sat = memData->satellites[index];

    // Set all values, enable per-satellite override
    sat.control.gainDb.store(control.gainDb);
    sat.control.targetDb.store(control.targetDb);
    sat.control.ceilingDb.store(control.ceilingDb);
    sat.control.autoEnabled.store(control.autoEnabled);
    sat.control.riderAmount.store(control.riderAmount);
    sat.control.perSatelliteOverride.store(true);
    sat.control.controlledByMaster.store(false); // Only one mode active
    sat.control.controlUpdateTime.store(juce::Time::currentTimeMillis());
}

void SimpleGainAudioProcessor::releaseSatelliteControl(int index)
{
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr || index < 0 || index >= MAX_SATELLITES)
        return;
    
    memData->satellites[index].control.perSatelliteOverride.store(false);
    memData->satellites[index].control.controlledByMaster.store(false);
}

juce::String SimpleGainAudioProcessor::getSatellitesSummary() const
{
    // Source type names for better AI context
    const char* sourceNames[] = { "Vocals", "Background Vocals", "Kick", "Snare", "Hi-Hat", 
                                  "Drums", "Bass", "Electric Guitar", "Acoustic Guitar", 
                                  "Keys/Piano", "Synth", "Strings", "Other" };
    const int numSourceNames = 13;
    
    juce::String summary;
    int activeCount = 0;
    
    for (int i = 0; i < MAX_SATELLITES; ++i)
    {
        auto info = getSatelliteInfo(i);
        if (info.active)
        {
            if (activeCount > 0) summary << "\n";
            
            // Track name
            juce::String trackName = info.channelName.isEmpty() ? ("Track " + juce::String(i + 1)) : info.channelName;
            summary << "- " << trackName;
            
            // Source type in parentheses
            if (info.sourceType >= 0 && info.sourceType < numSourceNames)
                summary << " (" << sourceNames[info.sourceType] << ")";
            
            summary << ": ";
            
            // Levels
            summary << "RMS " << juce::String(info.rmsDb, 1) << "dB, ";
            summary << "Peak " << juce::String(info.peakDb, 1) << "dB, ";
            summary << "Crest " << juce::String(info.crestDb, 1) << "dB";
            
            // Current gain adjustment
            float gainDb = 20.0f * std::log10(std::max(0.0001f, info.currentGain));
            if (std::abs(gainDb) > 0.1f)
                summary << ", Gain " << (gainDb >= 0 ? "+" : "") << juce::String(gainDb, 1) << "dB";
            
            // Phase correlation warning
            if (info.phaseCorrelation < 0.3f)
                summary << " [Phase issues!]";
            
            if (info.controlledByMaster)
                summary << " [Master controlled]";
                
            activeCount++;
        }
    }
    
    if (activeCount == 0)
        summary = "No satellite tracks connected. User should load AR3S Satellite plugins on individual tracks for multi-track gain staging.";
    else
        summary << "\n(Total: " << activeCount << " active satellite tracks)";
    
    return summary;
}

void SimpleGainAudioProcessor::setAllSatellitesGain(float gainDb)
{
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr)
        return;
    
    for (int i = 0; i < MAX_SATELLITES; ++i)
    {
        auto info = getSatelliteInfo(i);
        if (info.active)
        {
            auto& sat = memData->satellites[i];
            sat.control.gainDb.store(gainDb);
            sat.control.controlledByMaster.store(true);
            sat.control.controlUpdateTime.store(juce::Time::currentTimeMillis());
        }
    }
}

void SimpleGainAudioProcessor::setAllSatellitesTarget(float targetDb)
{
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr)
        return;
    
    for (int i = 0; i < MAX_SATELLITES; ++i)
    {
        auto info = getSatelliteInfo(i);
        if (info.active)
        {
            auto& sat = memData->satellites[i];
            sat.control.targetDb.store(targetDb);
            sat.control.controlledByMaster.store(true);
            sat.control.controlUpdateTime.store(juce::Time::currentTimeMillis());
        }
    }
}

void SimpleGainAudioProcessor::setAllSatellitesCeiling(float ceilingDb)
{
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr)
        return;
    
    for (int i = 0; i < MAX_SATELLITES; ++i)
    {
        auto info = getSatelliteInfo(i);
        if (info.active)
        {
            auto& sat = memData->satellites[i];
            sat.control.ceilingDb.store(ceilingDb);
            sat.control.controlledByMaster.store(true);
            sat.control.controlUpdateTime.store(juce::Time::currentTimeMillis());
        }
    }
}

void SimpleGainAudioProcessor::autoGainAllSatellites(float targetDb)
{
    auto* memData = sharedMemory.getData();
    if (!sharedMemoryConnected || memData == nullptr)
        return;
    
    for (int i = 0; i < MAX_SATELLITES; ++i)
    {
        auto info = getSatelliteInfo(i);
        if (info.active)
        {
            // Calculate gain needed to reach target
            float neededGain = targetDb - info.rmsDb;
            neededGain = juce::jlimit(-24.0f, 12.0f, neededGain);
            
            auto& sat = memData->satellites[i];
            sat.control.gainDb.store(neededGain);
            sat.control.targetDb.store(targetDb);
            sat.control.autoEnabled.store(true);
            sat.control.controlledByMaster.store(true);
            sat.control.controlUpdateTime.store(juce::Time::currentTimeMillis());
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleGainAudioProcessor();
}
