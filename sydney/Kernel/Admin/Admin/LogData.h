// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.h -- Adminモジュール用の論理ログ
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_LOGDATA_H
#define	__SYDNEY_ADMIN_LOGDATA_H

#include "Admin/Module.h"
#include "Trans/LogData.h"

#include "Admin/Externalizable.h"

#include "ModArchive.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_ADMIN_BEGIN
_SYDNEY_ADMIN_LOG_BEGIN

//
//	CLASS
//	Admin::Log::ReplicationEndData -- 
//
//	NOTES
//
class ReplicationEndData : public Trans::Log::Data
{
	typedef Trans::Log::Data	Super;
	
public:
	// コンストラクタ
	ReplicationEndData();
	// デストラクタ
	~ReplicationEndData();

	// このクラスをシリアル化する
	void serialize(ModArchive& archiver_);
	// クラスIDを得る
	int getClassID() const
	{
		return
			Admin::Externalizable::Category::ReplicationEndLogData +
			Common::Externalizable::AdminClasses;
	}

	// スレーブで実行中のトランザクションの開始を表す論理ログの LSN を追加する
	void pushBackBeginLSN(Trans::Log::LSN lsn_)
		{ m_vecBeginLSN.pushBack(lsn_); }
	// スレーブで実行中のトランザクションの開始を表す論理ログの LSN の
	// 配列を設定する
	void setBeginLSN(const ModVector<Trans::Log::LSN>& beginLSN_)
		{ m_vecBeginLSN = beginLSN_; }
	// スレーブで実行中のトランザクションの開始を表す論理ログの LSN の配列を得る
	const ModVector<Trans::Log::LSN>& getBeginLSN() const
		{ return m_vecBeginLSN; }

	// 最後に受け取ったマスター側の論理ログの LSN を設定する
	void setLastMasterLSN(Trans::Log::LSN lsn_)
		{ m_ulLastMasterLSN = lsn_; }
	// 最後に受け取ったマスター側の論理ログの LSN を得る
	Trans::Log::LSN getLastMasterLSN() const
		{ return m_ulLastMasterLSN; }

private:
	// スレーブで実行中のトランザクションの開始を表す論理ログの LSN の配列
	ModVector<Trans::Log::LSN> m_vecBeginLSN;
	// 最後に受け取ったマスター側の論理ログの LSN
	Trans::Log::LSN m_ulLastMasterLSN;
};

_SYDNEY_ADMIN_LOG_END	
_SYDNEY_ADMIN_END
_SYDNEY_END

#endif	// __SYDNEY_ADMIN_LOGDATA_H

//
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
