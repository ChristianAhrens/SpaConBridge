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
        for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
        {
            auto& mappingAreaId = paramsByMappingsKV.first;
            if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
            {
                for (auto const& paramsKV : paramsByMappingsKV.second)
                {
                    if (!paramsKV.second._selected)
                        continue;

                    objectIdsToCache.push_back(paramsKV.first);
                }
            }
        }
        
        cacheObjectsXYPos(objectIdsToCache);
    };
    m_multiselectionVisualizer->onMouseXYPosChanged = [this](const juce::Point<int>& posDelta) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
        {
            auto& mappingAreaId = paramsByMappingsKV.first;
            if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
            {
                for (auto const& paramsKV : paramsByMappingsKV.second)
                {
                    if (!paramsKV.second._selected)
                        continue;

                    objectIdsToModify.push_back(paramsKV.first);
                }
            }
        }
        
        moveObjectsXYPos(objectIdsToModify, posDelta);
    };
    m_multiselectionVisualizer->onMouseXYPosFinished = [this](const juce::Point<int>& posDelta) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
        {
            auto& mappingAreaId = paramsByMappingsKV.first;
            if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
            {
                for (auto const& paramsKV : paramsByMappingsKV.second)
                {
                    if (!paramsKV.second._selected)
                        continue;

                    objectIdsToModify.push_back(paramsKV.first);
                }
            }
        }
        
        finalizeObjectsXYPos(objectIdsToModify, posDelta);
    };
    m_multiselectionVisualizer->onMouseRotAndScaleChanged = [this](const juce::Point<float>& cog, const float roation, const float scaling) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
        {
            auto& mappingAreaId = paramsByMappingsKV.first;
            if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
            {
                for (auto const& paramsKV : paramsByMappingsKV.second)
                {
                    if (!paramsKV.second._selected)
                        continue;

                    objectIdsToModify.push_back(paramsKV.first);
                }
            }
        }

        applyObjectsRotAndScale(objectIdsToModify, cog, roation, scaling);
    };
    m_multiselectionVisualizer->onMouseRotAndScaleFinished = [this](const juce::Point<float>& cog, const float roation, const float scaling) {
        auto objectIdsToModify = std::vector<SoundobjectProcessorId>();
        for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
        {
            auto& mappingAreaId = paramsByMappingsKV.first;
            if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
            {
                for (auto const& paramsKV : paramsByMappingsKV.second)
                {
                    if (!paramsKV.second._selected)
                        continue;

                    objectIdsToModify.push_back(paramsKV.first);
                }
            }
        }

        finalizeObjectsRotAndScale(objectIdsToModify, cog, roation, scaling);
    };

    for (auto i = int(MAI_First); i <= int(MAI_Fourth); i++)
    {
        auto mappingAreaId = static_cast<MappingAreaId>(i);
        m_mappingCornersReal[mappingAreaId] = { {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
        m_mappingCornersVirtual[mappingAreaId] = { {0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f } };
        m_mappingFlip[mappingAreaId] = false;
        m_mappingName[mappingAreaId] = juce::String("empty");
    }

    for (auto i = 1; i <= DS100_CHANNELCOUNT; i++)
    {
        auto channelId = static_cast<ChannelId>(i);
        m_speakerPositions[channelId] = std::make_pair(juce::Vector3D(0.0f, 0.0f, 0.0f), juce::Vector3D(0.0f, 0.0f, 0.0f));
    }

    lookAndFeelChanged();
}

/**
 * Object destructor.
 */
MultiSoundobjectSlider::~MultiSoundobjectSlider()
{
}

/**
 * Reimplemented method to handle colour changes for manually painted svg member objects
 */
void MultiSoundobjectSlider::lookAndFeelChanged()
{
    Component::lookAndFeelChanged();

    for (auto const& speakerDrawableKV : m_speakerDrawables)
        if (speakerDrawableKV.second)
            speakerDrawableKV.second->replaceColour(m_speakerDrawablesCurrentColour, getLookAndFeel().findColour(TextButton::textColourOnId));

    m_speakerDrawablesCurrentColour = getLookAndFeel().findColour(TextButton::textColourOnId);
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
    if (GetSelectedMapping() == MAI_Invalid)
    {
        if (!IsCoordinateMappingsSettingsDataReady() && !IsSpeakerPositionDataReady())
        {
            g.setColour(getLookAndFeel().findColour(TextEditor::textColourId));
            g.drawText("CoordinateMapping settings and speaker positions not yet read from device", getLocalBounds(), Justification::centred);
        }
        else
        {
            paintSpeakersAndMappingAreas2DVisu(g);
        }
    }
    else
    {
        paintMappingArea2DVisu(g);
    }
}

/**
 * Painting helper method for painting of all mapping areas, soundobjects and speakers at their positions
 * @param g		The graphics context that must be used to do the drawing operations.
 */
void MultiSoundobjectSlider::paintSpeakersAndMappingAreas2DVisu(Graphics& g)
{
    // Solid surface background
    auto backgroundRect = GetAspectAndMarginCorrectedBounds().toFloat().reduced(2);
    g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    g.fillRect(backgroundRect);
    
    // Surface frame
    g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
    g.drawRect(GetAspectAndMarginCorrectedBounds().toFloat(), 1.5f);

    // Mapping Areas
    for (auto i = int(MAI_First); i <= int(MAI_Fourth); i++)
    {
        auto mappingAreaId = static_cast<MappingAreaId>(i);

        g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));

        if (m_mappingAreaPaths.count(mappingAreaId) == 1 
            && m_mappingName.count(mappingAreaId) == 1
            && m_mappingTextAnchorPointAndRot.count(mappingAreaId) == 1
            && m_mappingCornersVirtual.count(mappingAreaId) == 1
            && m_mappingCornersVirtualPoints.count(mappingAreaId) == 1)
        {
            g.fillPath(m_mappingAreaPaths.at(mappingAreaId));

            auto mappingString = juce::String("CoordinateMapping ") + juce::String(mappingAreaId);
            if (m_mappingName.at(mappingAreaId).isNotEmpty())
                mappingString = m_mappingName.at(mappingAreaId);

            g.saveState();
            g.setFont(18);
            auto margin = 5;
            auto& anchorPoint = m_mappingTextAnchorPointAndRot.at(mappingAreaId).first;
            auto& angle = m_mappingTextAnchorPointAndRot.at(mappingAreaId).second;
            g.setOrigin(anchorPoint);
            g.addTransform(juce::AffineTransform().rotated(angle));
            g.drawSingleLineText(mappingString, margin, -margin);
            g.restoreState();

            g.setFont(12);
            auto& virtCorners = m_mappingCornersVirtual.at(mappingAreaId);
            auto& virtPoints = m_mappingCornersVirtualPoints.at(mappingAreaId);
            if (virtCorners.size() >= 2)
            {
                auto& p1Point = virtPoints.at(0);
                auto& p1Corner = virtCorners.at(0);
                juce::String p1Str;
                p1Str << "M" << mappingAreaId << "P1(" << p1Corner.x << "," << p1Corner.y << ")";
                g.drawSingleLineText(p1Str, p1Point.x + margin, p1Point.y - margin);

                auto& p3Point = virtPoints.at(1);
                auto& p3Corner = virtCorners.at(1);
                juce::String p3Str;
                p3Str << "M" << mappingAreaId << "P3(" << p3Corner.x << "," << p3Corner.y << ")";
                g.drawSingleLineText(p3Str, p3Point.x - g.getCurrentFont().getStringWidth(p3Str) - margin, p3Point.y + margin);
            }
        }
    }

    // Speaker positions
    g.setColour(m_speakerDrawablesCurrentColour);
    for (auto const& speakerDrawableKV : m_speakerDrawables)
    {
        // draw speaker icons in target area
        speakerDrawableKV.second->drawWithin(g, m_speakerDrawableAreas[speakerDrawableKV.first], juce::RectanglePlacement::centred, 1.0f);
        // draw framing rect around icons, 2px larger than icon target area itself
        g.drawRect(m_speakerDrawableAreas[speakerDrawableKV.first].expanded(2.0f));
    }

    // All cached soundobjects
    paintSoundobjects(g);
}

