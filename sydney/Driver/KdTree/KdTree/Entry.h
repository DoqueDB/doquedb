// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Entry.h -- 多次元ベクトル
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_ENTRY_H
#define __SYDNEY_KDTREE_ENTRY_H

#include "KdTree/Module.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class Allocator;
class AreaFile;

//	CLASS
//	KdTree::Entry -- オンメモリ索引が保持している多次元ベクトル
//
//	NOTES
//
class Entry
{
	//
	//【注意】	このクラスはメモリから直接キャストして利用されたり、
	//		   	コピーされたりするので、仮想関数を定義してはならない
	//
	
public:

	// 比較クラス
	class Less
	{
	public:
		Less(int d_) : m_dim(d_) {}
		~Less() {}

		// 比較する
		ModBoolean operator () (const Entry* p1, const Entry* p2) const
			{
				return (p1->getValue(m_dim) < p2->getValue(m_dim)) ?
					ModTrue : ModFalse;
			}

		int getDim() const { return m_dim; }
		
	private:
		int m_dim;
	};
	
	// コンストラクタ
	Entry() {}
	// デストラクタ
	~Entry() {}

	// IDを得る
	ModUInt32 getID() const { return m_uiID; }
	// 次元データを得る
	float getValue(int iDimension_) const { return m_pValue[iDimension_]; }
	// 次元数を得る
	int getDimensionSize() const { return m_iDimensionSize & 0xffff; }

	// 削除されているか
	bool isExpunge() const { return m_iDimensionSize & 0x10000; }
	// 削除フラグを設定する
	void expunge() { m_iDimensionSize |= 0x10000; }
	
	// 全体のバイト数を得る
	int getSize() const;
	
	// char* にキャストする (コピー等のため)
	operator const char* () const;
	operator char* ();

	// サイズを求める
	static int calcSize(int dsize_);

	// 各次元の差の二乗の和 -- この√が距離
	double calcDistance(const Entry* p) const;

	// 一番差の大きな次元を求める
	int getMaxDifferenceDimension(const Entry* p) const;
	
	// ID
	ModUInt32 m_uiID;
	// 次元数
	int m_iDimensionSize;
	// 多次元データ
	float m_pValue[1];
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_ENTRY_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
