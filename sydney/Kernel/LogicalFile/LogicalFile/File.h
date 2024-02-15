// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- 論理ファイルの基底クラス
// 
// Copyright (c) 1999, 2001, 2002, 2003, 2005, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_FILE_H
#define __SYDNEY_LOGICALFILE_FILE_H

#include "LogicalFile/Module.h"
#include "Common/ExecutableObject.h"
#include "Common/DataType.h"

#include "Admin/Verification.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Common
{
class Data;
class IntegerArrayData;
class StringArrayData;
}

namespace Trans
{
class Transaction;
class TimeStamp;
}

_SYDNEY_LOGICALFILE_BEGIN

class TreeNodeInterface;
class FileID;
class OpenOption;
class Locator;

//
//	CLASS
//	LogicalFile::File -- 論理ファイル基底クラス
//
//	NOTES
//	各論理ファイルの共通基底クラス
//
class SYD_LOGICALFILE_FUNCTION File : public Common::ExecutableObject
{
public:
	// Bit masks for operations which require Latch
	struct Operation
	{
		typedef unsigned int Value;
		enum _Value
		{
			None			= 0,
			Open			= 1,
			Close			= Open << 1,
			Reset			= Close << 1,
			GetProcessCost	= Reset << 1,
			GetOverhead		= GetProcessCost << 1,
			Fetch			= GetOverhead << 1,
			GetData			= Fetch << 1,
			Update			= GetData << 1,
		};
	};
	// Bit masks for capabilities of each file driver
	struct Capability
	{
		typedef unsigned int Value;
		enum _Value
		{
			None			= 0,
			Undo			= 1,		// undo expunge/update by driver
			EstimateCount	= 1 << 1	// estimate count using condition
		};
	};

	//コンストラクタ
	File();
	//デストラクタ
	virtual ~File();

	// 実体である OS ファイルが存在するか調べる
	virtual bool
	isAccessible( bool force = false ) const = 0;
	// マウントされているか調べる
	virtual bool
	isMounted(const Trans::Transaction& trans) const = 0;

	// 初期化/終了処理
	virtual void initializeInstance();
	virtual void terminateInstance();

	//
	//	Schema Information
	//
	//ファイルIDを得る
	virtual const LogicalFile::FileID& getFileID() const = 0;
	//論理ファイルサイズを得る
	virtual ModUInt64 getSize() const; // 下のgetSizeをオーバーライドしない場合はこちらを実装する
	virtual ModUInt64 getSize(const Trans::Transaction& cTrans_);
	//論理ファイルに挿入されているタプル数を得る
	virtual ModInt64 getCount() const = 0;

	//
	//	Query Optimization
	//

	//論理ファイルオープン時のオーバヘッドコストを得る
	virtual double getOverhead() const = 0;
	//ひとつのタプルを挿入or取得する際のプロセスコストを得る
	virtual double getProcessCost() const = 0;
	//検索オープンパラメータを得る
	virtual bool getSearchParameter(
		const LogicalFile::TreeNodeInterface*	pCondition_,
		LogicalFile::OpenOption&				cOpenOption_) const = 0;
	
	//プロジェクションオープンパラメータを得る
	virtual bool getProjectionParameter(
		const LogicalFile::TreeNodeInterface*	pNode_,
		LogicalFile::OpenOption&				cOpenOption_) const;
	//こちらは旧インターフェースで、利用されていない
	virtual bool getProjectionParameter(
		const Common::IntegerArrayData&	cProjection_,
		LogicalFile::OpenOption&		cOpenOption_) const;

	//更新オープンパラメータを得る
	virtual bool getUpdateParameter(
		const Common::IntegerArrayData&	cUpdateFields_,
		LogicalFile::OpenOption&		cOpenOption_) const = 0;
	
	//ソート順パラメータを設定する
	virtual bool getSortParameter(
		const LogicalFile::TreeNodeInterface*	pNode_,
		LogicalFile::OpenOption&				cOpenOption_) const;
	//こちらは旧インターフェースで、利用されていない
	virtual bool getSortParameter(
		const Common::IntegerArrayData&	cKeys_,
		const Common::IntegerArrayData&	cOrders_,
		LogicalFile::OpenOption&		cOpenOption_) const;
	
