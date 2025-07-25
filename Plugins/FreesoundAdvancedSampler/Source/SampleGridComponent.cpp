/*
  ==============================================================================

    SampleGridComponent.cpp
    Created: New grid component for displaying samples in 4x4 grid
    Author: Generated

  ==============================================================================
*/

#include "SampleGridComponent.h"

//==============================================================================
// SamplePad Implementation
//==============================================================================

SamplePad::SamplePad(int index)
    : padIndex(index)
    , processor(nullptr)
    , audioThumbnailCache(5)
    , audioThumbnail(512, formatManager, audioThumbnailCache)
    , freesoundId(String())
    , licenseType(String())
    , playheadPosition(0.0f)
    , isPlaying(false)
    , hasValidSample(false)
{
    formatManager.registerBasicFormats();

    // Generate a unique color for each pad
    float hue = (float)padIndex / 16.0f;
    padColour = Colour::fromHSV(hue, 0.3f, 0.8f, 1.0f);
}

SamplePad::~SamplePad()
{
}

void SamplePad::paint(Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    if (isPlaying)
    {
        g.setColour(padColour.brighter(0.3f));
    }
    else if (hasValidSample)
    {
        g.setColour(padColour);
    }
    else
    {
        g.setColour(Colours::darkgrey);
    }

    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

    // Border
    g.setColour(Colours::white.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(1), 4.0f, 1.0f);

    if (hasValidSample)
    {
        auto waveformBounds = bounds.reduced(8, 20);

        // Draw waveform
        drawWaveform(g, waveformBounds);

        // Draw playhead if playing
        if (isPlaying)
        {
            drawPlayhead(g, waveformBounds);
        }

        // Draw filename and author in black text at bottom left of waveform area
        g.setColour(Colours::black);
        g.setFont(9.0f);

        // Get the waveform bounds (same as used for drawing waveform)
        auto textBounds = bounds.reduced(8, 20);

        // Format filename to max 10 characters with ellipsis if needed
        String displayName = sampleName;
        if (displayName.length() > 10)
        {
            displayName = displayName.substring(0, 7) + "...";
        }

        // Format author name
        String displayAuthor = authorName;
        if (displayAuthor.length() > 10)
        {
            displayAuthor = displayAuthor.substring(0, 7) + "...";
        }

        // Create the full text string
        String displayText = displayName + " by " + displayAuthor;

        // Position at bottom left of waveform area
        auto filenameBounds = textBounds.removeFromBottom(12);
        g.drawText(displayText, filenameBounds, Justification::bottomLeft, true);

        // Draw "Web" badge in TOP LEFT corner
        if (freesoundId.isNotEmpty())
        {
            g.setFont(9.0f);

            // Create web badge in top-left corner
            auto webBounds = bounds.reduced(3);
            int badgeWidth = 30;
            int badgeHeight = 12;
            webBounds = webBounds.removeFromTop(badgeHeight).removeFromLeft(badgeWidth);

            // Draw blue background
            g.setColour(Colours::blue.withAlpha(0.8f));
            g.fillRoundedRectangle(webBounds.toFloat(), 2.0f);

            // Draw white text
            g.setColour(Colours::white);
            g.drawText("Web", webBounds, Justification::centred);
        }

        // Draw license badge in TOP RIGHT corner
        if (licenseType.isNotEmpty())
        {
            g.setFont(9.0f);
            String shortLicense = getLicenseShortName(licenseType);

            // Create license badge in top-right corner
            auto licenseBounds = bounds.reduced(3);
            int badgeWidth = 35;
            int badgeHeight = 12;
            licenseBounds = licenseBounds.removeFromTop(badgeHeight).removeFromRight(badgeWidth);

            // Draw orange background
            g.setColour(Colours::orange.withAlpha(0.9f));
            g.fillRoundedRectangle(licenseBounds.toFloat(), 2.0f);

            // Draw black text
            g.setColour(Colours::black);
            g.drawText(shortLicense, licenseBounds, Justification::centred);
        }

        // Draw "Drag" badge in BOTTOM LEFT corner
        {
            g.setFont(9.0f);

            // Create drag badge in bottom-left corner
            auto dragBounds = bounds.reduced(3);
            int badgeWidth = 30;
            int badgeHeight = 12;
            dragBounds = dragBounds.removeFromBottom(badgeHeight).removeFromLeft(badgeWidth);

            // Draw green background
            g.setColour(Colours::green.withAlpha(0.8f));
            g.fillRoundedRectangle(dragBounds.toFloat(), 2.0f);

            // Draw white text
            g.setColour(Colours::white);
            g.drawText("Drag", dragBounds, Justification::centred);
        }
    }
    else
    {
        // Empty pad
        g.setColour(Colours::white.withAlpha(0.5f));
        g.setFont(12.0f);
        g.drawText("Empty", bounds, Justification::centred);
    }

    // Pad number in top-left corner (over the web badge if present)
    g.setColour(Colours::white.withAlpha(0.9f));
    g.setFont(8.0f);
    auto numberBounds = bounds.reduced(2);
    auto numberRect = numberBounds.removeFromTop(10).removeFromLeft(15);
    g.drawText(String(padIndex + 1), numberRect, Justification::centred);
}

