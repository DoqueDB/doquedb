// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldIterator.cpp -- フィールド反復子クラスの実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModTypes.h"
#include "Common/Assert.h"
#include "Common/Data.h"
#include "FileCommon/DataManager.h"
#include "PhysicalFile/Page.h"
#include "Vector/FileParameter.h"
#include "Vector/OpenParameter.h"
#include "Vector/FieldIterator.h"

_SYDNEY_USING

using namespace Vector;

namespace {
	const ModUInt32 _UndefinedFieldID = ModUInt32Max;
}

//
//	FUNCTION public
//	Vector::FieldIterator::FieldIterator -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	  void* pBlock_ 
//		ブロック先頭へのポインタ。
//	  FileParameter& rFileParameter_
//		使用するファイルパラメータへの参照。
//	  OpenParameter& rOpenParameter_
//		使用するオープンパラメータへの参照。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//  なし
//
FieldIterator::FieldIterator(const PhysicalFile::Page* pPage_,
							 ModSize ulOffset_,
							 const FileParameter& rFileParameter_,
							 const OpenParameter& rOpenParameter_):
	m_pPage(pPage_),
	m_ulOffset(ulOffset_),
	m_rFileParameter(rFileParameter_),
	m_rOpenParameter(rOpenParameter_), 
	m_ulInnerFieldID(_UndefinedFieldID),
	m_ulInnerFieldNumber(m_rFileParameter.getInnerFieldNumber()),
	m_bEmpty(m_rOpenParameter.isEmptyInnerProjection())
{
}

//  FUNCTION
//  Vector::FieldIterator::~FieldIterator -- デストラクタ
//
//  NOTES
//  FieldIteratorはページのアタッチ･デタッチを行わないので
//  何もしなくてよい。
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
FieldIterator::~FieldIterator()
{
}

//	FUNCTION
//	Vector::FieldIterator::reset -- 再初期化を行う
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::Page*	pPage_
//		物理ページ
//	ModSize						uiOffset_
//		オフセット
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FieldIterator::reset(const PhysicalFile::Page* pPage_,
					 ModSize uiOffset_)
{
	m_pPage = pPage_;
	m_ulOffset = uiOffset_;
	m_ulInnerFieldID = _UndefinedFieldID;
}

//  FUNCTION
//  Vector::FieldIterator::getInnerFieldID -- 
//    反復子が現在指している、内部的な意味でのフィールドのIDを取得する
//
//  NOTES
//  反復子が現在指している、内部的な意味でのフィールドのIDを取得する。
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  ModUInt32
//	  フィールドID(内部的意味)。
//
//  EXCEPTIONS
//  なし
//
ModUInt32 
FieldIterator::getInnerFieldID() const
{
	return m_ulInnerFieldID;
}

//  FUNCTION
//  Vector::FieldIterator::readField -- 反復子が指しているフィールドを読み出す
//
//  NOTES
//	反復子が指しているフィールドを読み出す。
//
//  ARGUMENTS
// 	Common::Data&
//		値を格納するクラス
//
//  RETURN
//	なし
//
//  EXCEPTIONS
//  なし
//
void
FieldIterator::readField(Common::Data& cData_) const
{
	SydAssert(m_pPage!=0);
	; _SYDNEY_ASSERT(
		cData_.getType()
		== m_rFileParameter.getDataTypeForInnerFieldID(m_ulInnerFieldID));

	//	反復子が指しているフィールドを読んで値を返す
	FileCommon::DataManager::accessToCommonData
		(m_pPage,
		 m_ulOffset + m_rFileParameter.
						getInnerFieldOffset(m_ulInnerFieldID),
		 cData_,
		 FileCommon::DataManager::AccessRead);
}

//  FUNCTION
//  Vector::FieldIterator::writeField -- 
//	  フィールドの値をブロックに書き込む
//
//  NOTES
//  反復子がフィールドの値をブロックに書き込む。
//
//  ARGUMENTS
//  Common::Data* pCommonData_
//	  書き込む値が入っている変数。
//
//  RETURN
//  ModSize
//	  書き込んだバイト数。
//
//  EXCEPTIONS
//  なし
//
ModSize
FieldIterator::writeField(Common::Data* pCommonData_)
{
	SydAssert(m_pPage!=0);

	return FileCommon::DataManager::accessToCommonData
		(m_pPage,
		 m_ulOffset + m_rFileParameter.
						getInnerFieldOffset(m_ulInnerFieldID),
		 *pCommonData_,
		 FileCommon::DataManager::AccessWrite);
}

//	FUNCTION
//	Vector::FieldIterator::next -- 
//	  認識可能な次のフィールドに反復子を進める
//	NOTES
//	認識可能な次のフィールドに反復子を進める。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//	  true:  反復子を進めることができた
//	  false: 反復子を進めることができなかった
//
//	EXCEPTIONS
//	なし
//
bool
FieldIterator::next()
{
	if (m_bEmpty)
	{
		return false;
	}
	else if (m_ulInnerFieldID == _UndefinedFieldID)
	{
		m_ulInnerFieldID = 0;
		while (m_ulInnerFieldID < m_ulInnerFieldNumber) {
			if (m_rOpenParameter.getInnerMaskAt(m_ulInnerFieldID) == true)
				return true;
			else
				++m_ulInnerFieldID;
		}
		return false;
	}
	else
	{
		while (true) {
			if (++m_ulInnerFieldID == m_ulInnerFieldNumber) {
				return false;
			}
			if (m_rOpenParameter.getInnerMaskAt(m_ulInnerFieldID)) {
				return true;
			}
		}
	}
}

//	FUNCTION
//	Vector::FieldIterator::reset -- 
//	  プロジェクションが許す最初のフィールドに反復子を移動させる
//	NOTES
//	プロジェクションが許す最初のフィールドに反復子を移動させる。
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
void
FieldIterator::reset()
{
	m_ulInnerFieldID = _UndefinedFieldID;
}

#ifdef OBSOLETE
//	FUNCTION
//	Vector::FieldIterator::seek -- 
//	  任意のフィールドに反復子を移動させる
//	NOTES
//
//	任意のフィールドに反復子を移動させる。
//
//	ARGUMENTS
//	ModUInt32 ulInnerFieldID_
//	  移動先の内部的な意味でのフィールドID。
//
//	RETURN
//	bool
//	  true:  反復子を移動させることができた
//	  false: 反復子を移動させることができなかった
//
//	EXCEPTIONS
//	なし
//
bool
FieldIterator::seek(ModUInt32 ulInnerFieldID_)
{
	// 境界値のチェックのほか、Projection情報も確認する
	if (   ulInnerFieldID_ >= m_ulInnerFieldNumber
		|| !(m_rOpenParameter.getInnerMaskAt(ulInnerFieldID_))) {
		return false;
	}
	m_ulInnerFieldID = ulInnerFieldID_;
	return true;
}
#endif

//	FUNCTION
//	Vector::FieldIterator::seekForced -- 
//	  任意のフィールドに強制的に反復子を移動させる
//	NOTES
//	プロジェクションを無視して、
//	任意のフィールドに強制的に反復子を移動させる。
//	(Object::insertの実装に必要)
//
//	ARGUMENTS
//	ModUInt32 ulInnerFieldID_
//	  移動先の内部的な意味でのフィールドID。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
FieldIterator::seekForced(ModUInt32 ulInnerFieldID_)
{
	// 境界値のチェックのみ行う
	// 条件が逆だったので訂正(2001-01-31)
	if (ulInnerFieldID_ < m_ulInnerFieldNumber) {
		m_ulInnerFieldID = ulInnerFieldID_;
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
