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


#include "MultiSoundobjectSlider.h"

#include "Controller.h"

#include "CustomAudioProcessors/Parameters.h"
#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"

#include "MultiSOSelectionVisualizerComponent.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class MultiSoundobjectSlider
===============================================================================
*/

/**
 * Object constructor.
 */
MultiSoundobjectSlider::MultiSoundobjectSlider() :
    MultiSoundobjectSlider(false, false)
{
}

/**
 * Object constructor.
 */
MultiSoundobjectSlider::MultiSoundobjectSlider(bool spreadEnabled, bool reverbSndGainEnabled) :
	m_currentlyDraggedId(INVALID_PROCESSOR_ID),
	m_spreadEnabled(spreadEnabled),
	m_reverbSndGainEnabled(reverbSndGainEnabled),
    m_soundObjectNamesEnabled(false),
    m_selectedMapping(MappingAreaId::MAI_First)
{
    m_multiselectionVisualizer = std::make_unique<MultiSOSelectionVisualizerComponent>();
    m_multiselectionVisualizer->onMouseInteractionStarted = [this](void) {
        auto objectIdsToCache = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsKV : std::get<0>(m_cachedParameters))
        {
            if (!paramsKV.second._selected)
                continue;
        
            objectIdsToCache.push_back(paramsKV.first);
        }
        
        cacheObjectsXYPos(objectIdsToCache);
    };
    m_multiselectionVisualizer->onMouseXYPosChanged = [this](const juce::Point<int>& posDelta) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsKV : std::get<0>(m_cachedParameters))
        {
            if (!paramsKV.second._selected)
                continue;
        
            objectIdsToModify.push_back(paramsKV.first);
        }
        
        moveObjectsXYPos(objectIdsToModify, posDelta);
    };
    m_multiselectionVisualizer->onMouseXYPosFinished = [this](const juce::Point<int>& posDelta) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsKV : std::get<0>(m_cachedParameters))
        {
            if (!paramsKV.second._selected)
                continue;
        
            objectIdsToModify.push_back(paramsKV.first);
        }
        
        finalizeObjectsXYPos(objectIdsToModify, posDelta);
    };
    m_multiselectionVisualizer->onMouseRotAndScaleChanged = [this](const juce::Point<float>& cog, const float roation, const float scaling) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsKV : std::get<0>(m_cachedParameters))
        {
            if (!paramsKV.second._selected)
                continue;

            objectIdsToModify.push_back(paramsKV.first);
        }

        applyObjectsRotAndScale(objectIdsToModify, cog, roation, scaling);
    };
    m_multiselectionVisualizer->onMouseRotAndScaleFinished = [this](const juce::Point<float>& cog, const float roation, const float scaling) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsKV : std::get<0>(m_cachedParameters))
        {
            if (!paramsKV.second._selected)
                continue;

            objectIdsToModify.push_back(paramsKV.first);
        }

        finalizeObjectsRotAndScale(objectIdsToModify, cog, roation, scaling);
    };
}

/**
 * Object destructor.
 */
MultiSoundobjectSlider::~MultiSoundobjectSlider()
{
}

/**
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
MappingAreaId MultiSoundobjectSlider::GetSelectedMapping() const
{
	return m_selectedMapping;
}

/**
 * Set the currently selected coordinate mapping used for the multi-slider.
 * @param mapping	The new selected mapping area.
 */
void MultiSoundobjectSlider::SetSelectedMapping(MappingAreaId mapping)
{
	// set the incoming ID as currently selected Mapping Area
	m_selectedMapping = mapping;
}

/**
 * Getter for the bool flag that indicates if the spread factor value shall be visualized.
 * @return	True if the flag for spread factor visualizing is set, false if not.
 */
bool MultiSoundobjectSlider::IsSpreadEnabled()
{
	return m_spreadEnabled;
}

/**
 * Setter for the bool flag that indicates if the spread factor value shall be visualized.
 * @param	enabled		True if the flag for spread factor visualizing shall be set, false if not.
 */
void MultiSoundobjectSlider::SetSpreadEnabled(bool enabled)
{
	m_spreadEnabled = enabled;
}

/**
 * Getter for the bool flag that indicates if the reverb send gain value shall be visualized.
 * @return	True if the flag for reverb send gain visualizing is set, false if not.
 */
bool MultiSoundobjectSlider::IsReverbSndGainEnabled()
{
	return m_reverbSndGainEnabled;
}

/**
 * Setter for the bool flag that indicates if the reverb send gain value shall be visualized.
 * @param	enabled		True if the flag for reverb send gain visualizing shall be set, false if not.
 */
void MultiSoundobjectSlider::SetReverbSndGainEnabled(bool enabled)
{
	m_reverbSndGainEnabled = enabled;
}

/**
 * Getter for the bool flag that indicates if the soundobject name string shall be visualized.
 * @return	True if the flag for soundobject name visualizing is set, false if not.
 */
bool MultiSoundobjectSlider::IsSoundobjectNamesEnabled()
{
	return m_soundObjectNamesEnabled;
}

/**
 * Setter for the bool flag that indicates if the soundobject name string shall be visualized.
 * @param	enabled		True if the flag for reverb send gainsoundobject name visualizing shall be set, false if not.
 */
void MultiSoundobjectSlider::SetSoundobjectNamesEnabled(bool enabled)
{
	m_soundObjectNamesEnabled = enabled;
}

/**
 * Getter for the bool flag that indicates if multiselection visualization shall be used.
 * @return	True if the flag if the multiselection visualization shall be used is set, false if not.
 */
bool MultiSoundobjectSlider::IsMuSelVisuEnabled()
{
    return m_muselvisuEnabled;
}

/**
 * Setter for the bool flag that indicates if multiselection visualization shall be usedd.
 * @param	enabled		True if the flag for if the multiselection visualization shall be used is set, false if not.
 */
void MultiSoundobjectSlider::SetMuSelVisuEnabled(bool enabled)
{
    m_muselvisuEnabled = enabled;

    if (enabled)
        addAndMakeVisible(m_multiselectionVisualizer.get());
    else
        removeChildComponent(m_multiselectionVisualizer.get());

    resized();
}

/**
 * Helper method to check if a background image is set for the given mapping area id 
 * @param	mappingAreaId	The id of the mapping are to verify for if an image has been set as background
 */
