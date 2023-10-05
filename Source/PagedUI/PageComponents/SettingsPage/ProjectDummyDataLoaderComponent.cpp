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

namespace SpaConBridge
{

//==============================================================================
ProjectDummyDataLoaderComponent::ProjectDummyDataLoaderComponent()
{
	m_currentProjectDummyDataInfoLabel = std::make_unique<TextEditor>("CurrentProjectDummyDataInfoLabel");
    m_currentProjectDummyDataInfoLabel->setEnabled(false);
    m_currentProjectDummyDataInfoLabel->setReadOnly(true);
	addAndMakeVisible(m_currentProjectDummyDataInfoLabel.get());

    m_loadProjectDummyDataButton = std::make_unique<TextButton>("Load dbpr");
    m_loadProjectDummyDataButton->onClick = [=]() { loadProjectClicked(); };
	addAndMakeVisible(m_loadProjectDummyDataButton.get());

#ifdef USE_DBPR_PROJECT_UTILS
    setProjectDummyData(ProjectData());
#endif
}

ProjectDummyDataLoaderComponent::~ProjectDummyDataLoaderComponent()
{

}

void ProjectDummyDataLoaderComponent::resized()
{
	auto bounds = getLocalBounds();

    m_currentProjectDummyDataInfoLabel->setBounds(bounds.removeFromRight(static_cast<int>(0.6f * bounds.getWidth()) - 2));
	bounds.removeFromRight(4);
    m_loadProjectDummyDataButton->setBounds(bounds);
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
    auto chooser = std::make_unique<FileChooser>("Select a d&b project file to load...",
        File::getSpecialLocation(File::userDesktopDirectory), "*.dbpr", true, false, this); // onyl dbpr files allowed
    // and trigger opening it
    chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser)
        {
            auto file = chooser.getResult();
    
            // verify that the result is valid (ok clicked)
            if (!file.getFullPathName().isEmpty())
            {
                openAndReadProject(file.getFullPathName());
            }
            delete static_cast<const FileChooser*>(&chooser);
        });
    chooser.release();
}

void ProjectDummyDataLoaderComponent::openAndReadProject(const juce::String& fileName)
{
#ifdef USE_DBPR_PROJECT_UTILS
    auto projectData = ProjectData::OpenAndReadProject(fileName);

    setProjectDummyData(projectData);
        
    if (onProjectDummyDataLoaded)
        onProjectDummyDataLoaded(m_currentProjectDummyData.ToString());
#endif
}


}
