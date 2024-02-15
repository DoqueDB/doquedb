// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- 可変長レコードファイルクラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_FILE_H
#define __SYDNEY_RECORD_FILE_H

#include "LogicalFile/File.h"
#include "LogicalFile/ObjectID.h"
#include "PhysicalFile/Types.h"

#include "Record/Module.h"
#include "Record/Tools.h"
#include "Record/DirectFile.h"
#include "Record/MetaData.h"
#include "Record/OpenParameter.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
	class OpenOption;
	class TreeNodeInterface;
}

namespace Common
{
class DataArrayData;
class StringArrayData;
}

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
}

_SYDNEY_RECORD_BEGIN

class FreeAreaManager;
class FileInformation;
class DirectIterator;
class TargetFields;
class VariableDataObjectInformation;
class VariableFile;
class UseInfo;

//
//	CLASS
//	Record::File -- 可変長レコードファイルクラス
//
//	NOTES
//	可変長レコードファイルクラス
//
class SYD_RECORD_FUNCTION_TESTEXPORT File : public LogicalFile::File
{
public:

	// コンストラクタ
	File(const LogicalFile::FileID&	cFileID_);

	// デストラクタ
	~File();

	// 初期化処理/終了処理
	static void initialize();
	static void terminate();

	//
	//	Schema Information
	//

	// ファイルIDを返す
	const LogicalFile::FileID& getFileID() const;

	// レコードファイルサイズを返す
	ModUInt64 getSize() const;

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
	bool
		getSearchParameter(
			const LogicalFile::TreeNodeInterface*	pCondition_,
			LogicalFile::OpenOption&				cOpenOption_) const;

	// プロジェクションオープンパラメータを設定する
	bool
		getProjectionParameter(
			const Common::IntegerArrayData&	cProjection_,
			LogicalFile::OpenOption&		cOpenOption_) const;

	// 更新オープンパラメータを設定する
	bool
		getUpdateParameter(
			const Common::IntegerArrayData&	cUpdateFields_,
			LogicalFile::OpenOption&		cOpenOption_) const;

	// ソート順パラメータを設定する
	bool
		getSortParameter(
			const Common::IntegerArrayData&	cKeys_,
			const Common::IntegerArrayData&	cOrders_,
			LogicalFile::OpenOption&		cOpenOption_) const;

	//
	//	Data Manipulation
	//

	// レコードファイルを生成する
	const LogicalFile::FileID&
		create(const Trans::Transaction& cTransaction_);

	// レコードファイルを破棄する
	void destroy(const Trans::Transaction& cTransaction_);

	// 実体である OS ファイルが存在するか調べる
	bool
	isAccessible(bool force = false) const;
	// マウントされているか調べる
	bool
	isMounted(const Trans::Transaction& trans) const;

	// レコードファイルをオープンする
	void open(const Trans::Transaction&			cTransaction_,
			  const LogicalFile::OpenOption&	cOpenOption_);

	// レコードファイルをクローズする
	void close();

	// 検索条件 (オブジェクトID) を設定する
	void fetch(const Common::DataArrayData* pOption_);

	// 挿入されているオブジェクトを返す
	bool get(Common::DataArrayData* pTuple_);

	// オブジェクトを挿入する
	void insert(Common::DataArrayData* pTuple_);

