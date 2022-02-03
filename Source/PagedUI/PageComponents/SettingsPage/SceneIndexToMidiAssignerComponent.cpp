/* Copyright (c) 2020-2022, Christian Ahrens
 *
 * This file is part of SpaConBridge <https://github.com/ChristianAhrens/SpaConBridge>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "SceneIndexToMidiAssignerComponent.h"

#include "Image_utils.h"

namespace SpaConBridge
{

//==============================================================================
SceneIndexToMidiAssignerComponent::SceneIndexToMidiAssignerComponent(std::int16_t refId)
{
    setReferredId(refId);

	m_currentMidiAssisLabel = std::make_unique<TextEditor>("CurrentMidiAssisLabel");
    m_currentMidiAssisLabel->setText("0 assignments");
    m_currentMidiAssisLabel->setEnabled(false);
    m_currentMidiAssisLabel->setReadOnly(true);
	addAndMakeVisible(m_currentMidiAssisLabel.get());

    m_editAssignmentsButton = std::make_unique<TextButton>("Edit assignments");
    m_editAssignmentsButton->addListener(this);
	addAndMakeVisible(m_editAssignmentsButton.get());
    lookAndFeelChanged();

    m_addItemComponent = std::make_unique<AddItemCustomComponent>();
    m_addItemComponent->onAddItemClicked = [=](Component* sender) { ignoreUnused(sender); addAssignmentComponent(); };
    
    //startTimer(200);
}

SceneIndexToMidiAssignerComponent::~SceneIndexToMidiAssignerComponent()
{

}

void SceneIndexToMidiAssignerComponent::resized()
{
	auto bounds = getLocalBounds();

    m_currentMidiAssisLabel->setBounds(bounds.removeFromRight(static_cast<int>(0.5f * bounds.getWidth()) - 2));
	bounds.removeFromRight(4);
    m_editAssignmentsButton->setBounds(bounds);
}

void SceneIndexToMidiAssignerComponent::buttonClicked(Button* button)
{
	if (button == m_editAssignmentsButton.get())
	{
        triggerEditAssignments();
	}
}

//void SceneIndexToMidiAssignerComponent::timerCallback()
//{
//    if (isTimerUpdatingPopup())
//        updatePopupMenu();
//}

//void SceneIndexToMidiAssignerComponent::activateMidiInput()
//{
//    if (m_deviceIdentifier.isNotEmpty())
//    {
//        if (m_midiInput && m_midiInput->getDeviceInfo().identifier != m_deviceIdentifier)
//        {
//            DBG(String(__FUNCTION__) + " Deactivating old MIDI input " + m_midiInput->getName() + +" (" + m_midiInput->getIdentifier() + ")");
//            m_midiInput->stop();
//            m_midiInput.reset();
//        }
//        
//        if (m_midiInput && m_midiInput->getDeviceInfo().identifier == m_deviceIdentifier)
//        {
//            DBG(String(__FUNCTION__) + " MIDI input " + m_midiInput->getName() + +" (" + m_midiInput->getIdentifier() + ") is already active.");
//        }
//        else
//        {
//            m_midiInput = juce::MidiInput::openDevice(m_deviceIdentifier, this);
//            m_midiInput->start();
//            DBG(String(__FUNCTION__) + " Activated MIDI input " + m_midiInput->getName() + +" (" + m_midiInput->getIdentifier() + ")");
//        }
//    }
//}

//void SceneIndexToMidiAssignerComponent::deactivateMidiInput()
//{
//    if (m_midiInput)
//    {
//        DBG(String(__FUNCTION__) + " Deactivating MIDI input " + m_midiInput->getName() + +" (" + m_midiInput->getIdentifier() + ")");
//        m_midiInput->stop();
//        m_midiInput.reset();
//    }
//}

/**
 * Reimplemnted from Component to correctly adjust button drawable colouring
 */
void SceneIndexToMidiAssignerComponent::lookAndFeelChanged()
{
    Component::lookAndFeelChanged();

    //auto colourOn = getLookAndFeel().findColour(TextButton::ColourIds::textColourOnId);
    //auto colourOff = getLookAndFeel().findColour(TextButton::ColourIds::textColourOffId);
    //
    //std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
    //JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::school24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
    //    colourOn, colourOff, colourOff, colourOff, colourOn, colourOn, colourOn, colourOn);
    //
    //m_editAssignmentsButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
}

void SceneIndexToMidiAssignerComponent::addAssignmentComponent()
{
    m_assignmentComponents.push_back(std::make_unique<LearnerPopupCustomComponent>(m_referredId, "0.00", JUCEAppBasics::MidiCommandRangeAssignment()));
    updatePopupMenu();
}

void SceneIndexToMidiAssignerComponent::updatePopupMenu()
{
    m_popup.dismissAllActiveMenus();
    m_popup.clear();

    if (m_assignmentComponents.empty())
        m_popup.addItem(1, "No assignments present", false);
    else
    {
        auto resultId = 1;
        for (auto const& assignmentComponent : m_assignmentComponents)
        {
            PopupMenu::Item newItem;
            newItem.setID(resultId);
            newItem.setCustomComponent(assignmentComponent.get());
            m_popup.addItem(newItem);
            resultId++;
        }
    }

    m_popup.addSeparator();

    PopupMenu::Item newItem;
    newItem.setID(-1);
    newItem.setCustomComponent(m_addItemComponent.get());
    m_popup.addItem(newItem);

    m_popup.showMenuAsync(PopupMenu::Options(), [this](int resultingAssiIdx) { 
        /*handlePopupResult();*/ });
}

void SceneIndexToMidiAssignerComponent::triggerEditAssignments()
{
    updatePopupMenu();
}

void SceneIndexToMidiAssignerComponent::handlePopupResult()
{
    for (auto const& assignmentComponent : m_assignmentComponents)
    {
    }

    m_assignmentComponents.clear();

    m_popup.clear();
}

void SceneIndexToMidiAssignerComponent::setSelectedDeviceIdentifier(const String& deviceIdentifier)
{
    // a new deviceIdx cancels all ongoing action
    //deactivateMidiInput();
    m_popup.dismissAllActiveMenus();

    // sanity check of incoming deviceIdx
    auto midiDevicesInfos = juce::MidiInput::getAvailableDevices();
    bool newMidiDeviceFound = false;
    for (auto const& midiDeviceInfo : midiDevicesInfos)
    {
        if (midiDeviceInfo.identifier == deviceIdentifier)
        {
            newMidiDeviceFound = true;
            m_deviceIdentifier = midiDeviceInfo.identifier;
            m_deviceName = midiDeviceInfo.name;
            break;
        }
    }

    if (!newMidiDeviceFound)
    {
        m_deviceIdentifier.clear();
        m_deviceName.clear();
    }
}

void SceneIndexToMidiAssignerComponent::setReferredId(std::int16_t refId)
{
    m_referredId = refId;
}

std::int16_t SceneIndexToMidiAssignerComponent::getReferredId() const
{
    return m_referredId;
}

}