bool MultiSoundobjectSlider::HasBackgroundImage(MappingAreaId mappingAreaId)
{
	return m_backgroundImages.count(mappingAreaId) > 0;
}

/**
 * Helper method to get the background image currently used for the given mapping area id
 * @param	mappingAreaId	The id of the mapping are to get the background image for.
 * return	The image used as background for the given mapping area id, nullptr if none is set.
 */
const juce::Image& MultiSoundobjectSlider::GetBackgroundImage(MappingAreaId mappingAreaId)
{
    return m_backgroundImages[mappingAreaId];
}

/**
 * Helper method to set a background image for the given mapping area id
 * @param	mappingAreaId	The id of the mapping are to set the background image for
 * @param	backgroundImage	The image to set as background for the given mapping area id
 */
void MultiSoundobjectSlider::SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage)
{
    m_backgroundImages[mappingAreaId] = backgroundImage;

    repaint();
}

/**
 * Helper method to remove the background image for the given mapping area id
 * @param	mappingAreaId	The id of the mapping are to remove the background image for
 */
void MultiSoundobjectSlider::RemoveBackgroundImage(MappingAreaId mappingAreaId)
{
	m_backgroundImages.erase(mappingAreaId);

    repaint();
}

/**
 * Getter for the 'show only selected' state member.
 * @return True if internal member is set to show only selected SO.
 */
bool MultiSoundobjectSlider::IsHandlingSelectedSoundobjectsOnly()
{
	return m_handleSelectedOnly;
}

/**
 * Setter for the 'show only selected' state member.
 * @param	selectedOnly	True if internal member shall be set to show only selected SO.
 */
void MultiSoundobjectSlider::SetHandleSelectedSoundobjectsOnly(bool selectedOnly)
{
	m_handleSelectedOnly = selectedOnly;
}

/**
 * Reimplemented painting.
 * @param g		The graphics context that must be used to do the drawing operations.
 */
