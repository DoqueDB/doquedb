// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedLocationListIterator.h -- 文書内出現位置リストの反復子
// 
// Copyright (c) 1997, 1999, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedLocationListIterator_H__
#define __ModInvertedLocationListIterator_H__

#include "ModOs.h"
#include "ModInvertedTypes.h"
#include "ModInvertedManager.h"
#include "ModInvertedQueryInternalNode.h"

//
// CLASS
// ModInvertedLocationListIterator -- 文書内位置リストの反復子
//
// NOTES
// 文書内位置リストの反復子のインタフェースを規定する抽象クラス。
// 
class
ModInvertedLocationListIterator : public ModInvertedObject
{
public:
	//
	//	CLASS
	//	ModInvertedLocationListIterator::AutoPointer
	//
	class AutoPointer
	{
	public:
		AutoPointer(ModInvertedLocationListIterator* ite = 0)
			: iterator(ite), owner((ite == 0) ? false : true) {}
		~AutoPointer() { free(); }
		AutoPointer(const AutoPointer& dst)
			: owner(dst.owner),
			  iterator(const_cast<AutoPointer&>(dst).release()) {}
		AutoPointer& operator =(ModInvertedLocationListIterator* ite)
		{
			free();
			iterator = ite;
			owner = true;
			return *this;
		}
		AutoPointer& operator =(AutoPointer& dst)
		{
			if (iterator != dst.iterator) {
				owner = dst.owner;
				iterator = dst.release();
			}
			return *this;
		}

		ModInvertedLocationListIterator* release()
		{
			owner = false;
			return iterator;
		}
		ModInvertedLocationListIterator* get() const { return iterator; }
		ModInvertedLocationListIterator* operator ->() const { return get(); }
		ModInvertedLocationListIterator& operator *() const { return *get(); }

	private:
		void free()
		{
			if (owner) iterator->release();
		}

		bool owner;
		ModInvertedLocationListIterator* iterator;
	};


	ModInvertedLocationListIterator(ModInvertedQueryInternalNode* node_ = 0)
		: node(node_), nextInstance(0) {}
	virtual ~ModInvertedLocationListIterator() {}
	void operator++();
	virtual void next() = 0;
	virtual void reset() = 0;
	virtual ModBoolean isEnd() const = 0;
	virtual ModSize getLocation() = 0;
	virtual ModSize getEndLocation() = 0;

	virtual ModBoolean lowerBound(const ModSize);
	virtual ModBoolean find(ModSize);

	// endのアクセサ関数 OrderedDistance以外はendを持たないので0を返す
	virtual ModInvertedLocationListIterator* getEnd();
	virtual void setEnd(ModInvertedLocationListIterator* end_);

	// ※ 以下のものは全てのサブクラスで必要ではなく、
	//	CompressedLocationListIterator のサブクラスにのみ必要なので、
	//	そちらに移す。
//	virtual void setLength(const ModSize) = 0;
//	virtual ModSize getLength() = 0;

	virtual ModSize getFrequency() { return 0; }

	// 開放する
	virtual void release()
	{
		if (node) node->pushFreeList(this);
	}

	// ノード
	ModInvertedQueryInternalNode* node;
	// フリーリスト用
	ModInvertedLocationListIterator* nextInstance;

private:
	
};

//
// FUNCTION
// ModInvertedLocationListIterator::operator++ -- インクリメントオペレータ
//
// NOTES
// 文書内位置リストの反復子の現在位置を前に進める。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void 
ModInvertedLocationListIterator::operator++()
{
	next();
}

//
// FUNCTION
// ModInvertedLocationListIterator::lowerBound -- 下限検索
//
// NOTES
// 現在位置から、与えられた値以上で最小の値を検索する。
//
// ARGUMENTS
// const ModSize target_
//		検索対象の値
//
// RETURN
// 検索できれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedLocationListIterator::lowerBound(const ModSize target_)
{
	while (isEnd() == ModFalse) {
		if (getLocation() >= target_) {
			return ModTrue;
		}
		next();
	}
	return ModFalse;
}

//
// FUNCTION public
// ModInvertedLocationListIterator::getEnd -- 終端simpleTokenLeafNodeの位置情報へのポインタendのアクセサ関数。
//
// NOTES
// ModInvertedLocationListIterator::getEnd -- 終端simpleTokenLeafNodeの位置情報
// へのポインタendのアクセサ関数。endを返す。OrderedDistance以外の場合は本関数が
// コールされる。
//
// ARGUMENTS
// なし
//
// RETURN
// 常に0。OrderedDistance以外はendを持たないため。
//
// EXCEPTIONS
// なし
//
inline ModInvertedLocationListIterator* 
ModInvertedLocationListIterator::getEnd()
{
	return 0;
}

//
// FUNCTION public
// ModInvertedLocationListIterator::setEnd -- 終端simpleTokenLeafNodeの位置情報へのポインタendのアクセサ関数。
//
// NOTES
// ModInvertedLocationListIterator::getEnd -- 終端simpleTokenLeafNodeの位置情報
// へのポインタendのアクセサ関数。OrderedDistanceLocationListItratorのendをセッ
// トする。本関数がコールされるのはOrederedDistanceLocationListIterator以外の
// iteratorの場合であるため、なにもしない。
//
// ARGUMENTS
// ModInvertedLocationListIterator* end_
//		セットする位置情報
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline void
ModInvertedLocationListIterator::setEnd(ModInvertedLocationListIterator* end_)
{
}

//
// FUNCTION
// ModInvertedLocationListIterator::find -- 文書IDによる検索
//
// NOTES
// パラメータで指定された位置にビットが立っているか調べる。
// これを使用するのは、WordlocationCoderがUNAの場合にのみ。
//
// ARGUMENTS
// const ModSize target_
//      調査対象の値
//
// RETURN
// ビットが立っていれば ModTrue、そうでなければ ModFalse
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//
inline ModBoolean
ModInvertedLocationListIterator::find(ModSize target)
{
	return find(target);
}

/* purecov: begin deadcode */
//
//		ModList, ModMap を使用した場合、NTでは以下の定義が無いとコンパイル
//		エラーになる。ModVector の場合は不要だが、念のため残しておく
//
inline void
ModDestroy(ModInvertedLocationListIterator** pointer) {};
/* purecov: end */

#endif	// __ModInvertedLocationListIterator_H__

//
// Copyright (c) 1997, 1999, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
