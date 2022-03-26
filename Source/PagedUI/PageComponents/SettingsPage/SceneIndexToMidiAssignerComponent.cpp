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

void SceneIndexToMidiAssignerComponent::addAssignmentComponent()
{
    m_assignmentComponents.push_back(std::make_unique<LearnerPopupCustomComponent>(m_referredId, "1.00", m_deviceIdentifier, JUCEAppBasics::MidiCommandRangeAssignment()));
    m_assignmentComponents.back()->onAssignmentSet = [this](Component* sender, const String& sceneIndex, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssignment) {
        ignoreUnused(sender);
        ignoreUnused(sceneIndex);
        ignoreUnused(midiAssignment);

        std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> scenesToMidiAssi;
        for (auto const& assignmentComponent : m_assignmentComponents)
            scenesToMidiAssi.insert(std::make_pair(assignmentComponent->GetSceneIndex(), assignmentComponent->GetCurrentAssignment()));

        if (onAssignmentsSet)
            onAssignmentsSet(this, scenesToMidiAssi);

        if (m_currentMidiAssisLabel)
            m_currentMidiAssisLabel->setText(String(scenesToMidiAssi.size()) + " assignments");
    };
    
    m_popup.dismissAllActiveMenus();
   
    updatePopupMenu();
}

void SceneIndexToMidiAssignerComponent::updatePopupMenu()
{
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
        handlePopupResult(resultingAssiIdx); });
}

void SceneIndexToMidiAssignerComponent::triggerEditAssignments()
{
    updatePopupMenu();
}

void SceneIndexToMidiAssignerComponent::handlePopupResult(int resultingAssiIdx)
{
    std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> scenesToMidiAssi;
    for (auto const& assignmentComponent : m_assignmentComponents)
    {
        scenesToMidiAssi.insert(std::make_pair(assignmentComponent->GetSceneIndex(), assignmentComponent->GetCurrentAssignment()));
    }

    if (onAssignmentsSet)
        onAssignmentsSet(this, scenesToMidiAssi);

                                DBG(String(__FUNCTION__) + " resultingAssiIdx:" + String(resultingAssiIdx));
                                return;

    m_assignmentComponents.clear();

    m_popup.clear();
}

void SceneIndexToMidiAssignerComponent::setSelectedDeviceIdentifier(const String& deviceIdentifier)
{
    // a new deviceIdx cancels all ongoing action
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
