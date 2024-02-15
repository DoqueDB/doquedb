// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Hint.h --
// 
// Copyright (c) 2000, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SCHEMA_HINT_H
#define __SYDNEY_SCHEMA_HINT_H

#include "Common/Object.h"

#include "Schema/Module.h"
#include "Schema/Externalizable.h"
#include "Schema/TupleID.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common {
	class Data;
}

namespace Statement {
	class Object;
	class Hint;
	class Literal;
}

_SYDNEY_SCHEMA_BEGIN

//
//	CLASS
//	Schema::Hint --
//
//	NOTES
//
//
class Hint
	: public	Common::Object,
	  public	Externalizable
{
public:

	struct Category {

		//	ENUM
		//	Schema::Hint::Category::BitMap -- ヒントの種別を表すビットマップ
		//
		//	NOTES

		enum BitMap {
			Unknown =		0x0000,
			LogicalFile =	0x0001,				// 論理ファイルに素通し
			Heap =			0x0002,				// ヒープファイル使用の指示
			PartialImport =	0x0004,				// 索引への部分的な反映
			NonTruncate =	0x0008,				// 可変長で末尾の空白をとらない指示
			Unique =		0x0010,				// Unique制約
			Mask =			0x00ff
		};

		//	TYPEDEF
		//	Schema::Hint::Category::Value -- ヒントの種別を表す値
		//
		//	NOTES

		typedef int Value;
	};

	Hint();
	explicit Hint(const Statement::Hint& cStatement_);
	explicit Hint(const Statement::Literal& cStatement_);
	explicit Hint(const Hint& cHint_);
												// コンストラクタ
	virtual ~Hint();							// デストラクタ

	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const; // 種別を得る

	SYD_SCHEMA_FUNCTION
	const Common::Data*		getElement() const;	// ヒントの要素を得る

	SYD_SCHEMA_FUNCTION
	ModUnicodeString		getString() const;	// ヒントを文字列で得る

	ModUnicodeString		getWholeString() const;	// get all the hints

#ifndef SYD_COVERAGE // 索引を部分的に作成する非公開機能なのでカバレージの対象としない
	TupleID::Value			getLowerBound() const {return m_uLowerBound;}
	TupleID::Value			getUpperBound() const {return m_uUpperBound;}
#endif

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

protected:
private:
	void					setElement(Common::Data* cElement_);
												// ヒントの要素を設定する

	static Common::Data::Pointer
							makeElementData(const Statement::Object& cObject_,
											Category::Value* pCategory_,
											TupleID::Value* pLowerBound_,
											TupleID::Value* pUpperBound_,
											bool& bLower_);
												// ヒントを表すSQL構文要素を
												// データに変換する

	void					destruct();			// デストラクターの下位関数

	Category::Value			m_iCategory;		// 種別
	Common::Data::Pointer	m_pElement;			// ヒントを表すデータ
	TupleID::Value			m_uLowerBound;
	TupleID::Value			m_uUpperBound;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif //__SYDNEY_SCHEMA_HINT_H

//
//	Copyright (c) 2000, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
