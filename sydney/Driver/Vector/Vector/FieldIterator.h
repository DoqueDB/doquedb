// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldIterator.h -- フィールド反復子クラスのヘッダファイル
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_FIELDITERATOR_H
#define __SYDNEY_VECTOR_FIELDITERATOR_H

#include "Vector/Module.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common
{
class Data;
}

namespace PhysicalFile
{
class Page;
}

namespace Vector
{
class Projection;
class FileParameter;
class OpenParameter;

//	CLASS
//	Vector::FieldIterator -- フィールド反復子のクラス
//
//	NOTES
//	オブジェクトのフィールドをフィールド番号の順に走査し、
//  読み書きも行うクラス。
//
//  課題
//	反復子がコンストラクトされてからデストラクトされるまでの間にフィールド
//	値が更新されたり、オブジェクトそのものが削除されることはないと仮定する。
//	呼出側で、削除や更新が起きないことを保証しなければいけない。
//
class SYD_VECTOR_FUNCTION_TESTEXPORT FieldIterator : public Common::Object
{
public:
	// コンストラクタ
	FieldIterator(const PhysicalFile::Page*	pPage_,
				  ModSize				ulOffset_,
				  const FileParameter&	rFileParameter_,
				  const OpenParameter&	rOpenParameter_);
	// デストラクタ
	~FieldIterator();

	// 再初期化
	void reset(const PhysicalFile::Page*	pPage_,
			   ModSize						ulOffset_);

	//
	// アクセサ
	//

	// フィールドIDを取得
	ModUInt32	getInnerFieldID() const;

	// フィールド値を読み込む
	void readField(Common::Data& cData_) const;

	//
	// マニピュレータ
	//

	// フィールド値を更新する
	ModSize	writeField(Common::Data* pCommonData_);

	// 初期状態に戻す
	void reset();

	// 順方向への移動。Projectionの情報に従って適切な操作をする。
	bool next();

#ifdef OBSOLETE
	// 任意のフィールドID(解釈は内向け)に反復子を移動
	bool seek(ModUInt32 ulInnerFieldID_);
#endif

	// 任意のフィールドID(解釈は内向け)に強制的に反復子を移動
	void seekForced(ModUInt32 ulInnerFieldID_);

private:
	//- 規約5-20に従い、コピーコンストラクタと代入演算子の使用を禁止する
	FieldIterator(const FieldIterator& dummy_);
	FieldIterator& operator=(const FieldIterator& dummy_);
	
	// 対象のブロックを含むページ
	const PhysicalFile::Page*	m_pPage;
	// ページ先頭からブロック先頭までのオフセット
	ModOffset			m_ulOffset;

	// 反復子が指しているフィールドのID(内向け解釈)
	ModUInt32			m_ulInnerFieldID;

	//- ↓thread safeではなくなるのでstaticにはできないことに注意。

	// ファイルパラメータへの参照
	const FileParameter&	m_rFileParameter;

	// オープンパラメータへの参照
	const OpenParameter&	m_rOpenParameter;

	// フィールド数は頻用するのでキャッシュ
	ModSize					m_ulInnerFieldNumber;

	// プロジェクションが空か否か
	bool m_bEmpty;
};

} // end of namespace Vector

_SYDNEY_END

#endif /* __SYDNEY_VECTOR_FIELDITERATOR_H */

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
