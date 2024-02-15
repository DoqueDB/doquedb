// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectFile.h -- 代表オブジェクトクラスを格納するファイル
// 
// Copyright (c) 2001, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_DIRECTFILE_H
#define __SYDNEY_RECORD_DIRECTFILE_H

#include "Common/Common.h"
#include "PhysicalFile/Types.h"
#include "Record/Module.h"
#include "Record/FileBase.h"
#include "Record/Tools.h"
#include "Admin/Verification.h"

#include "ModMap.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

_SYDNEY_RECORD_BEGIN

class DirectIterator;
class FileInformation;
class MetaData;
class OpenParameter;
class TargetFields;
class UseInfo;
class VariableFile;

//
//	CLASS
//	Record::DirectFile -- 代表オブジェクトを格納するファイルのクラス
//
//	NOTES
//	代表オブジェクトを格納するファイルにアクセスする手段を提供する。
//
class DirectFile : public FileBase //実装の継承
{
public:
	// DirectIteratorはDirectFile内部を自由にアクセスできる
	friend class DirectIterator;

	//	CLASS
	//	Record::DirectFile::DataPackage --
	//		代表オブジェクトの内容を受け渡すのに使うクラス
	//
	//	NOTES

	class DataPackage : public Common::Object
	{
	public:
		DataPackage(const MetaData& cMetaData_,
					const TargetFields* pTargets_ = 0,
					const Common::DataArrayData* pData_ = 0);
		~DataPackage();

		// set data / allocate new data
		void
		allocate(Common::DataArrayData* pData_ = 0);
		// replace existing data by allocated data
		void
		reallocate();
		// free allocated data
		void
		free();

		Common::DataArrayData* get();
		const Common::DataArrayData* get() const;
		Common::DataArrayData* release();

		Tools::ObjectID getVariableID() const;
		void setVariableID(Tools::ObjectID iVariableID_);

		Tools::BitMap& getNullBitMap();
		const Tools::BitMap& getNullBitMap() const;
		void setNullBitMap(const Tools::BitMap& cNull_);
		bool isNull(Tools::FieldNum iFieldID_) const;

		void setData(const Common::DataArrayData* pData_);
		void mergeData(const DataPackage& cOther_);

		void setTargetField(const TargetFields* pTargets_);

	private:
		void setNullBitMap();
		bool checkType(Tools::FieldNum iFieldID_,
					   Tools::FieldNum iIndex_);

		union {							// 格納されているデータ
			Common::DataArrayData* m_pData;
			const Common::DataArrayData* m_pConstData;
		};
		Tools::BitMap			m_cNull;		// Nullデータを示す
		Tools::ObjectID			m_iVariableID;	// 可変長のID
		bool					m_bAllocated;
		const MetaData&			m_cMetaData;
		const TargetFields*		m_pTarget;
	};

	// コンストラクタ
	DirectFile(const Trans::Transaction& cTrans_,
			   const MetaData& cMetaData_);

	// デストラクタ
	~DirectFile();

	///////////////////////////////////
	// LogicalFile::Fileのための関数 //
	///////////////////////////////////

	// ファイルサイズを返す
//	ModUInt64 getSize() const;

	// オブジェクト数を返す
	ModInt64 getCount() const;

	// 検索時のオーバーヘッドを返す
	double getOverhead() const;

	// オブジェクト1件あたりのアクセス時間を返す
	double getProcessCost() const;

	//////////////////////////////
	// 運用管理のためのメソッド //
	//////////////////////////////

	// 代表オブジェクトファイルを作成する
//	void create();

	// 代表オブジェクトファイルを破棄する
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

	// 整合性検査を行う
	void startVerification(const Trans::Transaction&		Transaction_,
						const unsigned int				Treatment_,
						Admin::Verification::Progress&	Progress_,
						VariableFile* pVariableFile_);
	void endVerification(const Trans::Transaction&		Transaction_,
						Admin::Verification::Progress&	Progress_,
						VariableFile* pVariableFile_);
	void verifyPhysicalFile(Admin::Verification::Treatment::Value iTreatment_,
							Admin::Verification::Progress& cProgress_,
							VariableFile* pVariableFile_);
	void verifyContents(Admin::Verification::Treatment::Value iTreatment_,
						Admin::Verification::Progress& cProgress_);

