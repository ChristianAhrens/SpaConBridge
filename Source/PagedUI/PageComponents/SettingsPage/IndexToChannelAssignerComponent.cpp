/* Copyright(c) 2023, Christian Ahrens
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

#include "IndexToChannelAssignerComponent.h"

#include <Image_utils.h>
#include <FixedFontTextEditor.h>
#include <TextWithImageButton.h>

#include "../../PageContainerComponent.h"
#include "../../PageComponentManager.h"

#include <ProcessingEngine/ProcessingEngineConfig.h>

namespace SpaConBridge
{

//==============================================================================
IndexToChannelAssignerComponent::IndexToChannelAssignerComponent()
{
	m_currentIdxToChAssisLabel = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("CurrentIdxToChAssisLabel");
    m_currentIdxToChAssisLabel->setText("0 remappings");
    m_currentIdxToChAssisLabel->setEnabled(false);
    m_currentIdxToChAssisLabel->setReadOnly(true);
	addAndMakeVisible(m_currentIdxToChAssisLabel.get());

    m_editAssignmentsButton = std::make_unique<TextButton>("Edit remappings");
    m_editAssignmentsButton->addListener(this);
	addAndMakeVisible(m_editAssignmentsButton.get());
    lookAndFeelChanged();

}

IndexToChannelAssignerComponent::~IndexToChannelAssignerComponent()
{

}

void IndexToChannelAssignerComponent::resized()
{
	auto bounds = getLocalBounds();

    m_currentIdxToChAssisLabel->setBounds(bounds.removeFromRight(static_cast<int>(0.5f * bounds.getWidth()) - 2));
	bounds.removeFromRight(4);
    m_editAssignmentsButton->setBounds(bounds);
}

void IndexToChannelAssignerComponent::buttonClicked(Button* button)
{
	if (button == m_editAssignmentsButton.get())
	{
        triggerEditAssignments();
	}
}

void IndexToChannelAssignerComponent::triggerEditAssignments()
{
    m_assignmentsEditionOverlay = std::make_unique<IndexToChannelAssignmentsViewingComponent>(m_currentIdxToChAssignments);
    m_assignmentsEditionOverlay->SetPreferredWidth(300);
    m_assignmentsEditionOverlay->onAssigningFinished = [&](Component* sender, const std::map<int, ChannelId>& idxToChAssignments) {
        processAssignmentResults(sender, idxToChAssignments);
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

void IndexToChannelAssignerComponent::finishEditAssignments()
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

void IndexToChannelAssignerComponent::processAssignmentResult(Component* sender, int index, ChannelId channelAssignment)
{
    ignoreUnused(sender);

    if (0 <= index && 0 < channelAssignment)
        m_currentIdxToChAssignments[index] = channelAssignment;

    if (onAssignmentsSet)
        onAssignmentsSet(this, m_currentIdxToChAssignments);

    if (m_currentIdxToChAssisLabel)
        m_currentIdxToChAssisLabel->setText(String(m_currentIdxToChAssignments.size()) + " remappings");
}

void IndexToChannelAssignerComponent::processAssignmentResults(Component* sender, const std::map<int, ChannelId>& idxToChAssignment)
{
    ignoreUnused(sender);

    m_currentIdxToChAssignments.clear();
    for (auto const& assi : idxToChAssignment)
        if (0 <= assi.first)
            m_currentIdxToChAssignments[assi.first] = assi.second;

    if (onAssignmentsSet)
        onAssignmentsSet(this, m_currentIdxToChAssignments);

    if (m_currentIdxToChAssisLabel)
        m_currentIdxToChAssisLabel->setText(String(m_currentIdxToChAssignments.size()) + " remappings");
}

void IndexToChannelAssignerComponent::setCurrentIndexToChannelAssignments(const std::map<int, ChannelId>& currentAssignments)
{
    m_currentIdxToChAssignments = currentAssignments;

    if (m_currentIdxToChAssisLabel)
        m_currentIdxToChAssisLabel->setText(String(m_currentIdxToChAssignments.size()) + " remappings");
}

IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::IndexToChannelAssignmentEditComponent(int index, ChannelId currentAssi)
    :   AssignmentEditOverlayBaseComponents::AssignmentEditComponent(),
        m_currentIndex(index),
        m_currentChannelAssignment(currentAssi)
{
    // create and setup index textedit
    m_indexEditComponent = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("indexAssignment");
    m_indexEditComponent->onFocusLost = [=]() {
        handleEditorInput();
    };
    m_indexEditComponent->onReturnKey = [=]() {
        handleEditorInput();
    };
    addAndMakeVisible(m_indexEditComponent.get());

    // create and setup channelId textedit
    m_channelAssignmentEditComponent = std::make_unique<JUCEAppBasics::FixedFontTextEditor>("ChannelRemapAssignment");
    m_channelAssignmentEditComponent->onEscapeKey = [=]() {
        handleIndexToChannelAssiReset(); 
    };
    m_channelAssignmentEditComponent->onFocusLost = [=]() {
        handleEditorInput();
    };
    m_channelAssignmentEditComponent->onReturnKey = [=]() {
        handleEditorInput();
    };
    addAndMakeVisible(m_channelAssignmentEditComponent.get());

    // set incoming start values
    m_indexEditComponent->setText(juce::String(index));
    m_channelAssignmentEditComponent->setText(juce::String(currentAssi));
}

IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::~IndexToChannelAssignmentEditComponent()
{
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::handleEditorInput()
{
    if (m_channelAssignmentEditComponent)
    {
        auto index = m_indexEditComponent->getText().getIntValue();
        auto channelAssi = m_channelAssignmentEditComponent->getText().getIntValue();
        
        handleIndexToChannelAssiSet(std::make_pair(index, channelAssi));
    }
}

int IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::GetCurrentIndex()
{
    return m_currentIndex;
}

ChannelId IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::GetCurrentChannelAssignment()
{
    return m_currentChannelAssignment;
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::resized()
{
    auto bounds = getLocalBounds();

    m_channelAssignmentEditComponent->setBounds(bounds.removeFromRight(static_cast<int>(0.5f * bounds.getWidth()) - 2));
    bounds.removeFromRight(4);
    m_indexEditComponent->setBounds(bounds);
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::handleIndexToChannelAssiSet(const std::pair<int, ChannelId>& idxToChannelAssi)
{
    m_currentIndex = idxToChannelAssi.first;
    m_currentChannelAssignment = idxToChannelAssi.second;

    if (onAssignmentSet)
        onAssignmentSet(this, idxToChannelAssi);
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentEditComponent::handleIndexToChannelAssiReset()
{
    if (m_channelAssignmentEditComponent)
        m_channelAssignmentEditComponent->setText(juce::String(m_currentIndex + 1));
}

IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::IndexToChannelAssignmentsListingComponent(const std::map<int, ChannelId>& initialAssignments)
    : AssignmentEditOverlayBaseComponents::AssignmentsListingComponent()
{
    m_editorHeight = 25;
    m_editorMargin = 2;

    m_beaconIdxHeader = std::make_unique<juce::Label>("BeaconIdxHeaderLabel", "Beacon Idx");
    addAndMakeVisible(m_beaconIdxHeader.get());
    m_channelAssignmentHeader = std::make_unique<juce::Label>("ChannelAssiHeaderLabel", "Channel assignment");
    addAndMakeVisible(m_channelAssignmentHeader.get());

    for (auto const& assignment : initialAssignments)
    {
        m_editComponents.push_back(std::make_unique<IndexToChannelAssignmentEditComponent>(assignment.first, assignment.second));
        addAndMakeVisible(m_editComponents.back().get());
    }
}

IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::~IndexToChannelAssignmentsListingComponent()
{
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::setWidth(int width)
{
    auto editsCount = m_editComponents.size();
    auto totalEditsHeight = static_cast<int>((editsCount + 1) * (m_editorHeight + 2.0f * m_editorMargin));

    if (totalEditsHeight < m_minHeight)
        setSize(width, m_minHeight);
    else
        setSize(width, totalEditsHeight);
}

std::map<int, ChannelId> IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::GetCurrentAssignments()
{
    std::map<int, ChannelId> currentAssignments;
    for (auto const& editComponent : m_editComponents)
    {
        auto indexToChannelEditComponent = reinterpret_cast<IndexToChannelAssignmentEditComponent*>(editComponent.get());
        if (indexToChannelEditComponent)
            currentAssignments.insert(std::make_pair(indexToChannelEditComponent->GetCurrentIndex(), indexToChannelEditComponent->GetCurrentChannelAssignment()));
    }

    return currentAssignments;
}

bool IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::AddAssignment()
{
    auto startIdx = 1;
    auto startChannel = 1;

    if (!m_editComponents.empty())
    {
        auto edit = dynamic_cast<IndexToChannelAssignmentEditComponent*>(m_editComponents.back().get());
        if (edit)
        {
            startIdx = edit->GetCurrentIndex() + 1;
            startChannel = edit->GetCurrentChannelAssignment() + 1;
        }
    }
        
    m_editComponents.push_back(std::make_unique<IndexToChannelAssignmentEditComponent>(startIdx, startChannel));
    addAndMakeVisible(m_editComponents.back().get());

    resized();

    return !IsAvailableUiAreaExceeded();
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::resized()
{
    auto bounds = getLocalBounds();

    auto headerBounds = bounds.removeFromTop(m_editorHeight).reduced(static_cast<int>(2.0f * m_editorMargin));
    m_beaconIdxHeader->setBounds(headerBounds.removeFromLeft(headerBounds.getWidth() / 2));
    m_channelAssignmentHeader->setBounds(headerBounds);

    juce::FlexBox editsBox;
    editsBox.flexWrap = juce::FlexBox::Wrap::wrap;
    editsBox.flexDirection = juce::FlexBox::Direction::column;
    editsBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    for (auto const& editComponent : m_editComponents)
        editsBox.items.add(juce::FlexItem(*editComponent).withHeight(static_cast<float>(m_editorHeight)).withWidth(static_cast<float>(bounds.getWidth() - 6 * m_editorMargin)).withMargin(static_cast<float>(m_editorMargin)));
    editsBox.performLayout(bounds.reduced(static_cast<int>(2.0f * m_editorMargin)));
}

const String IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::DumpCurrentAssignmentsToCsvString()
{
    auto csvString = String();

    csvString += "Index;ChannelId;\n";
    for (auto const& editComponent : m_editComponents)
    {
        auto indexToChannelEditComponent = reinterpret_cast<IndexToChannelAssignmentEditComponent*>(editComponent.get());
        if (indexToChannelEditComponent)
            csvString += juce::String(indexToChannelEditComponent->GetCurrentIndex())
            + ";" + juce::String(indexToChannelEditComponent->GetCurrentChannelAssignment())
            + ";\n";
    }

    return csvString;
}

bool IndexToChannelAssignerComponent::IndexToChannelAssignmentsListingComponent::ReadAssignmentsFromCsvString(const juce::String& csvAssignmentsString)
{
    std::vector<std::vector<juce::String>> assignments;

    auto separatedCsvAssignmentStrings = juce::StringArray();
    separatedCsvAssignmentStrings.addTokens(csvAssignmentsString, "\n", "");
    
    if (separatedCsvAssignmentStrings.size() > 1 && separatedCsvAssignmentStrings[0] == "Index;ChannelId;")
        separatedCsvAssignmentStrings.remove(0);
    else
        return false;
    
    for (auto const& csvAssignmentString : separatedCsvAssignmentStrings)
    {
        auto csvAssignmentStringElements = juce::StringArray();
        csvAssignmentStringElements.addTokens(csvAssignmentString, ";", "");
    
        if (csvAssignmentStringElements.size() != 3)
            continue;
    
        auto assiElements = std::vector<juce::String>();
        assiElements.push_back(csvAssignmentStringElements[0]);
        assiElements.push_back(csvAssignmentStringElements[1]);
        assiElements.push_back(csvAssignmentStringElements[2]);
        assiElements.push_back(csvAssignmentStringElements[3]);
        assignments.push_back(assiElements);
    }
    
    if (assignments.empty())
        return false;
    
    m_editComponents.clear();
    for (auto const& assignment : assignments)
    {
        jassert(assignment.size() == 2);
        if (assignment.size() != 2)
            continue;

        //for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
        //{
        //    auto roid = static_cast<RemoteObjectIdentifier>(i);
        //    if (ProcessingEngineConfig::GetObjectDescription(roid).removeCharacters(" ") == assignment.at(0))
        //    {   
        //        auto oscAssiWithRange = std::make_pair(assignment.at(1), juce::Range<float>(assignment.at(2).getFloatValue(), assignment.at(3).getFloatValue()));
        //        m_editComponents.push_back(std::make_unique<IndexToChannelAssignmentEditComponent>(roid, oscAssiWithRange));
        //        addAndMakeVisible(m_editComponents.back().get());
        //        break;
        //    }
        //}
    }

    resized();

    return !m_editComponents.empty();
}

IndexToChannelAssignerComponent::IndexToChannelAssignmentsViewingComponent::IndexToChannelAssignmentsViewingComponent(const std::map<int, ChannelId>& initialAssignments)
    : AssignmentEditOverlayBaseComponents::AssignmentsViewingComponent()
{
    m_contentComponent = std::unique_ptr<AssignmentEditOverlayBaseComponents::AssignmentsListingComponent>(reinterpret_cast<AssignmentEditOverlayBaseComponents::AssignmentsListingComponent*>(new IndexToChannelAssignmentsListingComponent(initialAssignments)));
    if (m_contentViewport)
        m_contentViewport->setViewedComponent(m_contentComponent.get(), false);

    lookAndFeelChanged();
}

IndexToChannelAssignerComponent::IndexToChannelAssignmentsViewingComponent::~IndexToChannelAssignmentsViewingComponent()
{
}

std::map<int, ChannelId> IndexToChannelAssignerComponent::IndexToChannelAssignmentsViewingComponent::GetCurrentAssignments()
{
    auto contentComponent = reinterpret_cast<IndexToChannelAssignmentsListingComponent*>(m_contentComponent.get());

    return contentComponent ? contentComponent->GetCurrentAssignments() : std::map<int, ChannelId>();
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentsViewingComponent::onExportClicked()
{
    // prepare a default filename suggestion based on current date and app name
    auto initialFolderPathName = File::getSpecialLocation(File::userDocumentsDirectory).getFullPathName();
    auto initialFileNameSuggestion = Time::getCurrentTime().formatted("%Y-%m-%d_") + JUCEApplication::getInstance()->getApplicationName() + "_IndexToChannelMapping";
    auto initialFilePathSuggestion = initialFolderPathName + File::getSeparatorString() + initialFileNameSuggestion;
    auto initialFileSuggestion = File(initialFilePathSuggestion);
    
    // create the file chooser dialog
    auto chooser = std::make_unique<FileChooser>("Save current index to channel mapping file as...",
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
                ShowUserErrorNotification(SEC_SaveIdxToCh_CannotWrite);
        }
        else
            ShowUserErrorNotification(SEC_SaveCustomOSC_CannotAccess);
    
    }
    delete static_cast<const FileChooser*>(&chooser);
        });
    chooser.release();
}

void IndexToChannelAssignerComponent::IndexToChannelAssignmentsViewingComponent::onImportClicked()
{
    // create the file chooser dialog
    auto chooser = std::make_unique<FileChooser>("Select an index to channel mapping file to import...",
        File::getSpecialLocation(File::userDocumentsDirectory), juce::String(), true, false, this); // all filepatterns are allowed for loading (currently seems to not work on iOS and not be regarded on macOS at all)
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

void IndexToChannelAssignerComponent::IndexToChannelAssignmentsViewingComponent::onCloseClicked()
{
    if (onAssigningFinished)
        onAssigningFinished(this, GetCurrentAssignments());
}


}
