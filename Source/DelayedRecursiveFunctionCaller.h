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

#include "LookAndFeel.h"
#include "SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * Helper class to be used to have a given void or int parameter function called back recursively with a given
 * message queue delay time inbetween. This is useful to have the UI being updated in an otherwise blocking recursive call
 * on main thread.
 */
class DelayedRecursiveFunctionCaller
{
public:
	DelayedRecursiveFunctionCaller(std::function<void()> voidFunction, int recursionCount, bool selfDestroy = true, int callbackDelayMs = 200);
	DelayedRecursiveFunctionCaller(std::function<void(int)> intFunction, std::vector<int> intRecursionContainer, bool selfDestroy = true, int callbackDelayMs = 200);
	~DelayedRecursiveFunctionCaller();

	void Run();

	void SetFinalFunctionCall(std::function<void()> voidFunction);

private:
	void RunRecursiveFunctionCallsCount();
	void RunRecursiveFunctionCallsContainerSize();

	void ExecuteVoidFunctionCallbackWithCount();
	void ExecuteIntFunctionCallbackWithContainerSize();

	std::function<void()>		m_voidFunction;
	std::function<void(int)>	m_intFunction;
	std::function<void()>		m_finalVoidFunction;
	int							m_recursionCount{ -1 };
	int							m_recursionCounter{ -1 };
	std::vector<int>			m_intRecursionContainer;
	int							m_callbackDelayMs{ 0 };
	bool						m_selfDestroy{ false };

};


} // namespace SpaConBridge
