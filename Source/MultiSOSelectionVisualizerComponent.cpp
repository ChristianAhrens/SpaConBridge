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


#include "MultiSOSelectionVisualizerComponent.h"


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
    m_handleSize = 35.0f;

    m_cog_svg_xml = XmlDocument::parse(BinaryData::translate24dp_svg);
    m_secHndl_svg_xml = XmlDocument::parse(BinaryData::croprotate24dp_svg);

    lookAndFeelChanged();
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
 * Getter for the list of points currently known in the muselvisu.
 * @return  The list of known points.
 */
const std::vector<juce::Point<float>>& MultiSOSelectionVisualizerComponent::GetSelectionPoints()
{
    return m_selectionPoints;
}

/**
 * Setter for the list of points that are selected and shall be used as base for multiselection visu.
 * This DOES process the COG and secHndl from the input points.
 * @param   points     The list of points to copy into internal member list.
 */
void MultiSOSelectionVisualizerComponent::SetSelectionPoints(const std::vector<juce::Point<float>>& points)
{
    if (points.size() > 1)
    {
        // Iterate over new points to calculate COG
        auto sum = juce::Point<float>(0.0f, 0.0f);
        for (auto const& coord : points)
            sum += coord;

        // calculate COG
        m_startCOG = sum / points.size(); // zerodivision is prevented in condition above
        m_currentVirtCOG = m_startCOG;

        // Iterate over new points to collect angles that can be used for sorting counterclockwise
        std::vector<std::pair<juce::Point<float>, float>> pointsToAngles;
        for (auto const& coord : points)
            pointsToAngles.push_back(std::make_pair(coord, m_startCOG.getAngleToPoint(coord)));

        // sort points based on their angles around COG
        std::sort(pointsToAngles.begin(), pointsToAngles.end(), [](const std::pair<juce::Point<float>, float>& a, const std::pair<juce::Point<float>, float>& b) { return a.second < b.second; });
        m_selectionPoints.clear();
        m_selectionPoints.reserve(pointsToAngles.size());
        for (auto const& pointToAngle : pointsToAngles)
            m_selectionPoints.push_back(pointToAngle.first);

        // calculate secondary handle position
        m_startSecondaryHandle = DeriveSecondaryHandleFromCOG(m_startCOG);
        m_currentVirtSecondaryHandle = m_startSecondaryHandle;
    }
}

/**
 * Updater for the list of points that are selected and shall be used as base for multiselection visu.
 * This does NOT process the COG and secHndl from the input points.
 * @param   points     The list of points to copy into internal member list.
 */
void MultiSOSelectionVisualizerComponent::UpdateSelectionPoints(const std::vector<juce::Point<float>>& points)
{
    if (points.size() > 1)
    {
        // Iterate over new points to collect angles that can be used for sorting counterclockwise
        std::vector<std::pair<juce::Point<float>, float>> pointsToAngles;
        for (auto const& coord : points)
            pointsToAngles.push_back(std::make_pair(coord, m_startCOG.getAngleToPoint(coord)));

        // sort points based on their angles around COG
        std::sort(pointsToAngles.begin(), pointsToAngles.end(), [](const std::pair<juce::Point<float>, float>& a, const std::pair<juce::Point<float>, float>& b) { return a.second < b.second; });
        m_selectionPoints.clear();
        m_selectionPoints.reserve(pointsToAngles.size());
        for (auto const& pointToAngle : pointsToAngles)
            m_selectionPoints.push_back(pointToAngle.first);
    }
}

/**
 * Reimplemented from component to update button drawables correctly
 */
void MultiSOSelectionVisualizerComponent::lookAndFeelChanged()
{
    Component::lookAndFeelChanged();

    m_multitselectionIndicationColour = getLookAndFeel().findColour(TextButton::textColourOnId);

    m_cog_drawable = Drawable::createFromSVG(*(m_cog_svg_xml.get()));
    m_cog_drawable->replaceColour(Colours::black, m_multitselectionIndicationColour);

    m_secHndl_drawable = Drawable::createFromSVG(*(m_secHndl_svg_xml.get()));
    m_secHndl_drawable->replaceColour(Colours::black, m_multitselectionIndicationColour);
}

/**
 * Helper to calculate the secondary handle position from internal known selectionpoints and given cog
 * @param   cog     The center of gravity position to use for calculation
 * @return  The calculated secondary handle position.
 */
const juce::Point<float> MultiSOSelectionVisualizerComponent::DeriveSecondaryHandleFromCOG(const juce::Point<float>& cog)
{
    auto secHndlToCogHOffset = 0.0f;

    auto w = getLocalBounds().getWidth();
    auto cogIsOffsetRight = (cog.getX() > (0.5f * w));

    if (m_selectionPoints.size() > 1)
    {
        auto avgRadius = 0.0f;
        for (auto const& p : m_selectionPoints)
            avgRadius += cog.getDistanceFrom(p);
        avgRadius = avgRadius / m_selectionPoints.size();

        secHndlToCogHOffset = cogIsOffsetRight ? -2.0f * avgRadius : 2.0f * avgRadius;
    }
    else
    {
        // why would we only have one selection point in this muselvisu instance and have to default to this calculation?!
        jassertfalse;

        // if no sufficient amount of points is available, use
        // area width to derive a feasible position from
        secHndlToCogHOffset = cogIsOffsetRight ? -(0.5f * cog.getX()) : (0.5f * (w - cog.getX()));
    }

    return cog.translated(secHndlToCogHOffset, 0.0f);
}