void MultiSoundobjectSlider::paint(Graphics& g)
{
    // Solid surface background area if no image is set
    auto backgroundRect = getLocalBounds().toFloat().reduced(2);
    if (!HasBackgroundImage(GetSelectedMapping()))
    {
        g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
        g.fillRect(backgroundRect);
    }
    else
    {
        g.drawImage(GetBackgroundImage(GetSelectedMapping()), backgroundRect);
    }

	auto w = getLocalBounds().toFloat().getWidth();
	auto h = getLocalBounds().toFloat().getHeight();

	// Draw grid
	const float dashLengths[2] = { 5.0f, 6.0f };
	const float lineThickness = 1.0f;
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId).brighter(0.15f));
	g.drawDashedLine(Line<float>(w * 0.25f, 0.0f, w * 0.25f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(w * 0.50f, 0.0f, w * 0.50f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(w * 0.75f, 0.0f, w * 0.75f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.25f, w, h * 0.25f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.50f, w, h * 0.50f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.75f, w, h * 0.75f), dashLengths, 2, lineThickness);

	// Surface frame
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawRect(Rectangle<float>(0.0f, 0.0f, w, h), 1.5f);


    // painting for all cached soundobjects, based on cached parameter values
	float refKnobSize = 10.0f;

    auto& soundobjectParameterMap = std::get<0>(m_cachedParameters);
    auto& parameterFlags = std::get<1>(m_cachedParameters);

    auto multiselectionActive = false;
    if ((parameterFlags & CacheFlag::MultiSelection) == CacheFlag::MultiSelection)
        multiselectionActive = true;

    auto selectedCoords = std::vector<juce::Point<float>>();

	for (auto const& paramsKV : soundobjectParameterMap)
	{
		auto const& isSelected = paramsKV.second._selected;

		if (m_handleSelectedOnly && !isSelected)
			continue;

		auto knobColour = paramsKV.second._colour;

		auto knobSizeScaleFactor = static_cast<float>(1.0f + (2.0f * paramsKV.second._size));
		auto knobSize = refKnobSize * knobSizeScaleFactor;
		auto knobThickness = 3.0f * knobSizeScaleFactor;

		// Map the x/y coordinates to the pixel-wise dimensions of the surface area.
		auto const& pt = paramsKV.second._pos;

        auto currentCoords = juce::Point<float>(pt.x * w, h - (pt.y * h));
        selectedCoords.push_back(currentCoords);
        auto const& x = currentCoords.getX();
        auto const& y = currentCoords.getY();

		auto metaInfoSize = 6 * refKnobSize;
		auto innerRadius = 0.5f * knobSize;

		if (m_currentlyDraggedId == paramsKV.first)
		{
			// Paint 'currently dragged crosshair'
			auto& crosshairColour = knobColour;
			g.setColour(crosshairColour);
			g.drawLine(0, y, w, y, 1);
			g.drawLine(x, 0, x, h, 1);

			// Paint 'currently dual-multitouch points indication'
			auto& p1 = m_multiTouchPoints._p2_init;
			auto& p2 = m_multiTouchPoints._p2;
            auto goodVisibilityDistance = 16;
			switch (m_multiTouchTargetOperation)
			{
			case MTDT_HorizontalEnSpaceSendGain:
			{
				g.setColour(crosshairColour);
				g.drawDashedLine(Line<float>(p1.toFloat().getX(), 0.0f, p1.toFloat().getX(), h), dashLengths, 2, lineThickness);
				g.drawDashedLine(Line<float>(p2.toFloat().getX(), 0.0f, p2.toFloat().getX(), h), dashLengths, 2, lineThickness);
				g.setOpacity(0.15f);
				g.fillRect(Rectangle<float>(p1.toFloat().getX(), 0.0f, p2.toFloat().getX() - p1.toFloat().getX(), h));
                
				auto font = Font(static_cast<float>(goodVisibilityDistance), Font::plain);
                g.setFont(font);
                g.setOpacity(1.0f);
				auto textLabel = String("EnSpace Gain ") + String(paramsKV.second._reverbSndGain, 2) + String("dB");
				auto fontDependantWidth = font.getStringWidth(textLabel);
                auto textLeftOfMouse = (getWidth() - p2.getX() - goodVisibilityDistance) < fontDependantWidth;
                if (textLeftOfMouse)
                    g.drawText(textLabel, goodVisibilityDistance, goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centred, true);
                else
                    g.drawText(textLabel, getWidth() - goodVisibilityDistance - fontDependantWidth, goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centredLeft, true);
			}
			break;
			case MTDT_VerticalSpread:
			{
				g.setColour(crosshairColour);
				g.drawDashedLine(Line<float>(0.0f, p1.toFloat().getY(), w, p1.toFloat().getY()), dashLengths, 2, lineThickness);
				g.drawDashedLine(Line<float>(0.0f, p2.toFloat().getY(), w, p2.toFloat().getY()), dashLengths, 2, lineThickness);
				g.setOpacity(0.15f);
				g.fillRect(Rectangle<float>(0.0f, p1.toFloat().getY(), w, p2.toFloat().getY() - p1.toFloat().getY()));

				auto font = Font(static_cast<float>(goodVisibilityDistance), Font::plain);
                g.setFont(font);
                g.setOpacity(1.0f);
				auto textLabel = String("Spread Factor ") + String(paramsKV.second._spread, 2);
				auto fontDependantWidth = font.getStringWidth(textLabel);
                auto textBelowMouse = (p2.getY() - goodVisibilityDistance) < goodVisibilityDistance;
                if (textBelowMouse)
                    g.drawText(textLabel, goodVisibilityDistance, getHeight() - 2 * goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centred, true);
                else
                    g.drawText(textLabel, goodVisibilityDistance, goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centred, true);
			}
			break;
			case MTDT_PendingInputDecision:
			default:
				break;
			}
		}
        
		// Paint spread if enabled
		if (m_spreadEnabled)
		{
			auto spreadSize = metaInfoSize * paramsKV.second._spread;
			auto spreadColour = knobColour;

			auto outerRadius = refKnobSize + (0.5f * spreadSize);

			auto spreadPath = juce::Path();
			spreadPath.startNewSubPath(x, y);
			spreadPath.addCentredArc(x, y, outerRadius, outerRadius, 0, 0.0f, 2.0f * juce::MathConstants<float>::pi);
			spreadPath.addCentredArc(x, y, innerRadius, innerRadius, 0, 2.0f * juce::MathConstants<float>::pi, 0.0f);
			spreadPath.closeSubPath();

			g.setColour(spreadColour);
			g.setOpacity(0.4f);
			g.fillPath(spreadPath);
		}

		// Paint reverbSendGain if enabled
		if (m_reverbSndGainEnabled)
		{
			auto miRevSndGainRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_ReverbSendGain);
			auto normalizedRevSndGain = jmap(paramsKV.second._reverbSndGain, miRevSndGainRange.getStart(), miRevSndGainRange.getEnd(), 0.0f, 1.0f);
			auto reverbSize = metaInfoSize * normalizedRevSndGain;
			auto reverbColour = knobColour;

			auto outerRadius = refKnobSize + (0.5f * reverbSize);

			auto reverbPath = juce::Path();
			reverbPath.startNewSubPath(x, y);
			reverbPath.addCentredArc(x, y, outerRadius, outerRadius, 0, 0.35f * juce::MathConstants<float>::pi, 0.65f * juce::MathConstants<float>::pi);
			reverbPath.addCentredArc(x, y, innerRadius, innerRadius, 0, 0.65f * juce::MathConstants<float>::pi, 0.35f * juce::MathConstants<float>::pi);
			reverbPath.closeSubPath();
			reverbPath.startNewSubPath(x, y);
			reverbPath.addCentredArc(x, y, outerRadius, outerRadius, 0, 1.35f * juce::MathConstants<float>::pi, 1.65f * juce::MathConstants<float>::pi);
			reverbPath.addCentredArc(x, y, innerRadius, innerRadius, 0, 1.65f * juce::MathConstants<float>::pi, 1.35f * juce::MathConstants<float>::pi);
			reverbPath.closeSubPath();

			g.setColour(reverbColour);
			g.setOpacity(0.6f);
			g.fillPath(reverbPath);
		}

		// Paint knob
		g.setColour(knobColour);
		g.setOpacity(1.0f);
        if (isSelected && !IsMuSelVisuEnabled())
		{
            // if the current SO is the only selected one, paint it with a circle indicator and solid fill
			auto fillSize = knobSize + knobThickness;
			auto outlineSize = 8 * refKnobSize;
			g.fillEllipse(Rectangle<float>(x - (fillSize / 2.0f), y - (fillSize / 2.0f), fillSize, fillSize));
			g.drawEllipse(Rectangle<float>(x - (outlineSize / 2.0f), y - (outlineSize / 2.0f), outlineSize, outlineSize), 1.0f);
		}
		else
		{
			g.drawEllipse(Rectangle<float>(x - (knobSize / 2.0f), y - (knobSize / 2.0f), knobSize, knobSize), knobThickness);
		}

		// Soundobject text labeling
        String textLabel;
		if (m_soundObjectNamesEnabled)
			textLabel = paramsKV.second._objectName;
		else
			textLabel = String(paramsKV.second._id);
		auto fontSizeScaleFactor = static_cast<float>(2.0f * paramsKV.second._size);
		auto font = Font(12.0f + 5.0f * fontSizeScaleFactor, Font::plain);
		auto fontDependantWidth = static_cast<float>(font.getStringWidth(textLabel));
		g.setFont(font);
		g.drawText(textLabel, Rectangle<float>(x - (0.5f * fontDependantWidth), y + 3, fontDependantWidth, knobSize * 2.0f), Justification::centred, true);
	}
    
    auto singleSoundobjectCurrentlyEdited = INVALID_PROCESSOR_ID != m_currentlyDraggedId;
    auto multitouchInputActive = MTDT_PendingInputDecision != m_multiTouchTargetOperation;
    if (!singleSoundobjectCurrentlyEdited && multitouchInputActive)
    {
        // Paint 'multi soundobject editing dual-multitouch points indication'
        auto& p1 = m_multiTouchPoints._p2_init;
        auto& p2 = m_multiTouchPoints._p2;
        auto goodVisibilityDistance = 16;
        auto multitouchIndicationColour = getLookAndFeel().findColour(TextButton::textColourOnId).brighter(0.15f);
        switch (m_multiTouchTargetOperation)
        {
            case MTDT_HorizontalEnSpaceSendGain:
                {
                    g.setColour(multitouchIndicationColour);
                    g.drawDashedLine(Line<float>(p1.toFloat().getX(), 0.0f, p1.toFloat().getX(), h), dashLengths, 2, lineThickness);
                    g.drawDashedLine(Line<float>(p2.toFloat().getX(), 0.0f, p2.toFloat().getX(), h), dashLengths, 2, lineThickness);
                    g.setOpacity(0.15f);
                    g.fillRect(Rectangle<float>(p1.toFloat().getX(), 0.0f, p2.toFloat().getX() - p1.toFloat().getX(), h));
                    
                    auto font = Font(static_cast<float>(goodVisibilityDistance), Font::plain);
                    g.setFont(font);
                    g.setOpacity(1.0f);
                    auto enSpacGainFactorRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_ReverbSendGain);
                    auto enSpacGainChangeVal = -1.0f * getMultiTouchFactorValue() * enSpacGainFactorRange.getLength();
                    auto textLabel = String("Adding ") + String(enSpacGainChangeVal, 2) + String("dB to EnSpace Gain");
                    auto fontDependantWidth = font.getStringWidth(textLabel);
                    auto isTextLeftOfMouse = (getWidth() - p2.getX() - goodVisibilityDistance) < fontDependantWidth;
                    if (isTextLeftOfMouse)
                        g.drawText(textLabel, goodVisibilityDistance, goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centred, true);
                    else
                        g.drawText(textLabel, getWidth() - goodVisibilityDistance - fontDependantWidth, goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centredLeft, true);
                }
                break;
            case MTDT_VerticalSpread:
                {
                    g.setColour(multitouchIndicationColour);
                    g.drawDashedLine(Line<float>(0.0f, p1.toFloat().getY(), w, p1.toFloat().getY()), dashLengths, 2, lineThickness);
                    g.drawDashedLine(Line<float>(0.0f, p2.toFloat().getY(), w, p2.toFloat().getY()), dashLengths, 2, lineThickness);
                    g.setOpacity(0.15f);
                    g.fillRect(Rectangle<float>(0.0f, p1.toFloat().getY(), w, p2.toFloat().getY() - p1.toFloat().getY()));
                    
                    auto font = Font(static_cast<float>(goodVisibilityDistance), Font::plain);
                    g.setFont(font);
                    g.setOpacity(1.0f);
                    auto spreadFactorRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_Positioning_SourceSpread);
                    auto spreadFactorChangeVal = -1.0f * getMultiTouchFactorValue() * spreadFactorRange.getLength();
                    auto textLabel = String("Adding ") + String(spreadFactorChangeVal, 2) + String(" to Spread Factor");
                    auto fontDependantWidth = font.getStringWidth(textLabel);
                    auto isTextBelowMouse = (p2.getY() - goodVisibilityDistance) < goodVisibilityDistance;
                    if (isTextBelowMouse)
                        g.drawText(textLabel, goodVisibilityDistance, getHeight() - 2 * goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centred, true);
                    else
                        g.drawText(textLabel, goodVisibilityDistance, goodVisibilityDistance, fontDependantWidth, goodVisibilityDistance, Justification::centred, true);
                }
                break;
            case MTDT_PendingInputDecision:
                jassertfalse;
            default:
                break;
        }
    }
}

