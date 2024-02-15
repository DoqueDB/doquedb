// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::QualifiedJoin -- QUALIFIED JOIN/NATURAL JOIN
// 
// Copyright (c) 2004, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_QUALIFIEDJOIN_H
#define __SYDNEY_STATEMENT_QUALIFIEDJOIN_H

#include "Statement/Object.h"
#include "Statement/JoinType.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

//
//	CLASS
//		QualifiedJoin --
//
//	NOTES
//
class SYD_STATEMENT_FUNCTION QualifiedJoin : public Statement::Object
{
public:
	// コンストラクタ (1)
	QualifiedJoin();
	// コンストラクタ (2)
	QualifiedJoin(JoinType::Value eType_, Object* pLeft_, Object* pRight_, Object* pJoinSpec_);
	// コピーコンストラクタ
	QualifiedJoin(const QualifiedJoin& cOther_);

	// JoinTypeを得る
	JoinType::Value getJoinType() const {return m_eType;}
	// JoinTypeを設定する
	void setJoinType(JoinType::Value eType_) {m_eType = eType_;}

	// Left を得る
	Object* getLeft() const;
	// Left を設定する
	void setLeft(Object* pLeft_);

	// Right を得る
	Object* getRight() const;
	// Right を設定する
	void setRight(Object* pRight_);

	// JoinSpec を得る
	Object* getJoinSpec() const;
	// JoinSpec を設定する
	void setJoinSpec(Object* pJoinSpec_);

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

private:
	// 代入オペレーターは使わない
	QualifiedJoin& operator=(const QualifiedJoin& cOther_);

	JoinType::Value m_eType;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_QUALIFIEDJOIN_H

//
// Copyright (c) 2004, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