void SamplePad::resized()
{
}

void SamplePad::mouseDown(const MouseEvent& event)
{
    if (!hasValidSample)
        return;

    // Check if clicked on web badge (top-left)
    if (freesoundId.isNotEmpty())
    {
        auto bounds = getLocalBounds();
        auto webBounds = bounds.reduced(3);
        int badgeWidth = 30;
        int badgeHeight = 12;
        webBounds = webBounds.removeFromTop(badgeHeight).removeFromLeft(badgeWidth);

        if (webBounds.contains(event.getPosition()))
        {
            // Open Freesound page in browser
            String freesoundUrl = "https://freesound.org/s/" + freesoundId + "/";
            URL(freesoundUrl).launchInDefaultBrowser();
            return;
        }
    }

    // Check if clicked on license badge (top-right)
    if (licenseType.isNotEmpty())
    {
        auto bounds = getLocalBounds();
        auto licenseBounds = bounds.reduced(3);
        int badgeWidth = 35;
        int badgeHeight = 12;
        licenseBounds = licenseBounds.removeFromTop(badgeHeight).removeFromRight(badgeWidth);

        if (licenseBounds.contains(event.getPosition()))
        {
            // Open the actual Creative Commons license URL
            URL(licenseType).launchInDefaultBrowser();
            return;
        }
    }

    // Check if clicked on drag badge (bottom-left)
    {
        auto bounds = getLocalBounds();
        auto dragBounds = bounds.reduced(3);
        int badgeWidth = 30;
        int badgeHeight = 12;
        dragBounds = dragBounds.removeFromBottom(badgeHeight).removeFromLeft(badgeWidth);

        if (dragBounds.contains(event.getPosition()))
        {
            // Start drag operation for this single file
            if (audioFile.existsAsFile())
            {
                StringArray filePaths;
                filePaths.add(audioFile.getFullPathName());
                performExternalDragDropOfFiles(filePaths, false); // false = don't allow moving
            }
            return;
        }
    }

    // Normal click - play the sample
    if (processor)
    {
        int noteNumber = padIndex + 36;
        processor->addToMidiBuffer(noteNumber);
    }
}

void SamplePad::setSample(const File& file, const String& name, const String& author, String fsId, String license)
{
    audioFile = file;
    sampleName = name;
    authorName = author;
    freesoundId = fsId;
    licenseType = license;

    loadWaveform();
    hasValidSample = audioFile.existsAsFile();
    repaint();
}

void SamplePad::loadWaveform()
{
    if (audioFile.existsAsFile())
    {
        audioThumbnail.setSource(new FileInputSource(audioFile));
    }
}

void SamplePad::drawWaveform(Graphics& g, Rectangle<int> bounds)
{
    if (!hasValidSample || audioThumbnail.getTotalLength() == 0.0)
        return;

    g.setColour(Colours::black.withAlpha(0.3f));
    g.fillRect(bounds);

    g.setColour(Colours::white.withAlpha(0.8f));
    audioThumbnail.drawChannels(g, bounds, 0.0, audioThumbnail.getTotalLength(), 1.0f);
}

