// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.h -- オブジェクト記述子のヘッダファイル
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

#ifndef __SYDNEY_VECTOR_OBJECT_H
#define __SYDNEY_VECTOR_OBJECT_H

#include "Common/Object.h"
#include "Vector/FieldIterator.h"

_SYDNEY_BEGIN

namespace Common
{
class DataArrayData;
}

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
class Page;
}

namespace Vector
{
class FileParameter;
class OpenParameter;
//class FieldIterator;

//	CLASS
//	Vector::Object -- オブジェクト(レコード)の記述子。
//	
//	NOTE
//
class Object : public Common::Object
{
public:
	// コンストラクタ
	Object(FileParameter&	rFileParameter_,
		   OpenParameter&	rOpenParameter_,
		   const PhysicalFile::Page* pPage_,
		   ModOffset		ulBlockOffset_,
		   ModOffset		ulBitMapOffset_,
		   char				ucsBitMask_);
	// デストラクタ
	~Object();

	// 再初期化
	void reset(const PhysicalFile::Page*	pPage_,
			   ModOffset					uiBlockOffset_,
			   ModOffset					uiBitMapOffset_,
			   char							ucsBitMask_);

	// アクセサ

	//
	PhysicalFile::Page* getPage();

	// 自分に対応するブロックが使用中か否かを返す
	bool isValid() const;

	// 生成時のブロックの状態を返す(PageManagerが利用)
	bool wasValid() const;

	// プロジェクションに対応したオブジェクトの内容を返す
	// isValid()がfalseならば例外を投げる
	void read(Common::DataArrayData& rData_,
			  int& iElement_);

	// マニピュレータ

	// オブジェクトの内容を更改する
	void update(const Common::DataArrayData& rData_);

	// オブジェクトに新たに内容を書き込む
	void insert(const Common::DataArrayData& rData_);

	// オブジェクトの内容を消去する
	void expunge();

	// ビットを立てる
	void setBit();

	// ビットを倒す
	void unsetBit();

private:
	// コピーコンストラクタと代入演算子の使用を禁止
	Object(Object& rObject_);
	Object& operator=(Object& rObject_);

	// オブジェクトが使用するファイルパラメータへの参照
	const FileParameter& m_rFileParameter;

	// オブジェクトが使用するオープンパラメータへの参照
	const OpenParameter& m_rOpenParameter;

	// 物理ページへのポインタ
	const PhysicalFile::Page*	m_pPage;
	// ビットマップ領域までのオフセット
	ModOffset m_ulBitMapOffset;

	//- m_pBitmapへのマスク(他のブロックに関係するビットを隠す)。
	char m_ucsBitMask;

	// 利用するFieldIterator
	FieldIterator m_cFieldIterator;

	// 生成時のブロックの状態
	bool m_bFormerValidity;
};

} // end of namespace Vector

_SYDNEY_END

#endif /* __SYDNEY_VECTOR_OBJECT_H */

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
