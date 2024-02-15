// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Bitset.h -- Bitset の定義ファイル
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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

#ifndef __UNA_LIBUNA_BITSET__H
#define __UNA_LIBUNA_BITSET__H

#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"
#include "Module.h"

_UNA_BEGIN
	const unsigned char	ByteWidth = 8;	// バイトのビット数
	//
	// 	CLASS
	// 		Bitset -- 1bit２進値の固定長Setを管理するクラス
	//
	class Bitset
	{
	public:
		//
		// CLASS
		// 	reference -- Bitsetの各bitへの参照。Bitset.operator[] で取得する。
		//
		class reference
		{
			friend class Bitset;
		public:
			// デストラクタ
			~reference();

			// for b[i] = bit_;
			reference&	operator=(ModBoolean bit_);

			// for b[i] = b[j];
			reference&	operator=(const reference& Bs_);

			// ビット反転
			ModBoolean	operator~() const;

			// for x = b[i];
			operator	ModBoolean() const;
		private:
			// コンストラクタ
			reference(Bitset& x_, unsigned p_);

			Bitset* _Pbs;
			unsigned _Off;
		};

		//
		// CLASS
		//	ConstIterator -- Bitset の参照専用イテレータ
		//
		class ConstIterator
		{
		public:
			// コンストラクタ
			ConstIterator(const Bitset* bitset_,
						  ModSize pos_);

			// ++演算子
			ConstIterator& operator ++();
			
			ConstIterator operator ++(int dummy_);

			// +演算子
			ConstIterator operator + ( int i_ );

			// *演算子
			ModBoolean operator *() const;

			// ==演算子
			bool operator ==(const ConstIterator& src_) const;
			
			// !=演算子
			bool operator !=(const ConstIterator& src_) const;

		private:
			// ビットセット
			const Bitset* _parent;
			// 位置
			ModSize _pos;
		};

		// Type Define
		typedef	unsigned int Pos;
		typedef	unsigned int Unit;
		typedef	unsigned int Index;

		// コンストラクタ
		Bitset(unsigned = 32, ModBoolean bit_ = ModFalse);

		// コピーコンストラクタ
		Bitset(const Bitset&);

		// デストラクタ
		~Bitset();

		// ビットの設定
		Bitset&			set(Pos, ModBoolean = ModTrue);

		// ビットの取得
		ModBoolean		test(Pos) const;

		// ビットの取得
		ModBoolean		test(Pos , Pos, ModBoolean check_ = ModTrue) const;

		// 範囲指定ビット数取得
		unsigned int	testCount(Pos min_, Pos max_, ModBoolean check_ = ModTrue) const;

		//オペレータ (非const版)
		reference		operator[](Pos);

		// オペレータ (const版)
		ModBoolean		operator[](Pos) const;

		// ModTrueとなっているビット数取得
		unsigned		count(void) const;

		// インスタンスの管理するbitサイズ
		unsigned		size(void) const;

		// &=演算子
		Bitset& 		operator&=(const Bitset&);

		// |= 演算子
		Bitset& 		operator|=(const Bitset&);

		// ^=演算子
		Bitset& 		operator^=(const Bitset&);

		// =演算子
		Bitset& 		operator=(const Bitset&);

		// ~=演算子
		Bitset&			operator ~ ();

		// ==演算子
		ModBoolean		operator == (const Bitset& other_);

		// !=演算子
		ModBoolean		operator != (const Bitset& other_);

		// 演算子
		ModUnicodeString getString() const;

		// イテレータアクセサ
		ConstIterator	begin() const;
		ConstIterator	end() const;

	protected:
		// ユニットインデックスとマスクを取得する。
		void			getIndex(const Pos, Index&, Unit&) const;

		// ビットサイズから1ユニットのビット数を勘案してユニットのサイズを求めるヘルパーメソッド
		Index			getUnitSize(const unsigned&) const;

		// メソッドを起動したBitsetインスタンスのユニットサイズを求める
		Index			getUnitSize() const;

		// ビットデータを取得する。
		Unit*			getBitData(void) const;

		// データ領域を拡張する。
		void			expand(unsigned);

	private:
		// ビットデータ
		Unit* _bitdata;

		// ビットサイズ
		unsigned _bitsize;
	};
_UNA_END

#endif // __UNA_LIBUNA_BITSET__H


//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
