// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- ベクタファイルクラスのヘッダファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_FILE_H
#define __SYDNEY_VECTOR_FILE_H

#include "Vector/Module.h"
#include "Vector/FileParameter.h"

#include "LogicalFile/File.h"
#include "LogicalFile/VectorKey.h"
#include "LogicalFile/OpenOption.h"
#include "PhysicalFile/Types.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace LogicalFile
{
class TreeNodeInterface;
}

namespace Common
{
class DataArrayData;
class StringArrayData;
class DateTimeData;
}

namespace PhysicalFile
{
class File;
class Page;
}

_SYDNEY_VECTOR_BEGIN

class FreeAreaManager;
class FileInformation;
class MetaData;
class ObjectIterator;
class OpenParameter;
class PageManager;

//
//	CLASS
//	Vector::File -- 可変長ベクタファイルクラス
//
//	NOTES
//	可変長ベクタファイルクラス
//
class SYD_VECTOR_FUNCTION_TESTEXPORT File : public LogicalFile::File
{
public:

    // ファイルバージョン
    enum Vers
    {
        // 初期バージョン(2001-05-18版)
        Version1 = 0,
		Version2,
        // バージョン数
        VersionNum,
        // 現在のバージョン
        CurrentVersion = VersionNum - 1
    };

	// コンストラクタ
	File(const LogicalFile::FileID&	cFileOption_);

	// デストラクタ
	~File();

	// 初期化処理/終了処理
	static void	initialize();
	static void	terminate();

	//	Schema Information

	// 実体である OS ファイルが存在するか調べる
	bool
	isAccessible( bool Force_ = false ) const;
	// マウントされているか調べる
	bool
	isMounted(const Trans::Transaction& trans) const;

	// ファイルがオープンになっているかどうかを返す
	bool isOpen() const;

	// ファイルIDを返す
	const LogicalFile::FileID& getFileID() const;

	// ファイルサイズを返す
	ModUInt64	getSize() const;

	// 挿入されているオブジェクト数を返す
	//- ModInt64から型を変更することはできない
	ModInt64 getCount() const;

	////	Query Optimization

	// オブジェクト検索時のオーバヘッドを返す
	double	getOverhead() const;

	// オブジェクトへアクセスする際のプロセスコストを返す
	double	getProcessCost() const;

	// 検索オープンパラメータを設定する
	bool	getSearchParameter(
		const LogicalFile::TreeNodeInterface*	pCondition_,
		      LogicalFile::OpenOption&			rOpenOption_) const;

	// プロジェクションオープンパラメータを設定する
	bool	getProjectionParameter(
		const LogicalFile::TreeNodeInterface* pNode_,
			  LogicalFile::OpenOption&	rOpenOption_) const;

	// 更新オープンパラメータを設定する
	bool	getUpdateParameter(
		const Common::IntegerArrayData&	rUpdateFields_,
			  LogicalFile::OpenOption&	rOpenOption_) const;

	// ソート順パラメータを設定する
	bool	getSortParameter(
		const Common::IntegerArrayData&	rKeys_,
		const Common::IntegerArrayData&	rOrders_,
		      LogicalFile::OpenOption&	rOpenOption_) const;

	////	Data Manipulation

	// ベクタファイルを生成する
	const LogicalFile::FileID&	create(const Trans::Transaction& rTransaction_);
	// ベクタファイルを破棄する
	void	destroy(const Trans::Transaction& rTransaction_);

	// ベクタファイルをマウントする
	const LogicalFile::FileID& mount(const Trans::Transaction& rTransaction_);
	// ベクタファイルをアンマウントする
	const LogicalFile::FileID& unmount(const Trans::Transaction& rTransaction_);
	// ベクタファイルをフラッシュする
	void	flush(const Trans::Transaction& rTransaction_);
	// ベクタファイルのバックアップを開始する
	void	startBackup(const Trans::Transaction& rTransaction_,
						const bool bRestorable_);
	// ベクタファイルのバックアップを終了する
	void	endBackup(const Trans::Transaction& rTransaction_);

	// ベクタファイルを障害から回復する
	void recover(const Trans::Transaction&	rTransaction_,
				 const Trans::TimeStamp&	rPoint_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::Transaction&	rTransaction_,
				 const Trans::TimeStamp&	rPoint_);

	// 整合性検査を行う
	void verify(const Trans::Transaction&					rTransaction_,
				const Admin::Verification::Treatment::Value	eTreatment_,
				Admin::Verification::Progress&				rProgress_);

