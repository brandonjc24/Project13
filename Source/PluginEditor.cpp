/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String getNameFromDSPOption(Project13AudioProcessor::DSP_Option option)
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

static Project13AudioProcessor::DSP_Option getDSPOptionFromName(juce::String name)
{
    if (name == "PHASE")
        return Project13AudioProcessor::DSP_Option::Phase;
    if (name == "CHORUS")
        return Project13AudioProcessor::DSP_Option::Chorus;
    if (name == "OVERDRIVE")
        return Project13AudioProcessor::DSP_Option::OverDrive;
    if (name == "LADDERFILTER")
        return Project13AudioProcessor::DSP_Option::LadderFilter;
    if (name == "GEN FILTER")
        return Project13AudioProcessor::DSP_Option::GeneralFilter;

    return Project13AudioProcessor::DSP_Option::END_OF_LIST;
}

//==============================================================================
ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner, Project13AudioProcessor::DSP_Option o) :
    juce::TabBarButton(name, owner),
    option(o)
{
    constrainer = std::make_unique<HorizontalConstrainer>([&owner]()
        {
            return owner.getLocalBounds();
        },
        [this]()
        {
            return getBounds();
        });
    constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);

}

int ExtendedTabBarButton::getBestTabLength(int depth)
{
    auto bestWidth = getLookAndFeel().getTabButtonBestWidth(*this, depth);

    auto& bar = getTabbedButtonBar();
    /*
     we want the tabs to occupy the entire TabBar width.
     so, after computing the best width for the button and depth,
     we choose whichever value is bigger, the bestWidth, or an equal division of the bar's width based on the number of tabs in the bar.
     */
    return juce::jmax(bestWidth,
        bar.getWidth() / bar.getNumTabs());
}

//==============================================================================
ExtendedTabbedButtonBar::ExtendedTabbedButtonBar() :
    juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    auto img = juce::Image(juce::Image::PixelFormat::SingleChannel, 1, 1, true);

    auto gfx = juce::Graphics(img);
    gfx.fillAll(juce::Colours::transparentBlack);

    dragImage = juce::ScaledImage(img, 1.0);
}

bool ExtendedTabbedButtonBar::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    if (dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
        return true;

    return false;
}

