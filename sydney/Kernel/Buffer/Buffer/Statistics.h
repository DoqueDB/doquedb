// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statistics.h -- 統計情報関連のクラス定義、関数宣言
// 
// Copyright (c) 2003, 2004, 2008, 2009, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_STATISTICS_H
#define	__SYDNEY_BUFFER_STATISTICS_H

#include "Buffer/Module.h"

#include "Os/CriticalSection.h"
#include "Os/File.h"
#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

class Page;
class Pool;
class StatisticsReporter;

//	CLASS
//	Buffer::Statistics -- バッファの統計情報を表すクラス
//
//	NOTES

class Statistics
{
	friend class File;
	friend class Page;
	friend class Pool;
	friend class StatisticsReporter;
	
public:
	struct Category
	{
		//	ENUM
		//	Buffer::Statistics::Category::Value --
		//		バッファの統計情報を種類を表す列挙型
		//
		//	NOTES

		typedef unsigned char	Value;
		enum
		{
			// バッファページのフィックス
			Fix =			0,
			// バッファページのアンフィックス
			Unfix,
			// ファイルからの読み出し
			Read,
			// ファイルへの書き込み
			Write,
			// バッファメモリの確保
			Allocate,
			// バッファメモリの破棄
			Free,
			// バッファメモリの再利用
			Replace,
			// バッファプールの枯渇
			Exhaust,
			// 値の数
			Count,
			// 不明
			Unknown =		Count
		};
	};

	// コンストラクター
	Statistics();
	// コピーコンストラクター
	SYD_BUFFER_FUNCTION
	Statistics(const Statistics& v);
	// デストラクター
	~Statistics();

	// 代入演算子
	Statistics& operator = (const Statistics& v);

	// バッファページに関する操作を記録する
	static void
	record(Category::Value category, Os::Memory::Size size);

	// バッファページに関する統計情報をログに出力する
	static void printLog();

private:
	// 操作ごとの回数
	unsigned int			_count[Category::Count];
	// 操作ごとのサイズ(B 単位)
	Os::File::Size			_size[Category::Count];

 	// 初期化する
	SYD_BUFFER_FUNCTION
	void
	clear();

	// 排他制御用のラッチ
	mutable Os::CriticalSection	_latch;
};

//	FUNCTION public
//	Buffer::Statistics::Statistics -- 統計情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Statistics::Statistics()
{
	// 初期化する

	clear();
}

//	FUNCTION public
//	Buffer::Statistics::~Statistics -- 統計情報を表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Statistics::~Statistics()
{}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_STATISTICS_H

//
// Copyright (c) 2003, 2004, 2008, 2009, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
