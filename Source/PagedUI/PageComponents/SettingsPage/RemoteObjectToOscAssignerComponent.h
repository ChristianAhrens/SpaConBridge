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
    std::function<void(Component*, std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>)> onAssignmentsSet;
    
    //==============================================================================
    void setCurrentRemoteObjecToOscAssignments(const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& currentAssignments);
    
private:
    class RemoteObjectToOscAssignmentEditComponent : public AssignmentEditOverlayBaseComponents::AssignmentEditComponent
    {
    public:
        explicit RemoteObjectToOscAssignmentEditComponent(const RemoteObjectIdentifier& remoteObjectId, const std::pair<juce::String, juce::Range<float>>& currentAssi);
        ~RemoteObjectToOscAssignmentEditComponent();
    
        const RemoteObjectIdentifier GetRemoteObjectId();
        const std::pair<juce::String, juce::Range<float>>& GetCurrentAssignment();
    
        void handleEditorInput();
        void handleRemoteObjectToOscAssiSet(const std::pair<juce::String, juce::Range<float>>& oscAssi);
        void handleRemoteObjectToOscAssiReset();
    
        //==============================================================================
        std::function<void(Component*, const RemoteObjectIdentifier&, const std::pair<juce::String, juce::Range<float>>&)> onAssignmentSet;
    
        //==============================================================================
        void resized() override;

        //==============================================================================
        void lookAndFeelChanged() override;
    
    private:
        //==============================================================================
        RemoteObjectIdentifier                      m_currentRemoteObjectId;
        std::pair<juce::String, juce::Range<float>> m_currentOscAssignment;
    
        std::unique_ptr<ComboBox>   m_remoteObjectSelect;
        std::unique_ptr<TextEditor> m_oscAssignmentEditComponent;
        std::unique_ptr<TextEditor> m_oscAssignmentMinRangeValEditComponent;
        std::unique_ptr<TextEditor> m_oscAssignmentMaxRangeValEditComponent;
    };

    class RemoteObjectToOscAssignmentsListingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsListingComponent
    {
    public:
        explicit RemoteObjectToOscAssignmentsListingComponent(const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& initialAssignments);
        ~RemoteObjectToOscAssignmentsListingComponent();

        //==============================================================================
        void setWidth(int width) override;

        //==============================================================================
        std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>> GetCurrentAssignments();
        bool AddAssignment() override;

        //==============================================================================
        const String DumpCurrentAssignmentsToCsvString() override;
        bool ReadAssignmentsFromCsvString(const String& csvAssignmentsString) override;

        //==============================================================================
        void resized() override;

        //==============================================================================
        std::function<void(Component*, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>&)> onAssigningFinished;

    };

    class RemoteObjectToOscAssignmentsViewingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsViewingComponent
    {
    public:
        explicit RemoteObjectToOscAssignmentsViewingComponent(const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& initialAssignments);
        ~RemoteObjectToOscAssignmentsViewingComponent();

        //==============================================================================
        std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>> GetCurrentAssignments();

        //==============================================================================
        void onExportClicked() override;
        void onImportClicked() override;
        void onCloseClicked() override;

        //==============================================================================
        std::function<void(Component*, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>&)> onAssigningFinished;

    private:
        //==============================================================================
    };

    void triggerEditAssignments();
    void finishEditAssignments();
    void processAssignmentResult(Component* sender, const RemoteObjectIdentifier& remoteObjectId, const std::pair<juce::String, juce::Range<float>>& roiToOscAssignment);
    void processAssignmentResults(Component* sender, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& roiToOscAssignments);

    std::unique_ptr<TextEditor>                                                     m_currentRoiToOscAssisLabel;
    std::unique_ptr<TextButton>                                                     m_editAssignmentsButton;

    std::unique_ptr<RemoteObjectToOscAssignmentsViewingComponent>                   m_assignmentsEditionOverlay;

    String                                                                          m_deviceIdentifier;
    String                                                                          m_deviceName;

    std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>   m_currentRoiToOscAssignments;
    std::int16_t                                                                    m_referredId{ -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemoteObjectToOscAssignerComponent)
};

};
