// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.h -- 
// 
// Copyright (c) 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_LOGICALINTERFACE_H
#define __SYDNEY_ARRAY_LOGICALINTERFACE_H

#include "Array/Module.h"
#include "Array/FileID.h"

#include "LogicalFile/File.h"
#include "LogicalFile/ObjectID.h"
#include "LogicalFile/OpenOption.h"

#include "Common/Data.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"

#include "Trans/Transaction.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

class ArrayFile;
class Condition;

//
//	CLASS
//	Array::LogicalInterface -- Btreeファイルの論理ファイルクラス
//
//	NOTES
//
class SYD_ARRAY_FUNCTION LogicalInterface : public LogicalFile::File
{
public:
	// コンストラクタ
	LogicalInterface(const LogicalFile::FileID& cFileID_);
	// デストラクタ
	virtual ~LogicalInterface();

	//
	//	Schema Information
	//

	// ファイルIDを返す
	const LogicalFile::FileID& getFileID() const;

	// レコードファイルサイズを返す
	ModUInt64 getSize(const Trans::Transaction& cTrans_);

	// Get the count of the tuples. NOT the entries.
	ModInt64 getCount() const;

	//
	//	Query Optimization
	//

	// オブジェクト検索時のオーバヘッドを返す
	double getOverhead() const;

	// オブジェクトへアクセスする際のプロセスコストを返す
	double getProcessCost() const;

	// 検索オープンパラメータを設定する
	bool getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
							LogicalFile::OpenOption& cOpenOption_) const;

	// プロジェクションオープンパラメータを設定する
	bool getProjectionParameter(const Common::IntegerArrayData&	cProjection_,
								LogicalFile::OpenOption& cOpenOption_) const;

	// 更新オープンパラメータを設定する
	bool getUpdateParameter(const Common::IntegerArrayData&	cUpdateFields_,
							LogicalFile::OpenOption& cOpenOption_) const;

	// ソート順パラメータを設定する
	bool getSortParameter(const Common::IntegerArrayData& cKeys_,
						  const Common::IntegerArrayData& cOrders_,
						  LogicalFile::OpenOption& cOpenOption_) const;

	//
	//	Data Manipulation
	//

	// レコードファイルを生成する
	const LogicalFile::FileID& create(const Trans::Transaction& cTransaction_);

	// レコードファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool bForce_ = false) const;
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& cTransaction_) const;

	// レコードファイルをオープンする
	void open(const Trans::Transaction&	cTransaction_,
			  const LogicalFile::OpenOption& cOpenOption_);

	// レコードファイルをクローズする
	void close();

	// オープンされているか
	bool isOpened() const;
	
	// 検索条件を設定する
	void fetch(const Common::DataArrayData* pOption_);

	// 挿入されているオブジェクトを返す
	bool get(Common::DataArrayData* pTuple_);

	// オブジェクトを挿入する
	void insert(Common::DataArrayData* pTuple_);

	// オブジェクトを更新する
	void update(const Common::DataArrayData* pKey_,
				Common::DataArrayData* pNewKey_);

	// オブジェクトを削除する
	void expunge(const Common::DataArrayData* pKey_);

	// 巻き戻しの位置を記録する
	void mark();

	// 記録した位置に戻る
	void rewind();

	// カーソルをリセットする
	void reset();

	// 比較
	bool equals(const Common::Object* pOther_) const;

	// 同期を取る
	void sync(const Trans::Transaction& cTransaction_,
			  bool& bIncomplete_, bool& bModified_);

	//
	// Utiliry
	//

	// ファイルを移動する
	void move(const Trans::Transaction&	cTransaction_,
			  const Common::StringArrayData& cArea_);

	// ラッチが不要なオペレーションを返す
	Operation::Value getNoLatchOperation();

	// Capabilities of file driver
	Capability::Value getCapability();

	// ファイルを識別するための文字列を返す
	ModUnicodeString toString() const;

	//
	// 運用管理のためのメソッド
	//

	// レコードファイルをマウントする
	const LogicalFile::FileID& mount(const Trans::Transaction& cTransaction_);
	// レコードファイルをアンマウントする
	const LogicalFile::FileID& unmount(const Trans::Transaction& cTransaction_);

	// レコードファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// レコードファイルに対してバックアップ開始を通知する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);

	// レコードファイルに対してバックアップ終了を通知する
	void endBackup(const Trans::Transaction& cTransaction_);

	// レコードファイルを障害回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	//
	// 整合性検査のためのメソッド
	//

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const unsigned int	uiTreatment_,
				Admin::Verification::Progress& cProgress_);

	// 以下publicであるが、外部には公開していないメソッド

	// Fileをattachする
	void attachFile();
	// Fileをdetachする
	void detachFile();
	// Fileがattachされているかどうか
	bool isAttached() const;

	// 全ページの変更を破棄する
	void recoverAllPages();
	// 全ページの変更を確定する
	void flushAllPages();

	// データベースを利用不可にする
	void setNotAvailable();

private:
	// 挿入されているオブジェクト数を返す
	ModInt64 getCount();

	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOptnMode_);
	// 実際にファイルを作成する
	void substantiate();

	// バッチモードの時必要に応じてページをdetachする
	void detachPageForBatch();

	// Get the tuple.
	bool getForVerify(Common::DataArrayData* pTuple_);
	// Get the RowID by BitSet.
	bool getByBitSet(Common::DataArrayData* pBitset_);
	// Get one RowID.
	bool getSingly(Common::DataArrayData* pData_);

	// ファイルID
	FileID m_cFileID;
	// Arrayファイル
	ArrayFile* m_pArrayFile;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// オープンモード
	LogicalFile::OpenOption::OpenMode::Value m_eOpenMode;

	// 検索条件
	ModVector<Condition*> m_vecpCondition;
	// 現在処理している検索条件
	ModVector<Condition*>::Iterator m_iterator;
	// 更新するフィールド
	ModVector<int> m_vecFieldID;
	// ビットセットで得るか
	bool m_bGetByBitSet;
	// 初めてのgetかどうか
	bool m_bFirstGet;
	// 直前に取得したタプルID
	unsigned int m_uiTupleID;
	// The TupleID for verify
	unsigned int m_uiVerifyTupleID;
	// 取得したタプルID
	Common::BitSet m_cTupleBit;
	// 一括更新の高速化のため
	Common::DataArrayData m_cResult;
	// SearchByBitSet
	Common::BitSet* m_pSearchByBitSet;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif // __SYDNEY_ARRAY_LOGICALINTERFACE_H

//
//	Copyright (c) 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
