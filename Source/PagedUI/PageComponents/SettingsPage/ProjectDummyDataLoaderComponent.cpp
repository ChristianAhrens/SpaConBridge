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

#include "ProjectDummyDataLoaderComponent.h"

#include <FixedFontTextEditor.h>

#include "../../../SpaConBridgeCommon.h"

namespace SpaConBridge
{

//==============================================================================
ProjectDummyDataLoaderComponent::ProjectDummyDataLoaderComponent()
{
	m_currentProjectDummyDataInfoLabel = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
    m_currentProjectDummyDataInfoLabel->setEnabled(false);
    m_currentProjectDummyDataInfoLabel->setReadOnly(true);
	addAndMakeVisible(m_currentProjectDummyDataInfoLabel.get());

    m_loadProjectDummyDataButton = std::make_unique<juce::TextButton>("Load dbpr");
    m_loadProjectDummyDataButton->onClick = [=]() { loadProjectClicked(); };
	addAndMakeVisible(m_loadProjectDummyDataButton.get());

    m_clearProjectDummyDataButton = std::make_unique<juce::DrawableButton>("Clear dbpr", DrawableButton::ButtonStyle::ImageOnButtonBackground);
    m_clearProjectDummyDataButton->onClick = [=]() { clearProjectClicked(); };
    addAndMakeVisible(m_clearProjectDummyDataButton.get());

#ifdef USE_DBPR_PROJECT_UTILS
    setProjectDummyData(ProjectData());
#endif

    // trigger lookandfeel update to init button images
    lookAndFeelChanged();
}

ProjectDummyDataLoaderComponent::~ProjectDummyDataLoaderComponent()
{

}

void ProjectDummyDataLoaderComponent::resized()
{
	auto bounds = getLocalBounds();

    m_clearProjectDummyDataButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
    bounds.removeFromRight(4);
    m_loadProjectDummyDataButton->setBounds(bounds.removeFromLeft(3 * bounds.getHeight()));
    bounds.removeFromLeft(4);
    m_currentProjectDummyDataInfoLabel->setBounds(bounds);
}

#ifdef USE_DBPR_PROJECT_UTILS
void ProjectDummyDataLoaderComponent::setProjectDummyData(const juce::String& dummyDataString)
{
    setProjectDummyData(ProjectData::FromString(dummyDataString));
}

void ProjectDummyDataLoaderComponent::setProjectDummyData(const ProjectData& dummyData)
{
    m_currentProjectDummyData = dummyData;

    if (m_currentProjectDummyDataInfoLabel)
        m_currentProjectDummyDataInfoLabel->setText(dummyData.IsEmpty() ? "<EMPTY>" : dummyData.GetInfoString());
}
#endif

void ProjectDummyDataLoaderComponent::loadProjectClicked()
{
    // create the file chooser dialog
    auto chooser = std::make_unique<FileChooser>("Select a d&b project file to load..."); // onyl dbpr files allowed
    // and trigger opening it
    chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser)
        {
#if JUCE_IOS || JUCE_ANDROID
            auto url = chooser.getURLResult();
        
#if JUCE_IOS
            auto inputStream = std::unique_ptr<juce::InputStream>(juce::URLInputSource(url).createInputStream());
#elif JUCE_ANDROID
            auto androidDocument = juce::AndroidDocument::fromDocument (url);
            auto inputStream = std::unique_ptr<juce::InputStream>(androidDocument.createInputStream());
#endif
        
            if (inputStream != nullptr)
            {
                openAndReadProject(inputStream);
            }
#else
            auto file = chooser.getResult();
    
            // verify that the result is valid (ok clicked)
            if (!file.getFullPathName().isEmpty())
            {
                openAndReadProject(file.getFullPathName());
            }
#endif
        
            delete static_cast<const FileChooser*>(&chooser);
        });
    chooser.release();
}

void ProjectDummyDataLoaderComponent::clearProjectClicked()
{
#ifdef USE_DBPR_PROJECT_UTILS
    ProjectData emptyPD;
    setProjectDummyData(emptyPD);

    if (onProjectDummyDataLoaded)
        onProjectDummyDataLoaded(emptyPD.ToString());
#endif
}

void ProjectDummyDataLoaderComponent::openAndReadProject(const juce::String& fileName)
{
#ifdef USE_DBPR_PROJECT_UTILS
    auto projectData = ProjectData::OpenAndReadProject(fileName);

    // some sanity checking
    if (projectData._coordinateMappingData.empty() || projectData._speakerPositionData.empty())
    {
        ShowUserErrorNotification(SEC_InvalidProjectFile);
        return;
    }

    setProjectDummyData(projectData);
        
    if (onProjectDummyDataLoaded)
        onProjectDummyDataLoaded(m_currentProjectDummyData.ToString());
#endif
}

void ProjectDummyDataLoaderComponent::openAndReadProject(const std::unique_ptr<juce::InputStream>& inputStream)
{
    juce::MemoryBlock destBlock;
    inputStream->readIntoMemoryBlock(destBlock);
    
    auto tempFileName = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getFullPathName() + "/tempFile.sq3";
    auto tempFile = juce::File(tempFileName);
    auto tempFileOutputStream = std::make_unique<juce::FileOutputStream>(tempFile);
    tempFileOutputStream->write(destBlock.getData(), destBlock.getSize());
    
    openAndReadProject(tempFileName);
    
    tempFileOutputStream.reset();
    tempFile.deleteFile();
}

void ProjectDummyDataLoaderComponent::lookAndFeelChanged()
{
    // first forward the call to base implementation
    Component::lookAndFeelChanged();

    // Update drawable button images with updated lookAndFeel colours
    UpdateDrawableButtonImages(m_clearProjectDummyDataButton, BinaryData::clear_black_24dp_svg, &getLookAndFeel());
}


}
