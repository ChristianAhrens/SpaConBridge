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

#pragma once

#include <JuceHeader.h>

#include "../../../SpaConBridgeCommon.h"

#include <TextWithImageButton.h>

#include "AssignmentEditOverlayBaseComponents.h"

namespace SpaConBridge
{

class RemoteObjectToOscAssignerComponent :
    public Component,
    public Button::Listener
{
public:
    explicit RemoteObjectToOscAssignerComponent();
    ~RemoteObjectToOscAssignerComponent();
	
    //==============================================================================
    void resized() override;

    //==============================================================================
    void buttonClicked(Button*) override;

    //==============================================================================
    std::function<void(Component*, std::map<RemoteObjectIdentifier, juce::String>)> onAssignmentsSet;
    
    //==============================================================================
    void setSelectedDeviceIdentifier(const String& deviceIdentifier);
    void setCurrentRemoteObjecToOscAssignments(const std::map<RemoteObjectIdentifier, juce::String>& currentAssignments);
    
private:
    class RemoteObjectToOscAssignmentEditComponent : public AssignmentEditOverlayBaseComponents::AssignmentEditComponent
    {
    public:
        explicit RemoteObjectToOscAssignmentEditComponent(const RemoteObjectIdentifier& remoteObjectId, const juce::String& currentAssi);
        ~RemoteObjectToOscAssignmentEditComponent();
    
        const RemoteObjectIdentifier GetRemoteObjectId();
        const juce::String& GetCurrentAssignment();
    
        void handleRemoteObjectToOscAssiSet(const juce::String& oscAssi);
        void handleRemoteObjectToOscAssiReset();
    
        //==============================================================================
        std::function<void(Component*, const RemoteObjectIdentifier&, const juce::String&)> onAssignmentSet;
    
        //==============================================================================
        void resized() override;
    
    private:
        //==============================================================================
        RemoteObjectIdentifier  m_currentRemoteObjectId;
        juce::String            m_currentOscAssignment;
    
        std::unique_ptr<ComboBox>   m_remoteObjectSelect;
        std::unique_ptr<TextEditor> m_oscAssignmentEditComponent;
    };

    class RemoteObjectToOscAssignmentsListingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsListingComponent
    {
    public:
        explicit RemoteObjectToOscAssignmentsListingComponent(const std::map<RemoteObjectIdentifier, juce::String>& initialAssignments);
        ~RemoteObjectToOscAssignmentsListingComponent();

        //==============================================================================
        std::map<RemoteObjectIdentifier, juce::String> GetCurrentAssignments();
        bool AddAssignment() override;

        //==============================================================================
        const String DumpCurrentAssignmentsToCsvString() override;
        bool ReadAssignmentsFromCsvString(const String& csvAssignmentsString) override;

        //==============================================================================
        std::function<void(Component*, const std::map<RemoteObjectIdentifier, juce::String>&)> onAssigningFinished;

    private:
        //==============================================================================

    };

    class RemoteObjectToOscAssignmentsViewingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsViewingComponent
    {
    public:
        explicit RemoteObjectToOscAssignmentsViewingComponent(const std::map<RemoteObjectIdentifier, juce::String>& initialAssignments);
        ~RemoteObjectToOscAssignmentsViewingComponent();

        //==============================================================================
        std::map<RemoteObjectIdentifier, juce::String> GetCurrentAssignments();

        //==============================================================================
        void onExportClicked() override;
        void onImportClicked() override;
        void onCloseClicked() override;

        //==============================================================================
        std::function<void(Component*, const std::map<RemoteObjectIdentifier, juce::String>&)> onAssigningFinished;

    private:
        //==============================================================================
    };

    void triggerEditAssignments();
    void finishEditAssignments();
    void processAssignmentResult(Component* sender, const RemoteObjectIdentifier& remoteObjectId, const juce::String& roiToOscAssignment);
    void processAssignmentResults(Component* sender, const std::map<RemoteObjectIdentifier, juce::String>& roiToOscAssignments);

    std::unique_ptr<TextEditor>                                     m_currentRoiToOscAssisLabel;
    std::unique_ptr<TextButton>                                     m_editAssignmentsButton;

    std::unique_ptr<RemoteObjectToOscAssignmentsViewingComponent>   m_assignmentsEditionOverlay;

    String                                                          m_deviceIdentifier;
    String                                                          m_deviceName;

    std::map<RemoteObjectIdentifier, juce::String>                  m_currentRoiToOscAssignments;
    std::int16_t                                                    m_referredId{ -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemoteObjectToOscAssignerComponent)
};

};
