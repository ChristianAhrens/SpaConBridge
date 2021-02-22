/*
  ==============================================================================

    MatrixIOPageComponent.h
    Created: 22 Feb 2021 4:21:34pm
    Author:  Christian Ahrens

  ==============================================================================
*/
/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
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

#include "PageComponentBase.h"


namespace SoundscapeBridgeApp
{


/**
 * Class MatrixIOPageComponent is a component that contains
 * matrix inputs and outputs processor elements
 */
class MatrixIOPageComponent : public PageComponentBase
{
public:
	MatrixIOPageComponent();
	~MatrixIOPageComponent() override;

	//==============================================================================
	void UpdateGui(bool init) override;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixIOPageComponent)
};


} // namespace SoundscapeBridgeApp
