// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NullBitmap.h -- ヌルビットマップクラスのヘッダファイル
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

#ifndef __SYDNEY_BTREE_NULLBITMAP_H
#define __SYDNEY_BTREE_NULLBITMAP_H

#include "Btree/Module.h"

#include "Common/Object.h"
#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE_BEGIN

//
//	CLASS
//	Btree::NullBitmap --
//		キーオブジェクト／バリューオブジェクトの
//		ヌルビットマップクラス
//
//	NOTES
//	キーオブジェクト／バリューオブジェクトの
//	ヌルビットマップクラス。
//
class NullBitmap
{
public:

	//
	//	STRUCT public
	//	Btree::NullBitmap::Access -- アクセス情報構造体
	//
	//	NOTES
	//	ヌルビットマップへアクセスする際の情報を
	//	保持する構造体。
	//
	struct Access
	{
		//
		//	ENUM public
		//	Btree::NullBitmap::Access::Mode -- アクセスモード
		//
		//	NOTES
		//	ヌルビットマップへのアクセスモード
		//
		enum Mode
		{
			ReadOnly = 0, // 参照のみ
			ReadWrite     // 更新可
		};
	};

	//
	//	TYPEDEF public
	//	Btree::NullBitmap::Size -- ヌルビットマップサイズの型
	//
	//	NOTES
	//	ヌルビットマップサイズの型。
	//
	typedef Os::Memory::Size	Size;

	//
	//	TYPEDEF public
	//	Btree::NullBitmap::Value -- ヌルビットマップの型
	//
	//	NOTES
	//	ヌルビットマップの型。
	//
	typedef unsigned char		Value;

	// コンストラクタ
	NullBitmap(const Value*			Top_,
			   const int			FieldNum_,
			   const Access::Mode	AccessMode_);

	// デストラクタ
	~NullBitmap();

	// 全ビットをOFFする
	void clear() const;

	// 全ビットをOFFする
	static void clear(Value*	Top_,
					  const int	FieldNum_);

	// ビットをONする
	void on(const int	FieldIndex_) const;

	// ビットをONする
	static void on(Value*		Top_,
				   const int	FieldNum_,
				   const int	FieldIndex_);

	// ビットをOFFする
	void off(const int	FieldIndex_) const;

	// ビットをOFFする
	static void off(Value*		Top_,
					const int	FieldNum_,
					const int	FieldIndex_);

	// ヌル値のフィールドが存在するかを知らせる
	bool existNull() const;

	// ヌル値のフィールドが存在するかを知らせる
	static bool existNull(const Value*	Top_,
						  const int		FieldNum_);

	// フィールド値がヌル値かどうかを知らせる
	bool
	isNull(int FieldIndex_) const;
	static bool
	isNull(const Value*	Top_, int FieldNum_, int FieldIndex_);

	// ビットマップの後ろの領域へのポインタを返す
	void*
	getTail() const;
	static void*
	getTail(Value* Top_, int FieldNum_);

	// ビットマップの後ろの領域へのポインタを返す
	const void* getConstTail() const;

	// ビットマップの後ろの領域へのポインタを返す
	static const void* getConstTail(const Value*	Top_,
									const int		FieldNum_);

	// ビットマップサイズを返す [byte]
	Size getSize() const;

	// ビットマップサイズを返す [byte]
	static Size getSize(const int	FieldNum_);

	// ビットマップインデックスを返す
	static int getIndex(const int	FieldIndex_);

	// ビット番号を返す
	static int getBitNo(const int	FieldIndex_);

private:

	// 有効な指定がされているかどうかをチェックする
	static bool isValid(const int	FieldNum_,
						const int	FieldIndex_);

	//
	// 静的データメンバ
	//

	// 1つのヌルビットマップ辺りのビット数
//	static const int    BitNumInValue; // = sizeof(Value) * 8
#define     NULLBITMAP__BITNUMINVALUE ((const int)(sizeof(Value) * 8))

	//
	// 非静的データメンバ
	//

	// ヌルビットマップ先頭へのポインタ
	Value*			m_Top;

	// フィールド数
	int				m_FieldNum;

	// アクセスモード
	Access::Mode	m_AccessMode;

}; // end of class Btree::NullBitmap

//
//	FUNCTION private
//	Btree::File::isValid --
//		有効な指定がされているかどうかをチェックする
//
//	NOTES
//	有効な指定がされているかどうかをチェックする。
//	※ for debug
//
//	ARGUMENTS
//	const int	FieldNum_
//		フィールド数
//	const int	FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	bool
//		有効な指定かどうか
//			true  : 有効
//			false : 無効
//
//	EXCEPTIONS
//	なし
//
// static
inline bool
NullBitmap::isValid(const int	FieldNum_,
					const int	FieldIndex_)
{
	return FieldIndex_ < FieldNum_;
}

