/* Copyright (c) 2022, Christian Ahrens
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

#include "SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * Helper class to encapsulate all processor related selection handling.
 * This is a singleton class, so only one instance can exist at a time and can be accessed
 * from throughout the app by calling GetInstance().
 */
class ProcessorSelectionManager
{
public:
	ProcessorSelectionManager();
	~ProcessorSelectionManager();
	static ProcessorSelectionManager* GetInstance();
	void DestroyInstance();

	//==========================================================================
	void SetSelectedSoundobjectProcessorIds(const std::vector<SoundobjectProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<SoundobjectProcessorId> GetSelectedSoundobjectProcessorIds();
	void SetSoundobjectProcessorIdSelectState(SoundobjectProcessorId soundobjectProcessorId, bool selected);
	bool IsSoundobjectProcessorIdSelected(SoundobjectProcessorId soundobjectProcessorId);

	//==========================================================================
	void SetSelectedMatrixInputProcessorIds(const std::vector<MatrixInputProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<MatrixInputProcessorId> GetSelectedMatrixInputProcessorIds();
	void SetMatrixInputProcessorIdSelectState(MatrixInputProcessorId matrixInputProcessorId, bool selected);
	bool IsMatrixInputProcessorIdSelected(MatrixInputProcessorId matrixInputProcessorId);

	//==========================================================================
	void SetSelectedMatrixOutputProcessorIds(const std::vector<MatrixOutputProcessorId>& processorIds, bool clearPrevSelection);
	const std::vector<MatrixOutputProcessorId> GetSelectedMatrixOutputProcessorIds();
	void SetMatrixOutputProcessorIdSelectState(MatrixOutputProcessorId matrixOutputProcessorId, bool selected);
	bool IsMatrixOutputProcessorIdSelected(MatrixOutputProcessorId matrixOutputProcessorId);

protected:
	static std::unique_ptr<ProcessorSelectionManager>	s_singleton;				/**< The one and only instance of ProcessorSelectionManager. */

private:
	std::map<SoundobjectProcessorId, bool>	m_soundobjectProcessorSelection;		/**< The current select state of sound objects. */
	std::map<MatrixInputProcessorId, bool>	m_matrixInputProcessorSelection;		/**< The current select state of matrix inputs. */
	std::map<MatrixOutputProcessorId, bool>	m_matrixOutputProcessorSelection;		/**< The current select state of matrix outputs. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorSelectionManager)
};


} // namespace SpaConBridge
