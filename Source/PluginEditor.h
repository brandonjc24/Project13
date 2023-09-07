/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Project13AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Project13AudioProcessorEditor (Project13AudioProcessor&);
    ~Project13AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


    /*
     https://forum.juce.com/t/draggabletabbedcomponent/13265/5?u=matkatmusic
     "You can create a subclass of TabbedButtonBar which is also a DragAndDropTarget. And you can create custom tab buttons which allow themselves to be dragged."
     */
    struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget
    {
        ExtendedTabbedButtonBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop) { }

        bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override { return false; }
        void itemDropped(const SourceDetails& dragSourceDetails) override { }
    };

    struct ExtendedTabBarButton : juce::TabBarButton
    {

    };

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Project13AudioProcessor& audioProcessor;
    juce::TextButton dspOrderButton{ "dsp order" };

    //    juce::TabbedComponent tabbedComponent { juce::TabbedButtonBar::Orientation::TabsAtTop };
    ExtendedTabbedButtonBar tabbedComponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project13AudioProcessorEditor)
};
