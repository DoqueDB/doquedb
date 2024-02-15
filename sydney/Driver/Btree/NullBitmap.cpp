// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NullBitmap.cpp -- ヌルビットマップクラスの実現ファイル
// 
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Btree/NullBitmap.h"

#include "Common/Assert.h"
#include "Exception/BadArgument.h"


_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	CONST private
//	Btree::NullBitmap::BitNumInValue --
//		1つのヌルビットマップ辺りのビット数
//
//	NOTES
//	1つのヌルビットマップ辺りのビット数。
//
// static
//const int
//NullBitmap::BitNumInValue = sizeof(NullBitmap::Value) << 3;

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::NullBitmap::NullBitmap -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Btree::NullBitmap::Value*			Top_
//		ヌルビットマップ先頭へのポインタ
//	const int								FieldNum_
//		フィールド数
//	const Btree::NullBitmap::Access::Mode	AccessMode_
//		アクセスモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
NullBitmap::NullBitmap(const Value*			Top_,
					   const int			FieldNum_,
					   const Access::Mode	AccessMode_)
	: m_Top(const_cast<NullBitmap::Value*>(Top_)),
	  m_FieldNum(FieldNum_),
	  m_AccessMode(AccessMode_)
{
	; _SYDNEY_ASSERT(Top_ != 0);
}

//
//	FUNCTION public
//	Btree::NullBitmap::~NullBitmap -- デストラクタ
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
NullBitmap::~NullBitmap()
{
}

//
//	FUNCTION public
//	Btree::NullBitmap::clear -- 全ビットをOFFする
//
//	NOTES
//	全ビットをOFFする。
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
NullBitmap::clear() const
{
	NullBitmap::clear(this->m_Top, this->m_FieldNum);
}

//
//	FUNCTION public
//	Btree::NullBitmap::clear -- 全ビットをOFFする
//
//	NOTES
//	全ビットをOFFする。
//
//	ARGUMENTS
//	NullBitmap::Value*	Top_
//		ヌルビットマップ先頭へのポインタ
//	const int			FieldNum_
//		フィールド数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
NullBitmap::clear(NullBitmap::Value*	Top_,
				  const int				FieldNum_)
{
	NullBitmap::Value*	tail =
		static_cast<NullBitmap::Value*>(NullBitmap::getTail(Top_,
															FieldNum_));

	for (NullBitmap::Value*	nullBitmap = Top_;
		 nullBitmap < tail;
		 nullBitmap++)
	{
		*nullBitmap = 0;
	}
}

//
//	FUNCTION pubilc
//	Btree::NullBitmap::on -- ビットをONする
//
//	NOTES
//	ビットをONする。
//
//	ARGUMENTS
//	const int	FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
void
NullBitmap::on(const int	FieldIndex_) const
{
	; _SYDNEY_ASSERT(this->m_AccessMode == NullBitmap::Access::ReadWrite);

	NullBitmap::on(this->m_Top, this->m_FieldNum, FieldIndex_);
}

//
//	FUNCTION public
//	Btree::NullBitmap::on -- ビットをONする
//
//	NOTES
//	ビットをONする。
//
//	ARGUMENTS
//	NullBitmap::Value*	Top_
//		ヌルビットマップ先頭へのポインタ
//	const int			FieldNum_
//		フィールド数
//	const int			FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
// static
void
NullBitmap::on(NullBitmap::Value*	Top_,
			   const int			FieldNum_,
			   const int			FieldIndex_)
{
	; _SYDNEY_ASSERT(NullBitmap::isValid(FieldNum_, FieldIndex_));

	NullBitmap::Value*	bitmap = Top_ + NullBitmap::getIndex(FieldIndex_);

	int	bitNo = NullBitmap::getBitNo(FieldIndex_);

	NullBitmap::Value	mask = 1 << bitNo;

	*bitmap |= mask;
}

