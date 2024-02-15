// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Locator.h -- 
// 
// Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOB_LOCATOR_H
#define __SYDNEY_LOB_LOCATOR_H

#include "Lob/Module.h"
#include "Lob/ObjectID.h"

#include "LogicalFile/Locator.h"
#include "Trans/Transaction.h"
#include "Common/Data.h"
#include "Common/UnsignedIntegerData.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

class LogicalInterface;

//
//	TYPEDEF
//	Lob::LogicalFileLocator --
//
//	NOTES
//	Lob::Locatorが直接LogicalFile::Locatorを継承できないので、
//	このtypedefを間に挟む。VC6のバグ。
//
typedef LogicalFile::Locator LogicalFileLocator;

//
//	CLASS
//	Lob::Locator -- ロケータ
//
//	NOTES
//
class SYD_LOB_FUNCTION Locator : public LogicalFileLocator
{
public:
	// コンストラクタ
	Locator(const Trans::Transaction& cTransaction_,
			const ObjectID& cObjectID_,
			LogicalInterface* pFile_);
	// デストラクタ
	virtual ~Locator();

	// 指定範囲のデータを取り出す
	bool get(const Common::UnsignedIntegerData* pPosition_,
			 const Common::UnsignedIntegerData* pLength_,
			 Common::Data* pResult_);
	// 指定範囲のデータを変更する
	void replace(const Common::UnsignedIntegerData* pPosition_,
				 const Common::Data* pData_);
	// データを末尾に追加する
	void append(const Common::Data* pData_);
	// データを末尾から指定サイズ分切り詰める
	void truncate(const Common::UnsignedIntegerData* pLength_);

	// サイズを得る
	void length(Common::UnsignedIntegerData* pResult_);

private:
	// オブジェクトID
	ObjectID m_cObjectID;
	// ファイル
	LogicalInterface* m_pFile;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_LOCATOR_H

//
//	Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
