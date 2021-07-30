/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in
and now in a derived version is part of SpaConBridge.

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
#include "SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * SurfaceSlider class provides a 2D-Slider or "X/Y controller".
 */
class SurfaceSlider  : public Component
{
public:
	SurfaceSlider(AudioProcessor* parent);
	~SurfaceSlider() override;

	void paint (Graphics& g) override;
	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

private:
	AudioProcessor*	m_parent = nullptr; /**> AudioProcessor object to act as parent to this component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SurfaceSlider)
};


/**
 * SurfaceSlider for displaying and controlling multiple sources.
 */
class SurfaceMultiSlider  : public Component
{
public:
	struct SoundobjectParameters
	{
		SoundobjectParameters() : 
			_id(-1), 
			_pos(Point<float>(0.0f, 0.0f)),
			_spread(0.0f),
			_reverbSndGain(0.0f),
			_selected(false) 
		{};
		SoundobjectParameters(SoundobjectId id, const Point<float>& pos, float spread, float reverbSendGain, bool selected) : 
			_id(id), 
			_pos(pos),
			_spread(spread),
			_reverbSndGain(reverbSendGain),
			_selected(selected)
		{};

		SoundobjectId	_id;
		Point<float>	_pos;
		float			_spread;
		float			_reverbSndGain;
		bool			_selected;
	};
	typedef std::map<SoundobjectProcessorId, SoundobjectParameters> ParameterCache;

	SurfaceMultiSlider();
	~SurfaceMultiSlider() override;

	void UpdateParameters(ParameterCache positions);

	void paint (Graphics& g) override;
	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SurfaceMultiSlider)
	SoundobjectProcessorId		m_currentlyDraggedId;	/**> ProcessorId of the currently selected knob, if any. */
	std::vector<SoundobjectId>	m_highlightedIds;		/**> SourceIds of the currently highlighted knobs, if any. */
	ParameterCache				m_cachedParameters;		/**> To save us from iterating over all Soundobject Processors at every click, cache their current parametervalues.
														 * Keys are the SoundobjectProcessorId of each object processor. */
};


} // namespace SpaConBridge