//
//	FUNCTION public
//	Btree::NullBitmap::off -- ビットをOFFする
//
//	NOTES
//	ビットをOFFする。
//
//	ARGUMENTS
//	const int	FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
void
NullBitmap::off(const int	FieldIndex_) const
{
	; _SYDNEY_ASSERT(this->m_AccessMode == NullBitmap::Access::ReadWrite);

	NullBitmap::off(this->m_Top, this->m_FieldNum, FieldIndex_);
}

//
//	FUNCTION public
//	Btree::NullBitmap::off -- ビットをOFFする
//
//	NOTES
//	ビットをOFFする。
//
//	ARGUMENTS
//	NullBitmap::Value*	Top_
//		ヌルビットマップ先頭へのポインタ
//	const int			FieldNum_
//		フィールド数
//	const int			FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
// static
void
NullBitmap::off(NullBitmap::Value*	Top_,
			   const int			FieldNum_,
			   const int			FieldIndex_)
{
	; _SYDNEY_ASSERT(NullBitmap::isValid(FieldNum_, FieldIndex_));

	NullBitmap::Value*	bitmap = Top_ + NullBitmap::getIndex(FieldIndex_);

	int	bitNo = NullBitmap::getBitNo(FieldIndex_);

	NullBitmap::Value	mask = 1 << bitNo;
	mask = ~mask;

	*bitmap &= mask;
}

//
//	FUNCTION public
//	Btree::NullBitmap::existNull --
//		ヌル値のフィールドが存在するかを知らせる
//
//	NOTES
//	ヌル値のフィールドが存在するかを知らせる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ヌル値のフィールドが存在するかどうか
//			true  : ヌル値のフィールドが存在する
//			false : ヌル値のフィールドが存在しない
//
//	EXCEPTIONS
//	なし
//
bool
NullBitmap::existNull() const
{
	return NullBitmap::existNull(this->m_Top, this->m_FieldNum);
}

//
//	FUNCTION public
//	Btree::NullBitmap::existNull --
//		ヌル値のフィールドが存在するかを知らせる
//
//	NOTES
//	ヌル値のフィールドが存在するかを知らせる。
//
//	ARGUMENTS
//	const Value*	Top_
//		ヌルビットマップ先頭へのポインタ
//	const int		FieldNum_
//		フィールド数
//
//	RETURN
//	bool
//		ヌル値のフィールドが存在するかどうか
//			true  : ヌル値のフィールドが存在する
//			false : ヌル値のフィールドが存在しない
//
//	EXCEPTIONS
//	なし
//
// static
bool
NullBitmap::existNull(const Value*	Top_,
					  const int		FieldNum_)
{
	int	lastBitmapIndex = NullBitmap::getIndex(FieldNum_ - 1);

	for (int i = 0; i <= lastBitmapIndex; i++)
	{
		if (*(Top_ + i) != 0)
		{
			return true;
		}
	}

	return false;
}

//	FUNCTION public
//	Btree::NullBitmap::isNull --
//		フィールド値がヌル値かどうかを知らせる
//
//	NOTES
//
//	ARGUMENTS
//	const NullBitmap::Value*	Top_
//		ヌルビットマップ先頭へのポインタ
//	int					FieldNum_
//		フィールド数
//	int					FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	bool
//		フィールド値がヌル値かどうか
//			true  : フィールド値がヌル値
//			false : フィールド値がヌル値ではない
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数

bool
NullBitmap::isNull(const Value* Top_, int FieldNum_, int FieldIndex_)
{
	; _SYDNEY_ASSERT(isValid(FieldNum_, FieldIndex_));

	const Value* bitmap = Top_ + getIndex(FieldIndex_);
	const Value mask = 1 << getBitNo(FieldIndex_);

	return *bitmap & mask;
}

//	FUNCTION public
//	Btree::NullBitmap::getTail -- 
//		ビットマップの後ろの領域へのポインタを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	void*
//		ビットマップの後ろの領域へのポインタ
//
//	EXCEPTIONS
//	なし

void*
NullBitmap::getTail() const
{
	; _SYDNEY_ASSERT(m_AccessMode == NullBitmap::Access::ReadWrite);

	return getTail(m_Top, m_FieldNum);
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
