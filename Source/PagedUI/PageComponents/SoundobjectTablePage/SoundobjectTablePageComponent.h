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

#include "../PageComponentBase.h"

#include "../../../SpaConBridgeCommon.h"
#include "../../../AppConfiguration.h"


namespace SpaConBridge
{


/**
 * Forward declarations
 */
class BridgingAwareTableHeaderComponent;
class SoundobjectTableComponent;
class SoundobjectProcessorEditor;


/**
 * Class SoundobjectTablePageComponent is just a component which contains the overview table 
 * and it's quick selection buttons.
 */
class SoundobjectTablePageComponent :	public PageComponentBase,
										public AppConfiguration::Watcher
{
public:
	SoundobjectTablePageComponent();
	~SoundobjectTablePageComponent() override;

	void SetRowHeight(int height);
	int GetRowHeight();

	//==============================================================================
	void UpdateGui(bool init) override;

	//==============================================================================
	void SetSoundsourceProcessorEditorActive(SoundobjectProcessorId processorId);
	void SetMultiSoundobjectComponentActive(bool active);

	//==========================================================================
	void onConfigUpdated() override;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	std::unique_ptr<SoundobjectTableComponent>		m_soundobjectsTable;				/**< The actual table model / component inside this component. */
	std::unique_ptr<SoundobjectProcessorEditor>		m_selectedProcessorInstanceEditor;	/**< The processor editor component corresponding to the selected row */

	bool											m_isHorizontalSlider;				/**< Indication if the layout slider currently is shown horizontally (vs. vertically). */
	std::unique_ptr<StretchableLayoutManager>		m_layoutManager;					/**< The layout manager object instance. */
	std::unique_ptr<StretchableLayoutResizerBar>	m_layoutResizerBar;					/**< The layout slider object instance. */

	bool	m_multiSoundobjectsActive;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundobjectTablePageComponent)
};


} // namespace SpaConBridge
