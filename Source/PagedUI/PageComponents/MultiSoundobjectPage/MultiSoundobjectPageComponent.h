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

#include "../PageComponentBase.h"

#include "../../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * Class MultiSoundobjectPageComponent is just a component which contains the multi-source slider
 * and the mapping selection control.
 */
class MultiSoundobjectPageComponent :	public PageComponentBase
{
public:
	MultiSoundobjectPageComponent();
	~MultiSoundobjectPageComponent() override;

	//==============================================================================
	void UpdateGui(bool init) override;

protected:
	//==============================================================================
	void resized() override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSoundobjectPageComponent)
};


} // namespace SpaConBridge