void SamplePad::drawPlayhead(Graphics& g, Rectangle<int> bounds)
{
    if (!isPlaying || !hasValidSample)
        return;

    float x = bounds.getX() + (playheadPosition * bounds.getWidth());

    g.setColour(Colours::red);
    g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 2.0f);
}

void SamplePad::setPlayheadPosition(float position)
{
    // Ensure GUI updates happen on the message thread
    MessageManager::callAsync([this, position]()
    {
        playheadPosition = jlimit(0.0f, 1.0f, position);
        repaint();
    });
}

void SamplePad::setIsPlaying(bool playing)
{
    // Ensure GUI updates happen on the message thread
    MessageManager::callAsync([this, playing]()
    {
        isPlaying = playing;
        repaint();
    });
}

void SamplePad::setProcessor(FreesoundAdvancedSamplerAudioProcessor* p)
{
    processor = p;
}

SamplePad::SampleInfo SamplePad::getSampleInfo() const
{
    SampleInfo info;
    info.audioFile = audioFile;
    info.sampleName = sampleName;
    info.authorName = authorName;
    info.freesoundId = freesoundId;
    info.licenseType = licenseType;
    info.hasValidSample = hasValidSample;
    info.padIndex = 0; // Will be set by SampleGridComponent
    return info;
}

String SamplePad::getLicenseShortName(const String& license) const
{
    // Parse the actual Creative Commons URLs to determine license type
    String lowerLicense = license.toLowerCase();

    if (lowerLicense.contains("creativecommons.org/publicdomain/zero") ||
        lowerLicense.contains("cc0") ||
        lowerLicense.contains("publicdomain/zero"))
        return "cc0";
    else if (lowerLicense.contains("creativecommons.org/licenses/by-nc") ||
             lowerLicense.contains("by-nc"))
        return "by-nc";
    else if (lowerLicense.contains("creativecommons.org/licenses/by/") ||
             (lowerLicense.contains("creativecommons.org/licenses/by") && !lowerLicense.contains("-nc")))
        return "by";
    else if (lowerLicense.contains("sampling+"))
        return "by-nc"; // Sampling+ is treated as by-nc according to Freesound
    else
        return "by-nc"; // Default to most restrictive for unknown licenses
}

//==============================================================================
// SampleGridComponent Implementation
//==============================================================================

SampleGridComponent::SampleGridComponent()
    : processor(nullptr)
{
    // Create and add sample pads
    for (int i = 0; i < TOTAL_PADS; ++i)
    {
        samplePads[i] = std::make_unique<SamplePad>(i);
        addAndMakeVisible(*samplePads[i]);
    }
}

SampleGridComponent::~SampleGridComponent()
{
    if (processor)
    {
        processor->removePlaybackListener(this);
    }
}

void SampleGridComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void SampleGridComponent::resized()
{
    auto bounds = getLocalBounds();
    int padding = 4;
    int totalPadding = padding * (GRID_SIZE + 1);

    int padWidth = (bounds.getWidth() - totalPadding) / GRID_SIZE;
    int padHeight = (bounds.getHeight() - totalPadding) / GRID_SIZE;

    for (int row = 0; row < GRID_SIZE; ++row)
    {
        for (int col = 0; col < GRID_SIZE; ++col)
        {
            // Calculate pad index starting from bottom left
            // Bottom row (row 3) = pads 0-3, next row up (row 2) = pads 4-7, etc.
            int padIndex = (GRID_SIZE - 1 - row) * GRID_SIZE + col;

            int x = padding + col * (padWidth + padding);
            int y = padding + row * (padHeight + padding);

            samplePads[padIndex]->setBounds(x, y, padWidth, padHeight);
        }
    }
}

void SampleGridComponent::setProcessor(FreesoundAdvancedSamplerAudioProcessor* p)
{
    if (processor)
    {
        processor->removePlaybackListener(this);
    }

    processor = p;

    if (processor)
    {
        processor->addPlaybackListener(this);
    }

    // Set processor for all pads
    for (auto& pad : samplePads)
    {
        pad->setProcessor(processor);
    }
}