void ExtendedTabbedButtonBar::itemDragExit(const SourceDetails& dragSourceDetails)
{
    //just call base class for now
    DBG("ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragExit(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDropped(const SourceDetails& dragSourceDetails)
{
    DBG("item dropped");
    //find the dropped item.  lock the position in.
    resized();

    //notify of the new tab order
    auto tabs = getTabs();
    Project13AudioProcessor::DSP_Order newOrder;

    jassert(tabs.size() == newOrder.size());
    for (size_t i = 0; i < tabs.size(); ++i)
    {
        auto tab = tabs[static_cast<int>(i)];
        if (auto* etbb = dynamic_cast<ExtendedTabBarButton*>(tab))
        {
            newOrder[i] = etbb->getOption();
        }
    }

    listeners.call([newOrder](Listener& l)
        {
            l.tabOrderChanged(newOrder);
        });
}

void ExtendedTabbedButtonBar::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDragEnter");
    //just call base class for now
    juce::DragAndDropTarget::itemDragEnter(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDragMove(const SourceDetails& dragSourceDetails)
{
    DBG("ETBB::itemDragMove");
    if (auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        //find tabBarBeingDragged in the tabs[] container.
        //tabs[] is private so you must use:
        //TabBarButton* getTabButton (int index) const
        //and
        //getNumTabs()
        //to first get a list of tabs to search through

        auto idx = findDraggedItemIndex(dragSourceDetails);
        if (idx == -1)
        {
            DBG("failed to find tab being dragged in list of tabs");
            jassertfalse;
            return;
        }

        //find the tab that tabBarBeingDragged is colliding with.
        //it might be on the right
        //it might be on the left
        //if it's on the right,
        //if tabBarBeingDragged's x is > nextTab.getX() + nextTab.getWidth() * 0.5, swap their position
        auto previousTabIndex = idx - 1;
        auto nextTabIndex = idx + 1;
        auto previousTab = getTabButton(previousTabIndex);
        auto nextTab = getTabButton(nextTabIndex);

        if (previousTab == nullptr && nextTab != nullptr)
        {
            //you're in the 0th position
            if (tabBarBeingDragged->getX() + tabBarBeingDragged->getWidth() > nextTab->getX() + nextTab->getWidth() * 0.5)
            {
                moveTab(idx, nextTabIndex);
            }
        }
        else if (previousTab != nullptr && nextTab == nullptr)
        {
            //you're in the last position
            if (tabBarBeingDragged->getX() < previousTab->getX() + previousTab->getWidth() * 0.5)
            {
                moveTab(idx, previousTabIndex);
            }
        }
        else
        {
            //you're in the middle
            if ((tabBarBeingDragged->getX() + tabBarBeingDragged->getWidth()) > nextTab->getX() + nextTab->getWidth() * 0.5)
            {
                moveTab(idx, nextTabIndex);
            }
            else if (tabBarBeingDragged->getX() < previousTab->getX() + previousTab->getWidth() * 0.5)
            {
                moveTab(idx, previousTabIndex);
            }
        }
        tabBarBeingDragged->toFront(true);
    }
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent& e)
{
    DBG("ExtendedTabbedButtonBar::mouseDown");
    if (auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>(e.originalComponent))
    {
        startDragging(tabBarBeingDragged->TabBarButton::getTitle(),
            tabBarBeingDragged,
            dragImage);
    }
}

void Project13AudioProcessorEditor::tabOrderChanged(Project13AudioProcessor::DSP_Order newOrder)
{
    audioProcessor.dspOrderFifo.push(newOrder);
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    auto dspOption = getDSPOptionFromName(tabName);
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this, dspOption);
    etbb->addMouseListener(this, false);

    return etbb.release();
}

void ExtendedTabbedButtonBar::addListener(ExtendedTabbedButtonBar::Listener* l)
{
    listeners.add(l);
}

void ExtendedTabbedButtonBar::removeListener(ExtendedTabbedButtonBar::Listener* l)
{
    listeners.remove(l);
}

juce::TabBarButton* ExtendedTabbedButtonBar::findDraggedItem(const SourceDetails& dragSourceDetails)
{
    return getTabButton(findDraggedItemIndex(dragSourceDetails));
}


int ExtendedTabbedButtonBar::findDraggedItemIndex(const SourceDetails& dragSourceDetails)
{
    if (auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        auto tabs = getTabs();

        //now search
        auto idx = tabs.indexOf(tabBarBeingDragged);
        return idx;
    }

    return -1;
}

juce::Array<juce::TabBarButton*> ExtendedTabbedButtonBar::getTabs()
{
    auto numTabs = getNumTabs();
    auto tabs = juce::Array<juce::TabBarButton*>();
    tabs.resize(numTabs);
    for (int i = 0; i < numTabs; ++i)
    {
        tabs.getReference(i) = getTabButton(i);
    }

    return tabs;
}

HorizontalConstrainer::HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter, std::function<juce::Rectangle<int>()> confineeBoundsGetter) : boundsToConfineToGetter(confinerBoundsGetter), boundsOfConfineeGetter(confineeBoundsGetter)
{

}

void HorizontalConstrainer::checkBounds(juce::Rectangle<int>& bounds,
    const juce::Rectangle<int>& previousBounds,
    const juce::Rectangle<int>& limits,
    bool isStretchingTop,
    bool isStretchingLeft,
    bool isStretchingBottom,
    bool isStretchingRight)
{
    /*
     'bounds' is the bounding box that we are TRYING to set componentToConfine to.
     we only want to support horizontal dragging within the TabButtonBar.

     so, retain the existing Y position given to the TabBarButton by the TabbedButtonBar when the button was created.
     */
    bounds.setY(previousBounds.getY());
    /*
     the X position needs to be limited to the left and right side of the owning TabbedButtonBar.
     however, to prevent the right side of the TabBarButton from being dragged outside the bounds of the TabbedButtonBar, we must subtract the width of this button from the right side of the TabbedButtonBar

     in order for this to work, we need to know the bounds of both the TabbedButtonBar and the TabBarButton.
     hence, loose coupling using lambda getter functions via the constructor parameters.
     Loose coupling is preferred vs tight coupling.
     */

    if (boundsToConfineToGetter != nullptr &&
        boundsOfConfineeGetter != nullptr)
    {
        auto boundsToConfineTo = boundsToConfineToGetter();
        auto boundsOfConfinee = boundsOfConfineeGetter();

        bounds.setX(juce::jlimit(boundsToConfineTo.getX(),
            boundsToConfineTo.getRight() - boundsOfConfinee.getWidth(),
            bounds.getX()));
    }
    else
    {
        bounds.setX(juce::jlimit(limits.getX(),
            limits.getY(),
            bounds.getX()));
    }
}
//==============================================================================
Project13AudioProcessorEditor::Project13AudioProcessorEditor (Project13AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGUI);

    tabbedComponent.addListener(this);
    startTimerHz(30);
    setSize(600, 400);
}

Project13AudioProcessorEditor::~Project13AudioProcessorEditor()
{
    tabbedComponent.removeListener(this);
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
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.removeFromTop(30));
    dspGUI.setBounds(bounds);
}

void Project13AudioProcessorEditor::timerCallback()
{

    if (audioProcessor.restoreDspOrderFifo.getNumAvailableForReading() == 0)
        return;

    using T = Project13AudioProcessor::DSP_Order;
    T newOrder;
    newOrder.fill(Project13AudioProcessor::DSP_Option::END_OF_LIST);
    auto empty = newOrder;
    while (audioProcessor.restoreDspOrderFifo.pull(newOrder))
    {
        ; // do nothing.   you'll do something with the most recently pulled order
    }

    if (newOrder != empty) //if you pulled nothing, newOrder will be filled with END_OF_LIST
    {

        addTabsFromDSPOrder(newOrder);
    }
}

void Project13AudioProcessorEditor::addTabsFromDSPOrder(Project13AudioProcessor::DSP_Order newOrder)
{
    tabbedComponent.clearTabs();
    for (auto v : newOrder)
    {
        tabbedComponent.addTab(getNameFromDSPOption(v), juce::Colours::white, -1);
    }

    //if the order is identical to the current order used by the audio side, this push will do nothing.
    audioProcessor.dspOrderFifo.push(newOrder);
}