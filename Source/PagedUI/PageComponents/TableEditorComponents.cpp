/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in and now in a derived version is part of SpaConBridge.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include "TableEditorComponents.h"

#include "TableModelComponent.h"

#include "../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"

#include "../../Controller.h"
#include "../../LookAndFeel.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
	Class TableEditorComponent
===============================================================================
*/

/**
 * Class constructor.
 */
TableEditorComponent::TableEditorComponent(TableModelComponent& td)
	: m_owner(td), m_row(0)
{
}

/**
 * Class destructor.
 */
TableEditorComponent::~TableEditorComponent()
{
}

/**
 * Getter for the parent table reference member
 * @return The reference to the parent table object
 */
TableModelComponent& TableEditorComponent::GetParentTable()
{
	return m_owner;
}

/**
 * Getter for the internal row number member
 * @return	The row number
 */
int TableEditorComponent::GetRow()
{
	return m_row;
}

/**
 * Setter for the internal row number member
 * @param	newRow	The new row number value to set to internal member
 */
void TableEditorComponent::SetRow(int newRow)
{
	m_row = newRow;
}


/*
===============================================================================
 Class ComboBoxContainer
===============================================================================
*/

/**
 * Class constructor.
 */
ComboBoxContainer::ComboBoxContainer(TableModelComponent& td)
	: TableEditorComponent(td)
{
	// Create and configure actual combo box component inside this container.
	m_comboBox.setEditableText(false);
	m_comboBox.addItem("1", 1);
	m_comboBox.addItem("2", 2);
	m_comboBox.addItem("3", 3);
	m_comboBox.addItem("4", 4);
	m_comboBox.addListener(this);
	m_comboBox.setWantsKeyboardFocus(false);
	addAndMakeVisible(m_comboBox);
}

/**
 * Class destructor.
 */
ComboBoxContainer::~ComboBoxContainer()
{
}

/**
 * Reimplemented from ComboBox::Listener, gets called whenever the selected combo box item is changed.
 * @param comboBox	The comboBox which has been changed.
 */
void ComboBoxContainer::comboBoxChanged(ComboBox *comboBox)
{
	if (&m_comboBox != comboBox)
		return;

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Get the list of rows which are currently selected on the table.
	std::vector<int> selectedRows = GetParentTable().GetSelectedRows();
	if ((selectedRows.size() < 2) ||
		(std::find(selectedRows.begin(), selectedRows.end(), GetRow()) == selectedRows.end()))
	{
		// If this comboBoxes row (m_row) is NOT selected, or if no multi-selection was made 
		// then modify the selectedRows list so that it only contains m_row.
		selectedRows.clear();
		selectedRows.push_back(GetRow());
	}

	// Get the IDs of the procssors on the selected rows.
	auto processorIds = GetParentTable().GetProcessorIdsForRows(selectedRows);

	// New MappingID which should be applied to all procssors in the selected rows.
	auto newMapping = static_cast<MappingId>(comboBox->getSelectedId());
	for (auto const& processorId : processorIds)
	{
		switch (GetParentTable().GetTableType())
		{
		case TT_MatrixInputs:
		case TT_MatrixOutputs:
			jassertfalse;
			break;
		case TT_Soundobjects:
		default:
			{
				auto processor = ctrl->GetSoundobjectProcessor(processorId);
				if (processor)
					processor->SetMappingId(DCP_SoundobjectTable, newMapping);
			}
			break;
		}
	}
}

/**
 * Reimplemented from Component, used to resize the actual combo box component inside.
 */
