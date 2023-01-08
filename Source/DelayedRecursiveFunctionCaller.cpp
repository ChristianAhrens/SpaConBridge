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


#include "DelayedRecursiveFunctionCaller.h"
#include "WaitingEntertainerComponent.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class DelayedRecursiveFunctionCaller
===============================================================================
*/

/**
 * Class constructor.
 * @param	voidFunction	The function to be called back when executing the recursion.
 * @param	recursionCount	The count of iterations when running the recursion.
 * @param	selfDestroy		Flag that defines if the object instance shall destroy itself once the timer driven recursion is completed.
 * @param	callbackDelayMs	Delay inbetween recursive calls (via message queue timer)
 */
DelayedRecursiveFunctionCaller::DelayedRecursiveFunctionCaller(std::function<void()> voidFunction, int recursionCount, bool selfDestroy, int callbackDelayMs)
	: m_voidFunction(voidFunction),
	  m_recursionCount(recursionCount),
	  m_recursionCounter(0),
	  m_callbackDelayMs(callbackDelayMs),
	  m_selfDestroy(selfDestroy)
{
}

/**
 * Class constructor.
 * @param	intFunction				The function to be called back when executing the recursion.
 * @param	intRecursionContainer	The container that holds the int parameters to be passed in each recursion iteration. Also defines the count of iterations through its size.
 * @param	selfDestroy				Flag that defines if the object instance shall destroy itself once the timer driven recursion is completed.
 * @param	callbackDelayMs			Delay inbetween recursive calls (via message queue timer)
 */
DelayedRecursiveFunctionCaller::DelayedRecursiveFunctionCaller(std::function<void(int)> intFunction, const std::vector<int>& intRecursionContainer, bool selfDestroy, int callbackDelayMs)
	: m_intFunction(intFunction),
      m_recursionCounter(0),
	  m_intRecursionContainer(intRecursionContainer),
	  m_callbackDelayMs(callbackDelayMs),
	  m_selfDestroy(selfDestroy)
{
    m_recursionCount = static_cast<int>(m_intRecursionContainer.size());
}

/**
 * Class destructor.
 */
DelayedRecursiveFunctionCaller::~DelayedRecursiveFunctionCaller()
{
	if (m_finalVoidFunction)
		m_finalVoidFunction();
}

/**
 * Method to start the recursion iteration.
 * It forwards the call to either container or count based internal run methods.
 */
void DelayedRecursiveFunctionCaller::Run()
{
	if (!m_intRecursionContainer.empty())
		RunRecursiveFunctionCallsContainerSize();
	else if (m_recursionCount > 0)
		RunRecursiveFunctionCallsCount();
	else
		jassertfalse;
}

/**
 * Internal method to start the recursion iteration based on count member.
 */
void DelayedRecursiveFunctionCaller::RunRecursiveFunctionCallsCount()
{
	WaitingEntertainerComponent::GetInstance()->Show();

	Timer::callAfterDelay(m_callbackDelayMs, std::bind(&DelayedRecursiveFunctionCaller::ExecuteVoidFunctionCallbackWithCount, this));
}

/**
 * Method to be called recursively from within itself to execute the void function callback member 
 * while pausing message queue usage for a defined amount of time inbetween and displaying a WaitingEntertainerComponent.
 */
void DelayedRecursiveFunctionCaller::ExecuteVoidFunctionCallbackWithCount()
{
	jassert(m_voidFunction);
	if (m_voidFunction)
		m_voidFunction();

	if (m_recursionCount > 0)
		WaitingEntertainerComponent::GetInstance()->SetNormalizedProgress(static_cast<double>((m_recursionCounter + 1) / static_cast<double>(m_recursionCount)));

	m_recursionCounter++;

	if (m_recursionCounter < m_recursionCount)
		Timer::callAfterDelay(m_callbackDelayMs, std::bind(&DelayedRecursiveFunctionCaller::ExecuteVoidFunctionCallbackWithCount, this));
	else
	{
		WaitingEntertainerComponent::GetInstance()->Hide();

		if (m_selfDestroy)
			std::unique_ptr<DelayedRecursiveFunctionCaller>(this);
	}
}

/**
 * Internal method to start the recursion iteration based on int parameter container member.
 */
void DelayedRecursiveFunctionCaller::RunRecursiveFunctionCallsContainerSize()
{
	WaitingEntertainerComponent::GetInstance()->Show();

	Timer::callAfterDelay(m_callbackDelayMs, std::bind(&DelayedRecursiveFunctionCaller::ExecuteIntFunctionCallbackWithContainerSize, this));
}

/**
 * Method to be called recursively from within itself to execute the void function callback member
 * while pausing message queue usage for a defined amount of time inbetween and displaying a WaitingEntertainerComponent.
 */
void DelayedRecursiveFunctionCaller::ExecuteIntFunctionCallbackWithContainerSize()
{
	jassert(m_intFunction);
	if (m_intFunction && m_intRecursionContainer.size() > m_recursionCounter)
		m_intFunction(m_intRecursionContainer.at(m_recursionCounter));

	if (m_recursionCount > 0)
		WaitingEntertainerComponent::GetInstance()->SetNormalizedProgress(static_cast<double>((m_recursionCounter + 1) / static_cast<double>(m_recursionCount)));

	m_recursionCounter++;

	if (m_recursionCounter < m_recursionCount)
		Timer::callAfterDelay(m_callbackDelayMs, std::bind(&DelayedRecursiveFunctionCaller::ExecuteIntFunctionCallbackWithContainerSize, this));
	else
	{
		WaitingEntertainerComponent::GetInstance()->Hide();

		if (m_selfDestroy)
			std::unique_ptr<DelayedRecursiveFunctionCaller>(this);
	}
}

/**
 * Setter for the optional function callback member that is supposed
 * to be called when destroying this caller object.
 * @param	voidFunction	The void function to be finally called when destroying this object.
 */
void DelayedRecursiveFunctionCaller::SetFinalFunctionCall(std::function<void()> voidFunction)
{
	m_finalVoidFunction = voidFunction;
}


} // namespace SpaConBridge
