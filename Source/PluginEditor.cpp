/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String getDSPOptionName(Project13AudioProcessor::DSP_Option option)
{
    switch (option)
    {
    case Project13AudioProcessor::DSP_Option::Phase:
        return "PHASE";
    case Project13AudioProcessor::DSP_Option::Chorus:
        return "CHORUS";
    case Project13AudioProcessor::DSP_Option::OverDrive:
        return "OVERDRIVE";
    case Project13AudioProcessor::DSP_Option::LadderFilter:
        return "LADDERFILTER";
    case Project13AudioProcessor::DSP_Option::GeneralFilter:
        return "GEN FILTER";
    case Project13AudioProcessor::DSP_Option::END_OF_LIST:
        jassertfalse;
    }
    return "NO SELECTION";
}

//==============================================================================
Project13AudioProcessorEditor::ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner) : juce::TabBarButton(name, owner)
{

}

juce::TabBarButton* Project13AudioProcessorEditor::ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    return new ExtendedTabBarButton(tabName, *this);
}
//==============================================================================
Project13AudioProcessorEditor::Project13AudioProcessorEditor (Project13AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    dspOrderButton.onClick = [this]()
    {
        juce::Random r;

        Project13AudioProcessor::DSP_Order dspOrder;

        auto range = juce::Range<int>(static_cast<int>(Project13AudioProcessor::DSP_Option::Phase),
            static_cast<int>(Project13AudioProcessor::DSP_Option::END_OF_LIST));
        tabbedComponent.clearTabs();

        for (auto& v : dspOrder)
        {
            auto entry = r.nextInt(range);
            v = static_cast<Project13AudioProcessor::DSP_Option>(entry);

            tabbedComponent.addTab(getDSPOptionName(v), juce::Colours::lightslategrey, -1);
        }

        DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));

        audioProcessor.dspOrderFifo.push(dspOrder);
        //jassertfalse;
    };
    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(tabbedComponent);
}

Project13AudioProcessorEditor::~Project13AudioProcessorEditor()
{
}

//==============================================================================
void Project13AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Project13AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150, 30));
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.withHeight(30));
}
