/*
  ==============================================================================

    WaveformDisplay.h
    Created: 3 Feb 2026 3:31:01am
    Author:  Bruno Teixeira

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class WaveformDisplay  : public juce::Component,
                         public juce::ChangeListener,
                         public juce::SettableTooltipClient
{
public:
    WaveformDisplay(juce::AudioFormatManager & formatManagerToUse,
                    juce::AudioThumbnailCache & cacheToUse);
    ~WaveformDisplay() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

    void loadURL(juce::URL audioURL);
    void clear();

    void setPositionRelative(double pos);

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    std::function<void(double)> onSeekRequested;

private:
    void handleSeekFromMouse(float mouseX);
    void updateTooltipForMouse(float mouseX);
    juce::String formatTime(double seconds) const;

    juce::AudioThumbnail audioThumb;
    bool fileLoaded;
    double position;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};
