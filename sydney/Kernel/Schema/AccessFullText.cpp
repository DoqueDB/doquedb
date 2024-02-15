// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessFullText.cpp -- 論理ログの反映に関連するクラスの定義
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyInclude.h"

#include "Schema/AccessFullText.h"
#include "Schema/TreeNode.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Common/UnicodeString.h"

#include "Exception/LogItemCorrupted.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"
#include "ModUnicodeChar.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION protected
//	Schema::AccessFullText::createVariable --
// 		定数に対応するTreeNodeInterfaceオブジェクトを作成する
//
//	NOTES
//		全文ファイルではNullDataは空文字列と同等の扱いをする
//		また常にListを作る
//
//	ARGUMENTS
//		const Common::Data::Pointer& pVariable_
//			変換するData
//
//	RETURN
//		定数を表すTreeNode
//
//	EXCEPTIONS

TreeNode::Base*
AccessFullText::
createVariable(const Common::Data::Pointer& pVariable_)
{
	if (pVariable_->isNull())
		// NullDataのときは空文字列を使ったノードにする
		return new TreeNode::Value(ModUnicodeString());

	switch (pVariable_->getType()) {
	case Common::DataType::Array:
	{
		if (pVariable_->getElementType() == Common::DataType::Data) {
			ModAutoPointer<TreeNode::List> pList = new TreeNode::List;
			const Common::DataArrayData& cData =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, *pVariable_);
			int n = cData.getCount();
			for (int i = 0; i < n; ++i) {
				pList->addNode(createVariable(cData.getElement(i)));
			}
			return pList.release();
		}
		// DataArrayData以外のArrayは想定していない
		; _SYDNEY_ASSERT(false);
	}
	break;
	default:
		break;
	}
	return new TreeNode::Variable(pVariable_);
}

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
