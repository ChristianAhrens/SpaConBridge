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


namespace SpaConBridge
{

/**
 * Fwd. decl.
 */
class MultiSoundobjectSlider;

/**
 * Class MultiSurfacePageComponent is just a component which contains the multi-source slider
 * and the mapping selection control.
 */
class MultiSurfacePageComponent :	public PageComponentBase,
									public ComboBox::Listener,
									public ToggleButton::Listener
{
public:
	MultiSurfacePageComponent();
	~MultiSurfacePageComponent() override;

	//==============================================================================
	void UpdateGui(bool init) override;

	//==============================================================================
	MappingAreaId GetSelectedMapping() const;
	bool SetSelectedMapping(MappingAreaId mapping);

	bool IsReverbEnabled() const;
	void SetReverbEnabled(bool enabled);
	bool IsSpreadEnabled() const;
	void SetSpreadEnabled(bool enabled);

	const juce::Image* GetBackgroundImage(MappingAreaId mappingAreaId);
	void SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage);
	void RemoveBackgroundImage(MappingAreaId mappingAreaId);

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

private:
	std::unique_ptr<MultiSoundobjectSlider>	m_multiSliderSurface;	/**> Multi-source 2D-Slider. */

	std::unique_ptr<ComboBox>			m_mappingAreaSelect;	/**> ComboBox selector for the coordinate mapping area. */
	std::unique_ptr<DrawableButton>		m_loadImage;			/**< Button to load background image. */
	std::unique_ptr<DrawableButton>		m_removeImage;			/**< Button to remove background image. */

	std::unique_ptr<DrawableButton>		m_objectNamesEnable;	/**> Checkbox for enabling showing object names. */

	std::unique_ptr<DrawableButton>		m_reverbEnable;			/**> Checkbox for reverb send gain enable. */

	std::unique_ptr<DrawableButton>		m_spreadEnable;			/**> Checkbox for spread factor enable. */


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSurfacePageComponent)
};


} // namespace SpaConBridge
