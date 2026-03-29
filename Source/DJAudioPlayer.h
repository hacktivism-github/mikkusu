/*
  ==============================================================================

    DJAudioPlayer.h
    Created: 30 Jan 2026 4:59:51pm
    Author:  Bruno Teixeira

  ==============================================================================
*/

//#pragma once
//
//#include "../JuceLibraryCode/JuceHeader.h"
//
//class DJAudioPlayer : public juce::AudioSource
//{
//public:
//    DJAudioPlayer(juce::AudioFormatManager& _formatManager);
//    ~DJAudioPlayer() override;
//
//    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
//    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
//    void releaseResources() override;
//
//    void loadURL(juce::URL audioURL);
//    void unloadTrack();
//
//    void setGain(double gain);
//    void setSpeed(double ratio);
//    void setPosition(double posInSecs);
//    void setPositionRelative(double pos);
//    void nudgePosition(double deltaSeconds);
//
//    void start();
//    void stop();
//    bool isPlaying() const;
//
//    double getPositionRelative();
//    double getCurrentPositionInSeconds() const;
//    double getTrackLengthInSeconds() const;
//
//    double getBPM() const;
//
//    void setCurrentTrackPath(const juce::String& path);
//    juce::String getCurrentTrackPath() const;
//
//    void setHotCue(int index, double positionSeconds);
//    void triggerHotCue(int index);
//    void updateHotCue(int index);
//    void clearHotCues();
//    double getHotCue(int index) const;
//    bool hotCueIsSet(int index) const;
//
//    void loadHotCuesForCurrentTrack();
//    void saveHotCuesForCurrentTrack();
//
//    void setLowEQ(double gainDb);
//    void setMidEQ(double gainDb);
//    void setHighEQ(double gainDb);
//
//    double getLowEQ() const;
//    double getMidEQ() const;
//    double getHighEQ() const;
//
//    void saveEQSettingsForCurrentTrack();
//    void loadEQSettingsForCurrentTrack();
//    void resetEQToOriginal();
//
//private:
//    juce::File getHotCueStoreFile() const;
//    juce::File getEQStoreFile() const;
//    void updateEQFilters();
//
//    juce::AudioFormatManager& formatManager;
//    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
//    juce::AudioTransportSource transportSource;
//    juce::ResamplingAudioSource resampleSource { &transportSource, false, 2 };
//
//    juce::dsp::ProcessorDuplicator<
//        juce::dsp::IIR::Filter<float>,
//        juce::dsp::IIR::Coefficients<float>
//    > lowShelf, midPeak, highShelf;
//
//    bool playing = false;
//    double currentSampleRate = 44100.0;
//
//    juce::String currentTrackPath;
//
//    std::array<double, 8> hotCues { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 };
//
//    double lowEQDb  = 0.0;
//    double midEQDb  = 0.0;
//    double highEQDb = 0.0;
//
//    double bpm = 0.0;
//};

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>
#include <array>

class DJAudioPlayer : public juce::AudioSource
{
public:
    DJAudioPlayer(juce::AudioFormatManager& _formatManager);
    ~DJAudioPlayer() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadURL(juce::URL audioURL);
    void unloadTrack();

    void setGain(double gain);
    void setSpeed(double ratio);
    double getSpeed() const;

    void setPosition(double posInSecs);
    void setPositionRelative(double pos);
    void nudgePosition(double deltaSeconds);

    void start();
    void stop();
    bool isPlaying() const;

    double getPositionRelative();
    double getCurrentPositionInSeconds() const;
    double getTrackLengthInSeconds() const;

    double getBPM() const;              // base BPM
    double getEffectiveBPM() const;     // BPM after tempo change
    bool hasTrackLoaded() const;
    bool syncToTargetBPM(double targetBPM);

    void setCurrentTrackPath(const juce::String& path);
    juce::String getCurrentTrackPath() const;

    void setHotCue(int index, double positionSeconds);
    void triggerHotCue(int index);
    void updateHotCue(int index);
    void clearHotCues();
    double getHotCue(int index) const;
    bool hotCueIsSet(int index) const;

    void loadHotCuesForCurrentTrack();
    void saveHotCuesForCurrentTrack();

    void setLowEQ(double gainDb);
    void setMidEQ(double gainDb);
    void setHighEQ(double gainDb);

    double getLowEQ() const;
    double getMidEQ() const;
    double getHighEQ() const;

    void saveEQSettingsForCurrentTrack();
    void loadEQSettingsForCurrentTrack();
    void resetEQToOriginal();

private:
    juce::File getHotCueStoreFile() const;
    juce::File getEQStoreFile() const;
    void updateEQFilters();

    double estimateBPMFromReader(juce::AudioFormatReader& reader) const;

    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource { &transportSource, false, 2 };

    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>
    > lowShelf, midPeak, highShelf;

    bool playing = false;
    double currentSampleRate = 44100.0;

    juce::String currentTrackPath;

    std::array<double, 8> hotCues { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 };

    double lowEQDb  = 0.0;
    double midEQDb  = 0.0;
    double highEQDb = 0.0;

    double bpm = 0.0;
    double speedRatio = 1.0;
};
