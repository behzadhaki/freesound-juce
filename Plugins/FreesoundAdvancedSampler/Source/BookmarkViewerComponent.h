/*
  ==============================================================================

    BookmarkViewerComponent.h
    Created: Bookmark viewer component with scrollable sample pads
    Author: Generated

  ==============================================================================
*/

#pragma once

#include "shared_plugin_helpers/shared_plugin_helpers.h"
#include "SampleGridComponent.h"
#include "BookmarkManager.h"
#include "PluginProcessor.h"
#include "CustomButtonStyle.h"

// Add forward declaration
class FreesoundAdvancedSamplerAudioProcessorEditor;

//==============================================================================
// Custom Sample Pad for Bookmarks (with click callback)
//==============================================================================
class BookmarkSamplePad : public SamplePad
{
public:
    BookmarkSamplePad(int index, const BookmarkInfo& bookmarkInfo);
    ~BookmarkSamplePad() override;

    std::function<void(const BookmarkInfo&)> onBookmarkClicked;

private:
    BookmarkInfo bookmark;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BookmarkSamplePad)
};

//==============================================================================
// Bookmark Viewer Component
//==============================================================================
class BookmarkViewerComponent : public Component,
                               public ScrollBar::Listener
{
public:
    BookmarkViewerComponent();
    ~BookmarkViewerComponent() override;

    void paint(Graphics& g) override;
    void resized() override;

    void setProcessor(FreesoundAdvancedSamplerAudioProcessor* p);
    void refreshBookmarks();

    // ScrollBar::Listener
    void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

private:
    FreesoundAdvancedSamplerAudioProcessor* processor;

    // UI Components
    Label titleLabel;
    StyledButton refreshButton { "Refresh", 10.0f, false }; // ↻ refresh symbol
    Viewport bookmarkViewport;
    Component bookmarkContainer;

    // Sample pads for bookmarks (non-searchable)
    OwnedArray<BookmarkSamplePad> bookmarkPads;

    // Current bookmark data
    Array<BookmarkInfo> currentBookmarks;

    // Layout management
    int currentScrollOffset = 0;
    int totalRows = 0;

    void updateBookmarkPads();
    void createBookmarkPads();
    void clearBookmarkPads();
    void updateScrollableArea();
    void loadBookmarkIntoSampleGrid(const BookmarkInfo& bookmark);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BookmarkViewerComponent)
};