/**
 * Painting helper method for classic painting of one of the four mapping areas and the related soundobjects
 * @param g		The graphics context that must be used to do the drawing operations.
 */
void MultiSoundobjectSlider::paintMappingArea2DVisu(Graphics & g)
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

    auto width = getLocalBounds().toFloat().getWidth();
    auto height = getLocalBounds().toFloat().getHeight();

    // Draw grid
    const float dashLengths[2] = { 5.0f, 6.0f };
    const float lineThickness = 1.0f;
    g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId).brighter(0.15f));
    g.drawDashedLine(Line<float>(width * 0.25f, 0.0f, width * 0.25f, height), dashLengths, 2, lineThickness);
    g.drawDashedLine(Line<float>(width * 0.50f, 0.0f, width * 0.50f, height), dashLengths, 2, lineThickness);
    g.drawDashedLine(Line<float>(width * 0.75f, 0.0f, width * 0.75f, height), dashLengths, 2, lineThickness);
    g.drawDashedLine(Line<float>(0.0f, height * 0.25f, width, height * 0.25f), dashLengths, 2, lineThickness);
    g.drawDashedLine(Line<float>(0.0f, height * 0.50f, width, height * 0.50f), dashLengths, 2, lineThickness);
    g.drawDashedLine(Line<float>(0.0f, height * 0.75f, width, height * 0.75f), dashLengths, 2, lineThickness);

    // Surface frame
    g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
    g.drawRect(Rectangle<float>(0.0f, 0.0f, width, height), 1.5f);

    // All cached soundobjects
    paintSoundobjects(g);
}

/**
 * Painting helper method for painting of soundobjects
 * @param g		The graphics context that must be used to do the drawing operations.
 */
