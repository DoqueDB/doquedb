// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::ExistsJoin -- EXISTS JOIN/NATURAL JOIN
// 
// Copyright (c) 2004, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_EXISTSJOIN_H
#define __SYDNEY_STATEMENT_EXISTSJOIN_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//		ExistsJoin --
//
//	NOTES
//
class SYD_STATEMENT_FUNCTION ExistsJoin : public Statement::Object
{
public:
	typedef Object Super;

	// コンストラクタ (1)
	ExistsJoin();
	// コンストラクタ (2)
	ExistsJoin(Object* pLeft_, Object* pRight_, Object* pCondition_, bool bFlag_);
	// コピーコンストラクタ
	ExistsJoin(const ExistsJoin& cOther_);

	// Left を得る
	Object* getLeft() const;
	// Left を設定する
	void setLeft(Object* pLeft_);

	// Right を得る
	Object* getRight() const;
	// Right を設定する
	void setRight(Object* pRight_);

	// Condition を得る
	Object* getCondition() const;
	// Condition を設定する
	void setCondition(Object* pCondition_);

	// exists/not existsを区別するフラグを得る
	bool getFlag() const {return m_bFlag;}
	// exists/not existsを区別するフラグを設定する
	void setFlag(bool bFlag_) {m_bFlag = bFlag_;}

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

protected:
#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
	// ハッシュコードを計算する
	virtual ModSize getHashCode();

	// 同じ型のオブジェクト同士でless比較する
	virtual bool compare(const Object& cObj_) const;
#endif

private:
	// 代入オペレーターは使わない
	ExistsJoin& operator=(const ExistsJoin& cOther_);

	bool m_bFlag;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_EXISTSJOIN_H

//
// Copyright (c) 2004, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
