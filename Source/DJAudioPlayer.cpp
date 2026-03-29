/*
  ==============================================================================

    DJAudioPlayer.cpp
    Created: 30 Jan 2026 4:59:51pm
    Author:  Bruno Teixeira

  ==============================================================================
*/

#include "DJAudioPlayer.h"
#include <cmath>
#include <vector>
#include <algorithm>

DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& _formatManager)
    : formatManager(_formatManager)
{
}

DJAudioPlayer::~DJAudioPlayer()
{
}

void DJAudioPlayer::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;

    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlockExpected;
    spec.numChannels = 2;

    lowShelf.prepare(spec);
    midPeak.prepare(spec);
    highShelf.prepare(spec);

    updateEQFilters();
}

void DJAudioPlayer::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (! playing)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    resampleSource.getNextAudioBlock(bufferToFill);

    juce::dsp::AudioBlock<float> block(*bufferToFill.buffer);
    auto subBlock = block.getSubBlock((size_t) bufferToFill.startSample,
                                      (size_t) bufferToFill.numSamples);
    juce::dsp::ProcessContextReplacing<float> context(subBlock);

    lowShelf.process(context);
    midPeak.process(context);
    highShelf.process(context);
}

void DJAudioPlayer::releaseResources()
{
    resampleSource.releaseResources();
    transportSource.releaseResources();
}

void DJAudioPlayer::loadURL (juce::URL audioURL)
{
    std::unique_ptr<juce::InputStream> stream;

    if (audioURL.isLocalFile())
        stream = audioURL.getLocalFile().createInputStream();
    else
        stream = audioURL.createInputStream(
            juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress));

    if (stream == nullptr)
        return;

    auto* reader = formatManager.createReaderFor(std::move(stream));

    if (reader != nullptr)
    {
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
        transportSource.stop();
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);

//        double duration = (double) reader->lengthInSamples / reader->sampleRate;
//        bpm = 120.0;
//
//        std::cout << "Track duration: " << duration << " seconds" << std::endl;
//        std::cout << "Estimated BPM: " << bpm << std::endl;
        
        
        
        double duration = (double) reader->lengthInSamples / reader->sampleRate;
        bpm = estimateBPMFromReader(*reader);
        
        if (bpm <= 0.0)
            bpm = 120.0; // fallback
        
        speedRatio = 1.0;

        std::cout << "Track duration: " << duration << " seconds" << std::endl;
        std::cout << "Estimated BPM: " << bpm << std::endl;

        readerSource = std::move(newSource);

        if (audioURL.isLocalFile())
            setCurrentTrackPath(audioURL.getLocalFile().getFullPathName());
        else
            setCurrentTrackPath(audioURL.toString(false));

        playing = false;
        transportSource.setPosition(0.0);
        
        resampleSource.flushBuffers();

        loadHotCuesForCurrentTrack();
        loadEQSettingsForCurrentTrack();
    }
}

void DJAudioPlayer::unloadTrack()
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    currentTrackPath.clear();
    bpm = 0.0;
    playing = false;

    for (auto& cue : hotCues)
        cue = -1.0;

    lowEQDb = 0.0;
    midEQDb = 0.0;
    highEQDb = 0.0;

    updateEQFilters();
}

void DJAudioPlayer::setGain (double gain)
{
    if (gain < 0.0 || gain > 1.0)
        return;

    transportSource.setGain((float) gain);
}

void DJAudioPlayer::setSpeed (double ratio)
{
    if (ratio <= 0.0 || ratio > 100.0)
        return;
    
    speedRatio = ratio;
    resampleSource.setResamplingRatio(ratio);
}

void DJAudioPlayer::setPosition (double posInSecs)
{
    transportSource.setPosition(posInSecs);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
    if (pos < 0.0 || pos > 1.0)
        return;

    const double len = transportSource.getLengthInSeconds();
    if (len <= 0.0)
        return;

    setPosition(len * pos);
}

void DJAudioPlayer::nudgePosition(double deltaSeconds)
{
    const double len = transportSource.getLengthInSeconds();
    if (len <= 0.0)
        return;

    const double newPos = juce::jlimit(0.0, len, transportSource.getCurrentPosition() + deltaSeconds);
    transportSource.setPosition(newPos);
}

