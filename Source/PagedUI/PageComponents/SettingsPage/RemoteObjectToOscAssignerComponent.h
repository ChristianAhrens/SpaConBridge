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

namespace SpaConBridge
{

class RemoteObjectToOscAssignerComponent :
    public Component,
    public Button::Listener
{
public:
    explicit RemoteObjectToOscAssignerComponent(std::int16_t refId = -1);
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

    void setReferredId(std::int16_t refId);
    std::int16_t getReferredId() const;
    
private:
    //class AssignmentEditComponent : public Component
    //{
    //public:
    //    explicit AssignmentEditComponent(std::int16_t refId, const String& deviceIdentifier, const RemoteObjectIdentifier& remoteObjectId, const juce::String& currentAssi);
    //    ~AssignmentEditComponent();
    //
    //    const RemoteObjectIdentifier GetRemoteObjectId();
    //    const juce::String& GetCurrentAssignment();
    //
    //    void handleRemoteObjectToOscAssiSet(Component* sender, const juce::String& oscAssi);
    //
    //    //==============================================================================
    //    std::function<void(Component*, const RemoteObjectIdentifier&, const juce::String&)> onAssignmentSet;
    //
    //    //==============================================================================
    //    void resized() override;
    //
    //private:
    //    //==============================================================================
    //    RemoteObjectIdentifier m_remoteObjectId;
    //
    //    //std::unique_ptr<TextEditor>                                 m_sceneIndexEdit;
    //    //std::unique_ptr<TextEditor::LengthAndCharacterRestriction>  m_sceneIndexEditFilter;
    //    //std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>        m_learnerComponent;
    //};

    //class AssignmentsListingComponent : public Component
    //{
    //public:
    //    explicit AssignmentsListingComponent(const String& deviceIdentifier, const std::map<RemoteObjectIdentifier, juce::String>& initialAssignments);
    //    ~AssignmentsListingComponent();
    //
    //    void setWidth(int width);
    //    void setMinHeight(int height);
    //
    //    //==============================================================================
    //    std::map<RemoteObjectIdentifier, juce::String> GetCurrentAssignments();
    //    bool AddAssignment();
    //    void ClearAssignments();
    //
    //    //==============================================================================
    //    const String DumpCurrentAssignmentsToCsvString();
    //    bool ReadAssignmentsFromCsvString(const String& csvAssignmentsString);
    //
    //    //==============================================================================
    //    void paint(Graphics&) override;
    //    void resized() override;
    //
    //    //==============================================================================
    //    std::function<void(Component*, const std::map<RemoteObjectIdentifier, juce::String>&)> onAssigningFinished;
    //
    //private:
    //    //==============================================================================
    //    bool IsAvailableUiAreaExceeded();
    //    String GetNextSceneIndex();
    //
    //    //==============================================================================
    //    std::vector<std::unique_ptr<AssignmentEditComponent>>   m_editComponents;
    //    String                                                  m_deviceIdentifier;
    //    int                                                     m_minHeight;
    //
    //    //==============================================================================
    //    const float m_editorWidth = 225.0f;
    //    const float m_editorHeight = 25.0f;
    //    const float m_editorMargin = 2.0f;
    //
    //};

    //class AssignmentsViewingComponent : public Component
    //{
    //public:
    //    explicit AssignmentsViewingComponent(const String& deviceIdentifier, const std::map<RemoteObjectIdentifier, juce::String>& initialAssignments);
    //    ~AssignmentsViewingComponent();
    //
    //    std::map<RemoteObjectIdentifier, juce::String> GetCurrentAssignments();
    //
    //    //==============================================================================
    //    void onAddClicked();
    //    void onClearClicked();
    //    void onExportClicked();
    //    void onImportClicked();
    //    void onCloseClicked();
    //
    //    //==============================================================================
    //    void paint(Graphics&) override;
    //    void resized() override;
    //
    //    //==========================================================================
    //    void lookAndFeelChanged() override;
    //
    //    //==============================================================================
    //    std::function<void(Component*, const std::map<RemoteObjectIdentifier, juce::String>&)> onAssigningFinished;
    //
    //private:
    //    //==============================================================================
    //    std::unique_ptr<AssignmentsListingComponent>    m_contentComponent;
    //    std::unique_ptr<Viewport>					    m_contentViewport;
    //    std::unique_ptr<TextButton>                     m_addButton;
    //    std::unique_ptr<TextButton>                     m_clearButton;
    //    std::unique_ptr<DrawableButton>                 m_exportButton;
    //    std::unique_ptr<DrawableButton>                 m_importButton;
    //    std::unique_ptr<TextButton>                     m_closeButton;
    //
    //    String m_deviceIdentifier;
    //
    //};

    void triggerEditAssignments();
    void finishEditAssignments();
    void processAssignmentResult(Component* sender, const RemoteObjectIdentifier& remoteObjectId, const juce::String& roiToOscAssignment);
    void processAssignmentResults(Component* sender, const std::map<RemoteObjectIdentifier, juce::String>& roiToOscAssignments);

    std::unique_ptr<TextEditor>                     m_currentRoiToOscAssisLabel;
    std::unique_ptr<TextButton>                     m_editAssignmentsButton;

    //std::unique_ptr<AssignmentsViewingComponent>    m_assignmentsEditionOverlay;

    String                                          m_deviceIdentifier;
    String                                          m_deviceName;

    std::map<RemoteObjectIdentifier, juce::String>  m_currentRoiToOscAssignments;
    std::int16_t                                    m_referredId{ -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemoteObjectToOscAssignerComponent)
};

};
