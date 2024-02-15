// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.h --
// 
// Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT_RESULTSET_H
#define __TRMEISTER_CLIENT_RESULTSET_H

#include "Client/Module.h"
#include "Client/Object.h"

#include "Common/DataArrayData.h"
#include "Common/ResultSetMetaData.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT_BEGIN

class Port;
class DataSource;

//
//	CLASS
//	Client::ResultSet --
//
//	NOTES
//
//
class SYD_FUNCTION ResultSet : public Object
{
public:
	//ステータス
	struct Status
	{
		enum Value
		{
			Undefined = 0,	//不明

			Data,			//データ
			EndOfData,		//データ終了
			Success,		//OKステータス
			Canceled,		//キャンセルされた
			Error,			//エラー発生
			MetaData		//結果集合メタデータ
		};
	};

	//コンストラクタ
	ResultSet(DataSource& cDataSource_, int iWorkerID_, Port* pPort_);
	//デストラクタ
	virtual ~ResultSet();

	//結果集合をクローズする
	void close();

	//ステータスを得る
	//	実行結果のステータスのみを返す。タプルデータがあっても読み捨てられる
	//	insert文やdelete文等のタプルを返さないSQL文用
	Status::Value getStatus();

	//次のタプルを得る
	//	タプルデータであればタプルデータを返し、
	//	ステータスであればステータスを返す。
	//	select文等のタプルを返すSQL文用であるが、
	//	タプルを返さないものでも問題はない。
	//	プロトコルバージョン2以上の場合は、更新系のSQL文でも結果が返る。
	//	更新系の場合は、影響を及ぼした行のROWIDが返ってくる。
	Status::Value getNextTuple(Common::DataArrayData*& pTuple_);

	//タプルのインスタンスを開放する
	static void releaseTuple(Common::DataArrayData* pTuple_);

	//実行をキャンセルする
	void cancel();

	//結果集合メタデータを得る
	const Common::ResultSetMetaData* getMetaData() const;

private:
	//データソース
	DataSource& m_cDataSource;
	//WorkerID
	int m_iWorkerID;
	//Port
	Port* m_pPort;

	//現在のステータス
	Status::Value m_eStatus;

	//結果集合メタデータ
	Common::ResultSetMetaData* m_pMetaData;
};

_TRMEISTER_CLIENT_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT_RESULTSET_H

//
//	Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