void DJAudioPlayer::start()
{
    const double length = transportSource.getLengthInSeconds();
    const double pos = transportSource.getCurrentPosition();

    if (length > 0.0 && pos >= length - 0.05)
        transportSource.setPosition(0.0);

    transportSource.start();
    playing = true;
}

void DJAudioPlayer::stop()
{
    transportSource.stop();
    playing = false;
}

bool DJAudioPlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

double DJAudioPlayer::getPositionRelative()
{
    const double length = transportSource.getLengthInSeconds();

    if (length <= 0.0)
        return 0.0;

    const double pos = transportSource.getCurrentPosition();

    if (! transportSource.isPlaying() && pos >= length - 0.05)
        playing = false;

    return juce::jlimit(0.0, 1.0, pos / length);
}

double DJAudioPlayer::getBPM() const
{
    return bpm;
}

double DJAudioPlayer::getSpeed() const
{
    return speedRatio;
}

double DJAudioPlayer::getEffectiveBPM() const
{
    if (bpm <= 0.0)
        return 0.0;

    return bpm * speedRatio;
}

bool DJAudioPlayer::hasTrackLoaded() const
{
    return readerSource != nullptr;
}

bool DJAudioPlayer::syncToTargetBPM(double targetBPM)
{
    if (bpm <= 0.0 || targetBPM <= 0.0)
        return false;

    const double newRatio = juce::jlimit(0.5, 2.0, targetBPM / bpm);
    setSpeed(newRatio);
    return true;
}

double DJAudioPlayer::estimateBPMFromReader(juce::AudioFormatReader& reader) const
{
    if (reader.sampleRate <= 0.0 || reader.lengthInSamples <= 0)
        return 0.0;

    const int64 totalSamples = reader.lengthInSamples;
    const int numChannels = (int) reader.numChannels;

    // Limit analysis to first 120 seconds for speed
    const double analysisSeconds = 120.0;
    const int64 maxSamplesToRead = (int64) std::min<double>(totalSamples, reader.sampleRate * analysisSeconds);

    // Downsample target rate for envelope extraction
    const double envelopeRate = 200.0;
    const int samplesPerEnvelopeFrame = std::max(1, (int) std::round(reader.sampleRate / envelopeRate));

    const int blockSize = 8192;
    juce::AudioBuffer<float> tempBuffer(numChannels, blockSize);

    std::vector<double> envelope;
    envelope.reserve((size_t) (maxSamplesToRead / samplesPerEnvelopeFrame) + 1);

    int64 samplesRead = 0;

    while (samplesRead < maxSamplesToRead)
    {
        const int samplesThisBlock = (int) std::min<int64>(blockSize, maxSamplesToRead - samplesRead);

        if (! reader.read(&tempBuffer, 0, samplesThisBlock, samplesRead, true, true))
            break;

        for (int start = 0; start < samplesThisBlock; start += samplesPerEnvelopeFrame)
        {
            const int frameLen = std::min(samplesPerEnvelopeFrame, samplesThisBlock - start);
            double energy = 0.0;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float* data = tempBuffer.getReadPointer(ch, start);
                for (int i = 0; i < frameLen; ++i)
                {
                    const double s = data[i];
                    energy += s * s;
                }
            }

            energy /= std::max(1, frameLen * numChannels);
            envelope.push_back(std::sqrt(energy));
        }

        samplesRead += samplesThisBlock;
    }

    if (envelope.size() < 32)
        return 0.0;

    // Simple high-pass on envelope: emphasize changes/onsets
    std::vector<double> novelty(envelope.size(), 0.0);
    for (size_t i = 1; i < envelope.size(); ++i)
    {
        const double diff = envelope[i] - envelope[i - 1];
        novelty[i] = diff > 0.0 ? diff : 0.0;
    }

    // Normalize
    double maxVal = *std::max_element(novelty.begin(), novelty.end());
    if (maxVal <= 0.0)
        return 0.0;

    for (auto& v : novelty)
        v /= maxVal;

    // Autocorrelation over plausible BPM range
    const double minBPM = 70.0;
    const double maxBPM = 180.0;

    const int minLag = (int) std::round((60.0 / maxBPM) * envelopeRate);
    const int maxLag = (int) std::round((60.0 / minBPM) * envelopeRate);

    double bestScore = -1.0;
    int bestLag = 0;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double score = 0.0;

        for (size_t i = (size_t) lag; i < novelty.size(); ++i)
            score += novelty[i] * novelty[i - lag];

        // slight preference for musically common tempos
        const double candidateBPM = 60.0 * envelopeRate / (double) lag;
        const double bias = 1.0 - 0.15 * std::abs(candidateBPM - 120.0) / 60.0;

        score *= bias;

        if (score > bestScore)
        {
            bestScore = score;
            bestLag = lag;
        }
    }

    if (bestLag <= 0)
        return 0.0;

    double estimated = 60.0 * envelopeRate / (double) bestLag;

    // Fold extremes toward common DJ range
    while (estimated < 70.0) estimated *= 2.0;
    while (estimated > 180.0) estimated *= 0.5;

    return estimated;
}