/**
 * Reimplemented component resize method to scale the currently selected Mapping Area's background
 * image correctly if any is set.
 */
void MultiSoundobjectSlider::resized()
{
	Component::resized();

    if (m_multiselectionVisualizer)
    {
        m_multiselectionVisualizer->setBounds(getLocalBounds());

        if (m_multiselectionVisualizer->IsSelectionVisuActive())
        {
            auto& soundobjectParameterMap = std::get<0>(m_cachedParameters);
            auto& parameterFlags = std::get<1>(m_cachedParameters);
            if ((parameterFlags & CacheFlag::MultiSelection) == CacheFlag::MultiSelection)
            {
                auto selectedCoords = std::vector<juce::Point<float>>();

                auto w = getLocalBounds().toFloat().getWidth();
                auto h = getLocalBounds().toFloat().getHeight();

                for (auto const& paramsKV : soundobjectParameterMap)
                {
                    auto const& isSelected = paramsKV.second._selected;
                    auto const& pt = paramsKV.second._pos;

                    if (isSelected)
                        selectedCoords.push_back(juce::Point<float>(pt.x * w, h - (pt.y * h)));
                }

                m_multiselectionVisualizer->SetSelectionPoints(selectedCoords);
            }
        }
    }
}

/**
 * Called when a mouse button is pressed. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred 
 */
void MultiSoundobjectSlider::mouseDown(const MouseEvent& e)
{
    DualPointMultitouchCatcherComponent::mouseDown(e);
    
    if (GetPrimaryMouseInputSourceIndex() != e.source.getIndex()) // dont check IsInFakeALTMultiTouch() here but somewhere below to allow to perform the click-hit-check first
        return;
    
	float w = static_cast<float>(getLocalBounds().getWidth());
	float h = static_cast<float>(getLocalBounds().getHeight());

	// Mouse click position (in pixel units)
	Point<float> mousePos(static_cast<float>(e.getMouseDownPosition().x), static_cast<float>(e.getMouseDownPosition().y));

	float refKnobSize = 10.0f;

	for (auto const& paramsKV : std::get<0>(m_cachedParameters))
	{
        auto const& selected = paramsKV.second._selected;

        if (m_handleSelectedOnly && !selected)
            continue;
        
		// Map the x/y coordinates to the pixel-wise dimensions of the surface area.
		auto const& pt = paramsKV.second._pos;
		float x = pt.x * w;
		float y = h - (pt.y * h);

		auto knobSizeScaleFactor = static_cast<float>(1.0f + (1.5f * paramsKV.second._size));
		auto knobSize = refKnobSize * knobSizeScaleFactor;
		auto knobThickness = 3.0f * knobSizeScaleFactor;

		Path knobPath;
		auto fillSize = knobSize + knobThickness;
		knobPath.addEllipse(Rectangle<float>(x - (fillSize / 2.0f), y - (fillSize / 2.0f), fillSize, fillSize));

		// Check if the mouse click landed inside any of the knobs.
		if (knobPath.contains(mousePos))
		{
			// Set this source as "selected" and begin a drag gesture.
			m_currentlyDraggedId = paramsKV.first;

			if (!IsInFakeALTMultiTouch() && m_multiTouchTargetOperation == MTDT_PendingInputDecision)
			{
				auto ctrl = Controller::GetInstance();
				if (ctrl)
				{
					auto processor = ctrl->GetSoundobjectProcessor(m_currentlyDraggedId);
					jassert(processor);
					if (processor)
					{
                        DBG(String(__FUNCTION__) + String(" BeginGuiGesture for id ") + String(m_currentlyDraggedId));
						auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_X]);
						if (param)
							param->BeginGuiGesture();
						param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_Y]);
						if (param)
							param->BeginGuiGesture();
					}
				}
			}

			// Found a knob to select, skip the rest.
            repaint();
			break;
		}
	}
}

