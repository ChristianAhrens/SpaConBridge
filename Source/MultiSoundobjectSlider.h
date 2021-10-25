/* Copyright (c) 2020-2021, Christian Ahrens
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

#include <JuceHeader.h>
#include "SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * SoundobjectSlider for displaying and controlling multiple sources.
 */
class MultiSoundobjectSlider  : public Component
{
public:
	struct SoundobjectParameters
	{
		SoundobjectParameters() : 
			_id(-1), 
			_pos(Point<float>(0.0f, 0.0f)),
			_spread(0.0f),
			_reverbSndGain(0.0f),
			_selected(false),
			_colour(Colours::black),
			_size(0.5f)
		{};
		SoundobjectParameters(SoundobjectId id, const Point<float>& pos, float spread, float reverbSendGain, bool selected, const Colour& colour, double size) : 
			_id(id), 
			_pos(pos),
			_spread(spread),
			_reverbSndGain(reverbSendGain),
			_selected(selected),
			_colour(colour),
			_size(size)
		{};

		SoundobjectId	_id;
		Point<float>	_pos;
		float			_spread;
		float			_reverbSndGain;
		bool			_selected;
		Colour			_colour;
		double			_size;
	};
	typedef std::map<SoundobjectProcessorId, SoundobjectParameters> ParameterCache;

	MultiSoundobjectSlider();
	MultiSoundobjectSlider(bool spreadEnabled, bool reverbSndGainEnabled);
	~MultiSoundobjectSlider() override;

	MappingAreaId GetSelectedMapping() const;
	void SetSelectedMapping(MappingAreaId mapping);

	bool IsSpreadEnabled();
	void SetSpreadEnabled(bool enabled);
	bool IsReverbSndGainEnabled();
	void SetReverbSndGainEnabled(bool enabled);

	bool HasBackgroundImage(MappingAreaId mappingAreaId);
	const juce::Image* GetBackgroundImage(MappingAreaId mappingAreaId);
	void SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage);
	void RemoveBackgroundImage(MappingAreaId mappingAreaId);

	void UpdateParameters(const ParameterCache& positions);

	void paint (Graphics& g) override;
	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSoundobjectSlider)
	SoundobjectProcessorId					m_currentlyDraggedId;	/**> ProcessorId of the currently selected knob, if any. */
	std::vector<SoundobjectId>				m_highlightedIds;		/**> SourceIds of the currently highlighted knobs, if any. */
	ParameterCache							m_cachedParameters;		/**> To save us from iterating over all Soundobject Processors at every click, cache their current parametervalues.
																	 * Keys are the SoundobjectProcessorId of each object processor. */
	bool									m_spreadEnabled;
	bool									m_reverbSndGainEnabled;
	MappingAreaId							m_selectedMapping;		/**< Remember the last selected coordinate mapping for the multi-slider. */
	std::map<MappingAreaId, juce::Image>	m_backgroundImages;
};


} // namespace SpaConBridge
