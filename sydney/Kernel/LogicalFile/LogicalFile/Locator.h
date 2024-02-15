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

#ifndef __SYDNEY_LOGICALFILE_LOCATOR_H
#define __SYDNEY_LOGICALFILE_LOCATOR_H

#include "LogicalFile/Module.h"

#include "Common/ExecutableObject.h"
#include "Common/Data.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Common
{
class UnsignedIntegerData;
}

namespace Trans
{
class Transaction;
}

_SYDNEY_LOGICALFILE_BEGIN

//
//	CLASS
//	LogicalFile::Locator -- ロケータの基底クラス
//
//	NOTES
//
class SYD_LOGICALFILE_FUNCTION Locator : public Common::ExecutableObject
{
public:
	// コンストラクタ
	Locator(const Trans::Transaction& cTransaction_);
	// デストラクタ
	virtual ~Locator();

	// 指定範囲のデータを取り出す
	virtual bool
		get(const Common::UnsignedIntegerData* pPosition_,
			const Common::UnsignedIntegerData* pLength_,
			Common::Data* pResult_) = 0;
	// 指定範囲のデータを変更する
	virtual void replace(const Common::UnsignedIntegerData* pPosition_,
						 const Common::Data* pData_) = 0;
	// データを末尾に追加する
	virtual void append(const Common::Data* pData_) = 0;
	// データを末尾から指定サイズ分切り詰める
	virtual void truncate(const Common::UnsignedIntegerData* pPosition_) = 0;

	// データ長を得る
	virtual void length(Common::UnsignedIntegerData* pResult_) = 0;

	// 無効かどうかをチェックする
	virtual bool isInvalid() const;

protected:
	// トランザクション記述子を得る
	const Trans::Transaction& getTransaction() const;

private:
	// トランザクション記述子
	Trans::Transaction* m_pTransaction;
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_LOCATOR_H

//
//	Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
