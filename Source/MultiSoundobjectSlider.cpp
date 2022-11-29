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


#include "MultiSoundobjectSlider.h"

#include "Controller.h"

#include "CustomAudioProcessors/Parameters.h"
#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"


namespace SpaConBridge
{


/*
===============================================================================
    Class MultiSOSelectionVisualizerComponent
===============================================================================
*/

/**
 * Default constructor
 */
MultiSOSelectionVisualizerComponent::MultiSOSelectionVisualizerComponent()
{

}

/**
 * Default destructor
 */
MultiSOSelectionVisualizerComponent::~MultiSOSelectionVisualizerComponent()
{

}

/**
 * Setter for the active state used to decide if painting if required or not.
 * @param   active  The boolean active state
 */
void MultiSOSelectionVisualizerComponent::SetSelectionVisuActive(bool active)
{
    m_selectionVisuActive = active;
}

/**
 * Setter for the list of points that are selected and shall be used as base for multiselection visu.
 * @param   points     The list of points to copy into internal member list.
 */
void MultiSOSelectionVisualizerComponent::SetSelectionPoints(const std::vector<juce::Point<float>>& points)
{
    m_selectionPoints = points;
}

/**
 * Reimplemented paint method to perform the actual visualization drawing
 * @param   g   The graphics object to use for painting.
 */
void MultiSOSelectionVisualizerComponent::paint(Graphics& g)
{
    Component::paint(g);

    // Paint the multiselection indication elements
    if (m_selectionVisuActive && !m_selectionPoints.empty())
    {
        auto multitselectionIndicationColour = getLookAndFeel().findColour(TextButton::textColourOnId).brighter(0.15f);
        g.setColour(multitselectionIndicationColour);

        auto prevCoord = m_selectionPoints.back();
        auto sum = juce::Point<float>(0.0f, 0.0f);
        for (auto const& coord : m_selectionPoints)
        {
            g.drawLine(prevCoord.getX(), prevCoord.getY(), coord.getX(), coord.getY(), 1.0f);
            prevCoord = coord;
            sum += coord;
        }

        auto cog = sum / m_selectionPoints.size();

        auto knobSizeScaleFactor = 2.0f;
        auto refKnobSize = 10.0f;
        auto knobSize = refKnobSize * knobSizeScaleFactor;
        auto knobThickness = 3.0f * knobSizeScaleFactor;
        auto fillSize = knobSize + knobThickness;
        auto outlineSize = 8 * refKnobSize;
        g.fillEllipse(Rectangle<float>(cog.getX() - (fillSize / 2.0f), cog.getY() - (fillSize / 2.0f), fillSize, fillSize));
        g.drawEllipse(Rectangle<float>(cog.getX() - (outlineSize / 2.0f), cog.getY() - (outlineSize / 2.0f), outlineSize, outlineSize), 1.0f);
    }

}

/**
 * Reimplemented mouse event handling to forward the event to parent component
 * in order to not block any user interaction from handling in parent component.
 * @param   e   The event that occurd and is forwarded to parent component.
 */
void MultiSOSelectionVisualizerComponent::mouseDown(const MouseEvent& e)
{
    getParentComponent()->mouseDown(e);
}

/**
 * Reimplemented mouse event handling to forward the event to parent component
 * in order to not block any user interaction from handling in parent component.
 * @param   e   The event that occurd and is forwarded to parent component.
 */
void MultiSOSelectionVisualizerComponent::mouseDrag(const MouseEvent& e)
{
    getParentComponent()->mouseDrag(e);
}

/**
 * Reimplemented mouse event handling to forward the event to parent component
 * in order to not block any user interaction from handling in parent component.
 * @param   e   The event that occurd and is forwarded to parent component.
 */
void MultiSOSelectionVisualizerComponent::mouseUp(const MouseEvent& e)
{
    getParentComponent()->mouseUp(e);
}


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
    addAndMakeVisible(m_multiselectionVisualizer.get());
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
	// remove background image component from being visualized if one was set for previously selected Mapping Area
	if (m_backgroundImages.count(m_selectedMapping) != 0 && m_backgroundImages.at(m_selectedMapping))
		removeChildComponent(m_backgroundImages.at(m_selectedMapping).get());

	// set the incoming ID as currently selected Mapping Area
	m_selectedMapping = mapping;
	
	// add background image associated with newly selected Mapping Area to be visualized if one is set
	if (m_backgroundImages.count(mapping) != 0 && m_backgroundImages.at(mapping))
		addAndMakeVisible(m_backgroundImages.at(mapping).get(), 0);
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
const juce::Image* MultiSoundobjectSlider::GetBackgroundImage(MappingAreaId mappingAreaId)
{
	if (HasBackgroundImage(mappingAreaId))
		return &m_backgroundImages.at(mappingAreaId)->getImage();
	else
		return nullptr;
}

/**
 * Helper method to set a background image for the given mapping area id
 * @param	mappingAreaId	The id of the mapping are to set the background image for
 * @param	backgroundImage	The image to set as background for the given mapping area id
 */
void MultiSoundobjectSlider::SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage)
{
	if (HasBackgroundImage(mappingAreaId))
	{
		removeChildComponent(m_backgroundImages.at(mappingAreaId).get());
		m_backgroundImages.erase(mappingAreaId);
	}
	
	auto imageComponent = std::make_unique<ImageComponent>();
	imageComponent->setImage(backgroundImage);
	imageComponent->setInterceptsMouseClicks(false, false); // make the imagecomponent oblivious to mouse interaction to allow us to handle mouse down/drag/up in this component for SO moving
	m_backgroundImages.insert(std::make_pair(mappingAreaId, std::move(imageComponent)));

	if (mappingAreaId == GetSelectedMapping())
		addAndMakeVisible(m_backgroundImages.at(mappingAreaId).get(), 0);

	resized();
}