/**
 * Called when the mouse is moved while a button is held down. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void MultiSoundobjectSlider::mouseDrag(const MouseEvent& e)
{
    DualPointMultitouchCatcherComponent::mouseDrag(e);
    
    if (GetPrimaryMouseInputSourceIndex() != e.source.getIndex() || IsInFakeALTMultiTouch())
        return;
    
    if (m_multiTouchTargetOperation == MTDT_PendingInputDecision)
    {
        auto ctrl = Controller::GetInstance();
        if (ctrl)
        {
            // if no multitouch operation is in progress, we have to deal with xy pos changes
            if (m_currentlyDraggedId != INVALID_PROCESSOR_ID)
            {
                auto processor = ctrl->GetSoundobjectProcessor(m_currentlyDraggedId);
                if (processor)
                {
                    // Get mouse pixel-wise position and scale it between 0 and 1.
                    auto const& pos = e.getPosition();
                    auto x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / getLocalBounds().getWidth())));
                    auto y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / getLocalBounds().getHeight())));

                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, x);
                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, y);
                }
            }
        }
    }
}

/**
 * Called when the mouse button is released.
 * Reimplemented just to call EndGuiGesture() to inform the host.
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void MultiSoundobjectSlider::mouseUp(const MouseEvent& e)
{
	auto wasInFakeALTMultiTouch = IsInFakeALTMultiTouch();
	auto isntPrimaryMouse = GetPrimaryMouseInputSourceIndex() != e.source.getIndex();
    auto validDraggedId = (m_currentlyDraggedId != INVALID_PROCESSOR_ID);

    DualPointMultitouchCatcherComponent::mouseUp(e);

    if (!(wasInFakeALTMultiTouch || isntPrimaryMouse))
    {
        auto ctrl = Controller::GetInstance();
        if (ctrl)
        {
            if (validDraggedId)
            {
                auto processor = ctrl->GetSoundobjectProcessor(m_currentlyDraggedId);
                if (processor)
                {
                    DBG(String(__FUNCTION__) + String(" EndGuiGesture for id ") + String(m_currentlyDraggedId));
                    auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_X]);
                    if (param)
                        param->EndGuiGesture();
                    param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_Y]);
                    if (param)
                        param->EndGuiGesture();

                    // Get mouse pixel-wise position and scale it between 0 and 1.
                    auto pos = e.getPosition();
                    auto x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / getLocalBounds().getWidth())));
                    auto y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / getLocalBounds().getHeight())));

                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, x);
                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, y);
                }
            }
        }
    }
    
    if (!isntPrimaryMouse)
    {
        // De-select knob.
        m_currentlyDraggedId = INVALID_PROCESSOR_ID;

        // trigger single repaint to get rid of 'currently dragged crosshair'
        repaint();
    }
}

/**
 * Implementation of pure virtual method to notify multitouch guesture start
 * @param p1	First multitouch point
 * @param p2	Second multitouch point
 */
void MultiSoundobjectSlider::dualPointMultitouchStarted(const juce::Point<int>& p1, const juce::Point<int>& p2)
{
    updateMultiTouch(p1, p2);
    
    repaint();
}

/**
 * Implementation of pure virtual method to notify multitouch guesture update
 * @param p1	First multitouch point
 * @param p2	Second multitouch point
 */
void MultiSoundobjectSlider::dualPointMultitouchUpdated(const juce::Point<int>& p1, const juce::Point<int>& p2)
{
    updateMultiTouch(p1, p2);
    
    auto validDraggedId = (m_currentlyDraggedId != INVALID_PROCESSOR_ID);
    
    auto ctrl = Controller::GetInstance();
    if (ctrl)
    {
        if (validDraggedId)
        {
            auto const& id = m_currentlyDraggedId;
            auto processor = ctrl->GetSoundobjectProcessor(m_currentlyDraggedId);
            if (processor)
            {
                switch (m_multiTouchTargetOperation)
                {
                    case MTDT_VerticalSpread:
                        {
                            auto spreadFactorRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_Positioning_SourceSpread);
                            auto newVal = jlimit(spreadFactorRange.getStart(), spreadFactorRange.getEnd(), m_multiTouchModNormalValues.at(id) - getMultiTouchFactorValue() * spreadFactorRange.getLength());
                            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_ObjectSpread, newVal);
                        }
                        break;
                    case MTDT_HorizontalEnSpaceSendGain:
                        {
                            auto enSpacGainFactorRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_ReverbSendGain);
                            auto newVal = jlimit(enSpacGainFactorRange.getStart(), enSpacGainFactorRange.getEnd(), m_multiTouchModNormalValues.at(id) - getMultiTouchFactorValue() * enSpacGainFactorRange.getLength());
                            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_ReverbSendGain, newVal);
                        }
                        break;
                    case MTDT_PendingInputDecision:
                    default:
                        break;
                }
            }
        }
        else
        {
            for (auto const& paramsKV : std::get<0>(m_cachedParameters))
            {
                auto const& selected = paramsKV.second._selected;
                if (selected)
                {
                    auto const& id = paramsKV.first;
                    auto processor = ctrl->GetSoundobjectProcessor(id);
                    if (processor)
                    {
                        switch (m_multiTouchTargetOperation)
                        {
                            case MTDT_VerticalSpread:
                                {
                                    auto spreadFactorRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_Positioning_SourceSpread);
                                    auto newVal = jlimit(spreadFactorRange.getStart(), spreadFactorRange.getEnd(), m_multiTouchModNormalValues.at(id) - getMultiTouchFactorValue() * spreadFactorRange.getLength());
                                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_ObjectSpread, newVal);
                                }
                                break;
                            case MTDT_HorizontalEnSpaceSendGain:
                                {
                                    auto enSpacGainRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_ReverbSendGain);
                                    auto newVal = jlimit(enSpacGainRange.getStart(), enSpacGainRange.getEnd(), m_multiTouchModNormalValues.at(id) - getMultiTouchFactorValue() * enSpacGainRange.getLength());
                                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_ReverbSendGain, newVal);
                                }
                                break;
                            case MTDT_PendingInputDecision:
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }
    repaint();
}

