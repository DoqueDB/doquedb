// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Program.h -- エグゼキュータが実行するプログラム
// 
// Copyright (c) 1999, 2002, 2004, 2005, 2006, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_PROGRAM_H
#define __SYDNEY_EXECUTION_PROGRAM_H

#include "Execution/Module.h"
#include "Execution/Interface/IProgramBridge.h"

#include "Common/Object.h"

#ifdef USE_OLDER_VERSION
#include "Plan/ObjectSet.h"
#include "Plan/RelationInterface.h"
#include "Plan/TypeDef.h"
#endif

_SYDNEY_BEGIN

namespace Communication
{
	class Connection;
}

_SYDNEY_EXECUTION_BEGIN

//
// CLASS
//	Execution::Program -- 問い合わせ実行プログラム
//
// NOTES
// エグゼキュータが実行する問い合わせ実行プログラム。
//
class SYD_EXECUTION_FUNCTION Program : public Common::Object
{
public:
	typedef Interface::IProgramBridge::Version Version;

	//コンストラクタ
	Program() : m_pImpl(0) {}
	//デストラクタ
	~Program();

	// 実装クラスを設定する
	void setImplementation(Version::Value iVersion_);
	// 実装クラスのバージョンを得る
	Version::Value getImplementationVersion()
	{return m_pImpl ? m_pImpl->getVersion() : Version::Unknown;}

#ifdef USE_OLDER_VERSION
/////////////////////////
// 古いバージョンのI/F

	//実行するプランを設定する
	void setPlan(const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan_,
				 const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder_,
				 bool bUpdate_, Trans::Transaction& cTrans_, Schema::Database* pDatabase_,
				 bool bPreparing_ = false);

	// Prepareで作ったものから設定する
	void setPlan(const Program& cSource_, Trans::Transaction& cTrans_);

	// 実行するプランがあるか
	bool isEmpty() const;
	// Check is there any place holder
	bool isHasParameter() const;

	// 実際に実行するプランを設定する
	void determine();
	// 実際に実行するプランを追加する
	void addDeterminedPlan(const Plan::RelationInterfacePointer& pPlan_);

	//実行するプランの数を得る
	int getPlanSize() const;

	//プランをひとつ得る
	Plan::RelationInterfacePointer getPlan(int iPos_) const;

	// パラメーターの値を設定する
	void assignParameter(const Common::DataArrayData* pData_);

	// Connectionを設定する
	void setConnection(Communication::Connection* pConnection_);
	// 設定されているConnectionを得る
	Communication::Connection* getConnection() const;

	// 初期化処理をする
	void initialize();
	// 後処理をする
	void terminate(bool bForce_ = false);

	// 正常終了したことをセットする
	void succeeded();

	// アクセサ
	Trans::Transaction* getTransaction();
	bool isPreparing() const;
	const Plan::ColumnInfoSet& getParameterInfo() const;

	void setDelayedRetrieval(bool bFlag_);
	bool isDelayedRetrieval() const;
#endif

//////////////////////////////////////////////////////////
// for new interface
	Interface::IProgram* getProgram();
	void setProgram(Interface::IProgram* pProgram_);
	Interface::IProgram* releaseProgram();

private:
	//コピーコンストラクタ(宣言のみ)
	Program(const Program&);
	//代入オペレータ(宣言のみ)
	Program& operator=(const Program&);

	Interface::IProgramBridge* m_pImpl;
};

_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_PROGRAM_H

//
//	Copyright (c) 1999, 2002, 2004, 2005, 2006, 2008, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
