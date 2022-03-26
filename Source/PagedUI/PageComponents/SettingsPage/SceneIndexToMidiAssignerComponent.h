/* Copyright (c) 2020-2022, Christian Ahrens
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

namespace SpaConBridge
{

class SceneIndexToMidiAssignerComponent :
    public Component,
    public Button::Listener
{
public:
    SceneIndexToMidiAssignerComponent(std::int16_t refId = -1);
    ~SceneIndexToMidiAssignerComponent();
	
    //==============================================================================
    void resized() override;

    //==============================================================================
    void buttonClicked(Button*) override;

    //==============================================================================
    std::function<void(Component*, std::map<String,JUCEAppBasics::MidiCommandRangeAssignment>)> onAssignmentsSet;
    
    //==============================================================================
    void setSelectedDeviceIdentifier(const String& deviceIdentifier);
    //void setCurrentMidiAssi(const JUCEAppBasics::MidiCommandRangeAssignment& currentAssi);

    void setReferredId(std::int16_t refId);
    std::int16_t getReferredId() const;
    
private:
    class LearnerPopupCustomComponent : public PopupMenu::CustomComponent
    {
    public:
        LearnerPopupCustomComponent(std::int16_t refId, const String& sceneIndex, const String& deviceIdentifier, const JUCEAppBasics::MidiCommandRangeAssignment& currentAssi)
            : PopupMenu::CustomComponent(false)
        {
            m_sceneIndex = sceneIndex;

            m_sceneIndexEdit = std::make_unique<TextEditor>("SceneIndexEditor");
            m_sceneIndexEdit->setText(sceneIndex);
            //m_sceneIndexEdit->setInputFilter()
            addAndMakeVisible(m_sceneIndexEdit.get());

            m_learnerComponent = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(refId, JUCEAppBasics::MidiLearnerComponent::AT_Trigger);
            m_learnerComponent->setSelectedDeviceIdentifier(deviceIdentifier);
            m_learnerComponent->setCurrentMidiAssi(currentAssi);
            addAndMakeVisible(m_learnerComponent.get());
            m_learnerComponent->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
        }
        ~LearnerPopupCustomComponent()
        {

        }

        const String GetSceneIndex()
        {
            return m_sceneIndexEdit->getText();
        }
        const JUCEAppBasics::MidiCommandRangeAssignment& GetCurrentAssignment()
        {
            return m_learnerComponent->getCurrentMidiAssi();
        }

        void resized() override
        {
            auto bounds = getLocalBounds();

            m_learnerComponent->setBounds(bounds.removeFromRight(static_cast<int>(0.7f * bounds.getWidth()) - 2));
            bounds.removeFromRight(4);
            m_sceneIndexEdit->setBounds(bounds);
        }

        void getIdealSize(int& idealWidth, int& idealHeight) override
        {
            idealWidth = 205;
            idealHeight = 25;
        }

        void handleMidiAssiSet(Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi)
        {
            auto learnerComponent = dynamic_cast<JUCEAppBasics::MidiLearnerComponent*>(sender);
            if (learnerComponent && onAssignmentSet)
            {
                onAssignmentSet(this, m_sceneIndex, midiAssi);
            }
        }

        std::function<void(Component*, const String&, const JUCEAppBasics::MidiCommandRangeAssignment&)> onAssignmentSet;

    private:
        String m_sceneIndex;
        
        std::unique_ptr<TextEditor>                             m_sceneIndexEdit;
        std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>    m_learnerComponent;
    };

    class AddItemCustomComponent :  public PopupMenu::CustomComponent, 
                                    public Button::Listener
    {
    public:
        AddItemCustomComponent()
        {
            m_addButton = std::make_unique<TextButton>("Add assignment", "AddItemButton");
            m_addButton->addListener(this);
            addAndMakeVisible(m_addButton.get());
        }
        ~AddItemCustomComponent()
        {

        }

        void resized() override
        {
            m_addButton->setBounds(getLocalBounds());
        }

        void getIdealSize(int& idealWidth, int& idealHeight) override
        {
            idealWidth = 205;
            idealHeight = 25;
        }

        //==============================================================================
        std::function<void(Component*)> onAddItemClicked;

        //==============================================================================
        void buttonClicked(Button*) override
        {
            if (onAddItemClicked)
                onAddItemClicked(this);
        }


    private:
        std::unique_ptr<TextButton> m_addButton;
    };

    void triggerEditAssignments();
    void addAssignmentComponent();
    void updatePopupMenu();
    void handlePopupResult(int resultingAssiIdx);

    std::unique_ptr<TextEditor>                                 m_currentMidiAssisLabel;
    std::unique_ptr<TextButton>                                 m_editAssignmentsButton;
    String                                                      m_deviceIdentifier;
    String                                                      m_deviceName;
    PopupMenu                                                   m_popup;

    std::unique_ptr<AddItemCustomComponent>                     m_addItemComponent;
    std::vector<std::unique_ptr<LearnerPopupCustomComponent>>   m_assignmentComponents;
    
    std::unique_ptr<MidiInput>                                  m_midiInput;
    std::int16_t                                                m_referredId{ -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneIndexToMidiAssignerComponent)
};

};
