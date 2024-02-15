// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalInterface.h -- 論理ファイル
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_LOGICALINTERFACE_H
#define __SYDNEY_KDTREE_LOGICALINTERFACE_H

#include "KdTree/Module.h"
#include "KdTree/FileID.h"
#include "KdTree/OpenOption.h"

#include "Buffer/Page.h"
#include "LogicalFile/File.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class KdTreeFile;

//
//	CLASS
//	KdTree::LogicalInterface -- KD-Tree
//
//	NOTES
//
class SYD_KDTREE_FUNCTION LogicalInterface : public LogicalFile::File
{
public:
	// コンストラクタ
	LogicalInterface(const LogicalFile::FileID& cFileID_);
	// デストラクタ
	virtual ~LogicalInterface();

	// 実体である OS ファイルが存在するか調べる
	bool isAccessible(bool force = false) const;
	// マウントされているか調べる
	bool isMounted(const Trans::Transaction& trans) const;

	//
	//  Schema Information
	//

	// ファイルIDを得る
	const LogicalFile::FileID& getFileID() const { return m_cFileID; }
	// 論理ファイルサイズを得る
	ModUInt64 getSize(const Trans::Transaction& cTrans_);
	// 論理ファイルに挿入されているタプル数を得る
	ModInt64 getCount() const;

	//
	//  Query Optimization
	//

	// 論理ファイルオープン時のオーバヘッドコストを得る
	double getOverhead() const;
	// ひとつのタプルを挿入or取得する際のプロセスコストを得る
	double getProcessCost() const;
	// オープンパラメータを得る
	bool getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
							LogicalFile::OpenOption& cOpenOption_) const;
	// プロジェクションオープンパラメータを得る
	bool getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
								LogicalFile::OpenOption& cOpenOption_) const;

	// 更新オープンパラメータを得る
	bool getUpdateParameter(const Common::IntegerArrayData& cUpdateFields_,
							LogicalFile::OpenOption& cOpenOption_) const;
	// ソート順パラメータを設定する
	bool getSortParameter(const LogicalFile::TreeNodeInterface* pNode_,
						  LogicalFile::OpenOption& cOpenOption_) const;
	// 取得数と取得位置を設定する
	bool getLimitParameter(const Common::IntegerArrayData& cSpec_,
						   LogicalFile::OpenOption& cOpenOption_) const;

	//
	//  Data Manipulation
	//

	// 論理ファイルを作成する
	const LogicalFile::FileID& create(const Trans::Transaction& cTransaction_);
	// 論理ファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// 論理ファイルをマウントする
	const LogicalFile::FileID& mount(const Trans::Transaction& cTransaction_);
	// 論理ファイルをアンマウントする
	const LogicalFile::FileID& unmount(const Trans::Transaction& cTransaction_);
	// 論理ファイルをフラッシュする
	void flush(const Trans::Transaction& cTransaction_);

	// 論理ファイルのバックアップを開始する
	void startBackup(const Trans::Transaction& cTransaction_,
					 const bool bRestorable_);
	// 論理ファイルのバックアップを終了する
	void endBackup(const Trans::Transaction& cTransaction_);

	// 論理ファイルを障害から回復する
	void recover(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);
	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction& cTransaction_,
				 const Trans::TimeStamp& cPoint_);

	// 整合性検査を行う
	void verify(const Trans::Transaction& cTransaction_,
				const Admin::Verification::Treatment::Value eTreatment_,
				Admin::Verification::Progress& cProgress_);

	// 論理ファイルをオープンする
	void open(const Trans::Transaction& cTransaction_,
			  const LogicalFile::OpenOption& cOption_);
	// 論理ファイルをクローズする
	void close();
	// データの取得を行なう
	bool get(Common::DataArrayData* pTuple_);
	// データの挿入を行なう
	void insert(Common::DataArrayData* pTuple_);
	// データの更新を行なう
	void update(const Common::DataArrayData* pKey_,
				Common::DataArrayData* pValue_);
	// データの削除を行なう
	void expunge(const Common::DataArrayData* pKey_);
	// 検索条件を設定する
	void fetch(const Common::DataArrayData* pOption_);
	// 巻き戻しの位置を記録する
	void mark();
	// 巻き戻す
	void rewind();
	// 論理ファイルへのカーソルをリセットする
	void reset();
	
	//
	//  Utility
	//

	// 自分自身との比較
	bool equals(const Common::Object* pOther_) const;
	// 同期を取る
	void sync(const Trans::Transaction& cTransaction_,
			  bool& bIncomplete_, bool& bModified_);
	// ファイルを移動する
	void move(const Trans::Transaction& cTransaction_,
			  const Common::StringArrayData& cArea_);
	// ラッチが不要なオペレーションを返す
	Operation::Value getNoLatchOperation();
	// Capabilities of file driver
	Capability::Value getCapability();
	// 文字列表現を得る
	ModUnicodeString toString() const;

	//
	//	_AutoAttachFile のためのインターフェース
	//

	// ファイルを attach する
	void attach();
	// ファイルを detach する
	void detach();
	// ファイルが attach されているかを調べる
	bool isAttached() const { return (m_pKdTreeFile != 0); }

	// バッチモードかどうか
	bool isBatch() const { return m_bBatch; }

	// すべてのファイルのページを確定する
	void flushAllPages();
	// すべてのファイルのページを破棄する
	void recoverAllPages();
	
private:
	// キー値を取り出す
	void getKey(const Common::DataArrayData* pTuple_,
				int iElement_,
				ModVector<float>& vecKey_);
	// バリューを取り出す
	void getValue(const Common::DataArrayData* pTuple_,
				  int iElement_,
				  ModUInt32& uiValue_);

	// ファイルID
	FileID m_cFileID;

	// KD木ファイル
	KdTreeFile* m_pKdTreeFile;

	// 検索条件
	ModVector<ModVector<float> > m_vecCondition;
	// 探索タイプ
	Node::TraceType::Value m_eTraceType;
	// 最大計算回数
	int m_iMaxCalculateCount;
	// 結果件数
	ModSize m_uiLimit;
	// プロジェクション
	ModVector<OpenOption::Projection::Value> m_vecProjection;
	
	// 検索結果
	ModVector<ModVector<ModPair<ModUInt32, double> > > m_vecResult;
	// 現在位置
	ModVector<ModVector<ModPair<ModUInt32, double> > >::Iterator m_ite;
	// 現在の位置の位置
	ModSize m_uiInnerPosition;
	
	// トランザクション
	const Trans::Transaction* m_pTransaction;
	// FIXモード
	Buffer::Page::FixMode::Value m_eFixMode;

	// 最初かどうか
	bool m_bFirst;
	// バッチかどうか
	bool m_bBatch;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_LOGICALINTERFACE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