/**
 * Implementation of pure virtual method to notify multitouch guesture end
 */
void MultiSoundobjectSlider::dualPointMultitouchFinished()
{
    auto validDraggedId = (m_currentlyDraggedId != INVALID_PROCESSOR_ID);
    
    auto ctrl = Controller::GetInstance();
    if (ctrl)
    {
        if (validDraggedId)
        {
            auto processor = ctrl->GetSoundobjectProcessor(m_currentlyDraggedId);
            if (processor)
            {
                auto param = static_cast<GestureManagedAudioParameterFloat*>(nullptr);
                
                switch (m_multiTouchTargetOperation)
                {
                    case MTDT_VerticalSpread:
                        param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ObjectSpread]);
                        jassert(param);
                        break;
                    case MTDT_HorizontalEnSpaceSendGain:
                        param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ReverbSendGain]);
                        jassert(param);
                        break;
                    case MTDT_PendingInputDecision:
                    default:
                        break;
                }
                
                if (param)
                    param->EndGuiGesture();
            }
        }
        else
        {
            for (auto const& paramsKV : std::get<0>(m_cachedParameters))
            {
                auto const& selected = paramsKV.second._selected;
                if (selected)
                {
                    auto processor = ctrl->GetSoundobjectProcessor(paramsKV.second._id);
                    if (processor)
                    {
                        auto param = static_cast<GestureManagedAudioParameterFloat*>(nullptr);
                        
                        switch (m_multiTouchTargetOperation)
                        {
                            case MTDT_VerticalSpread:
                                param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ObjectSpread]);
                                jassert(param);
                                break;
                            case MTDT_HorizontalEnSpaceSendGain:
                                param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ReverbSendGain]);
                                jassert(param);
                                break;
                            case MTDT_PendingInputDecision:
                            default:
                                break;
                        }
                        
                        if (param)
                            param->EndGuiGesture();
                    }
                }
            }
        }
    }
    
    updateMultiTouch(juce::Point<int>(0, 0), juce::Point<int>(0, 0));
    
    repaint();
}

/**
 * Helper method to process the current two multitouch points into what multitouch operation shall be performed.
 */
void MultiSoundobjectSlider::updateMultiTouch(const juce::Point<int>& p1, const juce::Point<int>& p2)
{
    if(p1.isOrigin() && p2.isOrigin())
    {
        m_multiTouchPoints.clear();
        m_multiTouchTargetOperation = MTDT_PendingInputDecision;
        m_multiTouchModNormalValues.clear();
    }
    else if (m_multiTouchPoints.isEmpty())
    {
        m_multiTouchPoints._p1 = p1;
        m_multiTouchPoints._p2_init = p2;
        m_multiTouchTargetOperation = MTDT_PendingInputDecision;
        m_multiTouchModNormalValues.clear();
    }
    else
    {
        m_multiTouchPoints._p1 = p1;
        m_multiTouchPoints._p2 = p2;
        
        auto validDraggedId = (m_currentlyDraggedId != INVALID_PROCESSOR_ID);
        auto isInitialUpdate = (MTDT_PendingInputDecision == m_multiTouchTargetOperation);
        if (isInitialUpdate)
        {
            auto horizontalDelta = std::fabs(m_multiTouchPoints._p2_init.getX() - m_multiTouchPoints._p2.getX());
            auto verticalDelta = std::fabs(m_multiTouchPoints._p2_init.getY() - m_multiTouchPoints._p2.getY());
            auto isEnSpaceGainChange = horizontalDelta > verticalDelta;
            auto isSpreadFactorChange = horizontalDelta < verticalDelta;
            
            auto ctrl = Controller::GetInstance();
            if (ctrl && (isEnSpaceGainChange || isSpreadFactorChange))
            {
                // if a soundobject is selected, modify its value absolute and directly
                if (validDraggedId)
                {
                    auto& id = m_currentlyDraggedId;
                    auto processor = ctrl->GetSoundobjectProcessor(m_currentlyDraggedId);
                    if (processor)
                    {
                        if (isSpreadFactorChange)
                        {
                            auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ObjectSpread]);
                            jassert(param);
                            if (param)
                                param->BeginGuiGesture();
                            
                            auto spreadFactorRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_Positioning_SourceSpread);
                            m_multiTouchModNormalValues[id] = jmap(processor->GetParameterValue(SPI_ParamIdx_ObjectSpread), spreadFactorRange.getStart(), spreadFactorRange.getEnd(), 0.0f, 1.0f);
                            
                            m_multiTouchTargetOperation = MTDT_VerticalSpread;
                        }
                        else if (isEnSpaceGainChange)
                        {
                            auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ReverbSendGain]);
                            jassert(param);
                            if (param)
                                param->BeginGuiGesture();
                            
                            auto enSpacGainRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_ReverbSendGain);
                            m_multiTouchModNormalValues[id] = jmap(processor->GetParameterValue(SPI_ParamIdx_ReverbSendGain), enSpacGainRange.getStart(), enSpacGainRange.getEnd(), 0.0f, 1.0f);
                            
                            m_multiTouchTargetOperation = MTDT_HorizontalEnSpaceSendGain;
                        }
                        else
                            jassertfalse;
                    }
                }
                else
                {
                    for (auto const& paramsKV : std::get<0>(m_cachedParameters))
                    {
                        auto const& selected = paramsKV.second._selected;
                        
                        if (selected)
                        {
                            auto& id = paramsKV.first;
                            auto processor = ctrl->GetSoundobjectProcessor(id);
                            if (processor)
                            {
                                if (isSpreadFactorChange)
                                {
                                    auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ObjectSpread]);
                                    jassert(param);
                                    if (param)
                                        param->BeginGuiGesture();
                                    
                                    auto spreadFactorRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_Positioning_SourceSpread);
                                    m_multiTouchModNormalValues[id] = jmap(processor->GetParameterValue(SPI_ParamIdx_ObjectSpread), spreadFactorRange.getStart(), spreadFactorRange.getEnd(), 0.0f, 1.0f);
                                    
                                    m_multiTouchTargetOperation = MTDT_VerticalSpread;
                                }
                                else if (isEnSpaceGainChange)
                                {
                                    auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_ReverbSendGain]);
                                    jassert(param);
                                    if (param)
                                        param->BeginGuiGesture();
                                    
                                    auto enSpacGainRange = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixInput_ReverbSendGain);
                                    m_multiTouchModNormalValues[id] = jmap(processor->GetParameterValue(SPI_ParamIdx_ReverbSendGain), enSpacGainRange.getStart(), enSpacGainRange.getEnd(), 0.0f, 1.0f);
                                    
                                    m_multiTouchTargetOperation = MTDT_HorizontalEnSpaceSendGain;
                                }
                                else
                                    jassertfalse;
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * Helper method to get a unity factor from the currently available two touch point values, depending on the current target operation mode.
 * @return  The unity factor or 1 as default.
 */
