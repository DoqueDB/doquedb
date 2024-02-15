// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.h -- 
// 
// Copyright (c) 2005, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_LOGICALINTERFACE_H
#define __SYDNEY_BITMAP_LOGICALINTERFACE_H

#include "Bitmap/Module.h"
#include "Bitmap/FileID.h"
#include "Bitmap/QueryNode.h"
#include "Bitmap/Condition.h"

#include "LogicalFile/File.h"
#include "LogicalFile/ObjectID.h"
#include "LogicalFile/OpenOption.h"

#include "Common/Data.h"
#include "Common/BitSet.h"

#include "Trans/Transaction.h"

#include "ModMap.h"
#include "ModVector.h"
#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class BitmapFile;
class Condition;

//
//	CLASS
//	Bitmap::LogicalInterface -- Bitmapファイルの論理ファイルクラス
//
//	NOTES
//
class SYD_BITMAP_FUNCTION LogicalInterface : public LogicalFile::File
{
public:
	// 比較クラス
	class _Less
	{
	public:
		ModBoolean operator () (const Common::Data::Pointer& p1,
								const Common::Data::Pointer& p2)
			{
				return (p1->compareTo(p2.get()) < 0) ? ModTrue : ModFalse;
			}
	};

	// キー値とビットセットのマップ
	typedef ModMap<Common::Data::Pointer,
				   Common::ObjectPointer<Common::BitSet>,
				   _Less> BitSetMap;

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

	// 挿入されているオブジェクト数を返す
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
	bool getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
								LogicalFile::OpenOption& cOpenOption_) const;
	
	// 更新オープンパラメータを設定する
	bool getUpdateParameter(const Common::IntegerArrayData&	cUpdateFields_,
							LogicalFile::OpenOption& cOpenOption_) const;

	//ソート順パラメータを設定する
	bool getSortParameter(const LogicalFile::TreeNodeInterface*	pNode_,
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
	
	// 検索条件 (オブジェクトID) を設定する
	void fetch(const Common::DataArrayData* pOption_);

	// 挿入されているオブジェクトを返す
	bool get(Common::DataArrayData* pTuple_);

	// オブジェクトを挿入する
	void insert(Common::DataArrayData* pTuple_);

	// オブジェクトを更新する
	void update(const Common::DataArrayData* pKey_,
				Common::DataArrayData* pTuple_);

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
	// Capabilityを返す
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
	// オープンする
	void open(const Trans::Transaction& cTransaction_,
			  LogicalFile::OpenOption::OpenMode::Value eOptnMode_);
	// 実際にファイルを作成する
	void substantiate();

	// 通常のgetのための検索
	void searchNormal(Common::BitSet* pBitSet_);
	// vrifyのgetのための検索
	void searchVerify();
	// group by のgetのための検索
	void searchGroupBy();

	// 検索結果件数見積り
	ModSize getEstimateCount();

	// group by のタプルを設定する
	void setTupleGroupBy(Common::DataArrayData* pTuple_);

	// ファイルID
	FileID m_cFileID;
	// Bitmapファイル
	BitmapFile* m_pBitmapFile;

	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// オープンモード
	LogicalFile::OpenOption::OpenMode::Value m_eOpenMode;

	// 検索条件
	ModUnicodeString m_cstrCondition;
	// ROWID
	ModUInt32 m_uiRowID;

	// GetByBitSetではないとき、または、group by で検索条件が指定されているとき
	// に結果を保持するBitSet
	Common::BitSet m_cBitSet;
	Common::BitSet::ConstIterator m_ite;

	// SearchByBitSetで指定されたBitSet
	const Common::BitSet* m_pNarrowingBitSet;

	// group by の結果を格納するマップ
	BitSetMap m_cBitSetMap;
	// group by の結果を格納するマップへのイテレータ
	BitSetMap::Iterator m_bitIte;

	// ビットセットで得るか
	bool m_bGetByBitSet;
	// 初めてのgetかどうか
	bool m_bFirstGet;
	// verify時の検索かどうか
	bool m_bVerify;

	// group by かどうか
	bool m_bGroupBy;
	// group by のソート順
	bool m_bIsAsc;
	// group by の時にキー値も取得するかどうか
	bool m_bGetKey;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_LOGICALINTERFACE_H

//
//	Copyright (c) 2005, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
