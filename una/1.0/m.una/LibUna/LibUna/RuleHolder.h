// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RuleHolder.h -- RuleHolder の定義ファイル
// 
// Copyright (c) 2004-2008, 2023 Ricoh Company, Ltd.
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

#ifndef __RULEHOLDER__HEADER__
#define __RULEHOLDER__HEADER__

#include "LibUna/AutoArrayPointer.h"
#include "LibUna/AutoMapPointer.h"
#include "LibUna/Rule.h"
#include "ModAutoPointer.h"
#include "ModMap.h"
#include "ModVector.h"
#include "LibUna/Module.h"

_UNA_BEGIN

	//
	//	CLASS
	//		RuleHolder -- Rule 保持クラス
	//
	//	MEMO
	//		言語ごとに Rule を保持するクラス。
	//		言語種別とルール配列をコンテナとして持つ。
	//		インターフェイスだけ存在する
	//		
	class RuleHolder
	{
	public:
		//	TYPEDEF
		//		RuleID -- ルールを識別する為のＩＤ
		//	MEMO
		//		現在は言語種別とモジュールの組み合わせである
		typedef				unsigned int		RuleID;

		// Rule クラスの登録
		static
		void				addRule(DicSet *dicSet_,
									ModAutoPointer<Rule>& rule_);

		// Rule クラスの取得
		static
		const ModVector<Rule*>&		getRule(DicSet *dicSet_);

		// Rule クラスの存在チェック
		static
		ModBoolean			isRule(DicSet *dicSet_);

		//
		// TYPEDEF
		//		RuleContainer -- Rule クラスの格納庫
		//
		// MEMO
		//		現在のところルールは逐次処理を考えているので vector で良い。
		//
		typedef			AutoArrayPointer< Rule > RuleContainer;

		// TYPEDEF
		//		Holder -- 言語毎に格納する Rule 格納庫
		typedef				AutoMapSecondPointer <RuleID, RuleContainer*, ModLess < RuleID > >
							Holder;

		// RuleID を取得する
		static
		RuleID				getRuleID(DicSet *dicSet_);

		// get criticalsection
		static ModCriticalSection* getLock();

	protected:
	private:

	};

_UNA_END
#endif // __RULEHOLDER__HEADER__

//
// Copyright (c) 2004-2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