double DJAudioPlayer::getCurrentPositionInSeconds() const
{
    return transportSource.getCurrentPosition();
}

double DJAudioPlayer::getTrackLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

void DJAudioPlayer::setCurrentTrackPath(const juce::String& path)
{
    currentTrackPath = path;
}

juce::String DJAudioPlayer::getCurrentTrackPath() const
{
    return currentTrackPath;
}

// -------------------- Hot cues --------------------

void DJAudioPlayer::setHotCue(int index, double positionSeconds)
{
    if (index < 0 || index >= (int) hotCues.size())
        return;

    if (positionSeconds < 0.0)
        return;

    hotCues[(size_t) index] = positionSeconds;
    saveHotCuesForCurrentTrack();
}

void DJAudioPlayer::triggerHotCue(int index)
{
    if (index < 0 || index >= (int) hotCues.size())
        return;

    if (hotCues[(size_t) index] >= 0.0)
        setPosition(hotCues[(size_t) index]);
}

void DJAudioPlayer::updateHotCue(int index)
{
    setHotCue(index, getCurrentPositionInSeconds());
}

void DJAudioPlayer::clearHotCues()
{
    for (auto& cue : hotCues)
        cue = -1.0;

    saveHotCuesForCurrentTrack();
}

double DJAudioPlayer::getHotCue(int index) const
{
    if (index < 0 || index >= (int) hotCues.size())
        return -1.0;

    return hotCues[(size_t) index];
}

bool DJAudioPlayer::hotCueIsSet(int index) const
{
    return getHotCue(index) >= 0.0;
}

juce::File DJAudioPlayer::getHotCueStoreFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("OtoDecks")
        .getChildFile("hotcues.json");
}

void DJAudioPlayer::saveHotCuesForCurrentTrack()
{
    if (currentTrackPath.isEmpty())
        return;

    juce::File file = getHotCueStoreFile();
    file.getParentDirectory().createDirectory();

    juce::var root;
    if (file.existsAsFile())
        root = juce::JSON::parse(file.loadFileAsString());

    juce::DynamicObject* rootObj = root.getDynamicObject();
    if (rootObj == nullptr)
    {
        root = juce::var(new juce::DynamicObject());
        rootObj = root.getDynamicObject();
    }

    juce::Array<juce::var> cueArray;
    for (const auto& cue : hotCues)
        cueArray.add(cue);

    rootObj->setProperty(currentTrackPath, juce::var(cueArray));
    file.replaceWithText(juce::JSON::toString(root));
}

void DJAudioPlayer::loadHotCuesForCurrentTrack()
{
    for (auto& cue : hotCues)
        cue = -1.0;

    if (currentTrackPath.isEmpty())
        return;

    juce::File file = getHotCueStoreFile();
    if (! file.existsAsFile())
        return;

    juce::var root = juce::JSON::parse(file.loadFileAsString());
    juce::DynamicObject* rootObj = root.getDynamicObject();
    if (rootObj == nullptr)
        return;

    juce::var cuesVar = rootObj->getProperty(currentTrackPath);
    if (! cuesVar.isArray())
        return;

    auto* arr = cuesVar.getArray();
    if (arr == nullptr)
        return;

    for (int i = 0; i < juce::jmin((int) arr->size(), 8); ++i)
        hotCues[(size_t) i] = (double) arr->getReference(i);
}

