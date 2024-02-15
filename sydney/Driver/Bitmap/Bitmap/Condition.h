// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.h --
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_CONDITION_H
#define __SYDNEY_BITMAP_CONDITION_H

#include "Bitmap/Module.h"
#include "Bitmap/Compare.h"
#include "Bitmap/Data.h"
#include "Bitmap/AutoPointer.h"
#include "Bitmap/FileID.h"
#include "Bitmap/OpenOption.h"
#include "Common/Object.h"

#include "ModVector.h"
#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class QueryNode;
class BitmapFile;

//
//	CLASS
//	Bitmap::Condition -- 条件作成クラス
//
//	NOTES
//
class Condition
{
public:
	//
	//	STRUCT
	//	Bitmap::Condition::LimitCond -- 条件をあらわすデータ
	//
	struct LimitCond
	{
		Data::MatchType::Value	m_eType;		// 一致条件
		AutoPointer<ModUInt32>	m_pBuffer;		// 検索条件
		Compare					m_cCompare;		// 比較クラス

		void clear()
		{
			m_pBuffer = 0;
			m_cCompare.setType(Data::Type::Unknown, false);
		}
	};

	//
	//	STRUCT
	//	Bitmap::Condition::Cond -- 条件をあらわすデータ
	//
	struct Cond
	{
		Data::MatchType::Value	m_eType;		// 一致条件
		AutoPointer<ModUInt32>	m_pBuffer;		// 検索条件
		// Optional character
		// For like, it's escape character.
		// For other than like, it's padding character.
		ModUnicodeChar			m_usOptionalChar;
	};

	// コンストラクタ
	Condition(const FileID& cFileID_);
	// デストラクタ
	virtual ~Condition();

	// 検索ノードを作成する
	QueryNode* createQueryNode(const ModUnicodeString& query_,
							   BitmapFile& cFile_,
							   bool bVerify_);

	// 下限条件を得る
	LimitCond& getLowerCondition() { return m_cLowerData; }
	// 上限条件を得る
	LimitCond& getUpperCondition() { return m_cUpperData; }

	// その他条件があるかどうか
	bool isOtherCondition() { return m_vecOtherCondition.getSize() != 0; }
	// その他条件にマッチしているかどうか
	bool isOtherConditionMatch(const ModUInt32* pBuffer_);

	// 有効な検索条件か
	bool isValid() const { return m_bValid; }
	
	// 文字列を設定する
	void setQueryString(const ModUnicodeChar*& p);

private:
	// 下限条件を設定する
	void setLowerData(const ModUnicodeChar*& p_);
	// 上限条件を設定する
	void setUpperData(const ModUnicodeChar*& p_);
	// その他条件を設定する
	void setOtherData(const ModUnicodeChar*& p_);

	// Common::Dataを作成する
	Common::Data::Pointer createCommonData(Data::Type::Value eType_,
										   ModUnicodeString& cstrValue_);

	// Cond構造体を作成する
	Cond makeCond(Data::MatchType::Value eType_,
				  const Common::Data& cData_,
				  ModUnicodeChar usOptionalChar_);

	// Get the key type.
	Data::Type::Value getKeyType(Data::Type::Value eType_,
								 ModUnicodeChar usOptionalChar_) const;

	// ファイルID
	const FileID& m_cFileID;
	// キータイプ
	Data::Type::Value m_eKeyType;

	// 下限条件
	LimitCond m_cLowerData;
	// 上限条件
	LimitCond m_cUpperData;

	// その他条件
	ModVector<Cond> m_vecOtherCondition;

	// 比較クラス
	Compare m_cCompare;
	// データクラス
	Data m_cData;

	// 有効な検索条件かどうか
	bool m_bValid;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_CONDITION_H

//
//	Copyright (c) 2005, 2006, 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
