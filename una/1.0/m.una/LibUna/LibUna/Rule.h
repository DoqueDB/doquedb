// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Rule.h -- Rule クラスの定義ファイル
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

#ifndef __RULE__HEADER__
#define __RULE__HEADER__

#include "LibUna/Module.h"
#include "LibUna/SmartPointer.h"
#include "LibUna/AutoArrayPointer.h"
#include "LibUna/Data.h"
#include "ModVector.h"

_UNA_BEGIN

//
//	CLASS
//		Rule -- ルールクラス
//
	class Rule
	{
	public:
		// TYPEDEF
		//		Command -- コマンド識別子
		//
		// MEMO
		//		各派生クラスがコマンドを定義できるように共通の型を識別子とする。
		//		enum で持つと管理が大変。
		typedef		int		Command;

		//	ENUM
		//		Step
		class Step {
		public:
			enum Type {
				Unknown = -1,
				ValueName = 0,
				Operate,
				RuleType,
				Option,
				Arg1,
				Arg2,
				Max
			};
		};

		// ENUM
		//		RuleCommand -- command type
		enum RuleCommand {
				Collect ,			// generate keyword
				DeleteFromWhole,	// delete keyword (from whole)
				DeleteFromBlock,	// delete keyword (from block)
		//		DeleteFromWholeDic,	// delete keyword using dictionary (from whole)
		//		DivProcess,			// divide process
				Group,				// group
				Language, 			// language set
				MaximumKeyword,		// set keyword maximum number
				PickupFromBlock,	// pickup keyword candidate (from block)
				PickupFromWhole,	// pickup keyword candidate (from whole)
				SameType,			// same part of speech gropu setting
				//Score,			// score adjust
				//SentenceEnd,		// Pause of sentence
				//UsingLen,			// target text length limitation
				Version,			// version setting
				DeleteTail,			// delete the word at the end of keyword
				Num
		};

		// コンストラクタ、デストラクタ
		Rule();
		~Rule();

		// TYPEDEF
		//		CommandOption -- コマンドオプション
		//
		// MEMO
		//		コマンドのオプション。Common::Object* の配列なのでコマンド毎に使用できる。
		typedef	AutoArrayPointer< Data::Object > CommandOption;

		// コマンド識別子の設定
		Command	getCommand() const;
		void	setCommand(Command com_);

		// コマンドオプションの設定
		const CommandOption&	getOption() const;

		// コマンドオプションの追加
		void	pushBackOption(Data::Object* option_);

	protected:
	private:

		// コマンド識別子
		Command			_command;
		// コマンドパラメータ
		CommandOption	_option;
	};
_UNA_END
#endif // __RULE__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
