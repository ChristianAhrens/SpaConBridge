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

#pragma once

#include <JuceHeader.h>

#include <MidiLearnerComponent.h>

#include "AssignmentEditOverlayBaseComponents.h"

/**
 * Fwd. decls.
 */
namespace JUCEAppBasics {
    class FixedFontTextEditor;
    class TextWithImageButton;
}

namespace SpaConBridge
{

class SceneIndexToMidiAssignerComponent :
    public Component,
    public Button::Listener
{
public:
    explicit SceneIndexToMidiAssignerComponent(std::int16_t refId = -1);
    ~SceneIndexToMidiAssignerComponent();
	
    //==============================================================================
    void resized() override;

    //==============================================================================
    void buttonClicked(Button*) override;

    //==============================================================================
    std::function<void(Component*, std::map<String,JUCEAppBasics::MidiCommandRangeAssignment>)> onAssignmentsSet;
    
    //==============================================================================
    void setSelectedDeviceIdentifier(const String& deviceIdentifier);
    void setCurrentScenesToMidiAssignments(const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& currentAssignments);

    void setReferredId(std::int16_t refId);
    std::int16_t getReferredId() const;
    
private:
    class SceneIndexAssignmentEditComponent : public AssignmentEditOverlayBaseComponents::AssignmentEditComponent
    {
    public:
        explicit SceneIndexAssignmentEditComponent(std::int16_t refId, const String& deviceIdentifier, const String& sceneIndex, const JUCEAppBasics::MidiCommandRangeAssignment& currentAssi);
        ~SceneIndexAssignmentEditComponent();

        const String GetSceneIndex();
        const JUCEAppBasics::MidiCommandRangeAssignment& GetCurrentAssignment();

        void handleMidiAssiSet(Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi);

        //==============================================================================
        std::function<void(Component*, const String&, const JUCEAppBasics::MidiCommandRangeAssignment&)> onAssignmentSet;

        //==============================================================================
        void resized() override;

    private:
        //==============================================================================
        String m_sceneIndex;

        std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>         m_sceneIndexEdit;
        std::unique_ptr<TextEditor::LengthAndCharacterRestriction>  m_sceneIndexEditFilter;
        std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>        m_learnerComponent;
    };

    class SceneIndexAssignmentsListingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsListingComponent
    {
    public:
        explicit SceneIndexAssignmentsListingComponent(const String& deviceIdentifier, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& initialAssignments);
        ~SceneIndexAssignmentsListingComponent();

        //==============================================================================
        void setWidth(int width) override;

        //==============================================================================
        std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> GetCurrentAssignments();
        bool AddAssignment() override;

        //==============================================================================
        const String DumpCurrentAssignmentsToCsvString() override;
        bool ReadAssignmentsFromCsvString(const String& csvAssignmentsString) override;
        
        //==============================================================================
        void resized() override;

        //==============================================================================
        std::function<void(Component*, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>&)> onAssigningFinished;

    private:
        //==============================================================================
        String GetNextSceneIndex();

        //==============================================================================
        String  m_deviceIdentifier;

    };

    class SceneIndexAssignmentsViewingComponent : public AssignmentEditOverlayBaseComponents::AssignmentsViewingComponent
    {
    public:
        explicit SceneIndexAssignmentsViewingComponent(const String& deviceIdentifier, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& initialAssignments);
        ~SceneIndexAssignmentsViewingComponent();

        //==============================================================================
        std::map<juce::String, JUCEAppBasics::MidiCommandRangeAssignment> GetCurrentAssignments();

        //==============================================================================
        void onExportClicked() override;
        void onImportClicked() override;
        void onCloseClicked() override;

        //==============================================================================
        std::function<void(Component*, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>&)> onAssigningFinished;

    private:
        //==============================================================================
        String m_deviceIdentifier;
    };

    void triggerEditAssignments();
    void finishEditAssignments();
    void processAssignmentResult(Component* sender, const String& sceneIndex, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssignment);
    void processAssignmentResults(Component* sender, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& midiAssignments);

    std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>         m_currentMidiAssisLabel;
    std::unique_ptr<TextButton>                                 m_editAssignmentsButton;

    std::unique_ptr<SceneIndexAssignmentsViewingComponent>      m_assignmentsEditionOverlay;

    String                                                      m_deviceIdentifier;
    String                                                      m_deviceName;

    std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> m_currentScenesToMidiAssignments;
    std::int16_t                                                m_referredId{ -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneIndexToMidiAssignerComponent)
};

};
