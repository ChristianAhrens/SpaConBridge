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
#include "../../PageContainerComponent.h"
#include "../../PageComponentManager.h"

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

void SceneIndexToMidiAssignerComponent::triggerEditAssignments()
{
    m_assignmentsEditionOverlay = std::make_unique<AssignmentsListingComponent>(m_deviceIdentifier, m_currentScenesToMidiAssignments);
    m_assignmentsEditionOverlay->onAssigningFinished = [&](Component* sender, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& midiAssignments) {
        processAssignmentResults(sender, midiAssignments);
        finishEditAssignments();
    };

    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        auto pageContainer = pageMgr->GetPageContainer();
        if (pageContainer)
            pageContainer->SetOverlayComponent(m_assignmentsEditionOverlay.get());
    }
}

void SceneIndexToMidiAssignerComponent::finishEditAssignments()
{

    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        auto pageContainer = pageMgr->GetPageContainer();
        if (pageContainer)
            pageContainer->ClearOverlayComponent();
    }

    m_assignmentsEditionOverlay.reset();
}

void SceneIndexToMidiAssignerComponent::processAssignmentResult(Component* sender, const String& sceneIndex, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssignment)
{
    ignoreUnused(sender);

    m_currentScenesToMidiAssignments[sceneIndex] = midiAssignment;

    if (onAssignmentsSet)
        onAssignmentsSet(this, m_currentScenesToMidiAssignments);

    if (m_currentMidiAssisLabel)
        m_currentMidiAssisLabel->setText(String(m_currentScenesToMidiAssignments.size()) + " assignments");
}

void SceneIndexToMidiAssignerComponent::processAssignmentResults(Component* sender, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& midiAssignments)
{
    ignoreUnused(sender);

    m_currentScenesToMidiAssignments = midiAssignments;

    if (onAssignmentsSet)
        onAssignmentsSet(this, m_currentScenesToMidiAssignments);

    if (m_currentMidiAssisLabel)
        m_currentMidiAssisLabel->setText(String(m_currentScenesToMidiAssignments.size()) + " assignments");
}

void SceneIndexToMidiAssignerComponent::setCurrentScenesToMidiAssignments(const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& currentAssignments)
{
    m_currentScenesToMidiAssignments = currentAssignments;

    if (m_currentMidiAssisLabel)
        m_currentMidiAssisLabel->setText(String(m_currentScenesToMidiAssignments.size()) + " assignments");
}

void SceneIndexToMidiAssignerComponent::setSelectedDeviceIdentifier(const String& deviceIdentifier)
{
    m_deviceIdentifier = deviceIdentifier;
}

void SceneIndexToMidiAssignerComponent::setReferredId(std::int16_t refId)
{
    m_referredId = refId;
}

std::int16_t SceneIndexToMidiAssignerComponent::getReferredId() const
{
    return m_referredId;
}

SceneIndexToMidiAssignerComponent::AssignmentEditComponent::AssignmentEditComponent(std::int16_t refId, const String& deviceIdentifier, const String& sceneIndex, const JUCEAppBasics::MidiCommandRangeAssignment& currentAssi)
    : Component("AssignmentEditComponent")
{
    m_sceneIndex = sceneIndex;

    m_sceneIndexEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(6, "1234567890."); // 6 digits: "99.999"

    m_sceneIndexEdit = std::make_unique<TextEditor>("SceneIndexEditor");
    m_sceneIndexEdit->setText(sceneIndex);
    m_sceneIndexEdit->setInputFilter(m_sceneIndexEditFilter.get(), false);
    addAndMakeVisible(m_sceneIndexEdit.get());

    m_learnerComponent = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(refId, JUCEAppBasics::MidiLearnerComponent::AT_Trigger);
    m_learnerComponent->setSelectedDeviceIdentifier(deviceIdentifier);
    m_learnerComponent->setCurrentMidiAssi(currentAssi);
    addAndMakeVisible(m_learnerComponent.get());
    m_learnerComponent->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };

}
SceneIndexToMidiAssignerComponent::AssignmentEditComponent::~AssignmentEditComponent()
{

}

const String SceneIndexToMidiAssignerComponent::AssignmentEditComponent::GetSceneIndex()
{
    return m_sceneIndexEdit->getText();
}

const JUCEAppBasics::MidiCommandRangeAssignment& SceneIndexToMidiAssignerComponent::AssignmentEditComponent::GetCurrentAssignment()
{
    return m_learnerComponent->getCurrentMidiAssi();
}

void SceneIndexToMidiAssignerComponent::AssignmentEditComponent::resized()
{
    auto bounds = getLocalBounds();

    m_learnerComponent->setBounds(bounds.removeFromRight(static_cast<int>(0.7f * bounds.getWidth()) - 2));
    bounds.removeFromRight(4);
    m_sceneIndexEdit->setBounds(bounds);
}

void SceneIndexToMidiAssignerComponent::AssignmentEditComponent::handleMidiAssiSet(Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi)
{
    auto learnerComponent = dynamic_cast<JUCEAppBasics::MidiLearnerComponent*>(sender);
    if (learnerComponent && onAssignmentSet)
    {
        onAssignmentSet(this, m_sceneIndex, midiAssi);
    }
}

SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::AssignmentsListingComponent(const String& deviceIdentifier, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& initialAssignments) 
{
    m_deviceIdentifier = deviceIdentifier;

    m_addButton = std::make_unique<TextButton>("Add assignment");
    m_addButton->addListener(this);
    addAndMakeVisible(m_addButton.get());

    m_closeButton = std::make_unique<TextButton>("Close");
    m_closeButton->addListener(this);
    addAndMakeVisible(m_closeButton.get());

    auto refId = std::int16_t(1);
    for (auto const& assignment : initialAssignments)
    {
        m_editComponents.push_back(std::make_unique<AssignmentEditComponent>(refId++, m_deviceIdentifier, assignment.first, assignment.second));
        addAndMakeVisible(m_editComponents.back().get());

        if (IsAvailableUiAreaExceeded())
        {
            m_addButton->setEnabled(false);
            break;
        }
    }
}

SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::~AssignmentsListingComponent() 
{
}

std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::GetCurrentAssignments() 
{
    std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> currentAssignments;
    for (auto const& editComponent : m_editComponents)
    {
        currentAssignments.insert(std::make_pair(editComponent->GetSceneIndex(), editComponent->GetCurrentAssignment()));
    }

    return currentAssignments;
}

void SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::paint(Graphics& g) 
{
    // Transparent background overlay
    g.setColour(Colours::black);
    g.setOpacity(0.5f);
    g.fillRect(getLocalBounds());
    g.setOpacity(1.0f);

    auto bounds = getLocalBounds().reduced(55, 25);

    g.setColour(getLookAndFeel().findColour(AlertWindow::outlineColourId));
    g.drawRect(bounds.toFloat(), 1.0f);

    bounds.reduce(1, 1);
    g.reduceClipRegion(bounds);

    // Background
    g.setColour(getLookAndFeel().findColour(AlertWindow::backgroundColourId));
    g.fillRect(bounds.toFloat());
}

void SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::resized() 
{
    auto bounds = getLocalBounds().reduced(55, 25);

    juce::FlexBox editsBox;
    editsBox.flexWrap = juce::FlexBox::Wrap::wrap;
    editsBox.flexDirection = juce::FlexBox::Direction::column;
    editsBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    for (auto const& editComponent : m_editComponents)
        editsBox.items.add(juce::FlexItem(*editComponent).withHeight(25.0f).withWidth(205.0f).withMargin(2));

    juce::FlexBox controlsBox;
    controlsBox.flexWrap = juce::FlexBox::Wrap::wrap;
    controlsBox.flexDirection = juce::FlexBox::Direction::row;
    controlsBox.justifyContent = juce::FlexBox::JustifyContent::flexEnd;
    controlsBox.items.add(juce::FlexItem(*m_addButton).withHeight(25.0f).withWidth(205.0f).withMargin(2));
    controlsBox.items.add(juce::FlexItem(*m_closeButton).withHeight(25.0f).withWidth(205.0f).withMargin(2));

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.items.add(juce::FlexItem(editsBox).withFlex(2.5));
    fb.items.add(juce::FlexItem().withHeight(2));
    fb.items.add(juce::FlexItem(controlsBox).withHeight(27.0f));
    fb.performLayout(bounds.reduced(4));

    if (m_addButton)
        m_addButton->setEnabled(!IsAvailableUiAreaExceeded());
}

void SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::buttonClicked(Button* button)
{
    if (m_addButton && m_addButton.get() == button)
    {
        m_editComponents.push_back(std::make_unique<AssignmentEditComponent>(static_cast<int16_t>(m_editComponents.size()), m_deviceIdentifier, GetNextSceneIndex(), JUCEAppBasics::MidiCommandRangeAssignment()));
        addAndMakeVisible(m_editComponents.back().get());

        m_addButton->setEnabled(!IsAvailableUiAreaExceeded());

        resized();
    }
    else if (m_closeButton.get() == button)
    {
        if (onAssigningFinished)
            onAssigningFinished(this, GetCurrentAssignments());
    }
}

bool SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::IsAvailableUiAreaExceeded()
{
    auto bounds = getLocalBounds().reduced(55, 25).toFloat();

    // don't mess up when ui simply is not yet initialized
    if (bounds.getWidth() == 0 && bounds.getHeight() == 0)
        return false;

    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    // substract controls height
    h -= 33.0f;
    if (h > 0)
    {
        auto totalElmsHeight = static_cast<float>(33 * (m_editComponents.size() + 1)); // one additional edit, to achieve the 'forecast' behaviour of the method
        auto colCount = static_cast<int>((totalElmsHeight / h) + 0.5f);

        auto requiredWidth = colCount * 210;

        return requiredWidth >= w;
    }

    return true;
}

String SceneIndexToMidiAssignerComponent::AssignmentsListingComponent::GetNextSceneIndex()
{
    auto maxIdx = 0.0f;
    for (auto const& edit : m_editComponents)
    {
        auto idx = edit->GetSceneIndex().getFloatValue();
        if (idx > maxIdx)
            maxIdx = idx;
    }
    
    auto newMajorIdx = maxIdx + 1;

    return String(newMajorIdx) + ".00";
}


}