// -------------------- EQ --------------------

void DJAudioPlayer::setLowEQ(double gainDb)
{
    lowEQDb = juce::jlimit(-24.0, 24.0, gainDb);
    updateEQFilters();
    saveEQSettingsForCurrentTrack();
}

void DJAudioPlayer::setMidEQ(double gainDb)
{
    midEQDb = juce::jlimit(-24.0, 24.0, gainDb);
    updateEQFilters();
    saveEQSettingsForCurrentTrack();
}

void DJAudioPlayer::setHighEQ(double gainDb)
{
    highEQDb = juce::jlimit(-24.0, 24.0, gainDb);
    updateEQFilters();
    saveEQSettingsForCurrentTrack();
}

double DJAudioPlayer::getLowEQ() const  { return lowEQDb; }
double DJAudioPlayer::getMidEQ() const  { return midEQDb; }
double DJAudioPlayer::getHighEQ() const { return highEQDb; }

void DJAudioPlayer::resetEQToOriginal()
{
    lowEQDb = 0.0;
    midEQDb = 0.0;
    highEQDb = 0.0;
    updateEQFilters();
    saveEQSettingsForCurrentTrack();
}

void DJAudioPlayer::updateEQFilters()
{
    if (currentSampleRate <= 0.0)
        return;

    *lowShelf.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
        currentSampleRate, 200.0, 0.707f, juce::Decibels::decibelsToGain((float) lowEQDb));

    *midPeak.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        currentSampleRate, 1000.0, 0.707f, juce::Decibels::decibelsToGain((float) midEQDb));

    *highShelf.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        currentSampleRate, 5000.0, 0.707f, juce::Decibels::decibelsToGain((float) highEQDb));
}

juce::File DJAudioPlayer::getEQStoreFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("OtoDecks")
        .getChildFile("eqsettings.json");
}

void DJAudioPlayer::saveEQSettingsForCurrentTrack()
{
    if (currentTrackPath.isEmpty())
        return;

    juce::File file = getEQStoreFile();
    file.getParentDirectory().createDirectory();

    juce::var root;
    if (file.existsAsFile())
        root = juce::JSON::parse(file.loadFileAsString());

    juce::DynamicObject* rootObj = root.getDynamicObject();
    if (rootObj == nullptr)
    {
        root = juce::var(new juce::DynamicObject());
        rootObj = root.getDynamicObject();
    }

    auto* eqObj = new juce::DynamicObject();
    eqObj->setProperty("lowOriginal", 0.0);
    eqObj->setProperty("midOriginal", 0.0);
    eqObj->setProperty("highOriginal", 0.0);
    eqObj->setProperty("lowLast", lowEQDb);
    eqObj->setProperty("midLast", midEQDb);
    eqObj->setProperty("highLast", highEQDb);

    rootObj->setProperty(currentTrackPath, juce::var(eqObj));
    file.replaceWithText(juce::JSON::toString(root));
}

void DJAudioPlayer::loadEQSettingsForCurrentTrack()
{
    lowEQDb = 0.0;
    midEQDb = 0.0;
    highEQDb = 0.0;

    if (currentTrackPath.isEmpty())
    {
        updateEQFilters();
        return;
    }

    juce::File file = getEQStoreFile();
    if (! file.existsAsFile())
    {
        updateEQFilters();
        return;
    }

    juce::var root = juce::JSON::parse(file.loadFileAsString());
    juce::DynamicObject* rootObj = root.getDynamicObject();
    if (rootObj == nullptr)
    {
        updateEQFilters();
        return;
    }

    juce::var eqVar = rootObj->getProperty(currentTrackPath);
    juce::DynamicObject* eqObj = eqVar.getDynamicObject();

    if (eqObj != nullptr)
    {
        lowEQDb  = (double) eqObj->getProperty("lowLast");
        midEQDb  = (double) eqObj->getProperty("midLast");
        highEQDb = (double) eqObj->getProperty("highLast");
    }

    updateEQFilters();
}
