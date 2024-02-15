// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSet.h -- 結果集合関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_CLIENT2_RESULTSET_H
#define __TRMEISTER_CLIENT2_RESULTSET_H

#include "Client2/Module.h"
#include "Client2/Object.h"

#include "Common/DataArrayData.h"
#include "Common/ResultSetMetaData.h"

_TRMEISTER_BEGIN
_TRMEISTER_CLIENT2_BEGIN

class Port;
class DataSource;

//	CLASS
//	Client2::ResultSet -- 結果集合クラス
//
//	NOTES

class SYD_FUNCTION ResultSet : public Object
{
public:

	// ステータス
	struct Status {

		enum Value {

			Undefined = 0,	// 不明なステータス

			Data,			// データである
			EndOfData,		// データ終了である
			Success,		// 正常終了
			Canceled,		// キャンセルされた
			Error,			// エラーが発生した
			MetaData,		// 結果集合メタデータ
			HasMoreData		// (複文で)続きのデータがある
		};
	};

	// コンストラクタ
	ResultSet(	DataSource&	cDataSource_,
				Port*		pPort_);

	// デストラクタ
	virtual ~ResultSet();

	// クローズする
	void close();

	// ステータスを得る
	Status::Value getStatus(bool bSkipAll_ = true);

	// 次のタプルを得る
	Status::Value getNextTuple(Common::DataArrayData*	pTuple_);

	// getNextTuple の後始末
	void terminateGetNextTuple();

	// 実行をキャンセルする
	void cancel();

	// ResultSetMetaData を得る
	const Common::ResultSetMetaData* getMetaData() const;

private:

	// メタデータから適切なデータ型が格納されたDataArrayDataを得る
	Common::DataArrayData* createTupleData();

	// データソース
	DataSource&					m_cDataSource;

	// 通信ポート
	Port*						m_pPort;

	// 結果集合メタデータ
	Common::ResultSetMetaData*	m_pMetaData;

	// 一行のデータ
	Common::DataArrayData*		m_pTupleData;

	// ステータス
	Status::Value				m_eStatus;
};

_TRMEISTER_CLIENT2_END
_TRMEISTER_END

#endif //__TRMEISTER_CLIENT2_RESULTSET_H

//
//	Copyright (c) 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