void ComboBoxContainer::resized()
{
	m_comboBox.setBoundsInset(BorderSize<int>(4, 4, 5, 4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updated the combo box's selected item according to that procssor's MappingID.
 * @param newRow	The new row number.
 */
void ComboBoxContainer::SetRow(int newRow)
{
	TableEditorComponent::SetRow(newRow);
	
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Find the procssor instance corresponding to the given row number.
	auto processorId = GetParentTable().GetProcessorIdForRow(newRow);

	switch (GetParentTable().GetTableType())
	{
	case TT_MatrixInputs:
	case TT_MatrixOutputs:
		jassertfalse;
		break;
	case TT_Soundobjects:
	default:
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
				m_comboBox.setSelectedId(processor->GetMappingId(), dontSendNotification);
		}
		break;
	}
}


/*
===============================================================================
 Class LabelContainer
===============================================================================
*/

/**
 * Class constructor.
 */
LabelContainer::LabelContainer(TableModelComponent& td)
	: TableEditorComponent(td)
{
	addAndMakeVisible(m_label);
}

/**
 * Class destructor.
 */
LabelContainer::~LabelContainer()
{
}

/**
 * Reimplemented from Component, used to resize the actual combo box component inside.
 */
void LabelContainer::resized()
{
	m_label.setBoundsInset(BorderSize<int>(4, 4, 5, 4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updated the combo box's selected item according to that procssor's MappingID.
 * @param newRow	The new row number.
 */
void LabelContainer::SetRow(int newRow)
{
	TableEditorComponent::SetRow(newRow);

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Find the procssor instance corresponding to the given row number.
	auto processorId = GetParentTable().GetProcessorIdForRow(newRow);

	switch (GetParentTable().GetTableType())
	{
	case TT_MatrixInputs:
	case TT_MatrixOutputs:
	case TT_Soundobjects:
	default:
	{
		auto processor = ctrl->GetSoundobjectProcessor(processorId);
		if (processor)
			m_label.setText(processor->getProgramName(processor->getCurrentProgram()), dontSendNotification);
	}
	break;
	}
}


/*
===============================================================================
 Class TextEditorContainer
===============================================================================
*/

/**
 * Class constructor.
 */
TextEditorContainer::TextEditorContainer(TableModelComponent& td)
	: TableEditorComponent(td)
{
	// Create and configure actual textEditor component inside this container.
	m_editor.addListener(this);
	addAndMakeVisible(m_editor);
}

/**
 * Class destructor.
 */
TextEditorContainer::~TextEditorContainer()
{
}

/**
 * Reimplemented from TextEditor::Listener, gets called whenever the TextEditor loses keyboard focus.
 * @param textEditor	The textEditor which has been changed.
 */
void TextEditorContainer::textEditorFocusLost(TextEditor& textEditor)
{
	if (&m_editor != &textEditor)
		return;

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// New SourceID which should be applied to all procssors in the selected rows.
	auto newSourceId = textEditor.getText().getIntValue();
	auto processorId = GetParentTable().GetProcessorIdForRow(GetRow());
	switch (GetParentTable().GetTableType())
	{
	case TT_MatrixInputs:
		{
			auto processor = ctrl->GetMatrixInputProcessor(processorId);
			if (processor)
				processor->SetMatrixInputId(DCP_MatrixInputTable, newSourceId);
		}
		break;
	case TT_MatrixOutputs:
		{
			auto processor = ctrl->GetMatrixOutputProcessor(processorId);
			if (processor)
				processor->SetMatrixOutputId(DCP_MatrixOutputTable, newSourceId);
		}
		break;
	case TT_Soundobjects:
	default:
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
				processor->SetSoundobjectId(DCP_SoundobjectTable, newSourceId);
		}
		break;
	}
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void TextEditorContainer::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	if (&m_editor != &textEditor)
		return;

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Get the list of rows which are currently selected on the table.
	std::vector<int> selectedRows = GetParentTable().GetSelectedRows();
	if ((selectedRows.size() < 2) ||
		(std::find(selectedRows.begin(), selectedRows.end(), GetRow()) == selectedRows.end()))
	{
		// If this comboBoxes row (m_row) is NOT selected, or if no multi-selection was made 
		// then modify the selectedRows list so that it only contains m_row.
		selectedRows.clear();
		selectedRows.push_back(GetRow());
	}

	// New SourceID which should be applied to all procssors in the selected rows.
	auto newSourceId = textEditor.getText().getIntValue();
	for (auto const& processorId : GetParentTable().GetProcessorIdsForRows(selectedRows))
	{
		switch (GetParentTable().GetTableType())
		{
		case TT_MatrixInputs:
		{
			auto processor = ctrl->GetMatrixInputProcessor(processorId);
			if (processor)
				processor->SetMatrixInputId(DCP_MatrixInputTable, newSourceId);
		}
		break;
		case TT_MatrixOutputs:
		{
			auto processor = ctrl->GetMatrixOutputProcessor(processorId);
			if (processor)
				processor->SetMatrixOutputId(DCP_MatrixOutputTable, newSourceId);
		}
		break;
		case TT_Soundobjects:
		default:
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
				processor->SetSoundobjectId(DCP_SoundobjectTable, newSourceId);
		}
		break;
		}
	}
}

/**
 * Reimplemented from Component, used to resize the actual component inside.
 */
void TextEditorContainer::resized()
{
	m_editor.setBoundsInset(BorderSize<int>(4, 4, 5, 4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the text inside the textEditor with the current SourceID
 * @param newRow	The new row number.
 */
void TextEditorContainer::SetRow(int newRow)
{
	TableEditorComponent::SetRow(newRow);
	
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Find the procssor instance corresponding to the given row number.
	auto processorId = GetParentTable().GetProcessorIdForRow(newRow);

	switch (GetParentTable().GetTableType())
	{
	case TT_MatrixInputs:
		{
			auto processor = ctrl->GetMatrixInputProcessor(processorId);
			if (processor)
				m_editor.setText(String(processor->GetMatrixInputId()), false);
		}
		break;
	case TT_MatrixOutputs:
		{
			auto processor = ctrl->GetMatrixOutputProcessor(processorId);
			if (processor)
				m_editor.setText(String(processor->GetMatrixOutputId()), false);
		}
		break;
	case TT_Soundobjects:
	default:
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
				m_editor.setText(String(processor->GetSoundobjectId()), false);
		}
		break;
	}
}

/**
 * Setter vor the values to use in internal length and character restriction filter instance.
 * @param maxNumChars			if this is > 0, it sets a maximum length limit; if <= 0, no
 * 								limit is set
 * @param allowedCharacters		if this is non-empty, then only characters that occur in
 * 								this string are allowed to be entered into the editor.
 */
void TextEditorContainer::setLengthAndCharacterRestriction(int maxNumChars, const juce::String& allowedCharacters)
{
	m_lengthAndCharacterFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(maxNumChars, allowedCharacters);
	m_editor.setInputFilter(m_lengthAndCharacterFilter.get(), false);
}


/*
===============================================================================
 Class RadioButtonContainer
===============================================================================
*/

/**
 * Class constructor.
 */
RadioButtonContainer::RadioButtonContainer(TableModelComponent& td)
	: TableEditorComponent(td),
	m_txButton("Tx", DrawableButton::ButtonStyle::ImageOnButtonBackground), 
	m_rxButton("Rx", DrawableButton::ButtonStyle::ImageOnButtonBackground)
{
	// Create and configure button components inside this container.
	m_txButton.setClickingTogglesState(true);
	m_txButton.setEnabled(true);
	m_txButton.addListener(this);
	addAndMakeVisible(m_txButton);

	m_rxButton.setClickingTogglesState(true);
	m_rxButton.setEnabled(true);
	m_rxButton.addListener(this);
	addAndMakeVisible(m_rxButton);

	lookAndFeelChanged();
}

/**
 * Class destructor.
 */
RadioButtonContainer::~RadioButtonContainer()
{
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void RadioButtonContainer::buttonClicked(Button *button)
{
	if ((button != &m_txButton) && (button != &m_rxButton))
		return;

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	bool newToggleState = button->getToggleState();

	// Get the list of rows which are currently selected on the table.
	auto selectedRows = GetParentTable().GetSelectedRows();
	if ((selectedRows.size() < 2) ||
		(std::find(selectedRows.begin(), selectedRows.end(), GetRow()) == selectedRows.end()))
	{
		// If this button's row (m_row) is NOT selected, or if no multi-selection was made 
		// then modify the selectedRows list so that it only contains m_row.
		selectedRows.clear();
		selectedRows.push_back(GetRow());
	}

	for (auto const& processorId : GetParentTable().GetProcessorIdsForRows(selectedRows))
	{
		switch (GetParentTable().GetTableType())
		{
		case TT_MatrixInputs:
			{
				auto processor = ctrl->GetMatrixInputProcessor(processorId);
				if (processor)
				{
					ComsMode oldMode = processor->GetComsMode();
					ComsMode newFlag = (button == &m_txButton) ? CM_Tx : CM_Rx;

					if (newToggleState == true)
						oldMode |= newFlag;
					else
						oldMode &= ~newFlag;

					processor->SetComsMode(DCP_MatrixInputTable, oldMode);
				}
			}
			break;
		case TT_MatrixOutputs:
			{
				auto processor = ctrl->GetMatrixOutputProcessor(processorId);
				if (processor)
				{
					ComsMode oldMode = processor->GetComsMode();
					ComsMode newFlag = (button == &m_txButton) ? CM_Tx : CM_Rx;

					if (newToggleState == true)
						oldMode |= newFlag;
					else
						oldMode &= ~newFlag;

					processor->SetComsMode(DCP_MatrixOutputTable, oldMode);
				}
			}
			break;
		case TT_Soundobjects:
		default:
			{
				auto processor = ctrl->GetSoundobjectProcessor(processorId);
				if (processor)
				{
					ComsMode oldMode = processor->GetComsMode();
					ComsMode newFlag = (button == &m_txButton) ? CM_Tx : CM_Rx;

					if (newToggleState == true)
						oldMode |= newFlag;
					else
						oldMode &= ~newFlag;

					processor->SetComsMode(DCP_SoundobjectTable, oldMode);
				}
			}
			break;
		}

	}
}

/**
 * Reimplemented from Component, used to resize the actual component inside.
 */
void RadioButtonContainer::resized()
{
	auto bounds = getLocalBounds();
	bounds.removeFromBottom(1);
	auto singleButtonWidth = bounds.getWidth() / 2;

	auto buttonRect = bounds.removeFromLeft(singleButtonWidth).reduced(4);
	m_txButton.setBounds(buttonRect);
	buttonRect = bounds.removeFromLeft(singleButtonWidth).reduced(4);
	m_rxButton.setBounds(buttonRect);
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the radio buttons with the current ComsMode.
 * @param newRow	The new row number.
 */
void RadioButtonContainer::SetRow(int newRow)
{
	TableEditorComponent::SetRow(newRow);

	// Find the procssor instance corresponding to the given row number.
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	switch (GetParentTable().GetTableType())
	{
	case TT_MatrixInputs:
		{
			auto processor = ctrl->GetMatrixInputProcessor(GetParentTable().GetProcessorIdForRow(newRow));
			if (processor)
			{
				ComsMode newMode = processor->GetComsMode();
				m_txButton.setToggleState(((newMode & CM_Tx) == CM_Tx), dontSendNotification);
				m_rxButton.setToggleState(((newMode & CM_Rx) == CM_Rx), dontSendNotification);
			}
			else
			{
				m_txButton.setEnabled(false);
				m_rxButton.setEnabled(false);
			}
		}
		break;
	case TT_MatrixOutputs:
		{
			auto processor = ctrl->GetMatrixOutputProcessor(GetParentTable().GetProcessorIdForRow(newRow));
			if (processor)
			{
				ComsMode newMode = processor->GetComsMode();
				m_txButton.setToggleState(((newMode & CM_Tx) == CM_Tx), dontSendNotification);
				m_rxButton.setToggleState(((newMode & CM_Rx) == CM_Rx), dontSendNotification);
			}
			else
			{
				m_txButton.setEnabled(false);
				m_rxButton.setEnabled(false);
			}
		}
		break;
	case TT_Soundobjects:
	default:
		{
			auto processor = ctrl->GetSoundobjectProcessor(GetParentTable().GetProcessorIdForRow(newRow));
			if (processor)
			{
				ComsMode newMode = processor->GetComsMode();
				m_txButton.setToggleState(((newMode & CM_Tx) == CM_Tx), dontSendNotification);
				m_rxButton.setToggleState(((newMode & CM_Rx) == CM_Rx), dontSendNotification);
			}
			else
			{
				m_txButton.setEnabled(false);
				m_rxButton.setEnabled(false);
			}
		}
		break;
	}
}

/**
 *
 */
void RadioButtonContainer::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(&m_txButton, BinaryData::call_made24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(&m_rxButton, BinaryData::call_received24px_svg, &getLookAndFeel());

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		auto blueColour = dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::ButtonBlueColor);
		m_txButton.setColour(TextButton::ColourIds::buttonOnColourId, blueColour.brighter(0.05f));
		m_rxButton.setColour(TextButton::ColourIds::buttonOnColourId, blueColour.brighter(0.05f));
	}
}


/*
===============================================================================
 Class MuteButtonContainer
===============================================================================
*/

/**
 * Class constructor.
 */
MuteButtonContainer::MuteButtonContainer(TableModelComponent& td)
	: TableEditorComponent(td)
{
	lookAndFeelChanged();
}

/**
 * Class destructor.
 */
MuteButtonContainer::~MuteButtonContainer()
{
}

/**
 * Helper method to update the map of bridging mute buttons by querying
 * data from controller. This should be called on configuration updates
 * that affect bridging protocol active state.
 */
void MuteButtonContainer::updateBridgingMuteButtons()
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// collect what bridging modules are active
	auto activeBridging = ctrl->GetActiveProtocolBridging();

	for (auto type : ProtocolBridgingTypes)
	{
		if (((activeBridging & type) == type) && (m_bridgingMutes.count(type) == 0))
		{
			m_bridgingMutes.insert(std::make_pair(type, std::make_unique<DrawableButton>("Mute", DrawableButton::ButtonStyle::ImageOnButtonBackground)));
			m_bridgingMutes[type]->setImages(m_NormalImage.get(), m_OverImage.get(), m_DownImage.get(), m_DisabledImage.get(), m_NormalOnImage.get(), m_OverOnImage.get(), m_DownOnImage.get(), m_DisabledOnImage.get());
			m_bridgingMutes[type]->setClickingTogglesState(true);
			m_bridgingMutes[type]->setColour(TextButton::ColourIds::buttonOnColourId, m_redColour.brighter(0.05f));
			m_bridgingMutes[type]->setEnabled(true);
			m_bridgingMutes[type]->addListener(this);
			addAndMakeVisible(m_bridgingMutes.at(type).get());
		}
		else if (((activeBridging & type) != type) && (m_bridgingMutes.count(type) > 0))
		{
			m_bridgingMutes.erase(type);
		}
	}

	resized();
}

/**
 * Reimplemented method to process updated lookandfeel settings.
 * In particular the 'mute' red colour and the button image data is updated.
 */
void MuteButtonContainer::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (!dblookAndFeel)
		return;

	// determine the right red colour from lookandfeel
	m_redColour = dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::ButtonRedColor);

	// create the required button drawable images based on lookandfeel colours
	String imageName = BinaryData::mobiledata_off24px_svg;
	JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, m_NormalImage, m_OverImage, m_DownImage, m_DisabledImage, m_NormalOnImage, m_OverOnImage, m_DownOnImage, m_DisabledOnImage,
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

	// set the images to button
	for (auto type : ProtocolBridgingTypes)
	{
		if (m_bridgingMutes.count(type) != 0)
		{
			m_bridgingMutes[type]->setColour(TextButton::ColourIds::buttonOnColourId, m_redColour.brighter(0.05f));
			m_bridgingMutes[type]->setImages(m_NormalImage.get(), m_OverImage.get(), m_DownImage.get(), m_DisabledImage.get(), m_NormalOnImage.get(), m_OverOnImage.get(), m_DownOnImage.get(), m_DisabledOnImage.get());
		}
	}
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void MuteButtonContainer::buttonClicked(Button* button)
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	for (auto type : ProtocolBridgingTypes)
	{
		if ((m_bridgingMutes.count(type) > 0) && (button == m_bridgingMutes.at(type).get()))
		{
			bool newToggleState = button->getToggleState();

			// Get the list of rows which are currently selected on the table.
			auto selectedRows = GetParentTable().GetSelectedRows();
			if ((selectedRows.size() < 2) ||
				(std::find(selectedRows.begin(), selectedRows.end(), GetRow()) == selectedRows.end()))
			{
				// If this button's row (m_row) is NOT selected, or if no multi-selection was made 
				// then modify the selectedRows list so that it only contains m_row.
				selectedRows.clear();
				selectedRows.push_back(GetRow());
			}

			switch (GetParentTable().GetTableType())
			{
			case TT_MatrixInputs:
				// Get the IDs of the processors on the selected rows.
				ctrl->SetMuteBridgingMatrixInputProcessorIds(type, GetParentTable().GetProcessorIdsForRows(selectedRows), newToggleState);
				break;
			case TT_MatrixOutputs:
				// Get the IDs of the processors on the selected rows.
				ctrl->SetMuteBridgingMatrixOutputProcessorIds(type, GetParentTable().GetProcessorIdsForRows(selectedRows), newToggleState);
				break;
			case TT_Soundobjects:
			default:
				// Get the IDs of the processors on the selected rows.
				ctrl->SetMuteBridgingSoundobjectProcessorIds(type, GetParentTable().GetProcessorIdsForRows(selectedRows), newToggleState);
				break;
			}

			GetParentTable().UpdateTable();
		}
	}
}

/**
 * Reimplemented from Component, used to resize the actual component inside.
 */
void MuteButtonContainer::resized()
{
	if (m_bridgingMutes.empty())
		return;

	auto bounds = getLocalBounds();
	bounds.removeFromBottom(1);
	auto singleButtonWidth = static_cast<int>(bounds.getWidth() / m_bridgingMutes.size());

	for (auto& buttonKV : m_bridgingMutes)
	{
		auto buttonRect = bounds.removeFromLeft(singleButtonWidth).reduced(4);
		buttonKV.second->setBounds(buttonRect);
	}
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the radio buttons with the current mute state.
 * @param newRow	The new row number.
 */
void MuteButtonContainer::SetRow(int newRow)
{
	TableEditorComponent::SetRow(newRow);

	// Find the procssor instance corresponding to the given row number.
	auto processorId = GetParentTable().GetProcessorIdForRow(newRow);
	auto ctrl = Controller::GetInstance();
	if (ctrl)
	{
		for (auto type : ProtocolBridgingTypes)
		{
			if (m_bridgingMutes.count(type) > 0)
			{
				switch (GetParentTable().GetTableType())
				{
				case TT_MatrixInputs:
					m_bridgingMutes.at(type)->setToggleState(ctrl->GetMuteBridgingMatrixInputProcessorId(type, processorId), dontSendNotification);
					break;
				case TT_MatrixOutputs:
					m_bridgingMutes.at(type)->setToggleState(ctrl->GetMuteBridgingMatrixOutputProcessorId(type, processorId), dontSendNotification);
					break;
				case TT_Soundobjects:
				default:
					m_bridgingMutes.at(type)->setToggleState(ctrl->GetMuteBridgingSoundobjectProcessorId(type, processorId), dontSendNotification);
					break;
				}
			}
		}
	}
}


/*
===============================================================================
 Class ColourAndSizePickerContainer
===============================================================================
*/

/**
 * Class constructor.
 */
ColourAndSizePickerContainer::ColourAndSizePickerContainer(TableModelComponent& td)
	: TableEditorComponent(td)
{
	m_colourAndSizePicker.onColourAndSizeSet = [=](const juce::Colour& colour, double size) {
		SetSoundObjectColourAndSize(colour, size);
	};
	addAndMakeVisible(m_colourAndSizePicker);
}

/**
 * Class destructor.
 */
ColourAndSizePickerContainer::~ColourAndSizePickerContainer()
{
}

/**
 * Reimplemented from Component, used to resize the actual component inside.
 */
void ColourAndSizePickerContainer::resized()
{
	m_colourAndSizePicker.setBoundsInset(BorderSize<int>(4, 4, 5, 4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the radio buttons with the current mute state.
 * @param newRow	The new row number.
 */
void ColourAndSizePickerContainer::SetRow(int newRow)
{
	TableEditorComponent::SetRow(newRow);

	// Find the procssor instance corresponding to the given row number.
	auto processorId = GetParentTable().GetProcessorIdForRow(newRow);
	auto ctrl = Controller::GetInstance();
	if (ctrl)
	{
		switch (GetParentTable().GetTableType())
		{
		case TT_MatrixInputs:
		case TT_MatrixOutputs:
			jassertfalse;
			break;
		case TT_Soundobjects:
		default:
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
			{
				auto colour = processor->GetSoundobjectColour();
				auto size = processor->GetSoundobjectSize();
				m_colourAndSizePicker.setCurrentColourAndSize(colour, size);
			}
		}
		break;
		}
	}
}

/**
 * Helper method to set a new colour and size for the soundobject associated with this table editor container
 * @param	colour	The new colour to set
 * @param	size	The new size to set
 */
void ColourAndSizePickerContainer::SetSoundObjectColourAndSize(const juce::Colour& colour, double size)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Get the list of rows which are currently selected on the table.
	std::vector<int> selectedRows = GetParentTable().GetSelectedRows();
	if ((selectedRows.size() < 2) ||
		(std::find(selectedRows.begin(), selectedRows.end(), GetRow()) == selectedRows.end()))
	{
		// If this ColourAndSizePicker's row (m_row) is NOT selected, or if no multi-selection was made 
		// then modify the selectedRows list so that it only contains m_row.
		selectedRows.clear();
		selectedRows.push_back(GetRow());
	}

	// Get the IDs of the procssors on the selected rows.
	auto processorIds = GetParentTable().GetProcessorIdsForRows(selectedRows);

	for (auto const& processorId : processorIds)
	{
		switch (GetParentTable().GetTableType())
		{
		case TT_MatrixInputs:
		case TT_MatrixOutputs:
			jassertfalse;
			break;
		case TT_Soundobjects:
		default:
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
			{
				processor->SetSoundobjectColour(DCP_SoundobjectTable, colour);
				processor->SetSoundobjectSize(DCP_SoundobjectTable, size);

				m_colourAndSizePicker.setCurrentColourAndSize(colour, size);
			}
		}
		break;
		}
	}
}


} // namespace SpaConBridge
