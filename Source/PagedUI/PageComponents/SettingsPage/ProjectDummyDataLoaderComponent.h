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

#pragma once

#include <JuceHeader.h>

#include "dbprProjectUtils.h"

 /**
 * Fwd. decls.
 */
namespace JUCEAppBasics {
    class FixedFontTextEditor;
}

namespace SpaConBridge
{

class ProjectDummyDataLoaderComponent :
    public juce::Component
{
public:
    explicit ProjectDummyDataLoaderComponent();
    ~ProjectDummyDataLoaderComponent();
	
    //==============================================================================
    void resized() override;

    //==============================================================================
    void lookAndFeelChanged() override;

    //==============================================================================
    std::function<void(const juce::String&)> onProjectDummyDataLoaded;
    
#ifdef USE_DBPR_PROJECT_UTILS
    //==============================================================================
    void setProjectDummyData(const juce::String& dummyDataString);
    void setProjectDummyData(const ProjectData& dummyData);
#endif
    
private:
    //==============================================================================
    void loadProjectClicked();
    void clearProjectClicked();
    void openAndReadProject(const juce::String& fileName);
    void openAndReadProject(const std::unique_ptr<juce::InputStream>& inputStream);

    //==============================================================================
    std::unique_ptr<JUCEAppBasics::FixedFontTextEditor> m_currentProjectDummyDataInfoLabel;
    std::unique_ptr<juce::TextButton>                   m_loadProjectDummyDataButton;
    std::unique_ptr<juce::DrawableButton>               m_clearProjectDummyDataButton;

#ifdef USE_DBPR_PROJECT_UTILS
    ProjectData     m_currentProjectDummyData;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectDummyDataLoaderComponent)
};

};
