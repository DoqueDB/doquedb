// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenParameter.h --
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_OPENPARAMETER_H
#define __SYDNEY_VECTOR_OPENPARAMETER_H

#include "ModVector.h"

#include "Vector/Module.h"

#include "Common/BitSet.h"

#include "FileCommon/OpenMode.h"
#include "LogicalFile/VectorKey.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class OpenOption;
}

namespace Vector
{

//
//	CLASS
//	Vector::OpenParameter -- オープンパラメータを保持するクラス
//
//	NOTES
//	オープンパラメータ(ファイルのオープン時に与えられ、
//	クローズするまで変わることのない値)を保持するクラス。
//
class SYD_VECTOR_FUNCTION_TESTEXPORT OpenParameter : public Common::Object
{
public:
	// コンストラクタ
	OpenParameter(const LogicalFile::OpenOption& cOpenOption,
				  ModUInt32 ulOuterFieldNumber_);
	//デストラクタ
	~OpenParameter();

	// アクセサ群

	// オープンモードを得る
	const FileCommon::OpenMode::Mode	getOpenMode() const;
	const FileCommon::ReadSubMode::Mode getReadSubMode() const;
#if 0
	// 評価モードでファイルをオープンしているか否か
	bool getEstimateMode() const;
#endif
	// get()の返り値をBitSetにするか否か
	bool getsByBitSet() const;
	// scanモードのときにどちらの方向にファイルを走査していくか
	bool getSortOrder() const;
	// searchモードのときに対象となるベクタキー
	ModUInt32 getSearchValue() const;
	// プロジェクションの対象になっているフィールドの個数
	int getSelectedFieldCount() const;
	// 内部的な意味でのprojectionが空かどうかを確かめる
	bool isEmptyInnerProjection() const;
	//フィールド番号を「外向けの」(VectorKeyを#0とする)仕様で解釈
	bool getOuterMaskAt(ModUInt32 ulOuterFieldID_) const;
	//フィールド番号を「内向けの」(値の第一要素をを#0とする)仕様で解釈
	bool getInnerMaskAt(ModUInt32 ulInnerFieldID_) const;

	// COUNTの特殊列の取得が指示されているか
	bool isGetCount() const;
	// COUNTの特殊列を取得したか
	bool isGottenCount() const;
	// COUNTの特殊列を取得したことにする
	void setGottenCount();
	// COUNTの特殊列を取得していないことにする
	void clearGottenCount();

	// バッチモードか
	bool isBatchMode() const;
private:
	// 安全のため、コピーコンストラクタ及び代入演算子の使用を禁止する
	OpenParameter(const OpenParameter& dummy_);
	OpenParameter& operator=(const OpenParameter& dummy_);
	
	// ファイルをオープンするモード(インサート、削除など)
	FileCommon::OpenMode::Mode	  m_iOpenMode;
	FileCommon::ReadSubMode::Mode m_iReadSubMode;

	// プロジェクション情報を入れるベクタ
	// ここの添え字の解釈は「外向け」
	Common::BitSet	m_cProjection;

	// プロジェクションが選択しているフィールドの個数
	int			m_iSelectedFieldCount;

#if 0
	// コストを見積もるためにオープンした場合は true。レコードファイルは
	// 見積もりの場合もそうでない場合もオープン時の動作は変化しないので、
	// この値によって動作が変化することはない
	bool		m_bEstimate;
#endif

	// scanモードで get する場合、trueならば降順で、
	// falseならば昇順でオブジェクトを取り出す。
	bool		m_bSortOrder;

	// 2001-02-23追加
	bool		m_bGetByBitSet;

	ModUInt32	m_ulSearchValue;

	// 特殊列COUNTのための変数
	bool m_bGetCount;					// COUNTを取得する
	bool m_bGottenCount;				// COUNTを取得した
};

}

_SYDNEY_END

#endif /* __SYDNEY_VECTOR_OPENPARAMETER_H */

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
