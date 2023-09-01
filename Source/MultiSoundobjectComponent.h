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

#include "SpaConBridgeCommon.h"

#include "StandalonePollingBase.h"


namespace SpaConBridge
{

/**
 * Fwd. decl.
 */
class MultiSoundobjectSlider;
class SelectGroupSelector;

/**
 * Class MultiSoundobjectComponent is just a component which contains the multi-source slider
 * and the mapping selection control.
 */
class MultiSoundobjectComponent :	public Component,
									public ComboBox::Listener,
									public ToggleButton::Listener,
									public StandalonePollingBase
{
public:
	MultiSoundobjectComponent();
	~MultiSoundobjectComponent() override;

	//==============================================================================
	void UpdateGui(bool init);

	//==============================================================================
	MappingAreaId GetSelectedMapping() const;
	bool SetSelectedMapping(MappingAreaId mapping);

	bool IsReverbVisuEnabled() const;
	void SetReverbVisuEnabled(bool enabled);
	bool IsSpreadVisuEnabled() const;
	void SetSpreadVisuEnabled(bool enabled);
	bool IsMuSelVisuEnabled() const;
	void SetMuSelVisuEnabled(bool enabled);

	const juce::Image* GetBackgroundImage(MappingAreaId mappingAreaId);
	void SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage);
	void RemoveBackgroundImage(MappingAreaId mappingAreaId);
	
	bool IsHandlingSelectedOnly() const;
	void SetHandleSelectedOnly(bool selectedOnly);

	//==============================================================================
	void lookAndFeelChanged() override;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==============================================================================
	void comboBoxChanged(ComboBox *comboBox) override;

	//==============================================================================
	void buttonClicked(Button* button) override;

	//==============================================================================
	void HandleObjectDataInternal(const RemoteObjectIdentifier& roi, const RemoteObjectMessageData& msgData) override;

private:
	//==============================================================================
	std::unique_ptr<MultiSoundobjectSlider>	m_multiSoundobjectSlider;	/**> Multi-source 2D-Slider. */

	std::unique_ptr<ComboBox>				m_mappingAreaSelect;		/**> ComboBox selector for the coordinate mapping area. */

	std::unique_ptr<DrawableButton>			m_loadImage;				/**< Button to load background image. */
	std::unique_ptr<DrawableButton>			m_removeImage;				/**< Button to remove background image. */

	std::unique_ptr<DrawableButton>			m_muselvisuEnable;			/**> Checkbox for enabling dedicated multiselection visu/interaction mode. */

	std::unique_ptr<SelectGroupSelector>	m_selectionGroupSelect;		/**> ComboBox selector for selection groups. */

	std::unique_ptr<DrawableButton>			m_objectNamesEnable;		/**> Checkbox for enabling showing object names. */

	std::unique_ptr<DrawableButton>			m_reverbEnable;				/**> Checkbox for reverb send gain enable. */

	std::unique_ptr<DrawableButton>			m_spreadEnable;				/**> Checkbox for spread factor enable. */


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSoundobjectComponent)
};


} // namespace SpaConBridge
