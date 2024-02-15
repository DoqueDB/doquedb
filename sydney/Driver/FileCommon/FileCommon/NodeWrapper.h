// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NodeWrapper.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef __SYDNEY_FILECOMMON_NODEWRAPPER_H
#define __SYDNEY_FILECOMMON_NODEWRAPPER_H

#include "Common/Common.h"
#include "LogicalFile/TreeNodeInterface.h"

_SYDNEY_BEGIN

namespace FileCommon
{

	//
	//	CLASS
	//	ListNodeWrapper -- 
	//
	//	NOTES
	//	TreeNodeInterface の List と、そうではないものを List のように
	//	扱えるようにした Wrapper クラス
	//
	class ListNodeWrapper
	{
	public:
		ListNodeWrapper(const LogicalFile::TreeNodeInterface* pNode_)
			: m_pNode(pNode_) {}

		int getSize() {
			if (m_pNode->getType() == LogicalFile::TreeNodeInterface::List)
				return static_cast<int>(m_pNode->getOperandSize());
			return 1;
		}

		const LogicalFile::TreeNodeInterface* get(int i) {
			if (m_pNode->getType() == LogicalFile::TreeNodeInterface::List)
				return m_pNode->getOperandAt(i);
			return m_pNode;
		}

	private:
		const LogicalFile::TreeNodeInterface* m_pNode;
	};

} // end of namespace FileCommon

_SYDNEY_END

#endif // __SYDNEY_FILECOMMON_NODEWRAPPER_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
