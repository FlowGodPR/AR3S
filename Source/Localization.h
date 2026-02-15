#pragma once

#include <juce_core/juce_core.h>
#include <map>

// Supported languages
enum class Language
{
    English = 0,
    Spanish,
    French,
    German,
    Portuguese,
    Italian,
    Japanese,
    Korean,
    Chinese,
    Russian,
    COUNT
};

// String keys for all UI text
enum class StringKey
{
    // Tab names
    TabMain,
    TabSettings,
    TabAssistant,
    
    // Main controls
    Gain,
    Target,
    AutoGain,
    VocalRider,
    Rider,
    Phase,
    GainAdjust,
    
    // Context
    Genre,
    Source,
    Stage,
    
    // Buttons
    AutoSet,
    AskAI,
    ApplyAI,
    SaveSettings,
    RefreshModels,
    Send,
    
    // Settings labels
    AIProvider,
    Model,
    APIKey,
    Theme,
    KnobStyle,
    AccentColor,
    Background,
    Language,
    
    // Equipment
    Microphone,
    Preamp,
    Interface,
    
    // Meters
    SignalAnalysis,
    Headroom,
    Dynamics,
    Clips,
    Peak,
    Width,
    MonoCompat,
    Correlation,
    RMS,
    FreqBalance,
    Low,
    Mid,
    High,
    QuickCheck,
    Satellites,
    
    // Quick check messages
    NoSignal,
    SignalQuiet,
    Clipping,
    PeaksHot,
    LevelsOK,
    OverCompressed,
    VeryDynamic,
    GoodDynamics,
    PhaseProblems,
    CheckMono,
    MonoOK,
    VeryWide,
    MonoSignal,
    
    // Platform targets
    PlatformTarget,
    Custom,
    
    // Misc
    NotSpecified,
    LoadSatellite,
    
    COUNT
};

class Localization
{
public:
    static Localization& getInstance()
    {
        static Localization instance;
        return instance;
    }
    
    void setLanguage(Language lang)
    {
        currentLanguage = lang;
    }
    
    Language getLanguage() const { return currentLanguage; }
    
    juce::String get(StringKey key) const
    {
        auto langIt = strings.find(currentLanguage);
        if (langIt != strings.end())
        {
            auto strIt = langIt->second.find(key);
            if (strIt != langIt->second.end())
                return strIt->second;
        }
        // Fallback to English
        auto engIt = strings.find(Language::English);
        if (engIt != strings.end())
        {
            auto strIt = engIt->second.find(key);
            if (strIt != engIt->second.end())
                return strIt->second;
        }
        return "???";
    }
    
    static juce::StringArray getLanguageNames()
    {
        return { "English", "Espanol", "Francais", "Deutsch", 
                 "Portugues", "Italiano", "Japanese", "Korean", 
                 "Chinese", "Russian" };
    }
    
private:
    Localization()
    {
        initializeStrings();
    }
    
