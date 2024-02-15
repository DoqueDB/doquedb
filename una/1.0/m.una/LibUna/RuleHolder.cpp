// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	RuleHolder.cpp -- Implement file of RuleHolder class
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

#include "LibUna/DicSet.h"
#include "LibUna/RuleHolder.h"
#include "ModAutoMutex.h"
#include "ModCriticalSection.h"
#include "ModMap.h"

#include "UnaReinterpretCast.h"

_UNA_USING

namespace Local {

	// プロセス内で唯一の Rule クラス格納庫
	RuleHolder::Holder		_holder;

	// _holder のクリティカルセクション
	ModCriticalSection		_holder_cs;

}

//
// FUNCTION pubic
// RuleHolder::addRule  -- Rule クラス
//
// NOTES
//
// ARGUMENTS
//	DicSet *dicSet_
//	ModuleID::Value id_
//		モジュールID
//	ModAutoPointer<Rule>& rule_
//		Rule class
//
// RETURN
//	void
//
// EXCEPTIONS
//
void
RuleHolder::addRule( DicSet *dicSet_, ModAutoPointer<Rule>& rule_)
{
	try {
		ModAutoMutex<ModCriticalSection> mutex(&Local::_holder_cs);
		mutex.lock();

		// 既存のルールがあるか
		RuleID ruleID = dicSet_->getRuleID();

		// holder 内の検索
		Holder::Iterator it = Local::_holder.find(ruleID);

		if ( it != Local::_holder.end() ) {

			// 既存ルールがあったのでルールコンテナに追加する
			//   追加の際は rule ポインタの管理を止める
			((*it).second)->insert(((*it).second)->end(), rule_.release());

		} else {

			// 既存ルールが無いのでルールコンテナを作成して追加する
			ModAutoPointer<RuleContainer> tmp = new RuleContainer;

			// rule ポインタの管理を止める
			tmp->pushBack(rule_.release());

			// insert
			Local::_holder.insert(ruleID, tmp.release());
		}

	} catch ( ModException& exp ) {
		ModErrorMessage << "rule can't add...[info:"
			<< exp << "]" << ModEndl;
		throw;
	}
}

//
// FUNCTION static
// RuleHolder::getRule  -- Rule class get
//
// NOTES
//
// ARGUMENTS
//	DicSet *dicSet_
//	ModuleID::Value id_
//		モジュール毎に割り当てられたID
//
// RETURN
//	const ModVector< ModAutoPointer< Rule > >&
//		rule class
//
// EXCEPTIONS
//
const ModVector<Rule*>&
RuleHolder::getRule( DicSet *dicSet_)
{
	ModAutoMutex<ModCriticalSection> mutex(&Local::_holder_cs);
	mutex.lock();

	// find rule in rule holder
	RuleID ruleID = dicSet_->getRuleID();

	Holder::Iterator it = Local::_holder.find(ruleID);

	if ( it != Local::_holder.end() ) {
		return *((*it).second);
	}

	// ルールが発見できず
	ModThrow(ModModuleStandard,
			 ModCommonErrorBadArgument,
			 ModErrorLevelError);

	// not found
	return *(una_reinterpret_cast<const ModVector<Rule*>* >((void*)0));
}

//
// FUNCTION static
// RuleHolder::isRule -- Rule class exit check
//
// NOTES
//	指定した言語、モジュール ID のルールが登録されているか調べる
//
// ARGUMENTS
//	DicSet *dicSet_
//		set dicSet
// 	ModuleID::Value id_
// 		モジュール毎に割り当てられたID
//
// RETURN
// 	ModBoolean
// 	rule exist or not
//
// EXCEPTIONS
//
ModBoolean
RuleHolder::isRule(DicSet *dicSet_)
{
	ModAutoMutex<ModCriticalSection> mutex(&Local::_holder_cs);
	mutex.lock();

	RuleID ruleID = dicSet_->getRuleID();

	if ( Local::_holder.find(ruleID) != Local::_holder.end() )
		return ModTrue;

	return ModFalse;
}

ModCriticalSection*
RuleHolder::getLock()
{
	return &Local::_holder_cs;
}

//
// Copyright (c) 2004-2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