void SampleGridComponent::updateSamples(const Array<FSSound>& sounds, const std::vector<StringArray>& soundInfo)
{
    clearSamples();

    if (!processor)
        return;

    File downloadDir = processor->getCurrentDownloadLocation();

    // Try to load from JSON metadata first
    File metadataFile = downloadDir.getChildFile("metadata.json");
    if (metadataFile.existsAsFile())
    {
        loadSamplesFromJson(metadataFile);
    }
    else
    {
        // Fallback to the old method using the provided arrays
        loadSamplesFromArrays(sounds, soundInfo, downloadDir);
    }
}

void SampleGridComponent::loadSamplesFromJson(const File& metadataFile)
{
    FileInputStream inputStream(metadataFile);
    if (!inputStream.openedOk())
        return;

    String jsonText = inputStream.readEntireStreamAsString();
    var parsedJson = juce::JSON::parse(jsonText);

    if (!parsedJson.isObject())
        return;

    var samplesArray = parsedJson.getProperty("samples", var());
    if (!samplesArray.isArray())
        return;

    File downloadDir = metadataFile.getParentDirectory();
    int numSamples = jmin(TOTAL_PADS, (int)samplesArray.size());

    for (int i = 0; i < numSamples; ++i)
    {
        var sample = samplesArray[i];
        if (!sample.isObject())
            continue;

        String fileName = sample.getProperty("file_name", "");
        String sampleName = sample.getProperty("original_name", "Sample " + String(i + 1));
        String authorName = sample.getProperty("author", "Unknown");
        String license = sample.getProperty("license", "Unknown");
        String freesoundId = sample.getProperty("freesound_id", "");

        if (fileName.isNotEmpty())
        {
            File audioFile = downloadDir.getChildFile(fileName);
            if (audioFile.existsAsFile())
            {
                samplePads[i]->setSample(audioFile, sampleName, authorName, freesoundId, license);
            }
        }
    }
}

void SampleGridComponent::loadSamplesFromArrays(const Array<FSSound>& sounds, const std::vector<StringArray>& soundInfo, const File& downloadDir)
{
    int numSamples = jmin(TOTAL_PADS, sounds.size());

    for (int i = 0; i < numSamples; ++i)
    {
        String sampleName = (i < soundInfo.size()) ? soundInfo[i][0] : "Sample " + String(i + 1);
        String authorName = (i < soundInfo.size()) ? soundInfo[i][1] : "Unknown";
        String license = (i < soundInfo.size() && soundInfo[i].size() > 2) ? soundInfo[i][2] : "Unknown";
        String freesoundId = sounds[i].id;

        // Create filename using simple ID-based naming scheme: FS_ID_XXXX.ogg
        String expectedFilename = "FS_ID_" + freesoundId + ".ogg";
        File audioFile = downloadDir.getChildFile(expectedFilename);

        // Only proceed if the file exists
        if (audioFile.existsAsFile())
        {
            samplePads[i]->setSample(audioFile, sampleName, authorName, freesoundId, license);
        }
    }
}

void SampleGridComponent::clearSamples()
{
    for (auto& pad : samplePads)
    {
        pad->setSample(File(), "", "", String(), String());
    }
}

void SampleGridComponent::noteStarted(int noteNumber, float velocity)
{
    // Convert MIDI note back to pad index
    int padIndex = noteNumber - 36; // Note 36 = pad 0, Note 37 = pad 1, etc.
    if (padIndex >= 0 && padIndex < TOTAL_PADS)
    {
        // These methods now handle message thread dispatching internally
        samplePads[padIndex]->setIsPlaying(true);
        samplePads[padIndex]->setPlayheadPosition(0.0f);
    }
}

void SampleGridComponent::noteStopped(int noteNumber)
{
    // Convert MIDI note back to pad index
    int padIndex = noteNumber - 36;
    if (padIndex >= 0 && padIndex < TOTAL_PADS)
    {
        // This method now handles message thread dispatching internally
        samplePads[padIndex]->setIsPlaying(false);
    }
}

void SampleGridComponent::playheadPositionChanged(int noteNumber, float position)
{
    // Convert MIDI note back to pad index
    int padIndex = noteNumber - 36;
    if (padIndex >= 0 && padIndex < TOTAL_PADS)
    {
        // This method now handles message thread dispatching internally
        samplePads[padIndex]->setPlayheadPosition(position);
    }
}

