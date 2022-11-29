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

#include <JuceHeader.h>
#include "SpaConBridgeCommon.h"

#include "DualPointMultitouchCatcherComponent.h"


namespace SpaConBridge
{


/**
 * SoundobjectSlider for displaying and controlling multiple sources.
 */
class MultiSoundobjectSlider  : public JUCEAppBasics::DualPointMultitouchCatcherComponent
{
public:
    enum MultiTouchDirectionTarget
    {
        MTDT_PendingInputDecision,
        MTDT_HorizontalEnSpaceSendGain,
        MTDT_VerticalSpread
    };
    
	struct SoundobjectParameters
	{
		SoundobjectParameters() : 
			_id(-1), 
			_pos(Point<float>(0.0f, 0.0f)),
			_spread(0.0f),
			_reverbSndGain(0.0f),
			_selected(false),
			_colour(Colours::black),
			_size(0.5f),
			_objectName(String())
		{};
		SoundobjectParameters(SoundobjectId id, const Point<float>& pos, float spread, float reverbSendGain, bool selected, const Colour& colour, double size, const String& objectName) : 
			_id(id), 
			_pos(pos),
			_spread(spread),
			_reverbSndGain(reverbSendGain),
			_selected(selected),
			_colour(colour),
			_size(size),
			_objectName(objectName)
		{};

		SoundobjectId	_id;
		Point<float>	_pos;
		float			_spread;
		float			_reverbSndGain;
		bool			_selected;
		Colour			_colour;
		double			_size;
		String			_objectName;
	};
	enum CacheFlag : std::uint16_t
	{
		None = 0x0000,
		MultiSelection = 0x0001,
	};
	using CacheFlags = std::uint16_t;
	typedef std::tuple<std::map<SoundobjectProcessorId, SoundobjectParameters>, CacheFlags> ParameterCache;
    
    struct MultiTouchPoints
    {
        MultiTouchPoints() :
            _p1(juce::Point<int>(0, 0)),
            _p2_init(juce::Point<int>(0, 0)),
            _p2(juce::Point<int>(0, 0))
        {};
        
        bool isEmpty(){ return _p1.isOrigin() && _p2.isOrigin() && _p2_init.isOrigin(); };
        void clear(){ _p1 = juce::Point<int>(0, 0); _p2 = juce::Point<int>(0, 0); _p2_init = juce::Point<int>(0, 0); };
        bool hasNotableValue(){ return _p2_init != _p2; };
        
        Point<int> _p1;
        Point<int> _p2_init;
        Point<int> _p2;
    };

	MultiSoundobjectSlider();
	MultiSoundobjectSlider(bool spreadEnabled, bool reverbSndGainEnabled);
	~MultiSoundobjectSlider() override;

	MappingAreaId GetSelectedMapping() const;
	void SetSelectedMapping(MappingAreaId mapping);

	bool IsSpreadEnabled();
	void SetSpreadEnabled(bool enabled);
	bool IsReverbSndGainEnabled();
	void SetReverbSndGainEnabled(bool enabled);
	bool IsSoundobjectNamesEnabled();
	void SetSoundobjectNamesEnabled(bool enabled);

	bool HasBackgroundImage(MappingAreaId mappingAreaId);
	const juce::Image* GetBackgroundImage(MappingAreaId mappingAreaId);
	void SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage);
	void RemoveBackgroundImage(MappingAreaId mappingAreaId);

	bool IsHandlingSelectedSoundobjectsOnly();
	void SetHandleSelectedSoundobjectsOnly(bool selectedOnly);

	void UpdateParameters(const ParameterCache& positions);

	void paintOverChildren (Graphics& g) override;

	void resized() override;

	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

	void dualPointMultitouchStarted(const juce::Point<int>& p1, const juce::Point<int>& p2) override;
	void dualPointMultitouchUpdated(const juce::Point<int>& p1, const juce::Point<int>& p2) override;
	void dualPointMultitouchFinished() override;

private:
    void updateMultiTouch(const juce::Point<int>& p1, const juce::Point<int>& p2);
    float getMultiTouchFactorValue();

	SoundobjectProcessorId										m_currentlyDraggedId;				                        /**< ProcessorId of the currently selected knob, if any. */
	std::vector<SoundobjectId>									m_highlightedIds;					                        /**< SourceIds of the currently highlighted knobs, if any. */
	ParameterCache												m_cachedParameters;					                        /**< To save us from iterating over all Soundobject Processors at every click, cache their current parametervalues.
																									                         *	 Keys are the SoundobjectProcessorId of each object processor. */
	bool														m_spreadEnabled{ false };			                        /**< Flag indication, if SO spread factor visu shall be painted for individual SOs. */
	bool														m_reverbSndGainEnabled{ false };	                        /**< Flag indication, if SO reverb send gaind visu shall be painted for individual SOs. */
	bool														m_soundObjectNamesEnabled{ false };	                        /**< Flag indication, if SO name strings shall be painted for individual SOs. */
	MappingAreaId												m_selectedMapping;					                        /**< Remember the last selected coordinate mapping for the multi-slider. */
	std::map<MappingAreaId, std::unique_ptr<ImageComponent>>	m_backgroundImages;					                        /**< Map of background images for the four Mapping Areas that can be displayed. */
	bool														m_handleSelectedOnly{ false };		                        /**< Indication if only selected SO shall be visualized. */
    MultiTouchPoints                                            m_multiTouchPoints;                                         /**< The two multitouch points currently tracked. */
    MultiTouchDirectionTarget                                   m_multiTouchTargetOperation{ MTDT_PendingInputDecision };   /**< Enum value defining how current multitouch input is interpreted. */
    std::map<SoundobjectProcessorId, float>                     m_multiTouchModNormalValues;								/**< Startvalues for multitouch multi-object modification, to be used as base for adding gesture deltas to create actual object values. */
	std::map<SoundobjectProcessorId, juce::Point<float>>		m_objectPosMultiEditStartValues;							/**< Startvalues for editing multiple SO positions. */
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSoundobjectSlider)
};


} // namespace SpaConBridge