	////////////////////////////////
	// データ操作のためのメソッド //
	////////////////////////////////

	// オブジェクトを読み込む
	bool read(Tools::ObjectID iObjectID_,
			  const TargetFields* pTarget_,
			  DataPackage& cData_);
	// null bitmapと可変長オブジェクトIDを読み込む
	void readObjectHeader(Tools::ObjectID iObjectID_, DataPackage& cData_);
	// 可変長オブジェクトIDを読み込む
	Tools::ObjectID readVariableID(Tools::ObjectID iObjectID_);

	// オブジェクトを挿入する
	Tools::ObjectID insert(DataPackage& cData_);

	// オブジェクトを更新する
	void update(Tools::ObjectID iObjectID_,
				DataPackage& cData_,
				const TargetFields* pTarget_);

	// オブジェクトを削除する
	void expunge(Tools::ObjectID iObjectID_);

	// 巻き戻しの位置を記録する
	void mark();

	// 記録した位置に戻る
	void rewind();

	// カーソルをリセットする
	void reset();

	// 物理ファイルをアタッチ/デタッチする
//	void attachFile(const OpenParameter* pOpenParam_ = 0);
	void detachFile();

	// アタッチされているかを得る
//	bool isAttached() const;

	// すべてのページをデタッチする
	void detachPageAll(bool bSucceeded_);

	// ヘッダーページのIDを得る
	PhysicalFile::PageID getInformationPageID() const;

	// ファイルが格納されているパス名を得る
//	const ModUnicodeString& getPath() const;

	// ファイル管理情報を得る(検索操作用)
	FileInformation& readFileInformation();
	// ファイル管理情報を得る(更新操作用)
	FileInformation& readFileInformationForUpdate();
	// ファイル管理情報を書き込む(更新操作用)
	void syncFileInformation();

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

	virtual void substantiate();		// 実体を作成する

private:
	// コピーを禁止する
	DirectFile(const DirectFile& cOther_);
	DirectFile& operator=(const DirectFile& cOther_);

	//////////////////
	// 内部メソッド //
	//////////////////

	// メタデータから物理ファイル用戦略を設定する
	void setStorategy(const MetaData& cMetaData_);

	// 物理ファイルマネージャーに使用中の物理ページを通知する
	void notifyUsePage(Admin::Verification::Treatment::Value iTreatment_,
					   Admin::Verification::Progress& cProgress_,
					   VariableFile* pVariableFile_);

	// verifyの下請け
	void verifyFirstObjectID(Admin::Verification::Treatment::Value iTreatment_,
							 Admin::Verification::Progress& cProgress_,
							 const FileInformation& cFileInfo_);
	void verifyLastObjectID(Admin::Verification::Treatment::Value iTreatment_,
							Admin::Verification::Progress& cProgress_,
							const FileInformation& cFileInfo_);
	void verifyObjectNumber(Admin::Verification::Treatment::Value iTreatment_,
							Admin::Verification::Progress& cProgress_,
							const FileInformation& cFileInfo_);
	void verifyFreeObjectID(Admin::Verification::Treatment::Value iTreatment_,
							Admin::Verification::Progress& cProgress_,
							const FileInformation& cFileInfo_);

	FileInformation* m_pFileInformation;
	FileInformation* m_pReadFileInformation;

	DirectIterator* m_pReadIterator;
	DirectIterator* m_pWriteIterator;

	Tools::ObjectID m_iMarkedObjectID;

	typedef ModMap<PhysicalFile::PageID, PhysicalFile::Page*, ModLess<PhysicalFile::PageID> > AttachedPageMap;
	mutable AttachedPageMap m_mapAttachedPage;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_DIRECTFILE_H

//
//	Copyright (c) 2001, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