Array<SamplePad::SampleInfo> SampleGridComponent::getAllSampleInfo() const
{
    Array<SamplePad::SampleInfo> allSamples;

    for (int i = 0; i < samplePads.size(); ++i)
    {
        SamplePad::SampleInfo info = samplePads[i]->getSampleInfo();
        if (info.hasValidSample)
        {
            // Add pad index to the info
            info.padIndex = i + 1; // 1-based index for user-friendly numbering
            allSamples.add(info);
        }
    }

    return allSamples;
}

//==============================================================================
// SampleDragArea Implementation
//==============================================================================

SampleDragArea::SampleDragArea()
    : sampleGrid(nullptr)
    , isDragging(false)
{
    setSize(80, 40);
}

SampleDragArea::~SampleDragArea()
{
}

void SampleDragArea::paint(Graphics& g)
{
    auto bounds = getLocalBounds();

    // Background
    if (isDragging)
    {
        g.setColour(Colours::blue.withAlpha(0.3f));
    }
    else
    {
        g.setColour(Colours::grey.withAlpha(0.2f));
    }
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);

    // Border
    g.setColour(Colours::white.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(1), 4.0f, 1.0f);

    // Icon and text
    g.setColour(Colours::white.withAlpha(0.8f));
    g.setFont(10.0f);

    // Draw a simple drag icon (three horizontal lines)
    auto iconBounds = bounds.removeFromLeft(20);
    int lineY = iconBounds.getCentreY() - 6;
    for (int i = 0; i < 3; ++i)
    {
        g.fillRect(iconBounds.getX() + 4, lineY + i * 4, 12, 2);
    }

    // Draw text
    g.drawText("Drag\nSamples", bounds, Justification::centred);
}

void SampleDragArea::resized()
{
}

void SampleDragArea::mouseDown(const MouseEvent& event)
{
    if (!sampleGrid)
        return;

    // Check if we have any samples to drag
    auto samples = sampleGrid->getAllSampleInfo();
    if (samples.isEmpty())
        return;

    isDragging = true;
    repaint();
}

void SampleDragArea::mouseDrag(const MouseEvent& event)
{
    if (!sampleGrid || !isDragging)
        return;

    auto samples = sampleGrid->getAllSampleInfo();
    if (samples.isEmpty())
        return;

    // Create StringArray of file paths for drag operation
    StringArray filePaths;

    for (const auto& sample : samples)
    {
        if (sample.hasValidSample && sample.audioFile.existsAsFile())
        {
            // Reference the existing file directly from the subfolder
            filePaths.add(sample.audioFile.getFullPathName());
        }
    }

    if (!filePaths.isEmpty())
    {
        // Start the drag operation with existing files
        performExternalDragDropOfFiles(filePaths, false); // false = don't allow moving
    }

    isDragging = false;
    repaint();
}

void SampleDragArea::setSampleGridComponent(SampleGridComponent* gridComponent)
{
    sampleGrid = gridComponent;
}

//==============================================================================
// DirectoryOpenButton Implementation
//==============================================================================

DirectoryOpenButton::DirectoryOpenButton()
    : processor(nullptr)
{
    setButtonText("View\nFiles");
    setSize(60, 40);

    // Set up the click callback using lambda
    onClick = [this]() {
        if (!processor)
            return;

        File currentFolder = processor->getCurrentDownloadLocation();

        if (currentFolder.exists())
        {
            currentFolder.revealToUser();
        }
        else
        {
            // If current session folder doesn't exist, open the main download directory
            File mainFolder = processor->tmpDownloadLocation;
            if (mainFolder.exists())
            {
                mainFolder.revealToUser();
            }
            else
            {
                // Create and open the main directory if it doesn't exist
                mainFolder.createDirectory();
                mainFolder.revealToUser();
            }
        }
    };
}

DirectoryOpenButton::~DirectoryOpenButton()
{
}

void DirectoryOpenButton::setProcessor(FreesoundAdvancedSamplerAudioProcessor* p)
{
    processor = p;
}