/**
 * Helper method to remove the background image for the given mapping area id
 * @param	mappingAreaId	The id of the mapping are to remove the background image for
 */
void MultiSoundobjectSlider::RemoveBackgroundImage(MappingAreaId mappingAreaId)
{
	m_backgroundImages.erase(mappingAreaId);

	resized();
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
 * Reimplemented paint event function.
 * Components can override this method to draw their content. The paint() method gets called when 
 * a region of a component needs redrawing, either because the component's repaint() method has 
 * been called, or because something has happened on the screen that means a section of a window needs to be redrawn.
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
        if (isSelected && !multiselectionActive)
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
		auto textLabel = String();
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

	if (m_backgroundImages.count(GetSelectedMapping()) != 0 && m_backgroundImages.at(GetSelectedMapping()))
		m_backgroundImages.at(GetSelectedMapping())->setBounds(getLocalBounds().reduced(2));

    if (m_multiselectionVisualizer)
        m_multiselectionVisualizer->setBounds(getLocalBounds());
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

    // if no multitouch operation is in progress and no SO was selected in loop above, we have to deal with xy pos changes for multi SOs
    if (MTDT_PendingInputDecision == m_multiTouchTargetOperation && INVALID_PROCESSOR_ID == m_currentlyDraggedId)
    {
        auto ctrl = Controller::GetInstance();
        if (ctrl)
        {
            for (auto const& paramsKV : std::get<0>(m_cachedParameters))
            {
                if (!paramsKV.second._selected)
                    continue;

                auto processor = ctrl->GetSoundobjectProcessor(paramsKV.first);
                if (processor)
                {
                    DBG(String(__FUNCTION__) + String(" BeginGuiGesture for id ") + String(paramsKV.first));
                    auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_X]);
                    if (param)
                        param->BeginGuiGesture();
                    param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_Y]);
                    if (param)
                        param->BeginGuiGesture();

                    m_objectPosMultiEditStartValues[paramsKV.first] = paramsKV.second._pos;
                }
            }
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
            else
            {
                for (auto const& paramsKV : std::get<0>(m_cachedParameters))
                {
                    if (!paramsKV.second._selected)
                        continue;

                    auto processor = ctrl->GetSoundobjectProcessor(paramsKV.first);
                    if (processor)
                    {
                        auto const& posDelta = Point<int>(e.getDistanceFromDragStartX(), e.getDistanceFromDragStartY());
                        auto xDelta = static_cast<float>(posDelta.getX()) / getLocalBounds().getWidth();
                        auto yDelta = static_cast<float>(posDelta.getY()) / getLocalBounds().getHeight();

                        auto const& cachedPos = m_objectPosMultiEditStartValues.at(paramsKV.first);
                        auto newPosX = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getX() + xDelta)));
                        auto newPosY = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getY() - yDelta)));

                        processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, newPosX);
                        processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, newPosY);
                    }
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
            else
            {
                for (auto const& paramsKV : std::get<0>(m_cachedParameters))
                {
                    if (!paramsKV.second._selected)
                        continue;

                    auto processor = ctrl->GetSoundobjectProcessor(paramsKV.first);
                    if (processor)
                    {
                        DBG(String(__FUNCTION__) + String(" EndGuiGesture for id ") + String(paramsKV.first));
                        auto param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_X]);
                        if (param)
                            param->EndGuiGesture();
                        param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[SPI_ParamIdx_Y]);
                        if (param)
                            param->EndGuiGesture();

                        // Get mouse pixel-wise position and scale it between 0 and 1.
                        auto const& posDelta = Point<int>(e.getDistanceFromDragStartX(), e.getDistanceFromDragStartY());
                        auto xDelta = static_cast<float>(posDelta.getX()) / getLocalBounds().getWidth();
                        auto yDelta = static_cast<float>(posDelta.getY()) / getLocalBounds().getHeight();

                        auto const& cachedPos = m_objectPosMultiEditStartValues.at(paramsKV.first);
                        auto newPosX = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getX() + xDelta)));
                        auto newPosY = jmin<float>(1.0, jmax<float>(0.0, (cachedPos.getY() - yDelta)));

                        processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, newPosX);
                        processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, newPosY);
                    }
                }


                m_objectPosMultiEditStartValues.clear();
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
            auto& id = m_currentlyDraggedId;
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
                    auto& id = paramsKV.first;
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
 * Update the local hash of processorIds and their current parameters.
 * @param parameters	Map where the keys are the processorIds of each soundobject, while values are pairs of the corresponding 
 *						soundobject number and position coordinates (0.0 to 1.0), spread, reverbSendGain and select state. 
 */
void MultiSoundobjectSlider::UpdateParameters(const ParameterCache& parameters)
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

            m_multiselectionVisualizer->SetSelectionPoints(selectedCoords);
            m_multiselectionVisualizer->SetSelectionVisuActive(true);
        }
        else
        {
            m_multiselectionVisualizer->SetSelectionPoints(std::vector<juce::Point<float>>());
            m_multiselectionVisualizer->SetSelectionVisuActive(false);
        }
    }
}


} // namespace SpaConBridge
