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

#include <JuceHeader.h>
#include "../SoundscapeBridgeAppCommon.h"


namespace SoundscapeBridgeApp
{


/**
 * SurfaceSlider class provides a 2D-Slider or "X/Y controller".
 */
class CSurfaceSlider  : public Component
{
public:
	CSurfaceSlider(AudioProcessor* parent);
	~CSurfaceSlider() override;

	void paint (Graphics& g) override;
	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

private:
	AudioProcessor*	m_parent = nullptr; /**> AudioProcessor object to act as parent to this component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CSurfaceSlider)
};


/**
 * SurfaceSlider for displaying and controlling multiple sources.
 */
class CSurfaceMultiSlider  : public Component
{
public:
	typedef std::map<ProcessorId, std::pair<int, Point<float>>> PositionCache;

	CSurfaceMultiSlider();
	~CSurfaceMultiSlider() override;

	void UpdatePositions(PositionCache positions);

	void paint (Graphics& g) override;
	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CSurfaceMultiSlider)
	ProcessorId		m_selected;			/**> ProcessorId of the currently selected knob, if any. */
	PositionCache	m_cachedPositions;	/**> To save us from iterating over all Plug-ins at every click, cache the source positions.
										 * Keys are the PluginIds of each source, while values are pairs of the corresponding
										 * input number and position coordinates (0.0 to 1.0). */
};


} // namespace SoundscapeBridgeApp
