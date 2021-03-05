/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in and now in a derived version is part of SoundscapeBridgeApp.

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

#include "PageComponentBase.h"

#include "../../SoundscapeBridgeAppCommon.h"
#include "../../AppConfiguration.h"


namespace SoundscapeBridgeApp
{


/**
 * Forward declarations
 */
class ComboBoxContainer;
class TextEditorContainer;
class RadioButtonContainer;
class EditableLabelContainer;
class SoundobjectProcessorEditor;


/**
 * Class CustomTableHeaderComponent acts as a table model and a component at the same time.
 */
class CustomTableHeaderComponent : public TableHeaderComponent
{
public:
	/**
	 * Enum defininig the table columns available for the channel table derivates.
	 */
	enum TableColumn
	{
		TC_None = 0,		//< Juce column IDs start at 1
		TC_TrackID,
		TC_SoundobjectID,
		TC_InputID,
		TC_OutputID,
		TC_InputEditor,
		TC_OutputEditor,
		TC_Mapping,
		TC_ComsMode,
		TC_BridgingMute,
		TC_MAX_COLUMNS
	};

	/**
	 * Structure that collects all properties needed to initialize a table column.
	 */
	struct ColumnProperties
	{
		ColumnProperties() {};
		ColumnProperties(
			String	columnName,
			int		width,
			int		minimumWidth,
			int		maximumWidth,
			int		propertyFlags,
			int		insertIndex = -1)
		{
			_columnName = columnName;
			_width = width;
			_minimumWidth = minimumWidth;
			_maximumWidth = maximumWidth;
			_propertyFlags = propertyFlags;
			_insertIndex = insertIndex;
		};

		String	_columnName;
		int		_width;
		int		_minimumWidth;
		int		_maximumWidth;
		int		_propertyFlags;
		int		_insertIndex;
	};

public:
	CustomTableHeaderComponent(const std::map<TableColumn, ColumnProperties>& tableColumns, TableColumn sortColumn = TC_None);
	~CustomTableHeaderComponent() override;

	void paint(Graphics& g) override;
	void resized() override;

	void updateBridgingTitles();
	void updateColumnWidths();

private:
	std::map<ProtocolBridgingType, bool>	m_bridgingProtocolActive;
};

/**
 * Class TableModelComponent acts as a table model and a component at the same time.
 */
class TableModelComponent : public Component,
							public TableListBoxModel
{
public:

	TableModelComponent();
	~TableModelComponent() override;

	void SetModel(TableListBoxModel* model);

	//==========================================================================
	virtual void RecreateTableRowIds() = 0;
	virtual void UpdateTable() = 0;

	//==========================================================================
	static bool LessThanSourceId(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanMapping(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanComsMode(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanBridgingMute(juce::int32 pId1, juce::int32 pId2);

	juce::int32 GetProcessorIdForRow(int rowNumber) const;
	std::vector<juce::int32> GetProcessorIdsForRows(const std::vector<int>& rowNumbers) const;
	int GetRowForProcessorId(juce::int32 processorId) const;
	std::vector<int> GetRowsForProcessorIds(const std::vector<juce::int32>& processorIds) const;

	TableListBox& GetTable() { return m_table; }
	std::vector<juce::int32>& GetProcessorIds() { return m_processorIds; }

	std::vector<int> GetSelectedRows() const;
	void SetSelectedRows(const std::vector<int>& rows);
	void SelectAllRows(bool all);

	//==========================================================================
	void backgroundClicked(const MouseEvent &) override;
	void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
	void sortOrderChanged(int newSortColumnId, bool isForwards) override;
	Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;
	int getColumnAutoSizeWidth(int columnId) override;
	void selectedRowsChanged(int lastRowSelected) override;

	//==========================================================================
	void resized() override;

	// Callback functions
	std::function<void(juce::int32)>	currentSelectedProcessorChanged;

private:
	TableListBox				m_table;			/**> The table component itself. */
	std::vector<juce::int32>	m_processorIds;		/**> Local list of Soundobject Processor instance IDs, one for each row in the table. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TableModelComponent)
};


/**
 * Class ComboBoxContainer is a container for the MappingId Combo box component used in the Overview table.
 */
class ComboBoxContainer : public Component,
	public ComboBox::Listener
{
public:
	explicit ComboBoxContainer(TableModelComponent& td);
	~ComboBoxContainer() override;

	void comboBoxChanged(ComboBox *comboBox) override;
	void resized() override;
	void SetRow(int newRow);

private:
	TableModelComponent&	m_owner;	/**> Table where this component is contained. */
	ComboBox				m_comboBox;	/**> Actual combo box component. */
	int						m_row;		/**< Row number where this component is located inside the table. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBoxContainer)
};


/**
 * Class TextEditorContainer is a container for the SourceID TextEditor component used in the Overview table.
 */
class TextEditorContainer : public Component,
	public TextEditor::Listener
{
public:
	explicit TextEditorContainer(TableModelComponent& td);
	~TextEditorContainer() override;

	void textEditorFocusLost(TextEditor &) override;
	void textEditorReturnKeyPressed(TextEditor &) override;
	void resized() override;
	void SetRow(int newRow);

private:
	TableModelComponent&	m_owner;	/**> Table where this component is contained. */
	TextEditor				m_editor;	/**> Actual text editor. */
	int						m_row;		/**> Row number where this component is located inside the table. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TextEditorContainer)
};


/**
 * Class RadioButtonContainer is a container for the Tx/Rx buttons used in the Overview table.
 */
class RadioButtonContainer : public Component,
	public Button::Listener
{
public:
	explicit RadioButtonContainer(TableModelComponent& td);
	~RadioButtonContainer() override;

	void lookAndFeelChanged() override;

	void buttonClicked(Button*) override;
	void resized() override;
	void SetRow(int newRow);
	void updateButtons();

private:
	TableModelComponent&	m_owner;	/**> Table where this component is contained. */
	DrawableButton			m_txButton;	/**> Actual Tx button. */
	DrawableButton			m_rxButton;	/**> Actual Rx button. */
	int						m_row;		/**> Row number where this component is located inside the table. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RadioButtonContainer)
};


/**
 * Class MuteButtonContainer is a container for the Bridging Mute buttons used in the Overview table.
 */
class MuteButtonContainer : public Component,
	public Button::Listener
{
public:
	explicit MuteButtonContainer(TableModelComponent& td);
	~MuteButtonContainer() override;

	void lookAndFeelChanged() override;

	void buttonClicked(Button*) override;
	void resized() override;
	void SetRow(int newRow);
	void updateBridgingMuteButtons();
	void updateDrawableButtonImageColours();

private:
	TableModelComponent&											m_owner;			/**< Table where this component is contained. */
	int																m_row;				/**< Row number where this component is located inside the table. */
	std::map<ProtocolBridgingType, std::unique_ptr<DrawableButton>>	m_bridgingMutes;	/**< The mute buttons currently in use. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MuteButtonContainer)
};


/**
 * Class EditableLabelContainer is a container for editable labels used in the Overview table.
 */
class EditableLabelContainer : public Label
{
public:
	explicit EditableLabelContainer(TableModelComponent& td);
	~EditableLabelContainer() override;

	void mouseDown(const MouseEvent& event) override;
	void mouseDoubleClick(const MouseEvent &) override;
	void SetRow(int newRow);

private:
	
	TableModelComponent&	m_owner;	/**> Table where this component is contained. */
	int						m_row;		/**> Row number where this component is located inside the table. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EditableLabelContainer)
};


} // namespace SoundscapeBridgeApp
