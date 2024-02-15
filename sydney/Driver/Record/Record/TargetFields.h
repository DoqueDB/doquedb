// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TargetFields.h -- 選択フィールドクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_TARGETFIELDS_H
#define __SYDNEY_RECORD_TARGETFIELDS_H

#include "Common/Common.h"
#include "Common/IntegerArrayData.h"
#include "Common/Object.h"

#include "Record/Module.h"
#include "Record/Tools.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_RECORD_BEGIN

class MetaData;

//
//	CLASS
//	Record::TargetFields -- 選択フィールドのクラス
//
//	NOTES
//	プロジェクションまたは更新の対象となるフィールドに関する情報のクラス。
//	このクラスは次の状態しか表せない。
//
//	１、全てのフィールドが処理対象として選択されている
//		(プロジェクションを使用していない場合)
//
//	２、いくつかのフィールドが処理対象として選択されている
//
//	初期状態は１である。
//	状態２の場合は選択されているフィールド番号を格納しているが、状態１は
//	フィールド番号などの情報を格納していない。
//
//	このクラスにおける「フィールド番号」は 0 番から始まり、0 番は
//	オブジェクトIDのフィールドに相当する。
// 
class TargetFields : public Common::Object
{
public:
	// コンストラクタ / デストラクタ
	explicit TargetFields(const ModSize elementNum_);
	~TargetFields();

	//
	// アクセッサ
	//

	// 選択されたフィールド番号を格納した配列から値を取り出す
	int	get(const int index_) const;

	// 選択されたフィールド番号を格納した配列から値を取り出す
	int	getIndex(const int index_) const;

	// 選択されたフィールド番号を格納した配列の要素数を返す
	int	getSize() const;

	//
	// マニピュレータ
	//

	// プロジェクションによって選択されているフィールド番号を格納する
	// (使用上の注意)
	// - フィールド番号は昇順に格納していくこと
	void	addFieldNumber(int iFieldID_, int iIndex_ = 0);

	// 固定長用と可変長用に分ける
	static void divide(TargetFields& cDirectTarget_,
					   TargetFields& cVariableTarget_,
					   TargetFields* pTargets_,
					   const MetaData& cMetaData_,
					   bool bIsUpdate_ = false);

private:
	// コピーコンストラクタ、代入演算子の使用を禁止する
	TargetFields(const TargetFields&);
	TargetFields& operator=(const TargetFields&);

	// 配列(配列の要素数はプロジェクションされたフィールド数以上である)
	ModVector<ModPair<int, int> >	m_vecPosition;
};

//	CLASS
//	TargetIterator --
//
//	NOTES

class TargetIterator
	: public Common::Object
{
public:
	TargetIterator(const TargetFields* pTarget_,
				   const MetaData* pMetaData_);

	// 次のターゲットがあるか
	bool hasNext() const
	{
		return (m_iTarget < m_iTargetMax);
	}

	// ターゲットとなるフィールドの数
	ModSize getSize() const
	{
		return m_iTargetMax;
	}

	// 次のターゲットとなるフィールドの位置
	Tools::FieldNum getNext()
	{
		return (this->*m_funcNext)();
	}

	// 現在のターゲットの順番
	Tools::FieldNum getIndex()
	{
		return (this->*m_funcIndex)();
	}

private:
	const TargetFields* m_pTarget;
	ModSize m_iTarget;
	ModSize m_iTargetMax;

	Tools::FieldNum	(TargetIterator::* m_funcNext)();
	Tools::FieldNum	(TargetIterator::* m_funcIndex)();

	Tools::FieldNum getNextTarget()
	{
		return m_pTarget->get(m_iTarget++);
	}
	Tools::FieldNum getNextID()
	{
		return m_iTarget++;
	}
	Tools::FieldNum getNextTargetIndex()
	{
		return m_pTarget->getIndex(m_iTarget - 1);
	}
	Tools::FieldNum getNextIndex()
	{
		return m_iTarget - 1;
	}

};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_TARGETFIELDS_H

//
//	Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