	//取得数と取得位置を設定する
	virtual bool getLimitParameter(
		const Common::IntegerArrayData&	cSpec_,
		LogicalFile::OpenOption&		cOpenOption_) const;

	//
	//	Data Manipulation
	//

	//論理ファイルを作成する
	virtual const LogicalFile::FileID& create(const Trans::Transaction& cTransaction_) = 0;
	//論理ファイルを破棄する
	virtual void destroy(const Trans::Transaction& cTransaction_) = 0;

	//論理ファイルをマウントする
	virtual const LogicalFile::FileID& mount(const Trans::Transaction& cTransaction_) = 0;
	//論理ファイルをアンマウントする
	virtual const LogicalFile::FileID& unmount(const Trans::Transaction& cTransaction_) = 0;
	//論理ファイルをフラッシュする
	virtual void flush(const Trans::Transaction& cTransaction_) = 0;

	//論理ファイルのバックアップを開始する
	virtual void startBackup(const Trans::Transaction& cTransaction_,
							 const bool bRestorable_) = 0;
	//論理ファイルのバックアップを終了する
	virtual void endBackup(const Trans::Transaction& cTransaction_) = 0;

	//論理ファイルを障害から回復する
	virtual void recover(const Trans::Transaction& cTransaction_,
						 const Trans::TimeStamp& cPoint_) = 0;
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	virtual void restore(const Trans::Transaction&	cTransaction_,
						 const Trans::TimeStamp&	cPoint_) = 0;

	// 整合性検査を行う
	virtual void verify(const Trans::Transaction&					cTransaction_,
						const Admin::Verification::Treatment::Value	eTreatment_,
						Admin::Verification::Progress&				cProgress_) = 0;

	//論理ファイルをオープンする
	virtual void open(
		const Trans::Transaction&		cTransaction_,
		const LogicalFile::OpenOption&	cOption_) = 0;
	//論理ファイルをクローズする
	virtual void close() = 0;
	//データの取得を行なう
	virtual bool get(Common::DataArrayData* pTuple_) = 0;
	//データの挿入を行なう
	virtual void insert(Common::DataArrayData* pTuple_) = 0;
	//データの更新を行なう
	virtual void update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_) = 0;
	//データの削除を行なう
	virtual void expunge(const Common::DataArrayData* pKey_) = 0;
	//検索条件を設定する
	virtual void fetch(const Common::DataArrayData* pOption_) = 0;
	//巻き戻しの位置を記録する
	virtual void mark() = 0;
	//巻き戻しの位置を記録する
	virtual void rewind() = 0;
	//論理ファイルへのカーソルをリセットする
	virtual void reset() = 0;
	//自分自身との比較
	virtual bool equals(const Common::Object* pOther_) const = 0;
	// 同期を取る
	virtual void
	sync(const Trans::Transaction& trans, bool& incomplete, bool& modified) = 0;
	
	// プロパティを得る
	virtual void getProperty(Common::DataArrayData* pKey_,
							 Common::DataArrayData* pValue_);
	// Locatorを得る
	virtual Locator* getLocator(const Common::DataArrayData* pKey_);
	// データの削除を取り消す
	virtual void undoExpunge(const Common::DataArrayData* pKey_);
	// データの更新を取り消す
	virtual void undoUpdate(const Common::DataArrayData* pKey_);
	// ファイルから不要なデータを削除する
	virtual void compact(const Trans::Transaction& cTransaction_,
						 bool& incomplete, bool& modified);

	//
	//	Utility
	//

	//ファイルを移動する
	virtual void move(
		const Trans::Transaction&		cTransaction_,
		const Common::StringArrayData&	cArea_) = 0;

	// Operations for which latch is not required
	virtual Operation::Value getNoLatchOperation();

	// Capabilities of file driver
	virtual Capability::Value getCapability();
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_FILE_H

//
//	Copyright (c) 1999, 2001, 2002, 2003, 2005, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
