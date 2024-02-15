// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.cpp -- オブジェクト記述子の実装ファイル
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/BadArgument.h"

#include "Vector/FileParameter.h"
#include "Vector/OpenParameter.h"
#include "Vector/FileInformation.h"
#include "Vector/FieldIterator.h"
#include "Vector/Object.h"

_SYDNEY_USING

using namespace Vector;

//	FUNCTION
//	Vector::Object::Object -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	FileParameter&	rFileParameter_
//	OpenParameter&	rOpenParameter_
//	char*			pBlock_
//	char*			pBitMap_
//	const char		ucsBitMask_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Object::Object(
	FileParameter&	 rFileParameter_,
	OpenParameter&	 rOpenParameter_,
	const PhysicalFile::Page* pPage_,
	ModOffset		ulBlockOffset_,
	ModOffset		ulBitMapOffset_,
	char			 ucsBitMask_):
		m_rFileParameter(rFileParameter_),
		m_rOpenParameter(rOpenParameter_),
		m_pPage(pPage_),
		m_ulBitMapOffset(ulBitMapOffset_),
		m_ucsBitMask(ucsBitMask_),
		m_cFieldIterator(pPage_, ulBlockOffset_, 
						 rFileParameter_, rOpenParameter_)
{		
	m_bFormerValidity = isValid();
}

//	FUNCTION
//	Vector::Object::~Object -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Object::~Object()
{
}

//
//	FUNCTION
//	Vector::Object::reset -- 再初期化
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::Page*	pPage_
//		物理ページ
//	ModOffset					uiBlockOffset_
//		ブロックオフセット
//	ModOffset					uiBitMapOffset_
//		ビットマップオフセット
//	char						ucsBitMask_
//		ビットマスク
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Object::reset(const PhysicalFile::Page* pPage_,
			  ModOffset uiBlockOffset_,
			  ModOffset uiBitMapOffset_,
			  char ucsBitMask_)
{
	m_pPage = pPage_;
	m_ulBitMapOffset = uiBitMapOffset_;
	m_ucsBitMask = ucsBitMask_;
	m_cFieldIterator.reset(pPage_, uiBlockOffset_);
	m_bFormerValidity = isValid();
}

// アクセサ

//
PhysicalFile::Page*
Object::getPage()
{
	return const_cast<PhysicalFile::Page*>(m_pPage);
}

//	FUNCTION
//	Vector::Object::isValid
//	  -- 自分に対応するブロックが埋まっていないか否かを返す
//
//	NOTES
//	自分に対応するブロックが埋まっていないか否かを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	なし
//
bool
Object::isValid() const
{
	SydAssert(m_pPage != 0);
	const char* pPageHead = (*m_pPage).operator const char*();
	return ((*(pPageHead + m_ulBitMapOffset) & m_ucsBitMask) != 0);
}

//	FUNCTION
//	Vector::Object::wasValid
//  -- 生成時のブロックの状態を返す。
//
//	NOTES
//  生成時のブロックの状態を返す。
//	この関数は、PageManagerがページ内のオブジェクト数を
//	操作するときに利用する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//	なし
//
bool
Object::wasValid() const
{
	return m_bFormerValidity;
}

//	FUNCTION
//	Object::read -- オブジェクトの中身を読み込む
//
//	NOTES
//	オブジェクトの中身を読み込む。
//	プロジェクションの対象でないフィールドは読み飛ばす。
//- !!Leakable as specification!!
//
//	ARGUMENTS
//	Common::DataArrayData& rData_
//	  返り値を収めるための配列型データ。
//	  VectorKeyは含まれない。(事実上の返り値)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//	該当のブロックが空である場合。
//
void
Object::read(Common::DataArrayData& rData_, int& iElement_)
{
	SydAssert(isValid());

	// 反復子を回しつつ各フィールドを読む
	while (m_cFieldIterator.next()) {
		Common::Data::Pointer p = rData_.getElement(iElement_++);
		m_cFieldIterator.readField(*p);
	}
	m_cFieldIterator.reset();
}

// マニピュレータ

//	FUNCTION
//	Object::update -- オブジェクトの中身を書き換える
//
//	NOTES
//	プロジェクションに従って、オブジェクトの中身を書き換える。
//	updateに失敗したときの内容を入れておくBufferは
//	上位の関数に作り、ここには作らない(MODに適当なのはないか?)
//
//	ARGUMENTS
//	Common::DataArrayData& rData_
//	  書き換える中身が入っている配列型データ。
//	
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//	該当のブロックが空である場合。
//
void
Object::update(const Common::DataArrayData& rData_)
{
	SydAssert(isValid());

	int i=0;
	while(m_cFieldIterator.next())
	{
		m_cFieldIterator.writeField(rData_.getElement(i).get());
		i++;
	}
	m_cFieldIterator.reset();
}

//	FUNCTION
//	Vector::Object::insert
//	  -- オブジェクトに新たに内容を書き込む
//
//	NOTES
//	オブジェクトに新たに内容を書き込む。
//
//	ARGUMENTS
//	Common::DataArrayData& rData_
//	  ベクタキーを除く各フィールドの情報。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//	  isValid()が真のとき
//
void
Object::insert(const Common::DataArrayData& rData_)
{
	SydAssert(!isValid());
	ModSize ulSize = m_rFileParameter.getInnerFieldNumber();
	try{
		for (ModSize i=0; i<ulSize; ++i) {
			m_cFieldIterator.seekForced(i);
			m_cFieldIterator.writeField(rData_.getElement(i).get());
		}
	}catch(...){
		_SYDNEY_RETHROW;
	}

	// 自分のビットマップ領域をonにする
	setBit();
}

//	FUNCTION
//	Vector::Object::expunge
//	  -- オブジェクトの内容を消去する
//
//	NOTES
//	オブジェクトの内容を消去する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//	  isValid()が偽のとき
//
void
Object::expunge()
{
	SydAssert(isValid());

	unsetBit();
}

//
//	FUNCTION
//	Vector::Object::setBit
//		自分のビットマップ領域をonにする
//
//	NOTES
//		自分のビットマップ領域をonにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Object::setBit()
{
	char* pPageHead = (*m_pPage).operator char*();
	*(pPageHead + m_ulBitMapOffset) |= m_ucsBitMask;
}

//
//	FUNCTION
//	Vector::Object::unsetBit
//		自分のビットマップ領域をoffにする
//
//	NOTES
//		自分のビットマップ領域をoffにする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
void
Object::unsetBit()
{
	char* pPageHead = (*m_pPage).operator char*();
	*(pPageHead + m_ulBitMapOffset) &= (~m_ucsBitMask);
}


//
//	Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
