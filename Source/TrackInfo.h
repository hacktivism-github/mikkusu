/*
  ==============================================================================

    TrackInfo.h
    Created: 1 Feb 2026 9:53:48pm
    Author:  Bruno Teixeira

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct TrackInfo
{
    juce::String title;
    juce::String filePath;
    double durationSeconds = 0.0;
};