float MultiSoundobjectSlider::getMultiTouchFactorValue()
{
    if (m_multiTouchPoints.hasNotableValue())
    {
        switch (m_multiTouchTargetOperation)
        {
            case MTDT_HorizontalEnSpaceSendGain:
                {
                    auto p2 = m_multiTouchPoints._p2.toFloat().getX();
                    auto p2_init = m_multiTouchPoints._p2_init.toFloat().getX();
                    
                    auto deltaP2 = -1.0f * (p2 - p2_init);
                    if (getWidth() != 0.0f)
                        return deltaP2 / getWidth();
                    else
                        return 0.0f;
                }
                break;
            case MTDT_VerticalSpread:
                {
                    auto p2 = m_multiTouchPoints._p2.toFloat().getY();
                    auto p2_init = m_multiTouchPoints._p2_init.toFloat().getY();
                    
                    auto deltaP2 = p2 - p2_init;
                    if (getHeight() != 0.0f)
                        return deltaP2 / getHeight();
                    else
                        return 0.0f;
                }
                break;
            case MTDT_PendingInputDecision:
            default:
                break;
        };
    }
    
    return 1.0f;
}

/**
 * Helper to cache xy object positions as starting point for multi-obj-modification
 * @param   objectIds           The ids of the soundobjects that shall be cached
 */
void MultiSoundobjectSlider::cacheObjectsXYPos(const std::vector<SoundobjectProcessorId>& objectIds)
{
    auto ctrl = Controller::GetInstance();
    if (!ctrl)
        return;

    for (auto const& objectId : objectIds)
    {
        auto processor = ctrl->GetSoundobjectProcessor(objectId);
        if (processor)
        {
            DBG(String(__FUNCTION__) + String(" BeginGuiGesture for id ") + String(objectId));
            auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_X]);
            if (param)
                param->BeginGuiGesture();
            param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_Y]);
            if (param)
                param->BeginGuiGesture();

            auto& soundobjectParameterMap = std::get<0>(m_cachedParameters);
            jassert(soundobjectParameterMap.count(objectId) > 0);
            m_objectPosMultiEditStartValues[objectId] = soundobjectParameterMap.at(objectId)._pos;
        }
    }
}

/**
 * Helper to batch modifiy xy object positions with a given position delta
 * @param   objectIds           The ids of the soundobjects that shall be modified
 * @param   positionMoveDelta   The xy delta to add to the existing positions
 */
void MultiSoundobjectSlider::moveObjectsXYPos(const std::vector<SoundobjectProcessorId>& objectIds, const juce::Point<int>& positionMoveDelta)
{
    auto ctrl = Controller::GetInstance();
    if (!ctrl)
        return;

    auto updatedScreenCoords = std::vector<juce::Point<float>>();
    auto w = getLocalBounds().toFloat().getWidth();
    auto h = getLocalBounds().toFloat().getHeight();

    for (auto const& objectId : objectIds)
    {
        auto processor = ctrl->GetSoundobjectProcessor(objectId);
        if (processor)
        {
            auto xDelta = static_cast<float>(positionMoveDelta.getX()) / getLocalBounds().getWidth();
            auto yDelta = static_cast<float>(positionMoveDelta.getY()) / getLocalBounds().getHeight();

            auto const& cachedPos = m_objectPosMultiEditStartValues.at(objectId);
            auto newPosX = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getX() + xDelta)));
            auto newPosY = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getY() - yDelta)));

            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, newPosX);
            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, newPosY);

            updatedScreenCoords.push_back(juce::Point<float>(newPosX * w, h - (newPosY * h)));
        }
    }

    if (m_multiselectionVisualizer)
        m_multiselectionVisualizer->UpdateSelectionPoints(updatedScreenCoords);
}

/**
 * Helper to finalize modification of xy object positions
 * @param   objectIds           The ids of the soundobjects that shall be finalized
 * @param   positionMoveDelta   The xy delta to finally add to the existing positions
 */
void MultiSoundobjectSlider::finalizeObjectsXYPos(const std::vector<SoundobjectProcessorId>& objectIds, const juce::Point<int>& positionMoveDelta)
{
    auto ctrl = Controller::GetInstance();
    if (!ctrl)
        return;

    auto updatedScreenCoords = std::vector<juce::Point<float>>();
    auto w = getLocalBounds().toFloat().getWidth();
    auto h = getLocalBounds().toFloat().getHeight();

    for (auto const& objectId : objectIds)
    {
        auto processor = ctrl->GetSoundobjectProcessor(objectId);
        if (processor)
        {
            DBG(String(__FUNCTION__) + String(" EndGuiGesture for id ") + String(objectId));
            auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_X]);
            if (param)
                param->EndGuiGesture();
            param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_Y]);
            if (param)
                param->EndGuiGesture();

            // Get mouse pixel-wise position and scale it between 0 and 1.
            auto xDelta = static_cast<float>(positionMoveDelta.getX()) / getLocalBounds().getWidth();
            auto yDelta = static_cast<float>(positionMoveDelta.getY()) / getLocalBounds().getHeight();

            auto const& cachedPos = m_objectPosMultiEditStartValues.at(objectId);
            auto newPosX = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getX() + xDelta)));
            auto newPosY = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getY() - yDelta)));

            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, newPosX);
            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, newPosY);

            updatedScreenCoords.push_back(juce::Point<float>(newPosX * w, h - (newPosY * h)));
        }
    }

    if (m_multiselectionVisualizer)
        m_multiselectionVisualizer->UpdateSelectionPoints(updatedScreenCoords);

    m_objectPosMultiEditStartValues.clear();
}