void MultiSoundobjectSlider::paintSoundobjects(Graphics& g)
{
    float refKnobSize = 10.0f;

    auto width = getLocalBounds().toFloat().getWidth();
    auto height = getLocalBounds().toFloat().getHeight();

    const float dashLengths[2] = { 5.0f, 6.0f };
    const float lineThickness = 1.0f;

    auto& soundobjectParameterMap = std::get<0>(m_cachedParameters);
    auto& parameterFlags = std::get<1>(m_cachedParameters);

    auto multiselectionActive = false;
    if ((parameterFlags & CacheFlag::MultiSelection) == CacheFlag::MultiSelection)
        multiselectionActive = true;

    auto selectedCoords = std::vector<juce::Point<float>>();

    for (auto const& paramsByMappingsKV : soundobjectParameterMap)
    {
        auto& mappingAreaId = paramsByMappingsKV.first;
        if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
        {
            for (auto const& paramsKV : paramsByMappingsKV.second)
            {
                auto const& isSelected = paramsKV.second._selected;

                if (m_handleSelectedOnly && !isSelected)
                    continue;

                auto currentCoords = GetPointForRelativePosOnMapping(paramsKV.second._pos, mappingAreaId);

                auto knobColour = paramsKV.second._colour;

                auto knobSizeScaleFactor = static_cast<float>(1.0f + (2.0f * paramsKV.second._size));
                auto knobSize = refKnobSize * knobSizeScaleFactor;
                auto knobThickness = 3.0f * knobSizeScaleFactor;

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
                    g.drawLine(0, y, width, y, 1);
                    g.drawLine(x, 0, x, height, 1);

                    // Paint 'currently dual-multitouch points indication'
                    auto& p1 = m_multiTouchPoints._p2_init;
                    auto& p2 = m_multiTouchPoints._p2;
                    auto goodVisibilityDistance = 16;
                    switch (m_multiTouchTargetOperation)
                    {
                    case MTDT_HorizontalEnSpaceSendGain:
                    {
                        g.setColour(crosshairColour);
                        g.drawDashedLine(Line<float>(p1.toFloat().getX(), 0.0f, p1.toFloat().getX(), height), dashLengths, 2, lineThickness);
                        g.drawDashedLine(Line<float>(p2.toFloat().getX(), 0.0f, p2.toFloat().getX(), height), dashLengths, 2, lineThickness);
                        g.setOpacity(0.15f);
                        g.fillRect(Rectangle<float>(p1.toFloat().getX(), 0.0f, p2.toFloat().getX() - p1.toFloat().getX(), height));

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
                        g.drawDashedLine(Line<float>(0.0f, p1.toFloat().getY(), width, p1.toFloat().getY()), dashLengths, 2, lineThickness);
                        g.drawDashedLine(Line<float>(0.0f, p2.toFloat().getY(), width, p2.toFloat().getY()), dashLengths, 2, lineThickness);
                        g.setOpacity(0.15f);
                        g.fillRect(Rectangle<float>(0.0f, p1.toFloat().getY(), width, p2.toFloat().getY() - p1.toFloat().getY()));

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
        }
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
            g.drawDashedLine(Line<float>(p1.toFloat().getX(), 0.0f, p1.toFloat().getX(), height), dashLengths, 2, lineThickness);
            g.drawDashedLine(Line<float>(p2.toFloat().getX(), 0.0f, p2.toFloat().getX(), height), dashLengths, 2, lineThickness);
            g.setOpacity(0.15f);
            g.fillRect(Rectangle<float>(p1.toFloat().getX(), 0.0f, p2.toFloat().getX() - p1.toFloat().getX(), height));

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
            g.drawDashedLine(Line<float>(0.0f, p1.toFloat().getY(), width, p1.toFloat().getY()), dashLengths, 2, lineThickness);
            g.drawDashedLine(Line<float>(0.0f, p2.toFloat().getY(), width, p2.toFloat().getY()), dashLengths, 2, lineThickness);
            g.setOpacity(0.15f);
            g.fillRect(Rectangle<float>(0.0f, p1.toFloat().getY(), width, p2.toFloat().getY() - p1.toFloat().getY()));

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

                for (auto const& paramsByMappingsKV : soundobjectParameterMap)
                {
                    auto& mappingAreaId = paramsByMappingsKV.first;
                    if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
                    {
                        for (auto const& paramsKV : paramsByMappingsKV.second)
                        {
                            auto const& isSelected = paramsKV.second._selected;
                            auto const& pt = paramsKV.second._pos;

                            if (isSelected)
                                selectedCoords.push_back(juce::Point<float>(pt.x * w, h - (pt.y * h)));
                        }
                    }
                }

                m_multiselectionVisualizer->SetSelectionPoints(selectedCoords);
            }
        }
    }

    PrerenderSpeakerAndMappingAreaInBounds();
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
    
    auto orig = GetAspectAndMarginCorrectedBounds().getTopLeft().toFloat();

	// Mouse click position (in pixel units)
	Point<float> mousePos(static_cast<float>(e.getMouseDownPosition().x), static_cast<float>(e.getMouseDownPosition().y));

	float refKnobSize = 10.0f;

    for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
    {
        auto& mappingAreaId = paramsByMappingsKV.first;
        if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
        {
            for (auto const& paramsKV : paramsByMappingsKV.second)
            {
                auto const& selected = paramsKV.second._selected;

                if (m_handleSelectedOnly && !selected)
                    continue;

                // Map the x/y coordinates to the pixel-wise dimensions of the surface area.
                auto pt = GetPointForRelativePosOnMapping(paramsKV.second._pos, mappingAreaId);

                auto knobSizeScaleFactor = static_cast<float>(1.0f + (1.5f * paramsKV.second._size));
                auto knobSize = refKnobSize * knobSizeScaleFactor;
                auto knobThickness = 3.0f * knobSizeScaleFactor;

                Path knobPath;
                auto fillSize = knobSize + knobThickness;
                knobPath.addEllipse(Rectangle<float>(pt.x - (fillSize / 2.0f), pt.y - (fillSize / 2.0f), fillSize, fillSize));

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
                    auto relPos = GetPosOnMappingForPoint(e.getPosition().toFloat(), MappingAreaId(processor->GetMappingId()));

                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, relPos.getX());
                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, relPos.getY());
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

                    auto relPos = GetPosOnMappingForPoint(e.getPosition().toFloat(), MappingAreaId(processor->GetMappingId()));

                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_X, relPos.getX());
                    processor->SetParameterValue(DCP_MultiSlider, SPI_ParamIdx_Y, relPos.getY());
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
            for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
            {
                auto& mappingAreaId = paramsByMappingsKV.first;
                if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
                {
                    for (auto const& paramsKV : paramsByMappingsKV.second)
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
            for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
            {
                auto& mappingAreaId = paramsByMappingsKV.first;
                if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
                {
                    for (auto const& paramsKV : paramsByMappingsKV.second)
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
                    for (auto const& paramsByMappingsKV : std::get<0>(m_cachedParameters))
                    {
                        auto& mappingAreaId = paramsByMappingsKV.first;
                        if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
                        {
                            for (auto const& paramsKV : paramsByMappingsKV.second)
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
            jassert(soundobjectParameterMap[GetSelectedMapping()].count(objectId) > 0);
            m_objectPosMultiEditStartValues[objectId] = soundobjectParameterMap.at(GetSelectedMapping()).at(objectId)._pos;
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

    auto relCOG = juce::Point<float>(cog.getX() / getLocalBounds().getWidth(), 1 - (cog.getY() / getLocalBounds().getHeight()));

    auto w = getLocalBounds().toFloat().getWidth();
    auto h = getLocalBounds().toFloat().getHeight();

    auto scalingMatrix = AffineTransform::scale(scaling, scaling, relCOG.getX(), relCOG.getY());
    auto rotationMatrix = AffineTransform::rotation(rotation, relCOG.getX(), relCOG.getY());

    auto updatedScreenCoords = std::vector<juce::Point<float>>();
    for (auto const& objectId : objectIds)
    {
        auto processor = ctrl->GetSoundobjectProcessor(objectId);
        if (processor)
        {
            auto const& cachedPos = m_objectPosMultiEditStartValues.at(objectId);

            auto newPos = cachedPos.transformedBy(scalingMatrix).transformedBy(rotationMatrix);
            
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

    auto relCOG = juce::Point<float>(cog.getX() / getLocalBounds().getWidth(), 1 - (cog.getY() / getLocalBounds().getHeight()));

    auto w = getLocalBounds().toFloat().getWidth();
    auto h = getLocalBounds().toFloat().getHeight();

    auto scalingMatrix = AffineTransform::scale(scaling, scaling, relCOG.getX(), relCOG.getY());
    auto rotationMatrix = AffineTransform::rotation(rotation, relCOG.getX(), relCOG.getY());

    auto updatedScreenCoords = std::vector<juce::Point<float>>();
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

            auto newPos = cachedPos.transformedBy(scalingMatrix).transformedBy(rotationMatrix);

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

            for (auto const& paramsByMappingsKV : soundobjectParameterMap)
            {
                auto& mappingAreaId = paramsByMappingsKV.first;
                if (GetSelectedMapping() == mappingAreaId || GetSelectedMapping() == MAI_Invalid)
                {
                    for (auto const& paramsKV : paramsByMappingsKV.second)
                    {
                        auto const& isSelected = paramsKV.second._selected;
                        auto const& pt = paramsKV.second._pos;

                        if (isSelected)
                            selectedCoords.push_back(juce::Point<float>(pt.x * w, h - (pt.y * h)));
                    }
                }
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

/**
 * Helper method to check the internal data maps for if all coordmapsettings have been received
 * @return	True if data for all four mapping areas is contained in the internal maps
 */
bool MultiSoundobjectSlider::CheckCoordinateMappingSettingsDataCompleteness()
{
    auto requiredRealPoints = 4;
    auto requiredVirtualPoints = 2;
    auto cornRealIsComplete = m_mappingCornersReal.count(MAI_First) == 1 && m_mappingCornersReal.count(MAI_Second) == 1 && m_mappingCornersReal.count(MAI_Third) == 1 && m_mappingCornersReal.count(MAI_Fourth) == 1
        && m_mappingCornersReal.at(MAI_First).size() == requiredRealPoints && m_mappingCornersReal.at(MAI_Second).size() == requiredRealPoints && m_mappingCornersReal.at(MAI_Third).size() == requiredRealPoints && m_mappingCornersReal.at(MAI_Fourth).size() == requiredRealPoints;
    auto cornVirtIsComplete = m_mappingCornersVirtual.count(MAI_First) == 1 && m_mappingCornersVirtual.count(MAI_Second) == 1 && m_mappingCornersVirtual.count(MAI_Third) == 1 && m_mappingCornersVirtual.count(MAI_Fourth) == 1
        && m_mappingCornersVirtual.at(MAI_First).size() == requiredVirtualPoints && m_mappingCornersVirtual.at(MAI_Second).size() == requiredVirtualPoints && m_mappingCornersVirtual.at(MAI_Third).size() == requiredVirtualPoints && m_mappingCornersVirtual.at(MAI_Fourth).size() == requiredVirtualPoints;
    auto flipIsComplete = m_mappingFlip.count(MAI_First) == 1 && m_mappingFlip.count(MAI_Second) == 1 && m_mappingFlip.count(MAI_Third) == 1 && m_mappingFlip.count(MAI_Fourth) == 1;
    auto nameIsComplete = m_mappingName.count(MAI_First) == 1 && m_mappingName.count(MAI_Second) == 1 && m_mappingName.count(MAI_Third) == 1 && m_mappingName.count(MAI_Fourth) == 1;

    return cornRealIsComplete && cornVirtIsComplete && flipIsComplete && nameIsComplete;
}

/**
 * Setter for the internal member m_coordinateMappingSettingsReady
 * @param	ready	Value to set as new m_coordinateMappingSettingsReady
 */
void MultiSoundobjectSlider::SetCoordinateMappingSettingsDataReady(bool ready)
{
    auto readyChange = (ready != m_coordinateMappingSettingsDataReady);

    m_coordinateMappingSettingsDataReady = ready;

    if (m_coordinateMappingSettingsDataReady && IsSpeakerPositionDataReady())
    {
        ComputeRealBoundingRect();

        PrerenderSpeakerAndMappingAreaInBounds();
    }
    else if (!m_coordinateMappingSettingsDataReady)
    {
        m_mappingCornersReal.clear();
        m_mappingCornersVirtual.clear();
        for (auto i = int(MAI_First); i <= int(MAI_Fourth); i++)
        {
            auto mappingAreaId = static_cast<MappingAreaId>(i);
            m_mappingCornersReal[mappingAreaId].resize(4);
            m_mappingCornersVirtual[mappingAreaId].resize(2);
        }
        m_mappingFlip.clear();
        m_mappingName.clear();
    }

    if (readyChange)
        repaint();
}

/**
 * Getter for the internal member m_coordinateMappingSettingsReady
 * @return Current value of m_coordinateMappingSettingsReady
 */
bool MultiSoundobjectSlider::IsCoordinateMappingsSettingsDataReady()
{
    return m_coordinateMappingSettingsDataReady;
}

/**
 * Getter for the mapping corner real point positions.
 * @return  The map of positions for all mappings
 */
const std::map<MappingAreaId, std::vector<juce::Vector3D<float>>>& MultiSoundobjectSlider::GetMappingCornersReal() 
{ 
    return m_mappingCornersReal; 
}

/**
 * Getter for the mapping corner virtual point positions.
 * @return  The map of positions for all mappings
 */
const std::map<MappingAreaId, std::vector<juce::Vector3D<float>>>& MultiSoundobjectSlider::GetMappingCornersVirtual() 
{ 
    return m_mappingCornersVirtual; 
}

/**
 * Getter for the flip property of all mappings
 * @return  The map of flip properties for all mappings
 */
const std::map<MappingAreaId, bool>& MultiSoundobjectSlider::GetMappingFlip() 
{ 
    return m_mappingFlip; 
}

/**
 * Getter for the name of all mappings
 * @return  The map of names for all mappings
 */
const std::map<MappingAreaId, juce::String>& MultiSoundobjectSlider::GetMappingName() 
{ 
    return m_mappingName; 
}

/**
 * Setter for a mapping corner real point position.
 * @param   mappingAreaId       The id of the mapping area to set the value for.
 * @param   cornerIndex         The index of the corner to set the value for.
 * @param   mappingCornerReal   The actual corner pos value to set.
 */
void MultiSoundobjectSlider::SetMappingCornerReal(const MappingAreaId mappingAreaId, int cornerIndex, const juce::Vector3D<float>& mappingCornerReal)
{
    if (m_mappingCornersReal[mappingAreaId].size() <= cornerIndex)
        return;

    m_mappingCornersReal[mappingAreaId][cornerIndex] = ComputeNonDBRealPointCoordinate(mappingCornerReal);
}

/**
 * Setter for a mapping corner virtual point position.
 * @param   mappingAreaId       The id of the mapping area to set the value for.
 * @param   cornerIndex         The index of the corner to set the value for.
 * @param   mappingCornerReal   The actual corner pos value to set.
 */
void MultiSoundobjectSlider::SetMappingCornerVirtual(const MappingAreaId mappingAreaId, int cornerIndex, const juce::Vector3D<float>& mappingCornerVirtual)
{
    if (m_mappingCornersVirtual[mappingAreaId].size() <= cornerIndex)
        return;

    m_mappingCornersVirtual[mappingAreaId][cornerIndex] = mappingCornerVirtual;
}

/**
 * Setter for the flip property of a mappings
 * @param   mappingAreaId   The id of the mapping area to set the value for.
 * @param   mappingFlip     The flip value to set.
 */
void MultiSoundobjectSlider::SetMappingFlip(const MappingAreaId mappingAreaId, bool mappingFlip)
{
    m_mappingFlip[mappingAreaId] = mappingFlip;
}

/**
 * Setter for the name of a mappings
 * @param   mappingAreaId   The id of the mapping area to set the value for.
 * @param   mappingName     The name string to set.
 */
void MultiSoundobjectSlider::SetMappingName(const MappingAreaId mappingAreaId, const juce::String& mappingName)
{
    m_mappingName[mappingAreaId] = mappingName;
}

/**
 * Helper method to check the internal data maps for if all coordmapsettings have been received
 * @return	True if data for all four mapping areas is contained in the internal maps
 */
bool MultiSoundobjectSlider::CheckSpeakerPositionDataCompleteness()
{
    for (auto i = 1; i <= DS100_CHANNELCOUNT; i++)
    {
        if (m_speakerPositions.count(i) != 1)
            return false;
    }

    return true;
}

/**
 * Setter for the internal member m_speakerPositionDataReady
 * @param	ready	Value to set as new m_speakerPositionDataReady
 */
void MultiSoundobjectSlider::SetSpeakerPositionDataReady(bool ready)
{
    auto readyChange = (ready != m_speakerPositionDataReady);

    m_speakerPositionDataReady = ready;

    if (m_speakerPositionDataReady && IsCoordinateMappingsSettingsDataReady())
    {
        ComputeRealBoundingRect();
        
        for (auto i = 1; i <= DS100_CHANNELCOUNT; i++)
        {
            auto channelId = static_cast<ChannelId>(i);
            if (m_speakerPositions.count(channelId) == 1 && m_speakerPositions.at(channelId).first.length() != 0.0f)
            {
                auto& rot = m_speakerPositions.at(channelId).second;
                if ((int(rot.y) % 180) > 75 && (int(rot.y) % 180) < 105) // use icon without directivity if angle is too steep (+-15deg)
                    m_speakerDrawables[channelId] = Drawable::createFromSVG(*XmlDocument::parse(BinaryData::loudspeaker_vert24px_svg));
                else
                    m_speakerDrawables[channelId] = Drawable::createFromSVG(*XmlDocument::parse(BinaryData::loudspeaker_hor24px_svg));
                auto& drawable = m_speakerDrawables.at(channelId);
                drawable->replaceColour(Colours::black, getLookAndFeel().findColour(TextButton::textColourOnId));
                m_speakerDrawablesCurrentColour = getLookAndFeel().findColour(TextButton::textColourOnId);
                auto drawableBounds = drawable->getBounds().toFloat();
                drawable->setTransform(juce::AffineTransform::rotation(juce::degreesToRadians(rot.x), drawableBounds.getCentreX(), drawableBounds.getCentreY()));
            }
        }

        PrerenderSpeakerAndMappingAreaInBounds();
    }
    else if (!m_speakerPositionDataReady)
    {
        m_speakerPositions.clear();
        m_speakerDrawableAreas.clear();
        m_speakerDrawables.clear();
    }

    if (readyChange)
        repaint();
}

/**
 * Getter for the internal member m_speakerPositionDataReady
 * @return Current value of m_speakerPositionDataReady
 */
bool MultiSoundobjectSlider::IsSpeakerPositionDataReady()
{
    return m_speakerPositionDataReady;
}

/**
 * Getter for the speakerpositions currently known.
 * @return  The map of speaker positions per channel.
 */
const std::map<ChannelId, std::pair<juce::Vector3D<float>, juce::Vector3D<float>>>& MultiSoundobjectSlider::GetSpeakerPositions()
{
    return m_speakerPositions;
}

/**
 * Setter for a speakerposition for a channel.
 * @param   channelId           The channel to set the speakerposition for.
 * @param   speakerPosition     The position to set.
 */
void MultiSoundobjectSlider::SetSpeakerPosition(const ChannelId channelId, const std::pair<juce::Vector3D<float>, const juce::Vector3D<float>>& speakerPosition)
{
    m_speakerPositions[channelId] = std::make_pair(ComputeNonDBRealPointCoordinate(speakerPosition.first), ComputeNonDBRealPointRotation(speakerPosition.second));
}

/**
 * Helper method to compute the bounding rect in real coordinates
 * of the internally available data for speaker positions and
 * mapping area corner points.
 */
void MultiSoundobjectSlider::ComputeRealBoundingRect()
{
    jassert(m_speakerPositions.count(1) == 1);
    m_realXBoundingRange.setStart(m_speakerPositions.at(1).first.x);
    m_realXBoundingRange.setEnd(m_speakerPositions.at(1).first.x);
    m_realYBoundingRange.setStart(m_speakerPositions.at(1).first.y);
    m_realYBoundingRange.setEnd(m_speakerPositions.at(1).first.y);

    for (auto i = 1; i <= DS100_CHANNELCOUNT; i++)
    {
        auto channelId = static_cast<ChannelId>(i);
        if (m_speakerPositions.count(channelId) == 1)
        {
            auto& p = m_speakerPositions.at(channelId).first;
            if (m_realXBoundingRange.getEnd() < p.x)
                m_realXBoundingRange.setEnd(p.x);
            if (m_realXBoundingRange.getStart() > p.x)
                m_realXBoundingRange.setStart(p.x);
            if (m_realYBoundingRange.getEnd() < p.y)
                m_realYBoundingRange.setEnd(p.y);
            if (m_realYBoundingRange.getStart() > p.y)
                m_realYBoundingRange.setStart(p.y);
        }
    }

    for (auto i = int(MAI_First); i <= int(MAI_Fourth); i++)
    {
        auto mappingAreaId = static_cast<MappingAreaId>(i);
        if (m_mappingCornersReal.count(mappingAreaId) == 1)
        {
            for (auto j = 0; j < 4; j++) // p1 real - p4 real
            {
                if (m_mappingCornersReal.at(mappingAreaId).size() > j)
                {
                    auto& p = m_mappingCornersReal.at(mappingAreaId).at(j);
                    if (m_realXBoundingRange.getEnd() < p.x)
                        m_realXBoundingRange.setEnd(p.x);
                    if (m_realXBoundingRange.getStart() > p.x)
                        m_realXBoundingRange.setStart(p.x);
                    if (m_realYBoundingRange.getEnd() < p.y)
                        m_realYBoundingRange.setEnd(p.y);
                    if (m_realYBoundingRange.getStart() > p.y)
                        m_realYBoundingRange.setStart(p.y);
                }
            }
        }
    }
}

/**
 * Helper method to convert a given real coordinate to a point in the screen bounds of this component
 * @param   realCoordinate  The real coordinate to convert
 * @return  The converted point
 */
juce::Point<float> MultiSoundobjectSlider::GetPointForRealCoordinate(const juce::Vector3D<float>& realCoordinate)
{
    if (m_realXBoundingRange.getLength() == 0.0f || m_realYBoundingRange.getLength() == 0.0f)
        return { 0.0f, 0.0f };

    auto relativeX = (realCoordinate.x - m_realXBoundingRange.getStart()) / m_realXBoundingRange.getLength();
    auto relativeY = (realCoordinate.y - m_realYBoundingRange.getStart()) / m_realYBoundingRange.getLength();

    return GetAspectAndMarginCorrectedBounds().getRelativePoint(relativeX, relativeY).toFloat();
}

/**
 * Helper method to convert a given position on screen in bounds of this component to a real coordinate
 * @param   pointInBounds   The point position in bounds of this component to convert
 * @return  The real coordinate for the given position
 */
juce::Vector3D<float> MultiSoundobjectSlider::GetRealCoordinateForPoint(const juce::Point<float>& pointInBounds)
{
    if (m_realXBoundingRange.getLength() == 0.0f || m_realYBoundingRange.getLength() == 0.0f)
        return { 0.0f, 0.0f, 0.0f};

    auto areaBounds = GetAspectAndMarginCorrectedBounds();
    auto relativeX = (pointInBounds.x - areaBounds.getTopLeft().x) / areaBounds.getWidth();
    auto relativeY = (pointInBounds.y - areaBounds.getTopLeft().y) / areaBounds.getHeight();

    auto realX = m_realXBoundingRange.getStart() + (m_realXBoundingRange.getLength() * relativeX);
    auto realY = m_realYBoundingRange.getStart() + (m_realYBoundingRange.getLength() * relativeY);

    return { realX, realY, 0.0f };
}

/**
 * Helper method to convert a given relative 0...1 range point to a position on screen.
 * In case a valid mapping area is the currently selected, the point is mapped to the entire screen space.
 * If currently selected is invalid, the screen position is calculated based on the real bounding rect
 * and the position of mapping areas within.
 * @param   relativePos     The relative position of a soundobject in a mappingarea
 * @param   mapping         The mappingarea the relative position relates to
 * @return  The derived position in pixel coordinates
 */
juce::Point<float> MultiSoundobjectSlider::GetPointForRelativePosOnMapping(const juce::Point<float>& relativePos, const MappingAreaId& mapping)
{
    if (GetSelectedMapping() == MAI_Invalid && IsCoordinateMappingsSettingsDataReady())
    {
        auto& mappingCornersReal = m_mappingCornersReal.at(mapping);
        auto& mappingP1 = mappingCornersReal.at(0);
        auto& mappingP2 = mappingCornersReal.at(1);
        auto& mappingP3 = mappingCornersReal.at(2);
        auto& mappingP4 = mappingCornersReal.at(3);
        auto& mappingCornersVirtual = m_mappingCornersVirtual.at(mapping);
        auto& mappingVirtP1 = mappingCornersVirtual.at(0);
        auto& mappingVirtP3 = mappingCornersVirtual.at(1);
        auto mappingVirtP2 = juce::Vector3D<float>(mappingVirtP1.x, mappingVirtP3.y, 0.0f);
        auto mappingVirtP4 = juce::Vector3D<float>(mappingVirtP3.x, mappingVirtP1.y, 0.0f);
        auto& isFlipped = m_mappingFlip.at(mapping);

        auto relPosWithSwap = isFlipped ? juce::Point<float>(relativePos.y, relativePos.x) : relativePos;
        
        auto vectorVirtX = mappingVirtP2 - mappingVirtP3;
        auto vectorX = mappingP2 - mappingP3;
        auto vectorVirtY = mappingVirtP4 - mappingVirtP3;
        auto vectorY = mappingP4 - mappingP3;
        
        // get a factor for inversion if virt point config suggests inverted movement
        auto xs = 1.0f;
        auto ys = 1.0f;
        if (vectorVirtX.x < 0 || vectorVirtX.y < 0)
            xs = -1.0f;
        if (vectorVirtY.x < 0 || vectorVirtY.y < 0)
            ys = -1.0f;
        
        // get real and relative origin vectors
        auto relOrigVector = (mappingVirtP3.x < mappingVirtP1.x && mappingVirtP3.y < mappingVirtP1.y) ? mappingVirtP3 : mappingVirtP1;
        auto origVector = (mappingVirtP3.x < mappingVirtP1.x && mappingVirtP3.y < mappingVirtP1.y) ? mappingP3 : mappingP1;
        
        // combine that information 
        auto relVectorX = vectorX * (relPosWithSwap.x - relOrigVector.x) / vectorVirtX.length() * xs;
        auto relVectorY = vectorY * (relPosWithSwap.y - relOrigVector.y) / vectorVirtY.length() * ys;
        
        auto realPos = origVector + relVectorX + relVectorY;

        return GetPointForRealCoordinate(realPos);
    }
    else if (GetSelectedMapping() == mapping)
    {
        auto w = getLocalBounds().toFloat().getWidth();
        auto h = getLocalBounds().toFloat().getHeight();
        // Map the x/y coordinates to the pixel-wise dimensions of the surface area.
        return juce::Point<float>(relativePos.x * w, h - (relativePos.y * h));
    }
    else
    {
        return getLocalBounds().getTopLeft().toFloat();
    }
}

/**
 * Helper method to convert a given position on screen to a relative 0...1 range point in the scope of a given mapping area.
 * In case a valid mapping area is the currently selected, the point is mapped to the entire screen space.
 * If currently selected is invalid, the screen position is calculated based on the real bounding rect
 * and the position of mapping areas within.
 * @param   pointInBounds   The position on screen (within bounds)
 * @param   mapping         The mappingarea the relative position shall relate
 * @return  The derived position in pixel coordinates
 */
juce::Point<float> MultiSoundobjectSlider::GetPosOnMappingForPoint(const juce::Point<float>& pointInBounds, const MappingAreaId& mapping)
{
    if (GetSelectedMapping() == MAI_Invalid && IsCoordinateMappingsSettingsDataReady())
    {
        auto realPos = GetRealCoordinateForPoint(pointInBounds);

        auto& mappingCornersReal = m_mappingCornersReal.at(mapping);
        auto& mappingP2 = mappingCornersReal.at(1);
        auto& mappingP3 = mappingCornersReal.at(2);
        auto& mappingP4 = mappingCornersReal.at(3);
        auto& mappingCornersVirtual = m_mappingCornersVirtual.at(mapping);
        auto& mappingVirtP1 = mappingCornersVirtual.at(0);
        auto& mappingVirtP3 = mappingCornersVirtual.at(1);
        auto mappingVirtP2 = juce::Vector3D<float>(mappingVirtP1.x, mappingVirtP3.y, 0.0f);
        auto mappingVirtP4 = juce::Vector3D<float>(mappingVirtP3.x, mappingVirtP1.y, 0.0f);
        auto& isFlipped = m_mappingFlip.at(mapping);
        
        auto vectorVirtX = mappingVirtP2 - mappingVirtP3;
        auto vectorX = mappingP2 - mappingP3;
        auto vectorVirtY = mappingVirtP4 - mappingVirtP3;
        auto vectorY = mappingP4 - mappingP3;

        // get a factor for inversion if virt point config suggests inverted movement
        auto xs = 1.0f;
        auto ys = 1.0f;
        if (vectorVirtX.x < 0 || vectorVirtX.y < 0)
            xs = -1.0f;
        if (vectorVirtY.x < 0 || vectorVirtY.y < 0)
            ys = -1.0f;

        // relative origin vector
        auto relOrigVector = mappingVirtP3;
        
        // calculate the actual relative position on mapping area
        auto deltaP3Pos_x = mappingP3.x - realPos.x;
        auto deltaP3Pos_y = mappingP3.y - realPos.y;
        auto deltaP2P3_x = mappingP2.x - mappingP3.x;
        auto deltaP2P3_y = mappingP2.y - mappingP3.y;
        auto deltaP4P3_x = mappingP4.x - mappingP3.x;
        auto deltaP4P3_y = mappingP4.y - mappingP3.y;

        auto dotP = deltaP3Pos_x * deltaP2P3_x + deltaP3Pos_y * deltaP2P3_y;

        auto P2P3sqrSum = deltaP2P3_x * deltaP2P3_x + deltaP2P3_y * deltaP2P3_y;
        auto P4P3sqrSum = deltaP4P3_x * deltaP4P3_x + deltaP4P3_y * deltaP4P3_y;
        if (P2P3sqrSum != 0.0f && P4P3sqrSum != 0.0f)
        {
            auto relX = -(dotP / P2P3sqrSum);
            auto relY = -((deltaP3Pos_x - relX * deltaP2P3_x) * deltaP4P3_x + (deltaP3Pos_y - relX * deltaP2P3_y) * deltaP4P3_y) / P4P3sqrSum;

            // apply potential alterations neccessary due to weird virtual mapping point configurations
            relX = relOrigVector.x + (relX * vectorVirtX.length() * xs);
            relY = relOrigVector.y + (relY * vectorVirtY.length() * ys);

            auto relPosWithSwap = isFlipped ? juce::Point<float>(relY, relX) : juce::Point<float>(relX, relY);

            return relPosWithSwap;
        }
        else
            return { 0.0f, 0.0f };
    }
    else if (GetSelectedMapping() == mapping)
    {
        auto orig = GetAspectAndMarginCorrectedBounds().getTopLeft().toFloat();
        auto w = static_cast<float>(GetAspectAndMarginCorrectedBounds().getWidth());
        auto h = static_cast<float>(GetAspectAndMarginCorrectedBounds().getHeight());

        // Get mouse pixel-wise position and scale it between 0 and 1.
        auto const& pos = pointInBounds - orig;
        auto relX = jmin<float>(1.0f, jmax<float>(0.0f, (pos.getX() / w)));
        auto relY = 1.0f - jmin<float>(1.0f, jmax<float>(0.0f, (pos.getY() / h)));

        return { relX, relY };
    }
    else
    {
        return { 0.0f ,0.0f };
    }
}

/**
 * Helper method to create a rect to use as bounds that respects the real aspect
 * ratio of the speaker and mappingarea dimensions
 * @return  The rectangle<int> that defines the max usable bounds and still keep the required aspect ratio
 */
juce::Rectangle<int> MultiSoundobjectSlider::GetAspectAndMarginCorrectedBounds()
{
    auto bounds = getLocalBounds();

    if (GetSelectedMapping() == MAI_Invalid)
    {
        bounds.reduce(12, 12);

        auto boundsAspect = bounds.toFloat().getAspectRatio();
        auto realAspect = m_realXBoundingRange.getLength() / m_realYBoundingRange.getLength();

        if (boundsAspect > realAspect)
        {
            //remove sth from bounds height
            auto widthToRemove = bounds.getWidth() * (1 - (realAspect / boundsAspect));
            bounds.removeFromLeft(static_cast<int>(0.5f * widthToRemove));
            bounds.removeFromRight(static_cast<int>(0.5f * widthToRemove));
        }
        else if (boundsAspect < realAspect)
        {
            //remove sth from bounds width
            auto heightToRemove = bounds.getHeight() * (1 - (boundsAspect / realAspect));
            bounds.removeFromTop(static_cast<int>(0.5f * heightToRemove));
            bounds.removeFromBottom(static_cast<int>(0.5f * heightToRemove));
        }
    }

    return bounds;
}

/**
 * Helper method to prepare speaker position drawables (pos+rot) as well
 * as mapping area paths to later be drawn in paint method
 */
void MultiSoundobjectSlider::PrerenderSpeakerAndMappingAreaInBounds()
{
    // Speaker positions
    for (auto i = 1; i <= DS100_CHANNELCOUNT; i++)
    {
        auto channelId = static_cast<ChannelId>(i);
        if (m_speakerDrawables.count(channelId) == 1 && m_speakerPositions.count(channelId) == 1 && m_speakerPositions.at(channelId).first.length() != 0.0f)
        {
            auto& p = m_speakerPositions.at(channelId).first;
            auto speakerCenterPoint = GetPointForRealCoordinate(p);
            auto speakerArea = juce::Rectangle<float>(
                speakerCenterPoint.getX() - 9.0f,
                speakerCenterPoint.getY() - 9.0f,
                18.0f,
                18.0f);
            m_speakerDrawableAreas[channelId] = speakerArea;
        }
    }

    // Mapping Areas
    for (auto i = int(MAI_First); i <= int(MAI_Fourth); i++)
    {
        auto mappingAreaId = static_cast<MappingAreaId>(i);
        m_mappingAreaPaths[mappingAreaId].clear();
        if (m_mappingCornersReal.count(mappingAreaId) == 1)
        {
            auto& p0 = m_mappingCornersReal.at(mappingAreaId).at(2);
            auto& p1 = m_mappingCornersReal.at(mappingAreaId).at(3);
            auto& p2 = m_mappingCornersReal.at(mappingAreaId).at(0);
            m_mappingTextAnchorPointAndRot[mappingAreaId].first = GetPointForRealCoordinate(p0).toInt();
            m_mappingTextAnchorPointAndRot[mappingAreaId].second = juce::Line<float>(p0.x, p0.y, p1.x, p1.y).getAngle();
            auto prevPoint = GetPointForRealCoordinate(m_mappingCornersReal.at(mappingAreaId).at(3));
            for (auto j = 0; j < 4; j++) // p1 real - p4 real
            {
                if (m_mappingCornersReal.at(mappingAreaId).size() > j)
                {
                    auto& p = m_mappingCornersReal.at(mappingAreaId).at(j);
                    auto mappingAreaCornerPoint = GetPointForRealCoordinate(p);
                    m_mappingAreaPaths[mappingAreaId].addLineSegment(juce::Line<float>(prevPoint.getX(), prevPoint.getY(), mappingAreaCornerPoint.getX(), mappingAreaCornerPoint.getY()), 2.0f);
                    prevPoint = mappingAreaCornerPoint;
                }
            }

            m_mappingCornersVirtualPoints[mappingAreaId].resize(2);
            m_mappingCornersVirtualPoints[mappingAreaId][0] = GetPointForRealCoordinate(p2).toInt();
            m_mappingCornersVirtualPoints[mappingAreaId][1] = GetPointForRealCoordinate(p0).toInt();
        }
    }
}

/**
 * Helper to compute a given points coordinates in outside world (non-d&b) nomenclature (x horizontally and y vertically).
 * @param   coordinate  The value to recompute into outside world nomenclature.
 * @return  The computed value.
 */
const juce::Vector3D<float>	MultiSoundobjectSlider::ComputeNonDBRealPointCoordinate(const juce::Vector3D<float>& coordinate)
{
    return juce::Vector3D<float>(coordinate.y, coordinate.x, coordinate.z);
}

/**
 * Helper to compute a given points coordinates in outside world (non-d&b) nomenclature (x horizontally and y vertically).
 * @param   coordinate  The value to recompute into outside world nomenclature.
 * @return  The computed value.
 */
const juce::Vector3D<float>	MultiSoundobjectSlider::ComputeNonDBRealPointRotation(const juce::Vector3D<float>& rotation)
{
    return juce::Vector3D<float>((rotation.x - 90) * -1.0f, rotation.y, rotation.z);
}


} // namespace SpaConBridge
