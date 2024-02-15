// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryNodeOperator.h
// 
// Copyright (c) 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_QUERYNODEOPERATOR_H
#define __SYDNEY_BITMAP_QUERYNODEOPERATOR_H

#include "Bitmap/Module.h"
#include "Bitmap/QueryNode.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	CLASS
//	Bitmap::QueryNodeOperator -- ANDとORの共通基底クラス
//
//	NOTES
//
class QueryNodeOperator : public QueryNode
{
public:
	// コンストラクタ
	QueryNodeOperator(BitmapFile& cFile_, Type::Value eType,
					  const ModUnicodeChar* p_, Condition& cCondition_);
	// デストラクタ
	virtual ~QueryNodeOperator();

	// 有効化する
	// This function is pulbic, but used only QueryNode*.
	void doValidate(const ModUnicodeChar*& p, Condition& cCondition_,
					ModUInt32& uiIteratorCount_, bool bVerify_);

	// 指定位置に移動する
	void seek(ModSize offset_);
	
protected:
	// ファイル
	BitmapFile& m_cFile;
	// ノード種別
	ModVector<QueryNode*> children;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_QUERYNODEOPERATOR_H

//
//	Copyright (c) 2005, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
