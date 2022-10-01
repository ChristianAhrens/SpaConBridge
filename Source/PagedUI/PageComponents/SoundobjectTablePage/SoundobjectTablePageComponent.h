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
 * Reimplementation of basic resizerbar for custom painting implementation
 */
class TriplePointResizerBar : public StretchableLayoutResizerBar
{
public:
    TriplePointResizerBar(StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical);
    ~TriplePointResizerBar() override;
    
    void paint(Graphics& g) override;

	void mouseUp(const MouseEvent& e) override;

	std::function<void()>	onResizeBarMoved;
};

/**
 * Class BlackFrameMultiSoundobjectComponentHelper is a minimal helper to draw a black frame around the MultiSoundobjectComponent
 * when used in SoundobjectTablePageComonent and handles its adding/removing from the class's ownership
 */
class BlackFrameMultiSoundobjectComponentHelper : public Component
{
public:
    BlackFrameMultiSoundobjectComponentHelper();
    ~BlackFrameMultiSoundobjectComponentHelper() override;
    
    void paint(Graphics& g) override;
    void resized() override;
    
    void addInternalComponent();
    void removeInternalComponent();
};

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

	void SetResizeBarRatio(float ratio);
	float GetResizeBarRatio();
    bool IsResizeBarRatioUpdatePending();

	void SetSingleSelectionOnly(bool singleSelectionOnly);
	bool GetSingleSelectionOnly();

	//==============================================================================
	void SetPageIsVisible(bool visible) override;

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

	//==============================================================================
	void activateStretchableSplitLayout();
	void deactivateStretchableSplitLayout();

private:
    void UpdateLayoutRatio();
    
	std::unique_ptr<SoundobjectTableComponent>		m_soundobjectsTable;				/**< The actual table model / component inside this component. */
	std::unique_ptr<SoundobjectProcessorEditor>		m_selectedProcessorInstanceEditor;	/**< The processor editor component corresponding to the selected row */

	bool											m_isHorizontalSlider;				    /**< Indication if the layout slider currently is shown horizontally (vs. vertically). */
	int												m_layoutManagerItemCount{ 0 };		    /**< Helper to keep track of the pages layouting 'mode'. */
	std::unique_ptr<StretchableLayoutManager>		m_layoutManager;					    /**< The layout manager object instance. */
	std::unique_ptr<TriplePointResizerBar>	        m_layoutResizeBar;					    /**< The layout slider object instance. */
	float											m_resizeBarRatio{ 0.5f };			    /**< The size ratio of table vs. details contents devided by the resizerbar. */
    bool                                            m_resizeBarRatioUpdatePending{ true };  /**< Indication if the last set resizeBarRatio was already applied. */

	bool	                                                    m_multiSoundobjectsActive;
    std::unique_ptr<BlackFrameMultiSoundobjectComponentHelper>  m_multiSoundobjectComponentContainer;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundobjectTablePageComponent)
};


} // namespace SpaConBridge
