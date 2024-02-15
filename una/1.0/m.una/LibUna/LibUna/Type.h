// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Type.h -- Definition file of Type
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#ifndef __UNA_LIBUNA_TYPE_H
#define __UNA_LIBUNA_TYPE_H

#include "ModTypes.h"
#include "ModAutoPointer.h"
#include "ModCriticalSection.h"
#include "ModUnicodeRegularExpression.h"

namespace UNA {

namespace Type {

		//
		//	TYPEDEF
		//		InterruptFunction -- 中断関数の型宣言
		//
		//	MEMO
		//		処理中にこの関数を呼ぶ。戻り値が 0 以外の場合、処理を中断する。
		//		1st Arg : この関数を呼ぶ際に第１引数に設定する任意の引数
		//		2nd Arg : 処理進捗を表す分子
		//		3rd Arg : 処理進捗全体の分母
		//		
		//		EXCEPTIONS
		//			InterruptException
		//				: _funcの実行結果が0以外の時スローする。
		//
		typedef	int	(*InterruptFunction)(void* , unsigned int, unsigned int);

		/////////////////////////////////////////////////////////////////////////
		//
		//	STRUCT
		//		InterruptInfo -- 中断関数に関する情報を持つ構造体
		//
		struct InterruptInfo {
			InterruptInfo(InterruptFunction func_, void* arg1_,
						  unsigned int mol_, unsigned int deno_)
				 : _func(func_), _arg1(arg1_), _mol(mol_), _deno(deno_)
			{}

			InterruptFunction	_func;
			void*				_arg1;
			unsigned int		_mol;
			unsigned int		_deno;

			void	isInterrupt(unsigned int mol_) {
				if ( _func != 0 )
					if(_func(_arg1, mol_, _deno))
						ModThrow(ModModuleStandard,
								 ModCommonErrorBadArgument,
								 ModErrorLevelError);
			}
		};

		//
		//	STRUCT
		//		Range -- 開始位置と長さを持つ構造体
		//
		//	MEMO
		//		開始位置の型は ModSize との - の演算ができなければならない
		//
		template < class T >
		struct Range
		{
			Range() 							: _start(0), _len(0) {}
			Range(T start_, ModSize len_)		: _start(start_), _len(len_) {}
			Range(T start_, T end_)				: _start(start_), _len(end_ - start_) {}

			// start position
			T		_start;

			// length
			ModSize		_len;
		};

		//
		//	ENUM
		//		CalcType -- 四則演算を表す列挙子
		//
		//	MEMO
		//		主に score ルールで使用している
		//
		class CalcType {
		public:
			enum Value
			{
				Add,
				Sub,
				Multi,
				Div,

				//-- Operate --
				Equal,
				OrEqual,
				AndEqual,
				XorEqual,

				Unknown
			};

			static
			ModUnicodeString
			getOperateString(Value value_);

			static
			Value
			getOperateValue(const ModUnicodeString& value_);

		};

		//
		//	STRUCT
		//		CalcMaterial -- 計算を行うための構造体
		//
		//	MEMO
		//		主に score ルールで使用している
		//
		struct	CalcMaterial
		{
			CalcMaterial() {}
			CalcMaterial(CalcType::Value type_, double mat_)
				 : _type(type_), _material(mat_){}
			CalcType::Value		_type;
			double			_material;
		};

		//
		//	STRUCT
		//		RegularExpression -- ModUnicodeRegularExpression と ModCriticalSection のペア
		//
		//	MEMO
		//		ModUnicodeRegularExpression の matchBegin と matchEnd がマルチスレッドで
		//		使用されることを前提としていないため、クリティカルセクションとのペアで管理する
		//
		struct RegularExpression
		{
			RegularExpression() {
				_rx = new ModUnicodeRegularExpression;
				_cs = new ModCriticalSection;
			}
			RegularExpression(ModUnicodeRegularExpression* rx_, ModCriticalSection* cs_)
				 : _rx(rx_), _cs(cs_) {}
			ModAutoPointer<ModUnicodeRegularExpression> _rx;
			ModAutoPointer<ModCriticalSection> _cs;
		};
	} // end of namespace Type

}

#endif // __UNA_LIBUNA_TYPE_H

//
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
