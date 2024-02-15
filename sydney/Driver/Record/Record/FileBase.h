// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileBase.h -- DirectFileとVariableFileの共通実装
// 
// Copyright (c) 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_SUBFILE_H
#define __SYDNEY_RECORD_SUBFILE_H

#include "Common/Common.h"
#include "Common/Object.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Types.h"
#include "Record/Module.h"
#include "Record/Tools.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

_SYDNEY_RECORD_BEGIN

class FileInformation;
class MetaData;
class OpenParameter;
class TargetFields;
class UseInfo;

//
//	CLASS
//	Record::FileBase -- レコードファイルを構成するファイルの基底クラス
//
//	NOTES
//	FileBaseとVariableFileで共通する実装を提供する。
//
class FileBase : public Common::Object
{
public:
	// コンストラクタ
	FileBase(const Trans::Transaction& cTrans_,
			const MetaData& cMetaData_);

	// デストラクタ
	virtual ~FileBase();

	///////////////////////////////////
	// LogicalFile::Fileのための関数 //
	///////////////////////////////////

	// ファイルサイズを返す
	ModUInt64 getSize() const;

	// オブジェクト数を返す -- サブクラスで実装
//	ModInt64 getCount() const;

	// 検索時のオーバーヘッドを返す -- サブクラスで実装
//	double getOverhead() const;

	// オブジェクト1件あたりのアクセス時間を返す -- サブクラスで実装
//	double getProcessCost() const;

	//////////////////////////////
	// 運用管理のためのメソッド //
	//////////////////////////////

	// ファイルを作成する
	void create();

	// ファイルを破棄する
	void destroy();

	// 実体である OS ファイルが存在するか調べる
	bool
	isAccessible(bool force = false) const;
	// マウントされているか調べる
	bool
	isMounted(const Trans::Transaction& trans) const;

	// アンマウント
	void unmount();
	// マウント
	void mount();
	// フラッシュ
	void flush();
	// バックアップ開始と終了
	void startBackup(const bool bRestorable_);
	void endBackup();
	// 障害回復
	void recover(const Trans::TimeStamp& cPoint_);
	// ある時点に開始されたトランザクションの版を最新にする
	void restore(const Trans::TimeStamp& cPoint_);

	// ファイルを移動する
	void move(bool bUndo_ = false);

	// 整合性検査を行う -- サブクラスで実装する

	// データ操作のためのメソッド -- サブクラスで実装する

	// 物理ファイルをアタッチ/デタッチする
	void attachFile(const OpenParameter* pOpenParam_ = 0);
	virtual void detachFile();

	// アタッチされているかを得る
	bool isAttached() const;

	// すべてのページをデタッチする -- サブクラスで実装する
//	void detachPageAll(bool bSucceeded_);

	// ファイルが格納されているパス名を得る
	const ModUnicodeString& getPath() const;

	// Batch mode?
	bool isBatch();

	//////////////////////////////////
	// 物理ファイルのためのメソッド //
	//////////////////////////////////

	// 同期をとる
	void
	sync(const Trans::Transaction& trans, bool& incomplete, bool& modified);

	// トランザクションを得る
	const Trans::Transaction& getTransaction() const
		{ return m_cTrans; }

protected:
	// attach/detach を正しく行うための補助クラス
	typedef AutoFile<FileBase> AutoAttachFile;

	//////////////////////////////////
	// サブクラスで実装するメソッド //
	//////////////////////////////////

	// ディレクトリーのパス名を得る
	virtual const ModUnicodeString& getPathPart() const = 0;

	//////////////////////
	// ファイル作成遅延 //
	//////////////////////

	virtual void substantiate() = 0;				// 実体を作成する

private:
	// コピーを禁止する
	FileBase(const FileBase& cOther_);
	FileBase& operator=(const FileBase& cOther_);

	//////////////////
	// 内部メソッド //
	//////////////////

	// moveの下請け
	void move(ModUnicodeString& cstrOldPath_,
			  ModUnicodeString& cstrNewPath_,
			  bool bUndo_ = false);

protected: // サブクラスでもメンバーは参照する

	const Trans::Transaction& m_cTrans;
	PhysicalFile::File* m_pFile;
	const MetaData& m_cMetaData;
	const OpenParameter* m_pOpenParam;

	PhysicalFile::File::StorageStrategy		m_cStorageStrategy;
	PhysicalFile::File::BufferingStrategy	m_cBufferingStrategy;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_SUBFILE_H

//
//	Copyright (c) 2002, 2003, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
