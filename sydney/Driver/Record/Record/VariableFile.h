// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableFile.h -- 可変長フィールドを格納するファイル
// 
// Copyright (c) 2001, 2003, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_VARIABLEFILE_H
#define __SYDNEY_RECORD_VARIABLEFILE_H

#include "Common/Common.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/File.h"
#include "Record/DirectFile.h"
#include "Record/FileBase.h"
#include "Record/Module.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

_SYDNEY_RECORD_BEGIN

class MetaData;
class OpenParameter;
class TargetFields;
class UseInfo;
class VariableIterator;

//
//	CLASS
//	Record::VariableFile -- 可変長フィールドを格納するファイルのクラス
//
//	NOTES
//	可変長フィールドを格納するファイルにアクセスする手段を提供する。
//
class VariableFile : public FileBase //実装の継承
{
public:
	// VariableIteratorはVariableFile内部を自由にアクセスできる
	friend class VariableIterator;

	// コンストラクタ
	VariableFile(const Trans::Transaction& cTrans_,
				 const MetaData& cMetaData_);

	// デストラクタ
	~VariableFile();

	///////////////////////////////////
	// LogicalFile::Fileのための関数 //
	///////////////////////////////////

	// ファイルサイズを返す
//	ModUInt64 getSize() const;

#ifdef OBSOLETE
	// オブジェクト数を返す
	ModInt64 getCount() const;
#endif //OBSOLETE

	// 検索時のオーバーヘッドを返す
	double getOverhead() const;

	// オブジェクト1件あたりのアクセス時間を返す
	double getProcessCost(ModInt64 iCount_) const;

	//////////////////////////////
	// 運用管理のためのメソッド //
	//////////////////////////////

	// 可変長フィールドファイルを作成する
//	void create();

	// 可変長フィールドファイルを破棄する
//	void destroy();

	// 実体である OS ファイルが存在するか調べる
//	bool
//	isAccessible(bool force = false) const;
	// マウントされているか調べる
//	bool
//	isMounted(const Trans::Transaction& trans) const;

	// マウント
//	void mount();
	// アンマウント
//	void unmount();
	// フラッシュ
//	void flush();
	// バックアップ開始と終了
//	void startBackup(const bool bRestorable_);
//	void endBackup();
	// 障害回復
//	void recover(const Trans::TimeStamp& cPoint_);
	// ある時点に開始されたトランザクションの版を最新にする
//	void restore(const Trans::TimeStamp& cPoint_);

	// ファイルを移動する
//	void move(bool bUndo_ = false);

	// 使用中の物理ページIDとエリアIDを集める
	void setUseInfo(Tools::ObjectID iObjectID_,
					UseInfo& cUseInfo_,
					Admin::Verification::Progress& cProgress_);
	// 使用中の物理ページIDとエリアIDを集める。（FreeArea 用）
	void setFreeAreaUseInfo(Tools::ObjectID iObjectID_,
					UseInfo& cUseInfo_,
					Admin::Verification::Progress& cProgress_);

	// 物理ファイルの整合性検査を行う
	void startVerification(const Trans::Transaction&		Transaction_,
						const unsigned int				Treatment_,
						Admin::Verification::Progress&	Progress_);
	void endVerification(const Trans::Transaction&		Transaction_,
						 Admin::Verification::Progress&	Progress_);
	void verifyPhysicalFile(UseInfo& cUseInfo_,
							Admin::Verification::Treatment::Value iTreatment_,
							Admin::Verification::Progress&		cProgress_);
	// 内容の整合性検査を行う
	void verifyContents(DirectFile::DataPackage& cData_,
						Admin::Verification::Treatment::Value iTreatment_,
						Admin::Verification::Progress&		cProgress_);
	// 空き領域リストの整合性検査を行う
	Tools::ObjectID verifyFreeObjectID(Admin::Verification::Treatment::Value iTreatment_,
									   Admin::Verification::Progress& cProgress_,
									   Tools::ObjectID iFreeID_);

	////////////////////////////////
	// データ操作のためのメソッド //
	////////////////////////////////

	// オブジェクトを読み込む
	void read(DirectFile::DataPackage& cData_,
			  const TargetFields* pTarget_);

	// オブジェクトを挿入する
	Tools::ObjectID insert(const DirectFile::DataPackage& cData_,
						   Tools::ObjectID& iFreeID_);

	// オブジェクトを更新する
	Tools::ObjectID update(const DirectFile::DataPackage& cOldObjectHeader_,
						   const DirectFile::DataPackage& cNewData_,
						   const TargetFields* pTarget_,
						   Tools::ObjectID& iFreeID_);

	// オブジェクトを削除する
	void expunge(Tools::ObjectID iObjectID_,
				 Tools::ObjectID& iFreeID_);

	// 物理ファイルをアタッチ/デタッチする
//	void attachFile(const OpenParameter* pOpenParam_ = 0);
	void detachFile();

	// アタッチされているかを得る
//	bool isAttached() const;

	// すべてのページをデタッチする
	void detachPageAll(bool bSucceeded_);

	// ファイルが格納されているパス名を得る
//	const ModUnicodeString& getPath() const;

	// 空き領域管理のためのインスタンスを用意/破棄する
	void prepareFreeAreaManager(bool bDoCache_ = false);
	void discardFreeAreaManager();

	//////////////////////////////////
	// 物理ファイルのためのメソッド //
	//////////////////////////////////

	// ファイルの同期を取る
//	void sync(const Trans::Transaction&	Transaction_);

protected:
	// ディレクトリーのパス名を得る
	virtual const ModUnicodeString& getPathPart() const;

	//////////////////////
	// ファイル作成遅延 //
	//////////////////////
//	bool isVacant() const;				// 実体の有無を調べる
	virtual void substantiate();		// 実体を作成する

private:
	// コピーを禁止する
	VariableFile(const VariableFile& cOther_);
	VariableFile& operator=(const VariableFile& cOther_);

	//////////////////
	// 内部メソッド //
	//////////////////

	// メタデータから物理ファイル用戦略を設定する
	void setStorategy(const MetaData& cMetaData_);

	// 使用中の物理ページIDとエリアIDを物理ファイルに知らせる
	void notifyUsePage(UseInfo& cUseInfo_,
					   Admin::Verification::Treatment::Value iTreatment_,
					   Admin::Verification::Progress& cProgress_);

	VariableIterator* m_pReadIterator;
	VariableIterator* m_pWriteIterator;

	// エリア確保のために使うクラス
	FreeAreaManager* m_pFreeAreaManager;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_VARIABLEFILE_H

//
//	Copyright (c) 2001, 2003, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
