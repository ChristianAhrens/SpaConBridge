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


#pragma once

#include "PageComponentBase.h"

#include "../../SpaConBridgeCommon.h"
#include "../../AppConfiguration.h"


namespace SpaConBridge
{


/**
 * Forward declarations
 */
class TableControlBarComponent;


/**
 * Class TableModelComponent acts as a table model and a component at the same time.
 */
class TableModelComponent : public Component,
							public TableListBoxModel
{
public:
	enum ControlBarPosition
	{
		CBP_Top,
		CBP_Bottom,
		CBP_Left,
		CBP_Right
	};

public:

	TableModelComponent(ControlBarPosition pos = ControlBarPosition::CBP_Bottom, bool tableCanCollapse = false);
	~TableModelComponent() override;

	void SetModel(TableListBoxModel* model);
	void SetControlBarPosition(ControlBarPosition pos);

	void SetTableType(TableType tt);
	TableType GetTableType();

	//==========================================================================
	virtual void RecreateTableRowIds() = 0;
	virtual void UpdateTable() = 0;

	//==========================================================================
	static bool LessThanSoundobjectId(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanMatrixInputId(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanMatrixOutputId(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanMapping(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanComsMode(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanSoundobjectBridgingMute(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanMatrixInputBridgingMute(juce::int32 pId1, juce::int32 pId2);
	static bool LessThanMatrixOutputBridgingMute(juce::int32 pId1, juce::int32 pId2);

	juce::int32 GetProcessorIdForRow(int rowNumber) const;
	std::vector<juce::int32> GetProcessorIdsForRows(const std::vector<int>& rowNumbers) const;
	int GetRowForProcessorId(juce::int32 processorId) const;
	std::vector<int> GetRowsForProcessorIds(const std::vector<juce::int32>& processorIds) const;

	TableListBox* GetTable() { return m_table.get(); }
	std::vector<juce::int32>& GetProcessorIds() { return m_processorIds; }

	int GetRowHeight();
	void SetRowHeight(int rowHeight);

	bool IsCollapsed();
	void SetCollapsed(bool collapsed);

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
	std::function<void(juce::int32)>	onCurrentSelectedProcessorChanged;
	std::function<void(int)>			onCurrentRowHeightChanged;
	std::function<void(bool)>			onCurrentCollapseStateChanged;

protected:
	//==============================================================================
	virtual void onAddProcessor() = 0;
	virtual void onRemoveProcessor() = 0;

	//==============================================================================
	void onSelectAllProcessors();
	void onDeselectAllProcessors();
	void onRowHeightSlided(int height);
	void onCollapseToggled(bool collapsed);

private:
	std::unique_ptr<TableListBox>				m_table;					/**> The table component itself. */
	TableType									m_tableType{ TT_Invalid };	/**> The type of table component. */
	std::unique_ptr<TableControlBarComponent>	m_tableControlBar;			/**> The control bottom bar. */
	ControlBarPosition							m_controlBarPosition;		/**> The position of the control bar. */
	std::vector<juce::int32>					m_processorIds;				/**> Local list of Processor instance IDs, one for each row in the table. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TableModelComponent)
};


} // namespace SpaConBridge
