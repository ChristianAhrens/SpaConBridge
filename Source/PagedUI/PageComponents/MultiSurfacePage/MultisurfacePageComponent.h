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
	void SetSelectedMapping(MappingAreaId mapping);

	bool IsReverbEnabled() const;
	void SetReverbEnabled(bool enabled);
	bool IsSpreadEnabled() const;
	void SetSpreadEnabled(bool enabled);

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
	MappingAreaId						m_selectedMapping{ MappingAreaId::MAI_First };	/**< Remember the last selected coordinate mapping for the multi-slider. */

	std::unique_ptr<DrawableButton>		m_reverbEnable;			/**> Checkbox for reverb send gain enable. */

	std::unique_ptr<DrawableButton>		m_spreadEnable;			/**> Checkbox for spread factor enable. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSurfacePageComponent)
};


} // namespace SpaConBridge