/**
 * Helper to batch modifiy object rotation and scaling relative to their common center of gravity
 * @param   objectIds           The ids of the soundobjects that shall be modified
 * @param   cog                 The center of gravity the rot/scale values refer to
 * @param   rotation            The rotation angle to apply
 * @param   scaling             The relative scaling to apply
 */
void MultiSoundobjectSlider::applyObjectsRotAndScale(const std::vector<SoundobjectProcessorId>& objectIds, const juce::Point<float>& cog, const float rotation, const float scaling)
{
    auto ctrl = Controller::GetInstance();
    if (!ctrl)
        return;
    if (getLocalBounds().getWidth() == 0 || getLocalBounds().getHeight() == 0)
        return;

    auto relCOG = juce::Point<float>(cog.getX() / getLocalBounds().getWidth(), cog.getY() / getLocalBounds().getHeight());

    auto updatedScreenCoords = std::vector<juce::Point<float>>();
    auto w = getLocalBounds().toFloat().getWidth();
    auto h = getLocalBounds().toFloat().getHeight();

    for (auto const& objectId : objectIds)
    {
        auto processor = ctrl->GetSoundobjectProcessor(objectId);
        if (processor)
        {
            auto const& cachedPos = m_objectPosMultiEditStartValues.at(objectId);

            auto v1 = cachedPos - relCOG;
            auto sv1 = v1 * scaling;
            auto newPos = relCOG + sv1;

            newPos -= relCOG;
            newPos = newPos.rotatedAboutOrigin(rotation) + relCOG;
            
            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, newPos.getX());
            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, newPos.getY());

            updatedScreenCoords.push_back(juce::Point<float>(newPos.getX() * w, h - (newPos.getY() * h)));
        }
    }

    if (m_multiselectionVisualizer)
        m_multiselectionVisualizer->UpdateSelectionPoints(updatedScreenCoords);
}

/**
 * Helper to finalize modification of object rotation and scaling relative to their common center of gravity
 * @param   objectIds           The ids of the soundobjects that shall be finalized
 * * @param   cog                 The center of gravity the rot/scale values refer to
 * @param   rotation            The rotation angle to apply
 * @param   scaling             The relative scaling to apply
 */
void MultiSoundobjectSlider::finalizeObjectsRotAndScale(const std::vector<SoundobjectProcessorId>& objectIds, const juce::Point<float>& cog, const float rotation, const float scaling)
{
    auto ctrl = Controller::GetInstance();
    if (!ctrl)
        return;
    if (getLocalBounds().getWidth() == 0 || getLocalBounds().getHeight() == 0)
        return;

    auto relCOG = juce::Point<float>(cog.getX() / getLocalBounds().getWidth(), cog.getY() / getLocalBounds().getHeight());

    auto updatedScreenCoords = std::vector<juce::Point<float>>();
    auto w = getLocalBounds().toFloat().getWidth();
    auto h = getLocalBounds().toFloat().getHeight();

    for (auto const& objectId : objectIds)
    {
        auto processor = ctrl->GetSoundobjectProcessor(objectId);
        if (processor)
        {
            DBG(String(__FUNCTION__) + String(" EndGuiGesture for id ") + String(objectId));
            auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_X]);
            if (param)
                param->EndGuiGesture();
            param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_Y]);
            if (param)
                param->EndGuiGesture();

            auto const& cachedPos = m_objectPosMultiEditStartValues.at(objectId);

            auto v1 = cachedPos - relCOG;
            auto sv1 = v1 * scaling;
            auto newPos = relCOG + sv1;

            newPos -= relCOG;
            newPos = newPos.rotatedAboutOrigin(rotation) + relCOG;

            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, newPos.getX());
            processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, newPos.getY());

            updatedScreenCoords.push_back(juce::Point<float>(newPos.getX() * w, h - (newPos.getY() * h)));
        }
    }

    if (m_multiselectionVisualizer)
        m_multiselectionVisualizer->UpdateSelectionPoints(updatedScreenCoords);

    m_objectPosMultiEditStartValues.clear();
}

/**
 * Update the local hash of processorIds and their current parameters.
 * @param   parameters	    Map where the keys are the processorIds of each soundobject, while values are pairs of the corresponding 
 *						    soundobject number and position coordinates (0.0 to 1.0), spread, reverbSendGain and select state. 
 * @param   externalTrigger Indicator if the change was triggered outside of the application itself    
 */
void MultiSoundobjectSlider::UpdateParameters(const ParameterCache& parameters, bool externalTrigger)
{
	m_cachedParameters = parameters;

    if (m_multiselectionVisualizer)
    {
        auto& soundobjectParameterMap = std::get<0>(m_cachedParameters);
        auto& parameterFlags = std::get<1>(m_cachedParameters);
        if ((parameterFlags & CacheFlag::MultiSelection) == CacheFlag::MultiSelection)
        {
            auto selectedCoords = std::vector<juce::Point<float>>();

            auto w = getLocalBounds().toFloat().getWidth();
            auto h = getLocalBounds().toFloat().getHeight();

            for (auto const& paramsKV : soundobjectParameterMap)
            {
                auto const& isSelected = paramsKV.second._selected;
                auto const& pt = paramsKV.second._pos;

                if (isSelected)
                    selectedCoords.push_back(juce::Point<float>(pt.x * w, h - (pt.y * h)));
            }

            auto selectionCountChanged = selectedCoords.size() != m_multiselectionVisualizer->GetSelectionPoints().size();
            auto intermediateSingleSOChange = m_currentlyDraggedId != INVALID_PROCESSOR_ID;

            if (!m_multiselectionVisualizer->IsSelectionVisuActive() || selectionCountChanged || intermediateSingleSOChange || externalTrigger)
            {
                m_multiselectionVisualizer->SetSelectionPoints(selectedCoords);
                m_multiselectionVisualizer->SetSelectionVisuActive(true);
            }
        }
        else
        {
            if (m_multiselectionVisualizer->IsSelectionVisuActive())
            {
                m_multiselectionVisualizer->SetSelectionPoints(std::vector<juce::Point<float>>());
                m_multiselectionVisualizer->SetSelectionVisuActive(false);
            }
        }
    }
}


} // namespace SpaConBridge
