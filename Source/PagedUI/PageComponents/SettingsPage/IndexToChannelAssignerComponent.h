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

#include "../../../SpaConBridgeCommon.h"

#include <TextWithImageButton.h>

#include "AssignmentEditOverlayBaseComponents.h"

namespace SpaConBridge
{

class IndexToChannelAssignerComponent :
    public Component,
    public Button::Listener
{
public:
    explicit IndexToChannelAssignerComponent();
    ~IndexToChannelAssignerComponent();
	
    //==============================================================================
    void resized() override;

    //==============================================================================
    void buttonClicked(Button*) override;

    //==============================================================================
    std::function<void(Component*, std::map<int, ChannelId>)> onAssignmentsSet;
    
    //==============================================================================
    void setCurrentIndexToChannelAssignments(const std::map<int, ChannelId>& currentAssignments);
    
private:
    class IndexToChannelAssignmentEditComponent : public AssignmentEditOverlayBaseComponents::AssignmentEditComponent
    {
    public:
        explicit IndexToChannelAssignmentEditComponent( int index, ChannelId currentChannelAssi);
        ~IndexToChannelAssignmentEditComponent();
    
        int GetCurrentIndex();
        ChannelId GetCurrentChannelAssignment();
    
        void handleEditorInput();
        void handleIndexToChannelAssiSet(ChannelId channelAssi);
        void handleIndexToChannelAssiReset();
    
        //==============================================================================
        std::function<void(Component*, int, ChannelId)> onAssignmentSet;
    
        //==============================================================================
        void resized() override;
    
    private:
        //==============================================================================
        int         m_currentIndex;
        ChannelId   m_currentChannelAssignment;
    
        std::unique_ptr<TextEditor> m_indexEditComponent;
        std::unique_ptr<TextEditor> m_channelAssignmentEditComponent;
    };

    class IndexToChannelAssignmentsListingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsListingComponent
    {
    public:
        explicit IndexToChannelAssignmentsListingComponent(const std::map<int, ChannelId>& initialAssignments);
        ~IndexToChannelAssignmentsListingComponent();

        //==============================================================================
        void setWidth(int width) override;

        //==============================================================================
        std::map<int, ChannelId> GetCurrentAssignments();
        bool AddAssignment() override;

        //==============================================================================
        const String DumpCurrentAssignmentsToCsvString() override;
        bool ReadAssignmentsFromCsvString(const String& csvAssignmentsString) override;

        //==============================================================================
        void resized() override;

        //==============================================================================
        std::function<void(Component*, const std::map<int, ChannelId>&)> onAssigningFinished;

        //==============================================================================
        std::unique_ptr<juce::Label>    m_beaconIdxHeader;
        std::unique_ptr<juce::Label>    m_channelAssignmentHeader;

    };

    class IndexToChannelAssignmentsViewingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsViewingComponent
    {
    public:
        explicit IndexToChannelAssignmentsViewingComponent(const std::map<int, ChannelId>& initialAssignments);
        ~IndexToChannelAssignmentsViewingComponent();

        //==============================================================================
        std::map<int, ChannelId> GetCurrentAssignments();

        //==============================================================================
        void onExportClicked() override;
        void onImportClicked() override;
        void onCloseClicked() override;

        //==============================================================================
        std::function<void(Component*, const std::map<int, ChannelId>&)> onAssigningFinished;

    private:
        //==============================================================================
    };

    void triggerEditAssignments();
    void finishEditAssignments();
    void processAssignmentResult(Component* sender, int index, ChannelId channelAssignment);
    void processAssignmentResults(Component* sender, const std::map<int, ChannelId>& indexToChannelAssignments);

    std::unique_ptr<TextEditor>                                 m_currentIdxToChAssisLabel;
    std::unique_ptr<TextButton>                                 m_editAssignmentsButton;

    std::unique_ptr<IndexToChannelAssignmentsViewingComponent>  m_assignmentsEditionOverlay;

    String                                                      m_deviceIdentifier;
    String                                                      m_deviceName;

    std::map<int, ChannelId>                                    m_currentIdxToChAssignments;
    std::int16_t                                                m_referredId{ -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndexToChannelAssignerComponent)
};

};
