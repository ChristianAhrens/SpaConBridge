/* Copyright(c) 2022, Christian Ahrens
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

#include "RemoteObjectToOscAssignerComponent.h"

#include "Image_utils.h"
#include "../../PageContainerComponent.h"
#include "../../PageComponentManager.h"

#include <ProcessingEngine/ProcessingEngineConfig.h>

namespace SpaConBridge
{

//==============================================================================
RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignerComponent()
{
	m_currentRoiToOscAssisLabel = std::make_unique<TextEditor>("CurrentRoiToOscAssisLabel");
    m_currentRoiToOscAssisLabel->setText("0 assignments");
    m_currentRoiToOscAssisLabel->setEnabled(false);
    m_currentRoiToOscAssisLabel->setReadOnly(true);
	addAndMakeVisible(m_currentRoiToOscAssisLabel.get());

    m_editAssignmentsButton = std::make_unique<TextButton>("Edit assignments");
    m_editAssignmentsButton->addListener(this);
	addAndMakeVisible(m_editAssignmentsButton.get());
    lookAndFeelChanged();

}

RemoteObjectToOscAssignerComponent::~RemoteObjectToOscAssignerComponent()
{

}

void RemoteObjectToOscAssignerComponent::resized()
{
	auto bounds = getLocalBounds();

    m_currentRoiToOscAssisLabel->setBounds(bounds.removeFromRight(static_cast<int>(0.5f * bounds.getWidth()) - 2));
	bounds.removeFromRight(4);
    m_editAssignmentsButton->setBounds(bounds);
}

void RemoteObjectToOscAssignerComponent::buttonClicked(Button* button)
{
	if (button == m_editAssignmentsButton.get())
	{
        triggerEditAssignments();
	}
}

void RemoteObjectToOscAssignerComponent::triggerEditAssignments()
{
    m_assignmentsEditionOverlay = std::make_unique<RemoteObjectToOscAssignmentsViewingComponent>(m_currentRoiToOscAssignments);
    m_assignmentsEditionOverlay->onAssigningFinished = [&](Component* sender, const std::map<RemoteObjectIdentifier, juce::String>& roiToOscAssignments) {
        processAssignmentResults(sender, roiToOscAssignments);
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

void RemoteObjectToOscAssignerComponent::finishEditAssignments()
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

void RemoteObjectToOscAssignerComponent::processAssignmentResult(Component* sender, const RemoteObjectIdentifier& remoteObjectId, const juce::String& roiToOscAssignment)
{
    ignoreUnused(sender);

    if (RemoteObjectIdentifier::ROI_Invalid != remoteObjectId)
        m_currentRoiToOscAssignments[remoteObjectId] = roiToOscAssignment;

    if (onAssignmentsSet)
        onAssignmentsSet(this, m_currentRoiToOscAssignments);

    if (m_currentRoiToOscAssisLabel)
        m_currentRoiToOscAssisLabel->setText(String(m_currentRoiToOscAssignments.size()) + " assignments");
}

void RemoteObjectToOscAssignerComponent::processAssignmentResults(Component* sender, const std::map<RemoteObjectIdentifier, juce::String>& roiToOscAssignment)
{
    ignoreUnused(sender);

    m_currentRoiToOscAssignments.clear();
    for (auto const& assi : roiToOscAssignment)
        if (RemoteObjectIdentifier::ROI_Invalid != assi.first)
            m_currentRoiToOscAssignments[assi.first] = assi.second;

    if (onAssignmentsSet)
        onAssignmentsSet(this, m_currentRoiToOscAssignments);

    if (m_currentRoiToOscAssisLabel)
        m_currentRoiToOscAssisLabel->setText(String(m_currentRoiToOscAssignments.size()) + " assignments");
}

void RemoteObjectToOscAssignerComponent::setCurrentRemoteObjecToOscAssignments(const std::map<RemoteObjectIdentifier, juce::String>& currentAssignments)
{
    m_currentRoiToOscAssignments = currentAssignments;

    if (m_currentRoiToOscAssisLabel)
        m_currentRoiToOscAssisLabel->setText(String(m_currentRoiToOscAssignments.size()) + " assignments");
}

void RemoteObjectToOscAssignerComponent::setSelectedDeviceIdentifier(const String& deviceIdentifier)
{
    m_deviceIdentifier = deviceIdentifier;
}

RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::RemoteObjectToOscAssignmentEditComponent(const RemoteObjectIdentifier& remoteObjectId, const juce::String& currentAssi)
    : AssignmentEditOverlayBaseComponents::AssignmentEditComponent(),
    m_currentRemoteObjectId(remoteObjectId),
    m_currentOscAssignment(currentAssi)
{
    // create and setup remote object dropdown
    m_remoteObjectSelect = std::make_unique<juce::ComboBox>("OscRemapObjectId");
    for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
    {
        auto roid = static_cast<RemoteObjectIdentifier>(i);
        m_remoteObjectSelect->setTextWhenNothingSelected("Select target");
        m_remoteObjectSelect->addItem(ProcessingEngineConfig::GetObjectDescription(roid), roid);
    }
    m_remoteObjectSelect->onChange = [=] {
        m_currentRemoteObjectId = static_cast<RemoteObjectIdentifier>(m_remoteObjectSelect->getSelectedId());
        m_remoteObjectSelect->setTooltip(ProcessingEngineConfig::GetObjectDescription(m_currentRemoteObjectId));
    };
    addAndMakeVisible(m_remoteObjectSelect.get());

    // create and setup osc string textedit
    m_oscAssignmentEditComponent = std::make_unique<juce::TextEditor>("OscRemapAssignment");
    m_oscAssignmentEditComponent->onEscapeKey = [=]() {
        handleRemoteObjectToOscAssiReset(); 
    };
    m_oscAssignmentEditComponent->onFocusLost = [=]() {
        if (m_oscAssignmentEditComponent)
        {
            auto oscAssi = m_oscAssignmentEditComponent->getText();
            handleRemoteObjectToOscAssiSet(oscAssi);
        }
    };
    m_oscAssignmentEditComponent->onReturnKey = [=]() {
        if (m_oscAssignmentEditComponent)
        {
            auto oscAssi = m_oscAssignmentEditComponent->getText();
            handleRemoteObjectToOscAssiSet(oscAssi);
        }
    };
    addAndMakeVisible(m_oscAssignmentEditComponent.get());

    // set incoming start values
    if (RemoteObjectIdentifier::ROI_Invalid != remoteObjectId)
    {
        m_remoteObjectSelect->setSelectedId(remoteObjectId);
        m_remoteObjectSelect->setTooltip(ProcessingEngineConfig::GetObjectDescription(remoteObjectId));
    }
    m_oscAssignmentEditComponent->setText(currentAssi);

    lookAndFeelChanged();
}

RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::~RemoteObjectToOscAssignmentEditComponent()
{
}

void RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::lookAndFeelChanged()
{
    m_oscAssignmentEditComponent->setTextToShowWhenEmpty("/some/osc/%1/path/%2", getLookAndFeel().findColour(TextEditor::ColourIds::textColourId).darker(0.6f));
}

const RemoteObjectIdentifier RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::GetRemoteObjectId()
{
    return m_currentRemoteObjectId;
}

const juce::String& RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::GetCurrentAssignment()
{
    return m_currentOscAssignment;
}

void RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::resized()
{
    auto bounds = getLocalBounds();

    m_oscAssignmentEditComponent->setBounds(bounds.removeFromRight(static_cast<int>(0.6f * bounds.getWidth()) - 2));
    bounds.removeFromRight(4);
    m_remoteObjectSelect->setBounds(bounds);
}

void RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::handleRemoteObjectToOscAssiSet(const juce::String& oscAssi)
{
    m_currentOscAssignment = oscAssi;

    if (onAssignmentSet)
        onAssignmentSet(this, m_currentRemoteObjectId, oscAssi);
}

void RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentEditComponent::handleRemoteObjectToOscAssiReset()
{
    if (m_oscAssignmentEditComponent)
        m_oscAssignmentEditComponent->setText(m_currentOscAssignment);
}

RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsListingComponent::RemoteObjectToOscAssignmentsListingComponent(const std::map<RemoteObjectIdentifier, juce::String>& initialAssignments)
    : AssignmentEditOverlayBaseComponents::AssignmentsListingComponent()
{
    m_editorWidth = 355.0f;
    m_editorHeight = 25.0f;
    m_editorMargin = 2.0f;

    for (auto const& assignment : initialAssignments)
    {
        m_editComponents.push_back(std::make_unique<RemoteObjectToOscAssignmentEditComponent>(assignment.first, assignment.second));
        addAndMakeVisible(m_editComponents.back().get());
    }
}

RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsListingComponent::~RemoteObjectToOscAssignmentsListingComponent()
{
}

std::map<RemoteObjectIdentifier, juce::String> RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsListingComponent::GetCurrentAssignments()
{
    std::map<RemoteObjectIdentifier, juce::String> currentAssignments;
    for (auto const& editComponent : m_editComponents)
    {
        auto remoteObjectToOscEditComponent = reinterpret_cast<RemoteObjectToOscAssignmentEditComponent*>(editComponent.get());
        if (remoteObjectToOscEditComponent)
            currentAssignments.insert(std::make_pair(remoteObjectToOscEditComponent->GetRemoteObjectId(), remoteObjectToOscEditComponent->GetCurrentAssignment()));
    }

    return currentAssignments;
}

bool RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsListingComponent::AddAssignment()
{
    m_editComponents.push_back(std::make_unique<RemoteObjectToOscAssignmentEditComponent>(RemoteObjectIdentifier::ROI_Invalid, juce::String()));
    addAndMakeVisible(m_editComponents.back().get());

    resized();

    return !IsAvailableUiAreaExceeded();
}

const String RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsListingComponent::DumpCurrentAssignmentsToCsvString()
{
    auto csvString = String();

    csvString += "RemoteObjectIdentifier;OscStringAssignment;\n";
    for (auto const& editComponent : m_editComponents)
    {
        auto remoteObjectToOscEditComponent = reinterpret_cast<RemoteObjectToOscAssignmentEditComponent*>(editComponent.get());
        if (remoteObjectToOscEditComponent)
            csvString += ProcessingEngineConfig::GetObjectDescription(remoteObjectToOscEditComponent->GetRemoteObjectId()).removeCharacters(" ") + ";" + remoteObjectToOscEditComponent->GetCurrentAssignment() + ";\n";
    }

    return csvString;
}

bool RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsListingComponent::ReadAssignmentsFromCsvString(const String& csvAssignmentsString)
{
    std::map<String, String> assignments;

    auto separatedCsvAssignmentStrings = StringArray();
    separatedCsvAssignmentStrings.addTokens(csvAssignmentsString, "\n", "");
    
    if (separatedCsvAssignmentStrings.size() > 1 && separatedCsvAssignmentStrings[0] == "RemoteObjectIdentifier;OscStringAssignment;")
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
    for (auto const& assignment : assignments)
    {
        for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
        {
            auto roid = static_cast<RemoteObjectIdentifier>(i);
            if (ProcessingEngineConfig::GetObjectDescription(roid).removeCharacters(" ") == assignment.first)
            {   
                m_editComponents.push_back(std::make_unique<RemoteObjectToOscAssignmentEditComponent>(roid, assignment.second));
                addAndMakeVisible(m_editComponents.back().get());
                break;
            }
        }
    }

    resized();

    return !m_editComponents.empty();
}

RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsViewingComponent::RemoteObjectToOscAssignmentsViewingComponent(const std::map<RemoteObjectIdentifier, juce::String>& initialAssignments)
    : AssignmentEditOverlayBaseComponents::AssignmentsViewingComponent()
{
    m_contentComponent = std::unique_ptr<AssignmentEditOverlayBaseComponents::AssignmentsListingComponent>(reinterpret_cast<AssignmentEditOverlayBaseComponents::AssignmentsListingComponent*>(new RemoteObjectToOscAssignmentsListingComponent(initialAssignments)));
    if (m_contentViewport)
        m_contentViewport->setViewedComponent(m_contentComponent.get(), false);

    lookAndFeelChanged();
}

RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsViewingComponent::~RemoteObjectToOscAssignmentsViewingComponent()
{
}

std::map<RemoteObjectIdentifier, juce::String> RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsViewingComponent::GetCurrentAssignments()
{
    auto contentComponent = reinterpret_cast<RemoteObjectToOscAssignmentsListingComponent*>(m_contentComponent.get());

    return contentComponent ? contentComponent->GetCurrentAssignments() : std::map<RemoteObjectIdentifier, juce::String>();
}

void RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsViewingComponent::onExportClicked()
{
    // prepare a default filename suggestion based on current date and app name
    auto initialFolderPathName = File::getSpecialLocation(File::userDocumentsDirectory).getFullPathName();
    auto initialFileNameSuggestion = Time::getCurrentTime().formatted("%Y-%m-%d_") + JUCEApplication::getInstance()->getApplicationName() + "_CustomOscMapping";
    auto initialFilePathSuggestion = initialFolderPathName + File::getSeparatorString() + initialFileNameSuggestion;
    auto initialFileSuggestion = File(initialFilePathSuggestion);
    
    // create the file chooser dialog
    auto chooser = std::make_unique<FileChooser>("Save current custom OSC mapping file as...",
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
                ShowUserErrorNotification(SEC_SaveCustomOSC_CannotWrite);
        }
        else
            ShowUserErrorNotification(SEC_SaveCustomOSC_CannotAccess);
    
    }
    delete static_cast<const FileChooser*>(&chooser);
        });
    chooser.release();
}

void RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsViewingComponent::onImportClicked()
{
    // create the file chooser dialog
    auto chooser = std::make_unique<FileChooser>("Select a custom OSC mapping file to import...",
        File::getSpecialLocation(File::userDocumentsDirectory), String(), true, false, this); // all filepatterns are allowed for loading (currently seems to not work on iOS and not be regarded on macOS at all)
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
                    ShowUserErrorNotification(SEC_LoadCustomOSC_InvalidFile);
        }
        else
            ShowUserErrorNotification(SEC_LoadCustomOSC_CannotAccess);
    }
    delete static_cast<const FileChooser*>(&chooser);
        });
    chooser.release();
}

void RemoteObjectToOscAssignerComponent::RemoteObjectToOscAssignmentsViewingComponent::onCloseClicked()
{
    if (onAssigningFinished)
        onAssigningFinished(this, GetCurrentAssignments());
}


}
