/* Copyright (c) 2020-2023, Christian Ahrens
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

#include <Image_utils.h>
#include <FixedFontTextEditor.h>
#include <TextWithImageButton.h>

#include "../../PageContainerComponent.h"
#include "../../PageComponentManager.h"

namespace SpaConBridge
{

//==============================================================================
SceneIndexToMidiAssignerComponent::SceneIndexToMidiAssignerComponent(std::int16_t refId)
{
    setReferredId(refId);

	m_currentMidiAssisLabel = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("CurrentMidiAssisLabel");
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
    m_assignmentsEditionOverlay = std::make_unique<SceneIndexAssignmentsViewingComponent>(m_deviceIdentifier, m_currentScenesToMidiAssignments);
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

SceneIndexToMidiAssignerComponent::SceneIndexAssignmentEditComponent::SceneIndexAssignmentEditComponent(std::int16_t refId, const String& deviceIdentifier, const String& sceneIndex, const JUCEAppBasics::MidiCommandRangeAssignment& currentAssi)
    : AssignmentEditOverlayBaseComponents::AssignmentEditComponent(),
      m_sceneIndex(sceneIndex)
{
    m_sceneIndexEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(6, "1234567890."); // 6 digits: "99.999"

    m_sceneIndexEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("SceneIndexEditor");
    m_sceneIndexEdit->setText(sceneIndex);
    m_sceneIndexEdit->setInputFilter(m_sceneIndexEditFilter.get(), false);
    addAndMakeVisible(m_sceneIndexEdit.get());

    m_learnerComponent = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(refId, JUCEAppBasics::MidiLearnerComponent::AssignmentType(0x01)); // xcode has some weired linker issue with resolving AT_Trigger constexpr here... ?!
    m_learnerComponent->setSelectedDeviceIdentifier(deviceIdentifier);
    m_learnerComponent->setCurrentMidiAssi(currentAssi);
    addAndMakeVisible(m_learnerComponent.get());
    m_learnerComponent->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };

}

SceneIndexToMidiAssignerComponent::SceneIndexAssignmentEditComponent::~SceneIndexAssignmentEditComponent()
{

}

const String SceneIndexToMidiAssignerComponent::SceneIndexAssignmentEditComponent::GetSceneIndex()
{
    return m_sceneIndexEdit->getText();
}

const JUCEAppBasics::MidiCommandRangeAssignment& SceneIndexToMidiAssignerComponent::SceneIndexAssignmentEditComponent::GetCurrentAssignment()
{
    return m_learnerComponent->getCurrentMidiAssi();
}

void SceneIndexToMidiAssignerComponent::SceneIndexAssignmentEditComponent::resized()
{
    auto bounds = getLocalBounds();

    m_learnerComponent->setBounds(bounds.removeFromRight(static_cast<int>(0.75f * bounds.getWidth()) - 2));
    bounds.removeFromRight(4);
    m_sceneIndexEdit->setBounds(bounds);
}

void SceneIndexToMidiAssignerComponent::SceneIndexAssignmentEditComponent::handleMidiAssiSet(Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi)
{
    auto learnerComponent = dynamic_cast<JUCEAppBasics::MidiLearnerComponent*>(sender);
    if (learnerComponent && onAssignmentSet)
    {
        onAssignmentSet(this, m_sceneIndex, midiAssi);
    }
}

SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::SceneIndexAssignmentsListingComponent(const String& deviceIdentifier, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& initialAssignments)
    : AssignmentEditOverlayBaseComponents::AssignmentsListingComponent(),
      m_deviceIdentifier(deviceIdentifier)
{
    m_editorWidth = 225;
    m_editorHeight = 25;
    m_editorMargin = 2;

    auto refId = std::int16_t(1);
    for (auto const& assignment : initialAssignments)
    {
        auto floatSceneIndex = assignment.first.getFloatValue();
        auto isValidSceneIndex = (1.0f <= floatSceneIndex && 99.999 >= floatSceneIndex);
        if (isValidSceneIndex)
        {
            auto stringSceneIndex = String(floatSceneIndex, 2);
            m_editComponents.push_back(std::make_unique<SceneIndexAssignmentEditComponent>(refId++, m_deviceIdentifier, stringSceneIndex, assignment.second));
            addAndMakeVisible(m_editComponents.back().get());
        }
    }
}

SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::~SceneIndexAssignmentsListingComponent()
{
}

void SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::setWidth(int width)
{
    auto editsCount = m_editComponents.size();
    auto fittingColumnCount = static_cast<int>(width / (m_editorWidth + 2.0f * m_editorMargin));
    auto totalEditsHeight = (editsCount + 1) * (m_editorHeight + 2.0f * m_editorMargin);
    auto minRequiredHeight = static_cast<int>(totalEditsHeight / fittingColumnCount);

    if (minRequiredHeight < m_minHeight)
        setSize(width, m_minHeight);
    else
        setSize(width, minRequiredHeight);
}

std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::GetCurrentAssignments()
{
    std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> currentAssignments;
    for (auto const& editComponent : m_editComponents)
    {
        auto sceneIndexEditComponent = reinterpret_cast<SceneIndexAssignmentEditComponent*>(editComponent.get());
        if (sceneIndexEditComponent)
            currentAssignments.insert(std::make_pair(sceneIndexEditComponent->GetSceneIndex(), sceneIndexEditComponent->GetCurrentAssignment()));
    }

    return currentAssignments;
}

bool SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::AddAssignment()
{
    m_editComponents.push_back(std::make_unique<SceneIndexAssignmentEditComponent>(static_cast<int16_t>(m_editComponents.size()), m_deviceIdentifier, GetNextSceneIndex(), JUCEAppBasics::MidiCommandRangeAssignment()));
    addAndMakeVisible(m_editComponents.back().get());

    resized();

    return !IsAvailableUiAreaExceeded();
}

void SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::resized()
{
    auto bounds = getLocalBounds();

    juce::FlexBox editsBox;
    editsBox.flexWrap = juce::FlexBox::Wrap::wrap;
    editsBox.flexDirection = juce::FlexBox::Direction::column;
    editsBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    for (auto const& editComponent : m_editComponents)
        editsBox.items.add(juce::FlexItem(*editComponent).withHeight(static_cast<float>(m_editorHeight)).withWidth(static_cast<float>(m_editorWidth)).withMargin(static_cast<float>(m_editorMargin)));
    editsBox.performLayout(bounds.reduced(static_cast<int>(2.0f * m_editorMargin)));
}

const String SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::DumpCurrentAssignmentsToCsvString()
{
    auto csvString = String();

    csvString += "SceneIndex;MidiAssignment;\n";
    for (auto const& editComponent : m_editComponents)
    {
        auto sceneIndexEditComponent = reinterpret_cast<SceneIndexAssignmentEditComponent*>(editComponent.get());
        if (sceneIndexEditComponent)
            csvString += sceneIndexEditComponent->GetSceneIndex() + ";" + sceneIndexEditComponent->GetCurrentAssignment().serializeToHexString() + ";\n";
    }

    return csvString;
}

bool SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::ReadAssignmentsFromCsvString(const String& csvAssignmentsString)
{
    std::map<String, String> assignments;

    auto separatedCsvAssignmentStrings = StringArray();
    separatedCsvAssignmentStrings.addTokens(csvAssignmentsString, "\n", "");

    if (separatedCsvAssignmentStrings.size() > 1 && separatedCsvAssignmentStrings[0] == "SceneIndex;MidiAssignment;")
        separatedCsvAssignmentStrings.remove(0);
    else
        return false;

    for (auto const& csvAssignmentString : separatedCsvAssignmentStrings)
    {
        auto csvAssignmentStringElements = StringArray();
        csvAssignmentStringElements.addTokens(csvAssignmentString, ";", "");

        if (csvAssignmentStringElements.size() != 3)
            continue;

        assignments.insert(std::make_pair(csvAssignmentStringElements[0], csvAssignmentStringElements[1]));
    }

    if (assignments.empty())
        return false;

    m_editComponents.clear();
    auto refId = std::int16_t(1);
    for (auto const& assignment : assignments)
    {
        JUCEAppBasics::MidiCommandRangeAssignment assi;
        if (assignment.second.isNotEmpty())
            assi.deserializeFromHexString(assignment.second);
        auto floatSceneIndex = assignment.first.getFloatValue();
        auto isValidSceneIndex = (1.0f <= floatSceneIndex && 99.999 >= floatSceneIndex);
        if (isValidSceneIndex)
        {
            auto stringSceneIndex = String(floatSceneIndex, 2);
            m_editComponents.push_back(std::make_unique<SceneIndexAssignmentEditComponent>(refId++, m_deviceIdentifier, stringSceneIndex, assi));
            addAndMakeVisible(m_editComponents.back().get());
        }
    }

    resized();

    return !m_editComponents.empty();
}

String SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsListingComponent::GetNextSceneIndex()
{
    auto maxIdx = 0.0f;
    for (auto const& editComponent : m_editComponents)
    {
        auto sceneIndexEditComponent = reinterpret_cast<SceneIndexAssignmentEditComponent*>(editComponent.get());
        if (sceneIndexEditComponent)
        {
            auto idx = sceneIndexEditComponent->GetSceneIndex().getFloatValue();
            if (idx > maxIdx)
                maxIdx = idx;
        }
    }
    
    auto newMajorIdx = static_cast<int>(maxIdx) + 1;
    if (newMajorIdx < 100)
        return String(newMajorIdx) + ".00";
    else
        return String("99.999");
}

SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsViewingComponent::SceneIndexAssignmentsViewingComponent(const String& deviceIdentifier, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& initialAssignments)
    :   AssignmentEditOverlayBaseComponents::AssignmentsViewingComponent(),
        m_deviceIdentifier(deviceIdentifier)
{
    m_contentComponent = std::unique_ptr<AssignmentEditOverlayBaseComponents::AssignmentsListingComponent>(reinterpret_cast<AssignmentEditOverlayBaseComponents::AssignmentsListingComponent*>(new SceneIndexAssignmentsListingComponent(deviceIdentifier, initialAssignments)));
    if (m_contentViewport)
        m_contentViewport->setViewedComponent(m_contentComponent.get(), false);

    lookAndFeelChanged();
}

SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsViewingComponent::~SceneIndexAssignmentsViewingComponent()
{
}

std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsViewingComponent::GetCurrentAssignments()
{
    auto contentComponent = reinterpret_cast<SceneIndexAssignmentsListingComponent*>(m_contentComponent.get());

    return contentComponent ? contentComponent->GetCurrentAssignments() : std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>();
}

void SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsViewingComponent::onExportClicked()
{
    // prepare a default filename suggestion based on current date and app name
    auto initialFolderPathName = File::getSpecialLocation(File::userDocumentsDirectory).getFullPathName();
    auto initialFileNameSuggestion = Time::getCurrentTime().formatted("%Y-%m-%d_") + JUCEApplication::getInstance()->getApplicationName() + "_scnIdxToMidiMapping";
    auto initialFilePathSuggestion = initialFolderPathName + File::getSeparatorString() + initialFileNameSuggestion;
    auto initialFileSuggestion = File(initialFilePathSuggestion);

    // create the file chooser dialog
    auto chooser = std::make_unique<FileChooser>("Save current Scene Index to MIDI mapping file as...",
        initialFileSuggestion, "*.csv", true, false, this);
    // and trigger opening it
    chooser->launchAsync(FileBrowserComponent::saveMode, [this](const FileChooser& chooser)
        {
            auto file = chooser.getResult();

            // verify that the result is valid (ok clicked)
            if (!file.getFullPathName().isEmpty())
            {
                // enforce the .config extension
                if (file.getFileExtension() != ".csv")
                    file = file.withFileExtension(".csv");

                if (file.hasWriteAccess())
                {
                    FileOutputStream outputStream(file);
                    if (outputStream.openedOk())
                    {
                        outputStream.setPosition(0);
                        outputStream.truncate();

                        if (m_contentComponent)
                        {
                            outputStream.writeText(m_contentComponent->DumpCurrentAssignmentsToCsvString(), false, false, nullptr);
                            outputStream.flush();
                        }
                    }
                    else
                        ShowUserErrorNotification(SEC_SaveScnIdxToMIDI_CannotWrite);
                }
                else
                    ShowUserErrorNotification(SEC_SaveScnIdxToMIDI_CannotAccess);
                
            }
            delete static_cast<const FileChooser*>(&chooser);
        });
    chooser.release();
}

void SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsViewingComponent::onImportClicked()
{
    // create the file chooser dialog
    auto chooser = std::make_unique<FileChooser>("Select a Scene Index to MIDI mapping file to import..."); // all filepatterns are allowed for loading (currently seems to not work on iOS and not be regarded on macOS at all)
    // and trigger opening it
    chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser)
        {
            auto file = chooser.getResult();

            // verify that the result is valid (ok clicked)
            if (!file.getFullPathName().isEmpty())
            {
                FileInputStream inputStream(file);
                if (inputStream.openedOk())
                {
                    auto csvFileContents = inputStream.readEntireStreamAsString();
                    if (m_contentComponent)
                        if (!m_contentComponent->ReadAssignmentsFromCsvString(csvFileContents))
                            ShowUserErrorNotification(SEC_LoadScnIdxToMIDI_InvalidFile);
                }
                else
                    ShowUserErrorNotification(SEC_LoadScnIdxToMIDI_CannotAccess);
            }
            delete static_cast<const FileChooser*>(&chooser);
        });
    chooser.release();
}

void SceneIndexToMidiAssignerComponent::SceneIndexAssignmentsViewingComponent::onCloseClicked()
{
    if (onAssigningFinished)
        onAssigningFinished(this, GetCurrentAssignments());
}



}