/**
 * Getter for the visu/musel active flag
 * @return  The active flag value (True if active, false if not)
 */
bool MultiSOSelectionVisualizerComponent::IsSelectionVisuActive()
{
    return m_selectionVisuActive;
}

/**
 * Getter for the 'currently interacted with' internal boolean state.
 * @return  True if currently interacted with, false if not
 */
bool MultiSOSelectionVisualizerComponent::IsPrimaryInteractionActive()
{
    return m_currentlyPrimaryInteractedWith;
}

/**
 * Getter for the 'currently interacted with' internal boolean state.
 * @return  True if currently interacted with, false if not
 */
bool MultiSOSelectionVisualizerComponent::IsSecondaryInteractionActive()
{
    return m_currentlySecondaryInteractedWith;
}

/**
 * Reimplemented paint method to perform the actual visualization drawing
 * @param   g   The graphics object to use for painting.
 */
void MultiSOSelectionVisualizerComponent::paint(Graphics& g)
{
    Component::paint(g);

    // Paint the multiselection indication elements
    if (m_selectionVisuActive && m_selectionPoints.size() > 1)
    {
        g.setColour(m_multitselectionIndicationColour);

        auto prevCoord = m_selectionPoints.back();
        for (auto const& coord : m_selectionPoints)
        {
            g.drawLine(prevCoord.getX(), prevCoord.getY(), coord.getX(), coord.getY(), 2.0f);
            prevCoord = coord;
        }

        g.drawLine(m_currentVirtCOG.getX(), m_currentVirtCOG.getY(), m_currentVirtSecondaryHandle.getX(), m_currentVirtSecondaryHandle.getY(), 2.0f);
        m_cog_drawable->drawWithin(g, juce::Rectangle<float>(m_currentVirtCOG.getX() - (m_handleSize / 2.0f), m_currentVirtCOG.getY() - (m_handleSize / 2.0f), m_handleSize, m_handleSize), juce::RectanglePlacement::fillDestination, 1.0f);
        m_secHndl_drawable->drawWithin(g, juce::Rectangle<float>(m_currentVirtSecondaryHandle.getX() - (m_handleSize / 2.0f), m_currentVirtSecondaryHandle.getY() - (m_handleSize / 2.0f), m_handleSize, m_handleSize), juce::RectanglePlacement::fillDestination, 1.0f);

        if (IsPrimaryInteractionActive())
        {
            auto w = static_cast<float>(getLocalBounds().getWidth());
            auto h = static_cast<float>(getLocalBounds().getHeight());
            g.drawLine(0, m_currentVirtCOG.getY(), w, m_currentVirtCOG.getY(), 1);
            g.drawLine(m_currentVirtCOG.getX(), 0, m_currentVirtCOG.getX(), h, 1);
        }
    }

}

/**
 * Reimplemented mouse event handling to forward the event to parent component
 * in order to not block any user interaction from handling in parent component.
 * @param   e   The event that occurd and is forwarded to parent component.
 */
void MultiSOSelectionVisualizerComponent::mouseDown(const MouseEvent& e)
{
    // no multitouch support, so only primary mouse clicks are handled
    if (0 == e.source.getIndex() && IsSelectionVisuActive())
    {
        auto mousePosF = e.getMouseDownPosition().toFloat();

        Path cogPath;
        cogPath.addEllipse(juce::Rectangle<float>(m_startCOG.x - (m_handleSize / 2.0f), m_startCOG.y - (m_handleSize / 2.0f), m_handleSize, m_handleSize));
        auto startPrimInteraction = cogPath.contains(mousePosF);

        Path secHndlPath;
        secHndlPath.addEllipse(juce::Rectangle<float>(m_startSecondaryHandle.getX() - (m_handleSize / 2.0f), m_startSecondaryHandle.getY() - (m_handleSize / 2.0f), m_handleSize, m_handleSize));
        auto startSecInteraction = secHndlPath.contains(mousePosF);

        // Check if the mouse click landed inside any of the knobs.
        if (startPrimInteraction || startSecInteraction)
        {
            if (startPrimInteraction)
            {
                jassert(!IsSecondaryInteractionActive());
                m_currentlyPrimaryInteractedWith = true;
            }
            else if (startSecInteraction)
            {
                jassert(!IsPrimaryInteractionActive());
                m_currentlySecondaryInteractedWith = true;
            }

            if (onMouseInteractionStarted)
                onMouseInteractionStarted();

            // trigger repaint to show the crosshair visu
            repaint();

            // do not continue to foward mouseDown to parent
            return;
        }
    }

    getParentComponent()->mouseDown(e);
}

/**
 * Reimplemented mouse event handling to forward the event to parent component
 * in order to not block any user interaction from handling in parent component.
 * @param   e   The event that occurd and is forwarded to parent component.
 */