    void initializeStrings()
    {
        // ============ ENGLISH ============
        auto& en = strings[Language::English];
        en[StringKey::TabMain] = "Main";
        en[StringKey::TabSettings] = "Settings";
        en[StringKey::TabAssistant] = "Assistant";
        en[StringKey::Gain] = "GAIN";
        en[StringKey::Target] = "TARGET";
        en[StringKey::AutoGain] = "Auto Gain";
        en[StringKey::VocalRider] = "Vocal Rider";
        en[StringKey::Rider] = "Rider";
        en[StringKey::Phase] = "PHASE";
        en[StringKey::GainAdjust] = "GAIN ADJUST";
        en[StringKey::Genre] = "Genre";
        en[StringKey::Source] = "Source";
        en[StringKey::Stage] = "Stage";
        en[StringKey::AutoSet] = "Auto-Set";
        en[StringKey::AskAI] = "Ask AI";
        en[StringKey::ApplyAI] = "Apply AI";
        en[StringKey::SaveSettings] = "Save Settings";
        en[StringKey::RefreshModels] = "Refresh Models";
        en[StringKey::Send] = "Send";
        en[StringKey::AIProvider] = "AI Provider";
        en[StringKey::Model] = "Model";
        en[StringKey::APIKey] = "API Key";
        en[StringKey::Theme] = "Theme";
        en[StringKey::KnobStyle] = "Knob Style";
        en[StringKey::AccentColor] = "Accent Color";
        en[StringKey::Background] = "Background";
        en[StringKey::Language] = "Language";
        en[StringKey::Microphone] = "Microphone";
        en[StringKey::Preamp] = "Preamp";
        en[StringKey::Interface] = "Interface";
        en[StringKey::SignalAnalysis] = "SIGNAL ANALYSIS";
        en[StringKey::Headroom] = "Headroom:";
        en[StringKey::Dynamics] = "Dynamics:";
        en[StringKey::Clips] = "Clips:";
        en[StringKey::Peak] = "Peak:";
        en[StringKey::Width] = "Width:";
        en[StringKey::MonoCompat] = "Mono Compat:";
        en[StringKey::Correlation] = "Correlation:";
        en[StringKey::RMS] = "RMS:";
        en[StringKey::FreqBalance] = "FREQ BALANCE";
        en[StringKey::Low] = "LOW";
        en[StringKey::Mid] = "MID";
        en[StringKey::High] = "HIGH";
        en[StringKey::QuickCheck] = "QUICK CHECK";
        en[StringKey::Satellites] = "SATELLITES";
        en[StringKey::NoSignal] = "No signal";
        en[StringKey::SignalQuiet] = "Signal very quiet";
        en[StringKey::Clipping] = "CLIPPING!";
        en[StringKey::PeaksHot] = "Peaks hot";
        en[StringKey::LevelsOK] = "Levels OK";
        en[StringKey::OverCompressed] = "Over-compressed";
        en[StringKey::VeryDynamic] = "Very dynamic";
        en[StringKey::GoodDynamics] = "Good dynamics";
        en[StringKey::PhaseProblems] = "Phase problems!";
        en[StringKey::CheckMono] = "Check mono";
        en[StringKey::MonoOK] = "Mono OK";
        en[StringKey::VeryWide] = "Very wide";
        en[StringKey::MonoSignal] = "Mono signal";
        en[StringKey::PlatformTarget] = "Target";
        en[StringKey::Custom] = "Custom";
        en[StringKey::NotSpecified] = "Not Specified";
        en[StringKey::LoadSatellite] = "Load AR3S Satellite on tracks";
        
        // ============ SPANISH ============
        auto& es = strings[Language::Spanish];
        es[StringKey::TabMain] = "Principal";
        es[StringKey::TabSettings] = "Ajustes";
        es[StringKey::TabAssistant] = "Asistente";
        es[StringKey::Gain] = "GANANCIA";
        es[StringKey::Target] = "OBJETIVO";
        es[StringKey::AutoGain] = "Ganancia Auto";
        es[StringKey::VocalRider] = "Rider Vocal";
        es[StringKey::Rider] = "Rider";
        es[StringKey::Phase] = "FASE";
        es[StringKey::GainAdjust] = "AJUSTE GANANCIA";
        es[StringKey::Genre] = "Genero";
        es[StringKey::Source] = "Fuente";
        es[StringKey::Stage] = "Etapa";
        es[StringKey::AutoSet] = "Auto-Ajuste";
        es[StringKey::AskAI] = "Preguntar IA";
        es[StringKey::ApplyAI] = "Aplicar IA";
        es[StringKey::SaveSettings] = "Guardar";
        es[StringKey::RefreshModels] = "Actualizar Modelos";
        es[StringKey::Send] = "Enviar";
        es[StringKey::AIProvider] = "Proveedor IA";
        es[StringKey::Model] = "Modelo";
        es[StringKey::APIKey] = "Clave API";
        es[StringKey::Theme] = "Tema";
        es[StringKey::KnobStyle] = "Estilo Perilla";
        es[StringKey::AccentColor] = "Color Acento";
        es[StringKey::Background] = "Fondo";
        es[StringKey::Language] = "Idioma";
        es[StringKey::Microphone] = "Microfono";
        es[StringKey::Preamp] = "Preamplificador";
        es[StringKey::Interface] = "Interfaz";
        es[StringKey::SignalAnalysis] = "ANALISIS DE SENAL";
        es[StringKey::Headroom] = "Margen:";
        es[StringKey::Dynamics] = "Dinamica:";
        es[StringKey::Clips] = "Clips:";
        es[StringKey::Peak] = "Pico:";
        es[StringKey::Width] = "Ancho:";
        es[StringKey::MonoCompat] = "Compat Mono:";
        es[StringKey::Correlation] = "Correlacion:";
        es[StringKey::RMS] = "RMS:";
        es[StringKey::FreqBalance] = "BALANCE FREQ";
        es[StringKey::Low] = "BAJO";
        es[StringKey::Mid] = "MEDIO";
        es[StringKey::High] = "ALTO";
        es[StringKey::QuickCheck] = "REVISION RAPIDA";
        es[StringKey::Satellites] = "SATELITES";
        es[StringKey::NoSignal] = "Sin senal";
        es[StringKey::SignalQuiet] = "Senal muy baja";
        es[StringKey::Clipping] = "CLIPEO!";
        es[StringKey::PeaksHot] = "Picos altos";
        es[StringKey::LevelsOK] = "Niveles OK";
        es[StringKey::OverCompressed] = "Sobre-comprimido";
        es[StringKey::VeryDynamic] = "Muy dinamico";
        es[StringKey::GoodDynamics] = "Buena dinamica";
        es[StringKey::PhaseProblems] = "Problemas de fase!";
        es[StringKey::CheckMono] = "Revisar mono";
        es[StringKey::MonoOK] = "Mono OK";
        es[StringKey::VeryWide] = "Muy ancho";
        es[StringKey::MonoSignal] = "Senal mono";
        es[StringKey::PlatformTarget] = "Objetivo";
        es[StringKey::Custom] = "Personalizado";
        es[StringKey::NotSpecified] = "No especificado";
        es[StringKey::LoadSatellite] = "Cargar AR3S Satellite en pistas";
        
        // ============ FRENCH ============
        auto& fr = strings[Language::French];
        fr[StringKey::TabMain] = "Principal";
        fr[StringKey::TabSettings] = "Parametres";
        fr[StringKey::TabAssistant] = "Assistant";
        fr[StringKey::Gain] = "GAIN";
        fr[StringKey::Target] = "CIBLE";
        fr[StringKey::AutoGain] = "Gain Auto";
        fr[StringKey::VocalRider] = "Rider Vocal";
        fr[StringKey::Rider] = "Rider";
        fr[StringKey::Phase] = "PHASE";
        fr[StringKey::GainAdjust] = "AJUST. GAIN";
        fr[StringKey::Genre] = "Genre";
        fr[StringKey::Source] = "Source";
        fr[StringKey::Stage] = "Etape";
        fr[StringKey::AutoSet] = "Auto-Regler";
        fr[StringKey::AskAI] = "Demander IA";
        fr[StringKey::ApplyAI] = "Appliquer IA";
        fr[StringKey::SaveSettings] = "Sauvegarder";
        fr[StringKey::RefreshModels] = "Rafraichir Modeles";
        fr[StringKey::Send] = "Envoyer";
        fr[StringKey::AIProvider] = "Fournisseur IA";
        fr[StringKey::Model] = "Modele";
        fr[StringKey::APIKey] = "Cle API";
        fr[StringKey::Theme] = "Theme";
        fr[StringKey::KnobStyle] = "Style Bouton";
        fr[StringKey::AccentColor] = "Couleur Accent";
        fr[StringKey::Background] = "Arriere-plan";
        fr[StringKey::Language] = "Langue";
        fr[StringKey::Microphone] = "Microphone";
        fr[StringKey::Preamp] = "Preampli";
        fr[StringKey::Interface] = "Interface";
        fr[StringKey::SignalAnalysis] = "ANALYSE SIGNAL";
        fr[StringKey::Headroom] = "Marge:";
        fr[StringKey::Dynamics] = "Dynamique:";
        fr[StringKey::Clips] = "Clips:";
        fr[StringKey::Peak] = "Crete:";
        fr[StringKey::Width] = "Largeur:";
        fr[StringKey::MonoCompat] = "Compat Mono:";
        fr[StringKey::Correlation] = "Correlation:";
        fr[StringKey::RMS] = "RMS:";
        fr[StringKey::FreqBalance] = "BALANCE FREQ";
        fr[StringKey::Low] = "BAS";
        fr[StringKey::Mid] = "MILIEU";
        fr[StringKey::High] = "HAUT";
        fr[StringKey::QuickCheck] = "VERIF RAPIDE";
        fr[StringKey::Satellites] = "SATELLITES";
        fr[StringKey::NoSignal] = "Pas de signal";
        fr[StringKey::SignalQuiet] = "Signal tres faible";
        fr[StringKey::Clipping] = "SATURATION!";
        fr[StringKey::PeaksHot] = "Cretes elevees";
        fr[StringKey::LevelsOK] = "Niveaux OK";
        fr[StringKey::OverCompressed] = "Sur-compresse";
        fr[StringKey::VeryDynamic] = "Tres dynamique";
        fr[StringKey::GoodDynamics] = "Bonne dynamique";
        fr[StringKey::PhaseProblems] = "Problemes phase!";
        fr[StringKey::CheckMono] = "Verifier mono";
        fr[StringKey::MonoOK] = "Mono OK";
        fr[StringKey::VeryWide] = "Tres large";
        fr[StringKey::MonoSignal] = "Signal mono";
        fr[StringKey::PlatformTarget] = "Cible";
        fr[StringKey::Custom] = "Personnalise";
        fr[StringKey::NotSpecified] = "Non specifie";
        fr[StringKey::LoadSatellite] = "Charger AR3S Satellite sur pistes";
        
        // ============ GERMAN ============
        auto& de = strings[Language::German];
        de[StringKey::TabMain] = "Haupt";
        de[StringKey::TabSettings] = "Einstellungen";
        de[StringKey::TabAssistant] = "Assistent";
        de[StringKey::Gain] = "PEGEL";
        de[StringKey::Target] = "ZIEL";
        de[StringKey::AutoGain] = "Auto-Pegel";
        de[StringKey::VocalRider] = "Vocal Rider";
        de[StringKey::Rider] = "Rider";
        de[StringKey::Phase] = "PHASE";
        de[StringKey::GainAdjust] = "PEGEL ANPASSEN";
        de[StringKey::Genre] = "Genre";
        de[StringKey::Source] = "Quelle";
        de[StringKey::Stage] = "Phase";
        de[StringKey::AutoSet] = "Auto-Setzen";
        de[StringKey::AskAI] = "KI fragen";
        de[StringKey::ApplyAI] = "KI anwenden";
        de[StringKey::SaveSettings] = "Speichern";
        de[StringKey::RefreshModels] = "Modelle aktualisieren";
        de[StringKey::Send] = "Senden";
        de[StringKey::AIProvider] = "KI-Anbieter";
        de[StringKey::Model] = "Modell";
        de[StringKey::APIKey] = "API-Schlussel";
        de[StringKey::Theme] = "Design";
        de[StringKey::KnobStyle] = "Regler-Stil";
        de[StringKey::AccentColor] = "Akzentfarbe";
        de[StringKey::Background] = "Hintergrund";
        de[StringKey::Language] = "Sprache";
        de[StringKey::Microphone] = "Mikrofon";
        de[StringKey::Preamp] = "Vorverstarker";
        de[StringKey::Interface] = "Interface";
        de[StringKey::SignalAnalysis] = "SIGNALANALYSE";
        de[StringKey::Headroom] = "Reserve:";
        de[StringKey::Dynamics] = "Dynamik:";
        de[StringKey::Clips] = "Clips:";
        de[StringKey::Peak] = "Spitze:";
        de[StringKey::Width] = "Breite:";
        de[StringKey::MonoCompat] = "Mono Komp:";
        de[StringKey::Correlation] = "Korrelation:";
        de[StringKey::RMS] = "RMS:";
        de[StringKey::FreqBalance] = "FREQ BALANCE";
        de[StringKey::Low] = "TIEF";
        de[StringKey::Mid] = "MITTE";
        de[StringKey::High] = "HOCH";
        de[StringKey::QuickCheck] = "SCHNELLCHECK";
        de[StringKey::Satellites] = "SATELLITEN";
        de[StringKey::NoSignal] = "Kein Signal";
        de[StringKey::SignalQuiet] = "Signal sehr leise";
        de[StringKey::Clipping] = "UBERSTEUERT!";
        de[StringKey::PeaksHot] = "Spitzen hoch";
        de[StringKey::LevelsOK] = "Pegel OK";
        de[StringKey::OverCompressed] = "Uberkomprimiert";
        de[StringKey::VeryDynamic] = "Sehr dynamisch";
        de[StringKey::GoodDynamics] = "Gute Dynamik";
        de[StringKey::PhaseProblems] = "Phasenprobleme!";
        de[StringKey::CheckMono] = "Mono prufen";
        de[StringKey::MonoOK] = "Mono OK";
        de[StringKey::VeryWide] = "Sehr breit";
        de[StringKey::MonoSignal] = "Mono Signal";
        de[StringKey::PlatformTarget] = "Ziel";
        de[StringKey::Custom] = "Benutzerdefiniert";
        de[StringKey::NotSpecified] = "Nicht angegeben";
        de[StringKey::LoadSatellite] = "AR3S Satellite auf Spuren laden";
        
        // ============ PORTUGUESE ============
        auto& pt = strings[Language::Portuguese];
        pt[StringKey::TabMain] = "Principal";
        pt[StringKey::TabSettings] = "Configuracoes";
        pt[StringKey::TabAssistant] = "Assistente";
        pt[StringKey::Gain] = "GANHO";
        pt[StringKey::Target] = "ALVO";
        pt[StringKey::AutoGain] = "Ganho Auto";
        pt[StringKey::VocalRider] = "Rider Vocal";
        pt[StringKey::Rider] = "Rider";
        pt[StringKey::Phase] = "FASE";
        pt[StringKey::GainAdjust] = "AJUSTE GANHO";
        pt[StringKey::Genre] = "Genero";
        pt[StringKey::Source] = "Fonte";
        pt[StringKey::Stage] = "Etapa";
        pt[StringKey::AutoSet] = "Auto-Definir";
        pt[StringKey::AskAI] = "Perguntar IA";
        pt[StringKey::ApplyAI] = "Aplicar IA";
        pt[StringKey::SaveSettings] = "Salvar";
        pt[StringKey::RefreshModels] = "Atualizar Modelos";
        pt[StringKey::Send] = "Enviar";
        pt[StringKey::AIProvider] = "Provedor IA";
        pt[StringKey::Model] = "Modelo";
        pt[StringKey::APIKey] = "Chave API";
        pt[StringKey::Theme] = "Tema";
        pt[StringKey::KnobStyle] = "Estilo Botao";
        pt[StringKey::AccentColor] = "Cor Destaque";
        pt[StringKey::Background] = "Fundo";
        pt[StringKey::Language] = "Idioma";
        pt[StringKey::Microphone] = "Microfone";
        pt[StringKey::Preamp] = "Pre-amplificador";
        pt[StringKey::Interface] = "Interface";
        pt[StringKey::SignalAnalysis] = "ANALISE SINAL";
        pt[StringKey::Headroom] = "Margem:";
        pt[StringKey::Dynamics] = "Dinamica:";
        pt[StringKey::Clips] = "Clips:";
        pt[StringKey::Peak] = "Pico:";
        pt[StringKey::Width] = "Largura:";
        pt[StringKey::MonoCompat] = "Compat Mono:";
        pt[StringKey::Correlation] = "Correlacao:";
        pt[StringKey::RMS] = "RMS:";
        pt[StringKey::FreqBalance] = "BALANCE FREQ";
        pt[StringKey::Low] = "BAIXO";
        pt[StringKey::Mid] = "MEDIO";
        pt[StringKey::High] = "ALTO";
        pt[StringKey::QuickCheck] = "VERIF RAPIDA";
        pt[StringKey::Satellites] = "SATELITES";
        pt[StringKey::NoSignal] = "Sem sinal";
        pt[StringKey::SignalQuiet] = "Sinal muito baixo";
        pt[StringKey::Clipping] = "CLIPANDO!";
        pt[StringKey::PeaksHot] = "Picos altos";
        pt[StringKey::LevelsOK] = "Niveis OK";
        pt[StringKey::OverCompressed] = "Sobre-comprimido";
        pt[StringKey::VeryDynamic] = "Muito dinamico";
        pt[StringKey::GoodDynamics] = "Boa dinamica";
        pt[StringKey::PhaseProblems] = "Problemas de fase!";
        pt[StringKey::CheckMono] = "Verificar mono";
        pt[StringKey::MonoOK] = "Mono OK";
        pt[StringKey::VeryWide] = "Muito largo";
        pt[StringKey::MonoSignal] = "Sinal mono";
        pt[StringKey::PlatformTarget] = "Alvo";
        pt[StringKey::Custom] = "Personalizado";
        pt[StringKey::NotSpecified] = "Nao especificado";
        pt[StringKey::LoadSatellite] = "Carregar AR3S Satellite nas faixas";
        
        // Copy English as base for remaining languages (Italian, Japanese, Korean, Chinese, Russian)
        // Users can contribute proper translations later
        
        // ============ ITALIAN ============
        auto& it = strings[Language::Italian];
        it[StringKey::TabMain] = "Principale";
        it[StringKey::TabSettings] = "Impostazioni";
        it[StringKey::TabAssistant] = "Assistente";
        it[StringKey::Gain] = "GUADAGNO";
        it[StringKey::Target] = "OBIETTIVO";
        it[StringKey::AutoGain] = "Guadagno Auto";
        it[StringKey::VocalRider] = "Vocal Rider";
        it[StringKey::Rider] = "Rider";
        it[StringKey::Phase] = "FASE";
        it[StringKey::GainAdjust] = "REGOLA GUADAGNO";
        it[StringKey::Genre] = "Genere";
        it[StringKey::Source] = "Sorgente";
        it[StringKey::Stage] = "Fase";
        it[StringKey::AutoSet] = "Auto-Imposta";
        it[StringKey::AskAI] = "Chiedi IA";
        it[StringKey::ApplyAI] = "Applica IA";
        it[StringKey::SaveSettings] = "Salva";
        it[StringKey::RefreshModels] = "Aggiorna Modelli";
        it[StringKey::Send] = "Invia";
        it[StringKey::AIProvider] = "Provider IA";
        it[StringKey::Model] = "Modello";
        it[StringKey::APIKey] = "Chiave API";
        it[StringKey::Theme] = "Tema";
        it[StringKey::KnobStyle] = "Stile Manopola";
        it[StringKey::AccentColor] = "Colore Accento";
        it[StringKey::Background] = "Sfondo";
        it[StringKey::Language] = "Lingua";
        it[StringKey::Microphone] = "Microfono";
        it[StringKey::Preamp] = "Preamplificatore";
        it[StringKey::Interface] = "Interfaccia";
        it[StringKey::SignalAnalysis] = "ANALISI SEGNALE";
        it[StringKey::Headroom] = "Margine:";
        it[StringKey::Dynamics] = "Dinamica:";
        it[StringKey::Clips] = "Clips:";
        it[StringKey::Peak] = "Picco:";
        it[StringKey::Width] = "Larghezza:";
        it[StringKey::MonoCompat] = "Comp Mono:";
        it[StringKey::Correlation] = "Correlazione:";
        it[StringKey::RMS] = "RMS:";
        it[StringKey::FreqBalance] = "BILANCIO FREQ";
        it[StringKey::Low] = "BASSI";
        it[StringKey::Mid] = "MEDI";
        it[StringKey::High] = "ALTI";
        it[StringKey::QuickCheck] = "CONTROLLO RAPIDO";
        it[StringKey::Satellites] = "SATELLITI";
        it[StringKey::NoSignal] = "Nessun segnale";
        it[StringKey::SignalQuiet] = "Segnale molto debole";
        it[StringKey::Clipping] = "DISTORSIONE!";
        it[StringKey::PeaksHot] = "Picchi alti";
        it[StringKey::LevelsOK] = "Livelli OK";
        it[StringKey::OverCompressed] = "Sovra-compresso";
        it[StringKey::VeryDynamic] = "Molto dinamico";
        it[StringKey::GoodDynamics] = "Buona dinamica";
        it[StringKey::PhaseProblems] = "Problemi di fase!";
        it[StringKey::CheckMono] = "Verifica mono";
        it[StringKey::MonoOK] = "Mono OK";
        it[StringKey::VeryWide] = "Molto ampio";
        it[StringKey::MonoSignal] = "Segnale mono";
        it[StringKey::PlatformTarget] = "Obiettivo";
        it[StringKey::Custom] = "Personalizzato";
        it[StringKey::NotSpecified] = "Non specificato";
        it[StringKey::LoadSatellite] = "Carica AR3S Satellite sulle tracce";
        
        // ============ JAPANESE ============
        auto& ja = strings[Language::Japanese];
        ja[StringKey::TabMain] = "\xe3\x83\xa1\xe3\x82\xa4\xe3\x83\xb3";  // メイン
        ja[StringKey::TabSettings] = "\xe8\xa8\xad\xe5\xae\x9a";  // 設定
        ja[StringKey::TabAssistant] = "\xe3\x82\xa2\xe3\x82\xb7\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\xb3\xe3\x83\x88";  // アシスタント
        ja[StringKey::Gain] = "\xe3\x82\xb2\xe3\x82\xa4\xe3\x83\xb3";  // ゲイン
        ja[StringKey::Target] = "\xe3\x82\xbf\xe3\x83\xbc\xe3\x82\xb2\xe3\x83\x83\xe3\x83\x88";  // ターゲット
        ja[StringKey::AutoGain] = "\xe3\x82\xaa\xe3\x83\xbc\xe3\x83\x88\xe3\x82\xb2\xe3\x82\xa4\xe3\x83\xb3";  // オートゲイン
        ja[StringKey::VocalRider] = "\xe3\x83\x9c\xe3\x83\xbc\xe3\x82\xab\xe3\x83\xab\xe3\x83\xa9\xe3\x82\xa4\xe3\x83\x80\xe3\x83\xbc";  // ボーカルライダー
        ja[StringKey::Rider] = "\xe3\x83\xa9\xe3\x82\xa4\xe3\x83\x80\xe3\x83\xbc";  // ライダー
        ja[StringKey::Phase] = "\xe4\xbd\x8d\xe7\x9b\xb8";  // 位相
        ja[StringKey::GainAdjust] = "\xe3\x82\xb2\xe3\x82\xa4\xe3\x83\xb3\xe8\xaa\xbf\xe6\x95\xb4";  // ゲイン調整
        ja[StringKey::Genre] = "\xe3\x82\xb8\xe3\x83\xa3\xe3\x83\xb3\xe3\x83\xab";  // ジャンル
        ja[StringKey::Source] = "\xe3\x82\xbd\xe3\x83\xbc\xe3\x82\xb9";  // ソース
        ja[StringKey::Stage] = "\xe3\x82\xb9\xe3\x83\x86\xe3\x83\xbc\xe3\x82\xb8";  // ステージ
        ja[StringKey::AutoSet] = "\xe8\x87\xaa\xe5\x8b\x95\xe8\xa8\xad\xe5\xae\x9a";  // 自動設定
        ja[StringKey::AskAI] = "AI\xe3\x81\xab\xe8\x81\x9e\xe3\x81\x8f";  // AIに聞く
        ja[StringKey::ApplyAI] = "AI\xe3\x82\x92\xe9\x81\xa9\xe7\x94\xa8";  // AIを適用
        ja[StringKey::SaveSettings] = "\xe4\xbf\x9d\xe5\xad\x98";  // 保存
        ja[StringKey::RefreshModels] = "\xe3\x83\xa2\xe3\x83\x87\xe3\x83\xab\xe6\x9b\xb4\xe6\x96\xb0";  // モデル更新
        ja[StringKey::Send] = "\xe9\x80\x81\xe4\xbf\xa1";  // 送信
        ja[StringKey::AIProvider] = "AI\xe3\x83\x97\xe3\x83\xad\xe3\x83\x90\xe3\x82\xa4\xe3\x83\x80";  // AIプロバイダ
        ja[StringKey::Model] = "\xe3\x83\xa2\xe3\x83\x87\xe3\x83\xab";  // モデル
        ja[StringKey::APIKey] = "API\xe3\x82\xad\xe3\x83\xbc";  // APIキー
        ja[StringKey::Theme] = "\xe3\x83\x86\xe3\x83\xbc\xe3\x83\x9e";  // テーマ
        ja[StringKey::KnobStyle] = "\xe3\x83\x8e\xe3\x83\x96\xe3\x82\xb9\xe3\x82\xbf\xe3\x82\xa4\xe3\x83\xab";  // ノブスタイル
        ja[StringKey::AccentColor] = "\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe8\x89\xb2";  // アクセント色
        ja[StringKey::Background] = "\xe8\x83\x8c\xe6\x99\xaf";  // 背景
        ja[StringKey::Language] = "\xe8\xa8\x80\xe8\xaa\x9e";  // 言語
        ja[StringKey::Microphone] = "\xe3\x83\x9e\xe3\x82\xa4\xe3\x82\xaf";  // マイク
        ja[StringKey::Preamp] = "\xe3\x83\x97\xe3\x83\xaa\xe3\x82\xa2\xe3\x83\xb3\xe3\x83\x97";  // プリアンプ
        ja[StringKey::Interface] = "\xe3\x82\xa4\xe3\x83\xb3\xe3\x82\xbf\xe3\x83\xbc\xe3\x83\x95\xe3\x82\xa7\xe3\x83\xbc\xe3\x82\xb9";  // インターフェース
        ja[StringKey::SignalAnalysis] = "\xe4\xbf\xa1\xe5\x8f\xb7\xe5\x88\x86\xe6\x9e\x90";  // 信号分析
        ja[StringKey::Headroom] = "\xe3\x83\x98\xe3\x83\x83\xe3\x83\x89\xe3\x83\xab\xe3\x83\xbc\xe3\x83\xa0:";  // ヘッドルーム:
        ja[StringKey::Dynamics] = "\xe3\x83\x80\xe3\x82\xa4\xe3\x83\x8a\xe3\x83\x9f\xe3\x82\xaf\xe3\x82\xb9:";  // ダイナミクス:
        ja[StringKey::Clips] = "\xe3\x82\xaf\xe3\x83\xaa\xe3\x83\x83\xe3\x83\x97:";  // クリップ:
        ja[StringKey::Peak] = "\xe3\x83\x94\xe3\x83\xbc\xe3\x82\xaf:";  // ピーク:
        ja[StringKey::Width] = "\xe5\xb9\x85:";  // 幅:
        ja[StringKey::MonoCompat] = "\xe3\x83\xa2\xe3\x83\x8e\xe4\xba\x92\xe6\x8f\x9b:";  // モノ互換:
        ja[StringKey::Correlation] = "\xe7\x9b\xb8\xe9\x96\xa2:";  // 相関:
        ja[StringKey::RMS] = "RMS:";
        ja[StringKey::FreqBalance] = "\xe5\x91\xa8\xe6\xb3\xa2\xe6\x95\xb0\xe3\x83\x90\xe3\x83\xa9\xe3\x83\xb3\xe3\x82\xb9";  // 周波数バランス
        ja[StringKey::Low] = "\xe4\xbd\x8e\xe5\x9f\x9f";  // 低域
        ja[StringKey::Mid] = "\xe4\xb8\xad\xe5\x9f\x9f";  // 中域
        ja[StringKey::High] = "\xe9\xab\x98\xe5\x9f\x9f";  // 高域
        ja[StringKey::QuickCheck] = "\xe3\x82\xaf\xe3\x82\xa4\xe3\x83\x83\xe3\x82\xaf\xe3\x83\x81\xe3\x82\xa7\xe3\x83\x83\xe3\x82\xaf";  // クイックチェック
        ja[StringKey::Satellites] = "\xe3\x82\xb5\xe3\x83\x86\xe3\x83\xa9\xe3\x82\xa4\xe3\x83\x88";  // サテライト
        ja[StringKey::NoSignal] = "\xe4\xbf\xa1\xe5\x8f\xb7\xe3\x81\xaa\xe3\x81\x97";  // 信号なし
        ja[StringKey::SignalQuiet] = "\xe4\xbf\xa1\xe5\x8f\xb7\xe3\x81\x8c\xe5\xbc\xb1\xe3\x81\x84";  // 信号が弱い
        ja[StringKey::Clipping] = "\xe3\x82\xaf\xe3\x83\xaa\xe3\x83\x83\xe3\x83\x94\xe3\x83\xb3\xe3\x82\xb0!";  // クリッピング!
        ja[StringKey::PeaksHot] = "\xe3\x83\x94\xe3\x83\xbc\xe3\x82\xaf\xe9\xab\x98";  // ピーク高
        ja[StringKey::LevelsOK] = "\xe3\x83\xac\xe3\x83\x99\xe3\x83\xabOK";  // レベルOK
        ja[StringKey::OverCompressed] = "\xe9\x81\x8e\xe5\x9c\xa7\xe7\xb8\xae";  // 過圧縮
        ja[StringKey::VeryDynamic] = "\xe9\x9d\x9e\xe5\xb8\xb8\xe3\x81\xab\xe3\x83\x80\xe3\x82\xa4\xe3\x83\x8a\xe3\x83\x9f\xe3\x83\x83\xe3\x82\xaf";  // 非常にダイナミック
        ja[StringKey::GoodDynamics] = "\xe8\x89\xaf\xe5\xa5\xbd\xe3\x81\xaa\xe3\x83\x80\xe3\x82\xa4\xe3\x83\x8a\xe3\x83\x9f\xe3\x82\xaf\xe3\x82\xb9";  // 良好なダイナミクス
        ja[StringKey::PhaseProblems] = "\xe4\xbd\x8d\xe7\x9b\xb8\xe5\x95\x8f\xe9\xa1\x8c!";  // 位相問題!
        ja[StringKey::CheckMono] = "\xe3\x83\xa2\xe3\x83\x8e\xe7\xa2\xba\xe8\xaa\x8d";  // モノ確認
        ja[StringKey::MonoOK] = "\xe3\x83\xa2\xe3\x83\x8e OK";  // モノ OK
        ja[StringKey::VeryWide] = "\xe9\x9d\x9e\xe5\xb8\xb8\xe3\x81\xab\xe5\xba\x83\xe3\x81\x84";  // 非常に広い
        ja[StringKey::MonoSignal] = "\xe3\x83\xa2\xe3\x83\x8e\xe4\xbf\xa1\xe5\x8f\xb7";  // モノ信号
        ja[StringKey::PlatformTarget] = "\xe3\x82\xbf\xe3\x83\xbc\xe3\x82\xb2\xe3\x83\x83\xe3\x83\x88";  // ターゲット
        ja[StringKey::Custom] = "\xe3\x82\xab\xe3\x82\xb9\xe3\x82\xbf\xe3\x83\xa0";  // カスタム
        ja[StringKey::NotSpecified] = "\xe6\x9c\xaa\xe6\x8c\x87\xe5\xae\x9a";  // 未指定
        ja[StringKey::LoadSatellite] = "\xe3\x83\x88\xe3\x83\xa9\xe3\x83\x83\xe3\x82\xaf\xe3\x81\xab\xe3\x82\xb5\xe3\x83\x86\xe3\x83\xa9\xe3\x82\xa4\xe3\x83\x88\xe3\x82\x92\xe8\xaa\xad\xe3\x81\xbf\xe8\xbe\xbc\xe3\x82\x80";  // トラックにサテライトを読み込む
        
        // ============ KOREAN ============
        auto& ko = strings[Language::Korean];
        ko[StringKey::TabMain] = "\xeb\xa9\x94\xec\x9d\xb8";  // 메인
        ko[StringKey::TabSettings] = "\xec\x84\xa4\xec\xa0\x95";  // 설정
        ko[StringKey::TabAssistant] = "\xec\x96\xb4\xec\x8b\x9c\xec\x8a\xa4\xed\x84\xb4\xed\x8a\xb8";  // 어시스턴트
        ko[StringKey::Gain] = "\xea\xb2\x8c\xec\x9d\xb8";  // 게인
        ko[StringKey::Target] = "\xed\x83\x80\xea\xb2\x9f";  // 타겟
        ko[StringKey::AutoGain] = "\xec\x98\xa4\xed\x86\xa0 \xea\xb2\x8c\xec\x9d\xb8";  // 오토 게인
        ko[StringKey::VocalRider] = "\xeb\xb3\xb4\xec\xbb\xac \xeb\x9d\xbc\xec\x9d\xb4\xeb\x8d\x94";  // 보컬 라이더
        ko[StringKey::Rider] = "\xeb\x9d\xbc\xec\x9d\xb4\xeb\x8d\x94";  // 라이더
        ko[StringKey::Phase] = "\xec\x9c\x84\xec\x83\x81";  // 위상
        ko[StringKey::GainAdjust] = "\xea\xb2\x8c\xec\x9d\xb8 \xec\xa1\xb0\xec\xa0\x95";  // 게인 조정
        ko[StringKey::Genre] = "\xec\x9e\xa5\xeb\xa5\xb4";  // 장르
        ko[StringKey::Source] = "\xec\x86\x8c\xec\x8a\xa4";  // 소스
        ko[StringKey::Stage] = "\xec\x8a\xa4\xed\x85\x8c\xec\x9d\xb4\xec\xa7\x80";  // 스테이지
        ko[StringKey::AutoSet] = "\xec\x9e\x90\xeb\x8f\x99 \xec\x84\xa4\xec\xa0\x95";  // 자동 설정
        ko[StringKey::AskAI] = "AI \xec\xa7\x88\xeb\xac\xb8";  // AI 질문
        ko[StringKey::ApplyAI] = "AI \xec\xa0\x81\xec\x9a\xa9";  // AI 적용
        ko[StringKey::SaveSettings] = "\xec\xa0\x80\xec\x9e\xa5";  // 저장
        ko[StringKey::RefreshModels] = "\xeb\xaa\xa8\xeb\x8d\xb8 \xec\x83\x88\xeb\xa1\x9c\xea\xb3\xa0\xec\xb9\xa8";  // 모델 새로고침
        ko[StringKey::Send] = "\xec\xa0\x84\xec\x86\xa1";  // 전송
        ko[StringKey::AIProvider] = "AI \xec\xa0\x9c\xea\xb3\xb5\xec\x9e\x90";  // AI 제공자
        ko[StringKey::Model] = "\xeb\xaa\xa8\xeb\x8d\xb8";  // 모델
        ko[StringKey::APIKey] = "API \xed\x82\xa4";  // API 키
        ko[StringKey::Theme] = "\xed\x85\x8c\xeb\xa7\x88";  // 테마
        ko[StringKey::KnobStyle] = "\xeb\x85\xb8\xeb\xb8\x8c \xec\x8a\xa4\xed\x83\x80\xec\x9d\xbc";  // 노브 스타일
        ko[StringKey::AccentColor] = "\xea\xb0\x95\xec\xa1\xb0\xec\x83\x89";  // 강조색
        ko[StringKey::Background] = "\xeb\xb0\xb0\xea\xb2\xbd";  // 배경
        ko[StringKey::Language] = "\xec\x96\xb8\xec\x96\xb4";  // 언어
        ko[StringKey::Microphone] = "\xeb\xa7\x88\xec\x9d\xb4\xed\x81\xac";  // 마이크
        ko[StringKey::Preamp] = "\xed\x94\x84\xeb\xa6\xac\xec\x95\xb0\xed\x94\x84";  // 프리앰프
        ko[StringKey::Interface] = "\xec\x9d\xb8\xed\x84\xb0\xed\x8e\x98\xec\x9d\xb4\xec\x8a\xa4";  // 인터페이스
        ko[StringKey::SignalAnalysis] = "\xec\x8b\xa0\xed\x98\xb8 \xeb\xb6\x84\xec\x84\x9d";  // 신호 분석
        ko[StringKey::Headroom] = "\xed\x97\xa4\xeb\x93\x9c\xeb\xa3\xb8:";  // 헤드룸:
        ko[StringKey::Dynamics] = "\xeb\x8b\xa4\xec\x9d\xb4\xeb\x82\x98\xeb\xaf\xb9\xec\x8a\xa4:";  // 다이나믹스:
        ko[StringKey::Clips] = "\xed\x81\xb4\xeb\xa6\xbd:";  // 클립:
        ko[StringKey::Peak] = "\xed\x94\xbc\xed\x81\xac:";  // 피크:
        ko[StringKey::Width] = "\xeb\x84\x93\xec\x9d\xb4:";  // 넓이:
        ko[StringKey::MonoCompat] = "\xeb\xaa\xa8\xeb\x85\xb8 \xed\x98\xb8\xed\x99\x98:";  // 모노 호환:
        ko[StringKey::Correlation] = "\xec\x83\x81\xea\xb4\x80:";  // 상관:
        ko[StringKey::RMS] = "RMS:";
        ko[StringKey::FreqBalance] = "\xec\xa3\xbc\xed\x8c\x8c\xec\x88\x98 \xeb\xb0\xb8\xeb\x9f\xb0\xec\x8a\xa4";  // 주파수 밸런스
        ko[StringKey::Low] = "\xec\xa0\x80\xec\x97\xad";  // 저역
        ko[StringKey::Mid] = "\xec\xa4\x91\xec\x97\xad";  // 중역
        ko[StringKey::High] = "\xea\xb3\xa0\xec\x97\xad";  // 고역
        ko[StringKey::QuickCheck] = "\xeb\xb9\xa0\xeb\xa5\xb8 \xed\x99\x95\xec\x9d\xb8";  // 빠른 확인
        ko[StringKey::Satellites] = "\xec\x83\x88\xed\x85\x94\xeb\x9d\xbc\xec\x9d\xb4\xed\x8a\xb8";  // 새텔라이트
        ko[StringKey::NoSignal] = "\xec\x8b\xa0\xed\x98\xb8 \xec\x97\x86\xec\x9d\x8c";  // 신호 없음
        ko[StringKey::SignalQuiet] = "\xec\x8b\xa0\xed\x98\xb8 \xec\x95\xbd\xed\x95\xa8";  // 신호 약함
        ko[StringKey::Clipping] = "\xed\x81\xb4\xeb\xa6\xac\xed\x95\x91!";  // 클리핑!
        ko[StringKey::PeaksHot] = "\xed\x94\xbc\xed\x81\xac \xeb\x86\x92\xec\x9d\x8c";  // 피크 높음
        ko[StringKey::LevelsOK] = "\xeb\xa0\x88\xeb\xb2\xa8 OK";  // 레벨 OK
        ko[StringKey::OverCompressed] = "\xea\xb3\xbc\xec\x95\x95\xec\xb6\x95";  // 과압축
        ko[StringKey::VeryDynamic] = "\xeb\xa7\xa4\xec\x9a\xb0 \xeb\x8b\xa4\xec\x9d\xb4\xeb\x82\x98\xeb\xaf\xb9";  // 매우 다이나믹
        ko[StringKey::GoodDynamics] = "\xec\xa2\x8b\xec\x9d\x80 \xeb\x8b\xa4\xec\x9d\xb4\xeb\x82\x98\xeb\xaf\xb9\xec\x8a\xa4";  // 좋은 다이나믹스
        ko[StringKey::PhaseProblems] = "\xec\x9c\x84\xec\x83\x81 \xeb\xac\xb8\xec\xa0\x9c!";  // 위상 문제!
        ko[StringKey::CheckMono] = "\xeb\xaa\xa8\xeb\x85\xb8 \xed\x99\x95\xec\x9d\xb8";  // 모노 확인
        ko[StringKey::MonoOK] = "\xeb\xaa\xa8\xeb\x85\xb8 OK";  // 모노 OK
        ko[StringKey::VeryWide] = "\xeb\xa7\xa4\xec\x9a\xb0 \xeb\x84\x93\xec\x9d\x8c";  // 매우 넓음
        ko[StringKey::MonoSignal] = "\xeb\xaa\xa8\xeb\x85\xb8 \xec\x8b\xa0\xed\x98\xb8";  // 모노 신호
        ko[StringKey::PlatformTarget] = "\xed\x83\x80\xea\xb2\x9f";  // 타겟
        ko[StringKey::Custom] = "\xec\x82\xac\xec\x9a\xa9\xec\x9e\x90 \xec\xa0\x95\xec\x9d\x98";  // 사용자 정의
        ko[StringKey::NotSpecified] = "\xec\xa7\x80\xec\xa0\x95 \xec\x95\x88 \xeb\x90\xa8";  // 지정 안 됨
        ko[StringKey::LoadSatellite] = "\xed\x8a\xb8\xeb\x9e\x99\xec\x97\x90 \xec\x83\x88\xed\x85\x94\xeb\x9d\xbc\xec\x9d\xb4\xed\x8a\xb8 \xeb\xa1\x9c\xeb\x93\x9c";  // 트랙에 새텔라이트 로드
        
        // ============ CHINESE ============
        auto& zh = strings[Language::Chinese];
        zh[StringKey::TabMain] = "\xe4\xb8\xbb\xe9\xa1\xb5";  // 主页
        zh[StringKey::TabSettings] = "\xe8\xae\xbe\xe7\xbd\xae";  // 设置
        zh[StringKey::TabAssistant] = "\xe5\x8a\xa9\xe6\x89\x8b";  // 助手
        zh[StringKey::Gain] = "\xe5\xa2\x9e\xe7\x9b\x8a";  // 增益
        zh[StringKey::Target] = "\xe7\x9b\xae\xe6\xa0\x87";  // 目标
        zh[StringKey::AutoGain] = "\xe8\x87\xaa\xe5\x8a\xa8\xe5\xa2\x9e\xe7\x9b\x8a";  // 自动增益
        zh[StringKey::VocalRider] = "\xe4\xba\xba\xe5\xa3\xb0\xe8\xb7\x9f\xe8\xb8\xaa";  // 人声跟踪
        zh[StringKey::Rider] = "\xe8\xb7\x9f\xe8\xb8\xaa";  // 跟踪
        zh[StringKey::Phase] = "\xe7\x9b\xb8\xe4\xbd\x8d";  // 相位
        zh[StringKey::GainAdjust] = "\xe5\xa2\x9e\xe7\x9b\x8a\xe8\xb0\x83\xe6\x95\xb4";  // 增益调整
        zh[StringKey::Genre] = "\xe9\xa3\x8e\xe6\xa0\xbc";  // 风格
        zh[StringKey::Source] = "\xe9\x9f\xb3\xe6\xba\x90";  // 音源
        zh[StringKey::Stage] = "\xe9\x98\xb6\xe6\xae\xb5";  // 阶段
        zh[StringKey::AutoSet] = "\xe8\x87\xaa\xe5\x8a\xa8\xe8\xae\xbe\xe7\xbd\xae";  // 自动设置
        zh[StringKey::AskAI] = "\xe8\xaf\xa2\xe9\x97\xae" "AI";  // 询问AI
        zh[StringKey::ApplyAI] = "\xe5\xba\x94\xe7\x94\xa8" "AI";  // 应用AI
        zh[StringKey::SaveSettings] = "\xe4\xbf\x9d\xe5\xad\x98";  // 保存
        zh[StringKey::RefreshModels] = "\xe5\x88\xb7\xe6\x96\xb0\xe6\xa8\xa1\xe5\x9e\x8b";  // 刷新模型
        zh[StringKey::Send] = "\xe5\x8f\x91\xe9\x80\x81";  // 发送
        zh[StringKey::AIProvider] = "AI" "\xe6\x8f\x90\xe4\xbe\x9b\xe5\x95\x86";  // AI提供商
        zh[StringKey::Model] = "\xe6\xa8\xa1\xe5\x9e\x8b";  // 模型
        zh[StringKey::APIKey] = "API" "\xe5\xaf\x86\xe9\x92\xa5";  // API密钥
        zh[StringKey::Theme] = "\xe4\xb8\xbb\xe9\xa2\x98";  // 主题
        zh[StringKey::KnobStyle] = "\xe6\x97\x8b\xe9\x92\xae\xe6\xa0\xb7\xe5\xbc\x8f";  // 旋钮样式
        zh[StringKey::AccentColor] = "\xe5\xbc\xba\xe8\xb0\x83\xe8\x89\xb2";  // 强调色
        zh[StringKey::Background] = "\xe8\x83\x8c\xe6\x99\xaf";  // 背景
        zh[StringKey::Language] = "\xe8\xaf\xad\xe8\xa8\x80";  // 语言
        zh[StringKey::Microphone] = "\xe9\xba\xa6\xe5\x85\x8b\xe9\xa3\x8e";  // 麦克风
        zh[StringKey::Preamp] = "\xe5\x89\x8d\xe7\xbd\xae\xe6\x94\xbe\xe5\xa4\xa7\xe5\x99\xa8";  // 前置放大器
        zh[StringKey::Interface] = "\xe9\x9f\xb3\xe9\xa2\x91\xe6\x8e\xa5\xe5\x8f\xa3";  // 音频接口
        zh[StringKey::SignalAnalysis] = "\xe4\xbf\xa1\xe5\x8f\xb7\xe5\x88\x86\xe6\x9e\x90";  // 信号分析
        zh[StringKey::Headroom] = "\xe9\xa1\xb6\xe7\xa9\xba:";  // 顶空:
        zh[StringKey::Dynamics] = "\xe5\x8a\xa8\xe6\x80\x81:";  // 动态:
        zh[StringKey::Clips] = "\xe5\x89\x8a\xe5\x88\x87:";  // 削切:
        zh[StringKey::Peak] = "\xe5\xb3\xb0\xe5\x80\xbc:";  // 峰值:
        zh[StringKey::Width] = "\xe5\xae\xbd\xe5\xba\xa6:";  // 宽度:
        zh[StringKey::MonoCompat] = "\xe5\x8d\x95\xe5\xa3\xb0\xe9\x81\x93\xe5\x85\xbc\xe5\xae\xb9:";  // 单声道兼容:
        zh[StringKey::Correlation] = "\xe7\x9b\xb8\xe5\x85\xb3\xe6\x80\xa7:";  // 相关性:
        zh[StringKey::RMS] = "RMS:";
        zh[StringKey::FreqBalance] = "\xe9\xa2\x91\xe7\x8e\x87\xe5\xb9\xb3\xe8\xa1\xa1";  // 频率平衡
        zh[StringKey::Low] = "\xe4\xbd\x8e\xe9\xa2\x91";  // 低频
        zh[StringKey::Mid] = "\xe4\xb8\xad\xe9\xa2\x91";  // 中频
        zh[StringKey::High] = "\xe9\xab\x98\xe9\xa2\x91";  // 高频
        zh[StringKey::QuickCheck] = "\xe5\xbf\xab\xe9\x80\x9f\xe6\xa3\x80\xe6\x9f\xa5";  // 快速检查
        zh[StringKey::Satellites] = "\xe5\x8d\xab\xe6\x98\x9f";  // 卫星
        zh[StringKey::NoSignal] = "\xe6\x97\xa0\xe4\xbf\xa1\xe5\x8f\xb7";  // 无信号
        zh[StringKey::SignalQuiet] = "\xe4\xbf\xa1\xe5\x8f\xb7\xe5\xbe\x88\xe5\xbc\xb1";  // 信号很弱
        zh[StringKey::Clipping] = "\xe5\x89\x8a\xe5\x88\x87!";  // 削切!
        zh[StringKey::PeaksHot] = "\xe5\xb3\xb0\xe5\x80\xbc\xe8\xbf\x87\xe9\xab\x98";  // 峰值过高
        zh[StringKey::LevelsOK] = "\xe7\x94\xb5\xe5\xb9\xb3OK";  // 电平OK
        zh[StringKey::OverCompressed] = "\xe8\xbf\x87\xe5\xba\xa6\xe5\x8e\x8b\xe7\xbc\xa9";  // 过度压缩
        zh[StringKey::VeryDynamic] = "\xe9\x9d\x9e\xe5\xb8\xb8\xe5\x8a\xa8\xe6\x80\x81";  // 非常动态
        zh[StringKey::GoodDynamics] = "\xe8\x89\xaf\xe5\xa5\xbd\xe5\x8a\xa8\xe6\x80\x81";  // 良好动态
        zh[StringKey::PhaseProblems] = "\xe7\x9b\xb8\xe4\xbd\x8d\xe9\x97\xae\xe9\xa2\x98!";  // 相位问题!
        zh[StringKey::CheckMono] = "\xe6\xa3\x80\xe6\x9f\xa5\xe5\x8d\x95\xe5\xa3\xb0\xe9\x81\x93";  // 检查单声道
        zh[StringKey::MonoOK] = "\xe5\x8d\x95\xe5\xa3\xb0\xe9\x81\x93OK";  // 单声道OK
        zh[StringKey::VeryWide] = "\xe9\x9d\x9e\xe5\xb8\xb8\xe5\xae\xbd";  // 非常宽
        zh[StringKey::MonoSignal] = "\xe5\x8d\x95\xe5\xa3\xb0\xe9\x81\x93\xe4\xbf\xa1\xe5\x8f\xb7";  // 单声道信号
        zh[StringKey::PlatformTarget] = "\xe7\x9b\xae\xe6\xa0\x87";  // 目标
        zh[StringKey::Custom] = "\xe8\x87\xaa\xe5\xae\x9a\xe4\xb9\x89";  // 自定义
        zh[StringKey::NotSpecified] = "\xe6\x9c\xaa\xe6\x8c\x87\xe5\xae\x9a";  // 未指定
        zh[StringKey::LoadSatellite] = "\xe5\x9c\xa8\xe8\xbd\xa8\xe9\x81\x93\xe4\xb8\x8a\xe5\x8a\xa0\xe8\xbd\xbd\xe5\x8d\xab\xe6\x98\x9f";  // 在轨道上加载卫星
        
        // ============ RUSSIAN ============
        auto& ru = strings[Language::Russian];
        ru[StringKey::TabMain] = "\xd0\x93\xd0\xbb\xd0\xb0\xd0\xb2\xd0\xbd\xd0\xb0\xd1\x8f";  // Главная
        ru[StringKey::TabSettings] = "\xd0\x9d\xd0\xb0\xd1\x81\xd1\x82\xd1\x80\xd0\xbe\xd0\xb9\xd0\xba\xd0\xb8";  // Настройки
        ru[StringKey::TabAssistant] = "\xd0\x9f\xd0\xbe\xd0\xbc\xd0\xbe\xd1\x89\xd0\xbd\xd0\xb8\xd0\xba";  // Помощник
        ru[StringKey::Gain] = "\xd0\xa3\xd0\xa1\xd0\x98\xd0\x9b.";  // УСИЛ.
        ru[StringKey::Target] = "\xd0\xa6\xd0\x95\xd0\x9b\xd0\xac";  // ЦЕЛЬ
        ru[StringKey::AutoGain] = "\xd0\x90\xd0\xb2\xd1\x82\xd0\xbe \xd0\xa3\xd1\x81\xd0\xb8\xd0\xbb.";  // Авто Усил.
        ru[StringKey::VocalRider] = "\xd0\x92\xd0\xbe\xd0\xba\xd0\xb0\xd0\xbb \xd0\xa0\xd0\xb0\xd0\xb9\xd0\xb4\xd0\xb5\xd1\x80";  // Вокал Райдер
        ru[StringKey::Rider] = "\xd0\xa0\xd0\xb0\xd0\xb9\xd0\xb4\xd0\xb5\xd1\x80";  // Райдер
        ru[StringKey::Phase] = "\xd0\xa4\xd0\x90\xd0\x97\xd0\x90";  // ФАЗА
        ru[StringKey::GainAdjust] = "\xd0\xa0\xd0\x95\xd0\x93. \xd0\xa3\xd0\xa1\xd0\x98\xd0\x9b.";  // РЕГ. УСИЛ.
        ru[StringKey::Genre] = "\xd0\x96\xd0\xb0\xd0\xbd\xd1\x80";  // Жанр
        ru[StringKey::Source] = "\xd0\x98\xd1\x81\xd1\x82\xd0\xbe\xd1\x87\xd0\xbd\xd0\xb8\xd0\xba";  // Источник
        ru[StringKey::Stage] = "\xd0\xad\xd1\x82\xd0\xb0\xd0\xbf";  // Этап
        ru[StringKey::AutoSet] = "\xd0\x90\xd0\xb2\xd1\x82\xd0\xbe";  // Авто
        ru[StringKey::AskAI] = "\xd0\xa1\xd0\xbf\xd1\x80\xd0\xbe\xd1\x81\xd0\xb8\xd1\x82\xd1\x8c AI";  // Спросить AI
        ru[StringKey::ApplyAI] = "\xd0\x9f\xd1\x80\xd0\xb8\xd0\xbc\xd0\xb5\xd0\xbd\xd0\xb8\xd1\x82\xd1\x8c AI";  // Применить AI
        ru[StringKey::SaveSettings] = "\xd0\xa1\xd0\xbe\xd1\x85\xd1\x80\xd0\xb0\xd0\xbd\xd0\xb8\xd1\x82\xd1\x8c";  // Сохранить
        ru[StringKey::RefreshModels] = "\xd0\x9e\xd0\xb1\xd0\xbd\xd0\xbe\xd0\xb2\xd0\xb8\xd1\x82\xd1\x8c \xd0\xbc\xd0\xbe\xd0\xb4\xd0\xb5\xd0\xbb\xd0\xb8";  // Обновить модели
        ru[StringKey::Send] = "\xd0\x9e\xd1\x82\xd0\xbf\xd1\x80\xd0\xb0\xd0\xb2\xd0\xb8\xd1\x82\xd1\x8c";  // Отправить
        ru[StringKey::AIProvider] = "AI \xd0\xbf\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xb9\xd0\xb4\xd0\xb5\xd1\x80";  // AI провайдер
        ru[StringKey::Model] = "\xd0\x9c\xd0\xbe\xd0\xb4\xd0\xb5\xd0\xbb\xd1\x8c";  // Модель
        ru[StringKey::APIKey] = "API \xd0\xba\xd0\xbb\xd1\x8e\xd1\x87";  // API ключ
        ru[StringKey::Theme] = "\xd0\xa2\xd0\xb5\xd0\xbc\xd0\xb0";  // Тема
        ru[StringKey::KnobStyle] = "\xd0\xa1\xd1\x82\xd0\xb8\xd0\xbb\xd1\x8c \xd1\x80\xd1\x83\xd1\x87\xd0\xba\xd0\xb8";  // Стиль ручки
        ru[StringKey::AccentColor] = "\xd0\x90\xd0\xba\xd1\x86\xd0\xb5\xd0\xbd\xd1\x82\xd0\xbd\xd1\x8b\xd0\xb9 \xd1\x86\xd0\xb2\xd0\xb5\xd1\x82";  // Акцентный цвет
        ru[StringKey::Background] = "\xd0\xa4\xd0\xbe\xd0\xbd";  // Фон
        ru[StringKey::Language] = "\xd0\xaf\xd0\xb7\xd1\x8b\xd0\xba";  // Язык
        ru[StringKey::Microphone] = "\xd0\x9c\xd0\xb8\xd0\xba\xd1\x80\xd0\xbe\xd1\x84\xd0\xbe\xd0\xbd";  // Микрофон
        ru[StringKey::Preamp] = "\xd0\x9f\xd1\x80\xd0\xb5\xd0\xb4\xd1\x83\xd1\x81\xd0\xb8\xd0\xbb\xd0\xb8\xd1\x82\xd0\xb5\xd0\xbb\xd1\x8c";  // Предусилитель
        ru[StringKey::Interface] = "\xd0\x98\xd0\xbd\xd1\x82\xd0\xb5\xd1\x80\xd1\x84\xd0\xb5\xd0\xb9\xd1\x81";  // Интерфейс
        ru[StringKey::SignalAnalysis] = "\xd0\x90\xd0\x9d\xd0\x90\xd0\x9b\xd0\x98\xd0\x97 \xd0\xa1\xd0\x98\xd0\x93\xd0\x9d\xd0\x90\xd0\x9b\xd0\x90";  // АНАЛИЗ СИГНАЛА
        ru[StringKey::Headroom] = "\xd0\x97\xd0\xb0\xd0\xbf\xd0\xb0\xd1\x81:";  // Запас:
        ru[StringKey::Dynamics] = "\xd0\x94\xd0\xb8\xd0\xbd\xd0\xb0\xd0\xbc\xd0\xb8\xd0\xba\xd0\xb0:";  // Динамика:
        ru[StringKey::Clips] = "\xd0\x9a\xd0\xbb\xd0\xb8\xd0\xbf\xd1\x8b:";  // Клипы:
        ru[StringKey::Peak] = "\xd0\x9f\xd0\xb8\xd0\xba:";  // Пик:
        ru[StringKey::Width] = "\xd0\xa8\xd0\xb8\xd1\x80\xd0\xb8\xd0\xbd\xd0\xb0:";  // Ширина:
        ru[StringKey::MonoCompat] = "\xd0\x9c\xd0\xbe\xd0\xbd\xd0\xbe \xd1\x81\xd0\xbe\xd0\xb2\xd0\xbc.:";  // Моно совм.:
        ru[StringKey::Correlation] = "\xd0\x9a\xd0\xbe\xd1\x80\xd1\x80\xd0\xb5\xd0\xbb\xd1\x8f\xd1\x86\xd0\xb8\xd1\x8f:";  // Корреляция:
        ru[StringKey::RMS] = "RMS:";
        ru[StringKey::FreqBalance] = "\xd0\xa7\xd0\x90\xd0\xa1\xd0\xa2\xd0\x9e\xd0\xa2\xd0\x9d\xd0\xab\xd0\x99 \xd0\x91\xd0\x90\xd0\x9b\xd0\x90\xd0\x9d\xd0\xa1";  // ЧАСТОТНЫЙ БАЛАНС
        ru[StringKey::Low] = "\xd0\x9d\xd0\x98\xd0\x97\xd0\xab";  // НИЗЫ
        ru[StringKey::Mid] = "\xd0\xa1\xd0\xa0\xd0\x95\xd0\x94.";  // СРЕД.
        ru[StringKey::High] = "\xd0\x92\xd0\xab\xd0\xa1\xd0\x9e\xd0\x9a\xd0\x98\xd0\x95";  // ВЫСОКИЕ
        ru[StringKey::QuickCheck] = "\xd0\x91\xd0\xab\xd0\xa1\xd0\xa2\xd0\xa0. \xd0\x9f\xd0\xa0\xd0\x9e\xd0\x92\xd0\x95\xd0\xa0\xd0\x9a\xd0\x90";  // БЫСТР. ПРОВЕРКА
        ru[StringKey::Satellites] = "\xd0\xa1\xd0\x90\xd0\xa2\xd0\x95\xd0\x9b\xd0\x9b\xd0\x98\xd0\xa2\xd0\xab";  // САТЕЛЛИТЫ
        ru[StringKey::NoSignal] = "\xd0\x9d\xd0\xb5\xd1\x82 \xd1\x81\xd0\xb8\xd0\xb3\xd0\xbd\xd0\xb0\xd0\xbb\xd0\xb0";  // Нет сигнала
        ru[StringKey::SignalQuiet] = "\xd0\xa1\xd0\xbb\xd0\xb0\xd0\xb1\xd1\x8b\xd0\xb9 \xd1\x81\xd0\xb8\xd0\xb3\xd0\xbd\xd0\xb0\xd0\xbb";  // Слабый сигнал
        ru[StringKey::Clipping] = "\xd0\x9a\xd0\x9b\xd0\x98\xd0\x9f\xd0\x9f\xd0\x98\xd0\x9d\xd0\x93!";  // КЛИППИНГ!
        ru[StringKey::PeaksHot] = "\xd0\x9f\xd0\xb8\xd0\xba\xd0\xb8 \xd0\xb2\xd1\x8b\xd1\x81\xd0\xbe\xd0\xba\xd0\xb8\xd0\xb5";  // Пики высокие
        ru[StringKey::LevelsOK] = "\xd0\xa3\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb5\xd0\xbd\xd1\x8c OK";  // Уровень OK
        ru[StringKey::OverCompressed] = "\xd0\x9f\xd0\xb5\xd1\x80\xd0\xb5\xd0\xba\xd0\xbe\xd0\xbc\xd0\xbf\xd1\x80\xd0\xb5\xd1\x81\xd1\x81\xd0\xb8\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb0\xd0\xbd\xd0\xbe";  // Перекомпрессировано
        ru[StringKey::VeryDynamic] = "\xd0\x9e\xd1\x87\xd0\xb5\xd0\xbd\xd1\x8c \xd0\xb4\xd0\xb8\xd0\xbd\xd0\xb0\xd0\xbc\xd0\xb8\xd1\x87\xd0\xbd\xd0\xbe";  // Очень динамично
        ru[StringKey::GoodDynamics] = "\xd0\xa5\xd0\xbe\xd1\x80\xd0\xbe\xd1\x88\xd0\xb0\xd1\x8f \xd0\xb4\xd0\xb8\xd0\xbd\xd0\xb0\xd0\xbc\xd0\xb8\xd0\xba\xd0\xb0";  // Хорошая динамика
        ru[StringKey::PhaseProblems] = "\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb1\xd0\xbb\xd0\xb5\xd0\xbc\xd1\x8b \xd1\x84\xd0\xb0\xd0\xb7\xd1\x8b!";  // Проблемы фазы!
        ru[StringKey::CheckMono] = "\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb2\xd0\xb5\xd1\x80\xd0\xb8\xd1\x82\xd1\x8c \xd0\xbc\xd0\xbe\xd0\xbd\xd0\xbe";  // Проверить моно
        ru[StringKey::MonoOK] = "\xd0\x9c\xd0\xbe\xd0\xbd\xd0\xbe OK";  // Моно OK
        ru[StringKey::VeryWide] = "\xd0\x9e\xd1\x87\xd0\xb5\xd0\xbd\xd1\x8c \xd1\x88\xd0\xb8\xd1\x80\xd0\xbe\xd0\xba\xd0\xbe";  // Очень широко
        ru[StringKey::MonoSignal] = "\xd0\x9c\xd0\xbe\xd0\xbd\xd0\xbe \xd1\x81\xd0\xb8\xd0\xb3\xd0\xbd\xd0\xb0\xd0\xbb";  // Моно сигнал
        ru[StringKey::PlatformTarget] = "\xd0\xa6\xd0\xb5\xd0\xbb\xd1\x8c";  // Цель
        ru[StringKey::Custom] = "\xd0\x9f\xd0\xbe\xd0\xbb\xd1\x8c\xd0\xb7\xd0\xbe\xd0\xb2\xd0\xb0\xd1\x82\xd0\xb5\xd0\xbb\xd1\x8c\xd1\x81\xd0\xba\xd0\xb8\xd0\xb9";  // Пользовательский
        ru[StringKey::NotSpecified] = "\xd0\x9d\xd0\xb5 \xd1\x83\xd0\xba\xd0\xb0\xd0\xb7\xd0\xb0\xd0\xbd\xd0\xbe";  // Не указано
        ru[StringKey::LoadSatellite] = "\xd0\x97\xd0\xb0\xd0\xb3\xd1\x80\xd1\x83\xd0\xb7\xd0\xb8\xd1\x82\xd1\x8c \xd1\x81\xd0\xb0\xd1\x82\xd0\xb5\xd0\xbb\xd0\xbb\xd0\xb8\xd1\x82 \xd0\xbd\xd0\xb0 \xd1\x82\xd1\x80\xd0\xb5\xd0\xba\xd0\xb8";  // Загрузить сателлит на треки
    }
    
    Language currentLanguage = Language::English;
    std::map<Language, std::map<StringKey, juce::String>> strings;
};

// Shorthand for getting localized strings
inline juce::String L(StringKey key)
{
    return Localization::getInstance().get(key);
}
