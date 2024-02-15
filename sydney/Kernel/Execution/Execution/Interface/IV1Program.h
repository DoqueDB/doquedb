// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IV1Program.h --
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IV1PROGRAM_H
#define __SYDNEY_EXECUTION_INTERFACE_IV1PROGRAM_H

#include "Execution/Interface/Module.h"
#include "Execution/Program.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Interface::IV1Program -- Interface for old executor program
//
//	NOTES
class IV1Program
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef IV1Program This;

	// destructor
	virtual ~IV1Program() {}

	//実行するプランを設定する
	virtual void setPlan(const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan_,
						 const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder_,
						 bool bUpdate_, Trans::Transaction& cTrans_, Schema::Database* pDatabase_,
						 bool bPreparing_ = false) = 0;

	// Prepareで作ったものから設定する
	virtual void setPlan(const This* pSource_, Trans::Transaction& cTrans_) = 0;

	// 実行するプランがあるか
	virtual bool isEmpty() const = 0;
	// Check is there any place holder
	virtual bool isHasParameter() const = 0;

	// 実際に実行するプランを設定する
	virtual void determine() = 0;
	// 実際に実行するプランを追加する
	virtual void addDeterminedPlan(const Plan::RelationInterfacePointer& pPlan_) = 0;

	//実行するプランの数を得る
	virtual int getPlanSize() const = 0;

	//プランをひとつ得る
	virtual Plan::RelationInterfacePointer getPlan(int iPos_) const = 0;

	// パラメーターの値を設定する
	virtual void assignParameter(const Common::DataArrayData* pData_) = 0;

	// Connectionを設定する
	virtual void setConnection(Communication::Connection* pConnection_) = 0;
	// 設定されているConnectionを得る
	virtual Communication::Connection* getConnection() const = 0;

	// 初期化処理をする
	virtual void initialize() = 0;
	// 後処理をする
	virtual void terminate(bool bForce_ = false) = 0;

	// 正常終了したことをセットする
	virtual void succeeded() = 0;

	// アクセサ
	virtual Trans::Transaction* getTransaction() = 0;
	virtual bool isPreparing() const = 0;
	virtual const Plan::ColumnInfoSet& getParameterInfo() const = 0;

	virtual void setDelayedRetrieval(bool bFlag_) = 0;
	virtual bool isDelayedRetrieval() const = 0;

protected:
	// constructor
	IV1Program() : Super() {}

private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IV1PROGRAM_H

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
