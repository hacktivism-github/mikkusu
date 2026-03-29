/*
  ==============================================================================

    PlaylistComponent.cpp
    Created: 2 Feb 2026 2:24:53pm
    Author:  Bruno Teixeira

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "PlaylistComponent.h"

PlaylistComponent::PlaylistComponent(DeckGUI& d1,
                                     DeckGUI& d2,
                                     juce::AudioFormatManager& fm)
    : deck1(d1), deck2(d2), formatManager(fm)
{
    addAndMakeVisible(addTracksButton);
    addTracksButton.addListener(this);

    addAndMakeVisible(clearLibraryButton);
    clearLibraryButton.addListener(this);

    tableComponent.getHeader().addColumn("Track Title", 1, 360);
    tableComponent.getHeader().addColumn("Duration", 2, 120);
    tableComponent.getHeader().addColumn("Deck 1", 3, 100);
    tableComponent.getHeader().addColumn("Deck 2", 4, 100);
    tableComponent.getHeader().addColumn("Remove", 5, 100);

    tableComponent.setModel(this);
    addAndMakeVisible(tableComponent);

    loadLibraryFromDisk();
}

PlaylistComponent::~PlaylistComponent()
{
    saveLibraryToDisk();
}

void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PlaylistComponent::resized()
{
    addTracksButton.setBounds(10, 10, 120, 30);
    clearLibraryButton.setBounds(140, 10, 140, 30);

    tableComponent.setBounds(10, 50, getWidth() - 20, getHeight() - 60);
}

int PlaylistComponent::getNumRows()
{
    return (int) tracks.size();
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g,
                                           int rowNumber,
                                           int width,
                                           int height,
                                           bool rowIsSelected)
{
    juce::ignoreUnused(rowNumber, width, height);

    if (rowIsSelected)
        g.fillAll(juce::Colours::orange);
    else
        g.fillAll(juce::Colours::darkgrey);
}

void PlaylistComponent::paintCell(juce::Graphics& g,
                                  int rowNumber,
                                  int columnId,
                                  int width,
                                  int height,
                                  bool rowIsSelected)
{
    juce::ignoreUnused(rowIsSelected);

    if (rowNumber < 0 || rowNumber >= (int) tracks.size())
        return;

    const auto& track = tracks[(size_t) rowNumber];

    juce::String text;

    if (columnId == 1)
        text = track.title;
    else if (columnId == 2)
        text = formatDuration(track.durationSeconds);

    g.setColour(juce::Colours::white);
    g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

juce::Component* PlaylistComponent::refreshComponentForCell(int rowNumber,
                                                            int columnId,
                                                            bool isRowSelected,
                                                            juce::Component* existingComponentToUpdate)
{
    juce::ignoreUnused(isRowSelected);

    if (columnId == 3 || columnId == 4 || columnId == 5)
    {
        if (existingComponentToUpdate == nullptr)
        {
            juce::String buttonText;

            if (columnId == 3) buttonText = "Load 1";
            else if (columnId == 4) buttonText = "Load 2";
            else if (columnId == 5) buttonText = "Remove";

            auto* btn = new juce::TextButton(buttonText);
            btn->addListener(this);
            existingComponentToUpdate = btn;
        }

        existingComponentToUpdate->setComponentID(
            std::to_string(rowNumber) + "_" + std::to_string(columnId));

        return existingComponentToUpdate;
    }

    return nullptr;
}

void PlaylistComponent::buttonClicked(juce::Button* button)
{
    if (button == &addTracksButton)
    {
        auto flags = juce::FileBrowserComponent::canSelectFiles
                   | juce::FileBrowserComponent::openMode
                   | juce::FileBrowserComponent::canSelectMultipleItems;

        fileChooser.launchAsync(flags,
                                [this](const juce::FileChooser& chooser)
                                {
                                    auto results = chooser.getResults();
                                    addTracks(results);
                                });
        return;
    }

    if (button == &clearLibraryButton)
    {
        auto result = juce::AlertWindow::showOkCancelBox(
            juce::AlertWindow::WarningIcon,
            "Clear Library",
            "Are you sure you want to remove all tracks from the library?"
        );

        if (result)
            clearLibrary();

        return;
    }

    auto id = button->getComponentID();
    auto parts = juce::StringArray::fromTokens(id, "_", "");

    if (parts.size() != 2)
        return;

    int row = parts[0].getIntValue();
    int columnId = parts[1].getIntValue();

    if (row < 0 || row >= (int) tracks.size())
        return;

    if (columnId == 5)
    {
        removeTrack(row);
        return;
    }

    juce::File file(tracks[(size_t) row].filePath);
    if (! file.existsAsFile())
        return;

    juce::URL url { file };

    if (columnId == 3)
        deck1.loadTrackFromURL(url);
    else if (columnId == 4)
        deck2.loadTrackFromURL(url);
}

void PlaylistComponent::addTracks(const juce::Array<juce::File>& files)
{
    for (const auto& file : files)
    {
        if (! file.existsAsFile())
            continue;

        bool alreadyExists = false;
        for (const auto& track : tracks)
        {
            if (track.filePath == file.getFullPathName())
            {
                alreadyExists = true;
                break;
            }
        }

        if (alreadyExists)
            continue;

        TrackInfo info;
        info.title = file.getFileName();
        info.filePath = file.getFullPathName();

        std::unique_ptr<juce::InputStream> stream(file.createInputStream());
        if (stream != nullptr)
        {
            if (auto* reader = formatManager.createReaderFor(std::move(stream)))
            {
                info.durationSeconds = (double) reader->lengthInSamples / reader->sampleRate;
                delete reader;
            }
        }

        tracks.push_back(info);
    }

    tableComponent.updateContent();
    repaint();
    saveLibraryToDisk();
}

juce::File PlaylistComponent::getLibraryFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("OtoDecks")
        .getChildFile("library.json");
}

void PlaylistComponent::saveLibraryToDisk()
{
    juce::Array<juce::var> arr;

    for (const auto& track : tracks)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("title", track.title);
        obj->setProperty("filePath", track.filePath);
        obj->setProperty("durationSeconds", track.durationSeconds);
        arr.add(juce::var(obj));
    }

    juce::File file = getLibraryFile();
    file.getParentDirectory().createDirectory();

    file.replaceWithText(juce::JSON::toString(juce::var(arr)));
}

void PlaylistComponent::loadLibraryFromDisk()
{
    tracks.clear();

    juce::File file = getLibraryFile();
    if (! file.existsAsFile())
        return;

    auto jsonText = file.loadFileAsString();
    auto parsed = juce::JSON::parse(jsonText);

    if (! parsed.isArray())
        return;

    auto* arr = parsed.getArray();
    if (arr == nullptr)
        return;

    for (const auto& item : *arr)
    {
        auto* obj = item.getDynamicObject();
        if (obj == nullptr)
            continue;

        TrackInfo info;
        info.title = obj->getProperty("title").toString();
        info.filePath = obj->getProperty("filePath").toString();
        info.durationSeconds = (double) obj->getProperty("durationSeconds");

        tracks.push_back(info);
    }

    tableComponent.updateContent();
    repaint();
}

juce::String PlaylistComponent::formatDuration(double seconds) const
{
    int totalSeconds = (int) std::round(seconds);
    int mins = totalSeconds / 60;
    int secs = totalSeconds % 60;

    return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
}

void PlaylistComponent::removeTrack(int rowIndex)
{
    if (rowIndex < 0 || rowIndex >= (int) tracks.size())
        return;

    const juce::String pathToRemove = tracks[(size_t) rowIndex].filePath;

    if (deck1.getCurrentTrackPath() == pathToRemove)
        deck1.unloadTrack();

    if (deck2.getCurrentTrackPath() == pathToRemove)
        deck2.unloadTrack();

    tracks.erase(tracks.begin() + rowIndex);

    tableComponent.updateContent();
    repaint();
    saveLibraryToDisk();
}

void PlaylistComponent::clearLibrary()
{
    tracks.clear();

    tableComponent.updateContent();
    repaint();
    saveLibraryToDisk();
}