	// オブジェクトを更新する
	void update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_);

	// オブジェクトを削除する
	void expunge(const Common::DataArrayData* pKey_);

	// 巻き戻しの位置を記録する
	void mark();

	// 記録した位置に戻る
	void rewind();

	// カーソルをリセットする
	void reset();

	// 比較
	bool equals(const Common::Object*	pOther_) const;

	// 同期を取る
	void
	sync(const Trans::Transaction& trans, bool& incomplete, bool& modified);

	//
	// Utiliry
	//

	// ファイルを移動する
	void move(const Trans::Transaction&			cTransaction_,
			  const Common::StringArrayData&	cArea_);

	// ラッチが不要なオペレーションを返す
	Operation::Value getNoLatchOperation();

	// ファイルを識別するための文字列を返す
	ModUnicodeString toString() const;

	//
	// 運用管理のためのメソッド
	//

	// レコードファイルをマウントする
	const LogicalFile::FileID& mount(const Trans::Transaction&	Transaction_);
	// レコードファイルをアンマウントする
	const LogicalFile::FileID& unmount(const Trans::Transaction&	Transaction_);

	// レコードファイルをフラッシュする
	void flush(const Trans::Transaction&	Transaction_);

	// レコードファイルに対してバックアップ開始を通知する
	void startBackup(const Trans::Transaction&	Transaction_,
					 const bool					Restorable_);

	// レコードファイルに対してバックアップ終了を通知する
	void endBackup(const Trans::Transaction&	Transaction_);

	// レコードファイルを障害回復する
	void recover(const Trans::Transaction&	Transaction_,
				 const Trans::TimeStamp&	Point_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction&	Transaction_,
				 const Trans::TimeStamp&	Point_);

	//
	// 整合性検査のためのメソッド
	//

	// 整合性検査を行う
	void verify(const Trans::Transaction&		Transaction_,
				const unsigned int				Treatment_,
				Admin::Verification::Progress&	Progress_);

private:

	friend class FileInformation;

	// 親クラスとクラス名が同じ場合の operator= 問題を回避する
	File& operator= (const File&	cObject_);

	// fetch へのオプションからオブジェクト ID への変換
	ModUInt64
		convertFetchOptionToObjectID(const Common::DataArrayData* pOption_) const;

	// 実際に挿入処理を行う
	Tools::ObjectID doInsert(DirectFile::DataPackage& cData_);

	// 実際に更新処理を行う
	// 可変長あり
	void doUpdate(Tools::ObjectID iObjectID_,
				  DirectFile::DataPackage& cNewDirectData_);
	// 可変長なし
	void doUpdate(Tools::ObjectID iObjectID_,
				  DirectFile::DataPackage& cOldObjectHeader_,
				  DirectFile::DataPackage& cNewDirectData_,
				  DirectFile::DataPackage& cNewVariableData_);

	// 実際に削除処理を行う
	void doExpunge(Tools::ObjectID iObjectID_, Tools::ObjectID iVariableID_);

	// オープンオプションをデータメンバに保存する
	void saveOpenOption(const LogicalFile::OpenOption& cOpenOption_ ,int& iOpenModeValue_);

	// フィールドを固定長と可変長に分ける
	void divideTargets(bool bIsUpdate_ = false);

	// 固定長/可変長用のファイルを表すクラスを生成する
	void initializeFiles(const Trans::Transaction& cTrans_);

	//
	// データメンバ
	//

	// プロジェクション情報へのポインタ
	// （全フィールドが指定されている場合は NULL ポインタ）
	TargetFields*						m_pTargetFields;
	// 固定長、可変長それぞれに対するプロジェクション情報
	TargetFields*						m_pDirectTargetFields;
	TargetFields*						m_pVariableTargetFields;
	bool								m_bTargetFieldsAllocated;

	// カーソル
	DirectIterator*						m_pDirectIterator;

	// fetch で指定されたオブジェクトID
	Tools::ObjectID						m_ullFetchObjectID;

	// トランザクション記述子
	const Trans::Transaction*			m_pTransaction;

	// 固定長フィールドを格納したファイルへのポインタ
	// （この値がゼロか否かで、オープンされているのかを判定する）
	DirectFile*							m_pDirectFile;

	// 可変長フィールドを格納したファイルへのポインタ
	// （この値がゼロか否かで、オープンされているのかを判定する）
	VariableFile*						m_pVariableFile;

	// メタデータへのポインタ
	// （メタデータが変化することはないので、意味的には const のはずだが、
	// 　create で LogicalFile::FileID が変化する。他に変化するものはない）
	MetaData							m_MetaData;
	// オープンパラメータへのポインタ
	OpenParameter						m_OpenParam;

	// get()で使用するDataPackage (毎回一時変数で取るのは遅いので...)
	DirectFile::DataPackage				m_DataPackage;

}; // end of class File

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_FILE_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
