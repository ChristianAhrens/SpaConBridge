/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

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


#pragma once

#include "../About.h"
#include "../Gui.h"
#include "../SoundscapeBridgeAppCommon.h"


namespace SoundscapeBridgeApp
{


/**
 * Forward declarations
 */
class COverviewTableContainer;
class CTableModelComponent;
class CComboBoxContainer;
class CTextEditorContainer;
class CRadioButtonContainer;
class CEditableLabelContainer;
class SoundsourceProcessorEditor;


/**
 * Class COverviewTableContainer is just a component which contains the overview table 
 * and it's quick selection buttons.
 */
class COverviewTableContainer : public AOverlay,
	public Button::Listener
{
public:
	COverviewTableContainer();
	~COverviewTableContainer() override;

	void UpdateGui(bool init) override;
	void buttonClicked(Button*) override;

	void onCurrentSelectedProcessorChanged(ProcessorId selectedProcessorId);

protected:
	void paint(Graphics&) override;
	void resized() override;

private:
	/**
	 * The actual table model / component inside this component.
	 */
	std::unique_ptr<CTableModelComponent> m_overviewTable;

	/**
	 * The processor editor component corresponding to the selected row
	 */
	std::unique_ptr<SoundsourceProcessorEditor> m_selectedProcessorInstanceEditor;

	/**
	 * Button to add a processor instance
	 */
	std::unique_ptr<CButton> m_addInstance;

	/**
	 * Button to remove the selected processor instance
	 */
	std::unique_ptr<CButton> m_removeInstance;

	/**
	 * Quick select label
	 */
	std::unique_ptr<CLabel>	m_selectLabel;

	/**
	 * Select all rows button.
	 */
	std::unique_ptr<CButton> m_selectAll;

	/**
	 * Select no rows button.
	 */
	std::unique_ptr<CButton> m_selectNone;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(COverviewTableContainer)
};


/**
 * Class CTableModelComponent acts as a table model and a component at the same time.
 */
class CTableModelComponent : public Component,
	public TableListBoxModel
{
public:

	/**
	 * Enum defininig the table columns used in the overview table.
	 */
	enum OverviewColumn
	{
		OC_None = 0,		//< Juce column IDs start at 1
		OC_TrackID,
		OC_SourceID,
		OC_Mapping,
		OC_ComsMode,
		OC_BridgingMute,
		OC_MAX_COLUMNS
	};

	CTableModelComponent();
	~CTableModelComponent() override;

	static bool LessThanSourceId(ProcessorId pId1, ProcessorId pId2);
	static bool LessThanMapping(ProcessorId pId1, ProcessorId pId2);
	static bool LessThanComsMode(ProcessorId pId1, ProcessorId pId2);
	static bool LessThanBridgingMute(ProcessorId pId1, ProcessorId pId2);

	ProcessorId GetProcessorIdForRow(int rowNumber);
	std::vector<ProcessorId> GetProcessorIdsForRows(std::vector<int> rowNumbers);
	std::vector<int> GetSelectedRows() const;
	void SelectAllRows(bool all);
	void RecreateTableRowIds();
	void UpdateTable();
	TableListBox& GetTable() { return m_table; }


	// Overriden methods from TableListBoxModel
	void backgroundClicked(const MouseEvent &) override;
	int getNumRows() override;
	void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
	void sortOrderChanged(int newSortColumnId, bool isForwards) override;
	Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
	int getColumnAutoSizeWidth(int columnId) override;
	void selectedRowsChanged(int lastRowSelected) override;


	// Overriden methods from Component
	void resized() override;

	// Callback functions
	std::function<void(ProcessorId)>	currentSelectedProcessorChanged;

private:
	/**
	 * The table component itself.
	 */
	TableListBox	m_table;

	/**
	 * Local list of Plug-in instance IDs, one for each row in the table.
	 */
	std::vector<ProcessorId> m_ids;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CTableModelComponent)
};


/**
 * Class CComboBoxContainer is a container for the MappingId Combo box component used in the Overview table.
 */
class CComboBoxContainer : public Component,
	public ComboBox::Listener
{
public:
	explicit CComboBoxContainer(CTableModelComponent& td);
	~CComboBoxContainer() override;

	void comboBoxChanged(ComboBox *comboBox) override;
	void resized() override;
	void SetRow(int newRow);

private:
	/**
	 * Table where this component is contained.
	 */
	CTableModelComponent&	m_owner;

	/**
	 * Actual combo box component.
	 */
	ComboBox				m_comboBox;

	/**
	 * Row number where this component is located inside the table.
	 */
	int						m_row;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CComboBoxContainer)
};


/**
 * Class CTextEditorContainer is a container for the SourceID CTextEditor component used in the Overview table.
 */
class CTextEditorContainer : public Component,
	public TextEditor::Listener
{
public:
	explicit CTextEditorContainer(CTableModelComponent& td);
	~CTextEditorContainer() override;

	void textEditorFocusLost(TextEditor &) override;
	void textEditorReturnKeyPressed(TextEditor &) override;
	void resized() override;
	void SetRow(int newRow);

private:
	/**
	 * Table where this component is contained.
	 */
	CTableModelComponent&	m_owner;

	/**
	 * Actual text editor.
	 */
	CTextEditor				m_editor;

	/**
	 * Row number where this component is located inside the table.
	 */
	int						m_row;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CTextEditorContainer)
};


/**
 * Class CRadioButtonContainer is a container for the Tx/Rx buttons used in the Overview table.
 */
class CRadioButtonContainer : public Component,
	public Button::Listener
{
public:
	explicit CRadioButtonContainer(CTableModelComponent& td);
	~CRadioButtonContainer() override;

	void buttonClicked(Button*) override;
	void resized() override;
	void SetRow(int newRow);

private:
	/**
	 * Table where this component is contained.
	 */
	CTableModelComponent&	m_owner;

	/**
	 * Actual Tx button.
	 */
	CButton				m_txButton;

	/**
	 * Actual Rx button.
	 */
	CButton				m_rxButton;

	/**
	 * Row number where this component is located inside the table.
	 */
	int						m_row;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CRadioButtonContainer)
};


/**
 * Class CMuteButtonContainer is a container for the Bridging Mute buttons used in the Overview table.
 */
class CMuteButtonContainer : public Component,
	public Button::Listener
{
public:
	explicit CMuteButtonContainer(CTableModelComponent& td);
	~CMuteButtonContainer() override;

	void buttonClicked(Button*) override;
	void resized() override;
	void SetRow(int newRow);

private:
	/**
	 * Table where this component is contained.
	 */
	CTableModelComponent& m_owner;

	/**
	 * Actual Mute button.
	 */
	CMuteButton				m_muteButton;

	/**
	 * Row number where this component is located inside the table.
	 */
	int						m_row;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CMuteButtonContainer)
};


/**
 * Class CEditableLabelContainer is a container for editable labels used in the Overview table.
 */
class CEditableLabelContainer : public Label
{
public:
	explicit CEditableLabelContainer(CTableModelComponent& td);
	~CEditableLabelContainer() override;

	void mouseDown(const MouseEvent& event) override;
	void mouseDoubleClick(const MouseEvent &) override;
	void SetRow(int newRow);

private:
	/**
	 * Table where this component is contained.
	 */
	CTableModelComponent&	m_owner;

	/**
	 * Row number where this component is located inside the table.
	 */
	int						m_row;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CEditableLabelContainer)
};


} // namespace SoundscapeBridgeApp