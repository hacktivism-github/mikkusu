/*
  ==============================================================================

    PlaylistComponent.h
    Created: 2 Feb 2026 2:24:53pm
    Author:  Bruno Teixeira

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DeckGUI.h"
#include "TrackInfo.h"
#include <vector>

class PlaylistComponent  : public juce::Component,
                           public juce::TableListBoxModel,
                           public juce::Button::Listener
{
public:
    PlaylistComponent(DeckGUI& deck1,
                      DeckGUI& deck2,
                      juce::AudioFormatManager& formatManager);
    ~PlaylistComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    int getNumRows() override;

    void paintRowBackground (juce::Graphics&,
                             int rowNumber,
                             int width,
                             int height,
                             bool rowIsSelected) override;

    void paintCell (juce::Graphics&,
                    int rowNumber,
                    int columnId,
                    int width,
                    int height,
                    bool rowIsSelected) override;

    juce::Component* refreshComponentForCell (int rowNumber,
                                              int columnId,
                                              bool isRowSelected,
                                              juce::Component* existingComponentToUpdate) override;

    void buttonClicked(juce::Button* button) override;

    void loadLibraryFromDisk();
    void saveLibraryToDisk();

private:
    void addTracks(const juce::Array<juce::File>& files);
    void removeTrack(int rowIndex);
    void clearLibrary();

    juce::File getLibraryFile() const;
    juce::String formatDuration(double seconds) const;

    DeckGUI& deck1;
    DeckGUI& deck2;
    juce::AudioFormatManager& formatManager;

    juce::TextButton addTracksButton { "Add Tracks" };
    juce::TextButton clearLibraryButton { "Clear Library" };

    juce::TableListBox tableComponent;
    juce::FileChooser fileChooser { "Select audio files..." };

    std::vector<TrackInfo> tracks;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistComponent)
};