	// ベクタファイルをオープンする
	void	open(
		const Trans::Transaction&		rTransaction_,
		const LogicalFile::OpenOption&	rOpenOption_);
	// ベクターファイルをクローズする
	void	close();

	// 検索条件 (ベクタキー) を設定する
	void fetch(const Common::DataArrayData* pOption_);
	// データの取得を行なう
	bool	get(Common::DataArrayData* pTuple_);
	// 巻き戻しの位置を記録する
	void	mark();
	// 記録した位置に戻る
	void	rewind();
	// カーソルをリセットする
	void	reset();

	// データの更新を行なう
	void update(const Common::DataArrayData* pKey_, Common::DataArrayData* pTuple_);
	// データの挿入を行なう
	void insert(Common::DataArrayData* pTuple_);
	// データの削除を行なう
	void expunge(const Common::DataArrayData* pKey_);

	// 比較
	bool	equals(const Common::Object* pOther_) const;

	// 同期を取る
	void
	sync(const Trans::Transaction& trans, bool& incomplete, bool& modified);

	// Utiliry

	// ファイルを移動する
	void	move(
		const Trans::Transaction&		rTransaction_,
		const Common::StringArrayData&	rArea_);

	// ラッチが不要なオペレーションを返す
	Operation::Value getNoLatchOperation();

	// ファイルを識別する文字列を返す
	ModUnicodeString	toString() const;

private:

	// 親クラスとクラス名が同じ場合の operator= 問題を回避する
	File&	operator= (const File&	rObject_);
	// ついでにコピーコンストラクタも禁止する
	File(const File& cObject_);

	// fetch()の引数をベクタキーに変換する
	ModUInt32 getVectorKeyFromObject(
		const Common::DataArrayData* pOption_) const;

	// rOpenOption_に値を実際に設定する。
	bool setProjectionOptions(
		const Common::IntegerArrayData&	rProjection_,
		LogicalFile::OpenOption&		rOpenOption_,
		const bool bUpdateMode_) const;

	// 更新系メソッドの引数検査関数
	// どれも結構長いので残す

	// insertObject に渡された引数を検査する
	void checkInsertArgument(
		Common::DataArrayData*	pObject_,
		ModUInt32&				ulVectorKey_,
		Common::DataArrayData*&	pArrayData_);

	// updateObject に渡された引数を検査する
	void	checkUpdateArgument(
		const Common::DataArrayData*	pKey_,
		Common::DataArrayData*			pObject_,
		ModUInt32&				ulVectorKey,
		Common::DataArrayData*&	pArrayData_);

	// deleteObject に渡された引数を検査する
	ModUInt32 checkDeleteArgument(const Common::DataArrayData* pObject_);

	// 整合性検査の下請け関数

	int verifyPageCount(PhysicalFile::Page* pPage_,
						ModUInt32 ulPageID_,
						Admin::Verification::Progress& eProgress_);

	void verifyFirstVectorKey(const Trans::Transaction&	rTransaction_,
							  PhysicalFile::File* pFile_,
							  ModUInt32 ulFirstVectorKey_,
							  Admin::Verification::Progress& eProgress_);

	void verifyLastVectorKey(const Trans::Transaction&	rTransaction_,
							 PhysicalFile::File* pFile_,
							 ModUInt32 ulLastVectorKey_,
							 Admin::Verification::Progress& eProgress_);

	//////////////
	// 作成遅延 //
	//////////////

	void substantiate();

	const Trans::Transaction* m_pTransaction;

	//
	// データメンバ
	//

	// ファイルパラメータ（実体で保持）
	FileParameter					m_cFileParameter;
	// オープンパラメータへのポインタ
	OpenParameter*					m_pOpenParameter;
	// ページマネージャへのポインタ。
	// オブジェクトや管理情報の出し入れは全てここを介して行う。
	PageManager*					m_pPageManager;

	// カーソル
	ObjectIterator*					m_pObjectIterator;

	// 読み取り専用トランザクションかどうか
	Trans::Transaction::Category::Value m_eTransactionCategory;

	// getCount() 値のキャッシュ
	mutable ModInt64	m_iCountCache;//キャッシュなので mutable


	//// staticなデータメンバ

	// 物理ファイル名
	static const char*				m_pszPhysicalFileName;
};

_SYDNEY_VECTOR_END
_SYDNEY_END

#endif // __SYDNEY_VECTOR_FILE_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