void MultiSOSelectionVisualizerComponent::mouseDrag(const MouseEvent& e)
{
    // no multitouch support, so only primary mouse clicks are handled
    if (0 == e.source.getIndex() && (IsPrimaryInteractionActive() || IsSecondaryInteractionActive()))
    {
        auto dragDelta = juce::Point<int>(e.getDistanceFromDragStartX(), e.getDistanceFromDragStartY());

        if (IsPrimaryInteractionActive())
        {
            if (onMouseXYPosChanged)
                onMouseXYPosChanged(dragDelta);

            m_currentVirtCOG = e.getPosition().toFloat();
            // inplicitly changed second handle needs recalc
            if (m_selectionPoints.size() > 1)
            {
                m_currentVirtSecondaryHandle = DeriveSecondaryHandleFromCOG(m_currentVirtCOG);
            }
        }
        else if (IsSecondaryInteractionActive())
        {
            auto rotDelta = 0.0f;
            auto scaleDelta = 0.0f;

            auto dist1 = m_startSecondaryHandle.getDistanceFrom(m_startCOG);
            auto dist2 = e.getPosition().toFloat().getDistanceFrom(m_startCOG);
            if (dist1 != 0.0f)
                scaleDelta = dist2 / dist1;

            auto angl1 = m_startCOG.getAngleToPoint(m_startSecondaryHandle);
            auto angl2 = m_startCOG.getAngleToPoint(e.getPosition().toFloat());
            rotDelta = -(angl2 - angl1);

            if (onMouseRotAndScaleChanged)
                onMouseRotAndScaleChanged(m_startCOG, rotDelta, scaleDelta);

            m_currentVirtSecondaryHandle = e.getPosition().toFloat();
            // implicitly changed cog need recalc
            if (m_selectionPoints.size() > 1)
            {
                auto sum = juce::Point<float>(0.0f, 0.0f);
                for (auto const& coord : m_selectionPoints)
                    sum += coord;
            
                m_currentVirtCOG = sum / m_selectionPoints.size(); // zerodivision is prevented in condition above
            }
        }

        // trigger repaint to show the crosshair visu
        repaint();

        // do not continue to foward mouseDown to parent
        return;
    }

    getParentComponent()->mouseDrag(e);
}

/**
 * Reimplemented mouse event handling to forward the event to parent component
 * in order to not block any user interaction from handling in parent component.
 * @param   e   The event that occurd and is forwarded to parent component.
 */
void MultiSOSelectionVisualizerComponent::mouseUp(const MouseEvent& e)
{
    // no multitouch support, so only primary mouse clicks are handled
    if (0 == e.source.getIndex() && (IsPrimaryInteractionActive() || IsSecondaryInteractionActive()))
    {
        auto dragDelta = juce::Point<int>(e.getDistanceFromDragStartX(), e.getDistanceFromDragStartY());

        if (IsPrimaryInteractionActive())
        {
            m_currentlyPrimaryInteractedWith = false;

            if (onMouseXYPosFinished)
                onMouseXYPosFinished(dragDelta);

            m_currentVirtCOG = e.getPosition().toFloat();
            m_startCOG = m_currentVirtCOG;
            // inplicitly changed second handle needs recalc
            if (m_selectionPoints.size() > 1)
            {
                m_startSecondaryHandle = DeriveSecondaryHandleFromCOG(m_currentVirtCOG);
                m_startSecondaryHandle = m_currentVirtSecondaryHandle;
            }
        }
        else if (IsSecondaryInteractionActive())
        {
            m_currentlySecondaryInteractedWith = false;
            
            auto rotDelta = 0.0f;
            auto scaleDelta = 0.0f;

            auto dist1 = m_startSecondaryHandle.getDistanceFrom(m_startCOG);
            auto dist2 = e.getPosition().toFloat().getDistanceFrom(m_startCOG);
            if (dist1 != 0.0f)
                scaleDelta = dist2 / dist1;

            auto angl1 = m_startCOG.getAngleToPoint(m_startSecondaryHandle);
            auto angl2 = m_startCOG.getAngleToPoint(e.getPosition().toFloat());
            rotDelta = -(angl2 - angl1);

            if (onMouseRotAndScaleFinished)
                onMouseRotAndScaleFinished(m_startCOG, rotDelta, scaleDelta);

            m_currentVirtSecondaryHandle = e.getPosition().toFloat();
            m_startSecondaryHandle = m_currentVirtSecondaryHandle;
            // implicitly changed cog need recalc
            if (m_selectionPoints.size() > 1)
            {
                auto sum = juce::Point<float>(0.0f, 0.0f);
                for (auto const& coord : m_selectionPoints)
                    sum += coord;

                m_currentVirtCOG = sum / m_selectionPoints.size(); // zerodivision is prevented in condition above
                m_startCOG = m_currentVirtCOG;
            }
        }

        // trigger repaint to show the crosshair visu
        repaint();

        // do not continue to foward mouseDown to parent
        return;
    }

    getParentComponent()->mouseUp(e);
}


} // namespace SpaConBridge