//
//	FUNCTION public
//	Btree::NullBitmap::getIndex -- ビットマップインデックスを返す
//
//	NOTES
//	ビットマップインデックスを返す。
//
//	ARGUMENTS
//	const int	FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	int
//		ビットマップインデックス
//
//	EXCEPTIONS
//	なし
//
// static
inline int
NullBitmap::getIndex(const int	FieldIndex_)
{
	return FieldIndex_ / NULLBITMAP__BITNUMINVALUE;
}

//
//	FUNCTION public
//	Btree::NullBitmap::getBitNo -- ビット番号を返す
//
//	NOTES
//	ビット番号を返す。
//
//	ARGUMENTS
//	const int	FieldIndex_
//		フィールドインデックス
//
//	RETURN
//	int
//		ビット番号
//
//	EXCEPTIONS
//	なし
//
// static
inline int
NullBitmap::getBitNo(const int	FieldIndex_)
{
	return FieldIndex_ % NULLBITMAP__BITNUMINVALUE;
}

//
//	FUNCTION public
//	Btree::NullBitmap::getSize -- ビットマップサイズを返す
//
//	NOTES
//	ビットマップサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	NullBitmap::Size
//		ビットマップサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
inline NullBitmap::Size
NullBitmap::getSize() const
{
	return NullBitmap::getSize(this->m_FieldNum);
}

//
//	FUNCTION public
//	Btree::NullBitmap::getSize -- ビットマップサイズを返す
//
//	NOTES
//	ビットマップサイズを返す。
//
//	ARGUMENTS
//	const int	FieldNum_
//		フィールド数
//
//	RETURN
//	Btree::NullBitmap::Size
//		ビットマップサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// static
inline NullBitmap::Size
NullBitmap::getSize(const int	FieldNum_)
{
	return (FieldNum_ + NULLBITMAP__BITNUMINVALUE - 1) / NULLBITMAP__BITNUMINVALUE;
}

#undef NULLBITMAP__BITNUMINVALUE

//	FUNCTION public
//	Btree::NullBitmap::getTail -- 
//		ビットマップの後ろの領域へのポインタを返す
//
//	NOTES
//	ビットマップの後ろの領域へのポインタを返す。
//
//	ARGUMENTS
//	Btree::NullBitmap::Value*	Top_
//		ビットマップ先頭へのポインタ
//	int					FieldNum_
//		フィールド数
//
//	RETURN
//	void*
//		ビットマップの後ろの領域へのポインタ
//
//	EXCEPTIONS
//	なし

// static
inline
void*
NullBitmap::getTail(Value* Top_, int FieldNum_)
{
	const int lastBitmapIndex = getIndex(FieldNum_ - 1);

	return Top_ + (lastBitmapIndex + 1);
}

//
//	FUNCTION public
//	Btree::NullBitmap::getConstTail -- 
//		ビットマップの後ろの領域へのポインタを返す
//
//	NOTES
//	ビットマップの後ろの領域へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const void*
//		ビットマップの後ろの領域へのポインタ
//
//	EXCEPTIONS
//	なし
//
inline const void*
NullBitmap::getConstTail() const
{
	return NullBitmap::getConstTail(this->m_Top, this->m_FieldNum);
}

//
//	FUNCTION public
//	Btree::NullBitmap::getConstTail -- 
//		ビットマップの後ろの領域へのポインタを返す
//
//	NOTES
//	ビットマップの後ろの領域へのポインタを返す。
//
//	ARGUMENTS
//	Btree::NullBitmap::Value*	Top_
//		ビットマップ先頭へのポインタ
//	const int					FieldNum_
//		フィールド数
//
//	RETURN
//	const void*
//		ビットマップの後ろの領域へのポインタ
//
//	EXCEPTIONS
//	なし
//
// static
inline const void*
NullBitmap::getConstTail(const NullBitmap::Value*	Top_,
						 const int					FieldNum_)
{
	int	lastBitmapIndex = NullBitmap::getIndex(FieldNum_ - 1);

	return Top_ + (lastBitmapIndex + 1);
}

//	FUNCTION public
//	Btree::NullBitmap::isNull --
//		フィールド値がヌル値かどうかを知らせる
//
//	NOTES
//
//	ARGUMENTS
//	int	FieldIndex_
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

inline
bool
NullBitmap::isNull(int FieldIndex_) const
{
	return isNull(m_Top, m_FieldNum, FieldIndex_);
}

_SYDNEY_BTREE_END
_SYDNEY_END

#endif //__SYDNEY_BTREE_NULLBITMAP_H

//
//	Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
