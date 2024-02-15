// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileInformation.h -- レコードファイル管理情報クラスのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_FILEINFORMATION_H
#define __SYDNEY_RECORD_FILEINFORMATION_H

#include "ModTypes.h"

#include "Common/DateTimeData.h"
#include "FileCommon/DataManager.h"
#include "PhysicalFile/Types.h"
//#include "PhysicalFile/Content.h"

#include "Record/Module.h"
#include "Record/Tools.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
class Page;
}

namespace Admin
{
	namespace Verification
	{
		class Progress;
	}
}

_SYDNEY_RECORD_BEGIN

class DirectIterator;

//
//	CLASS
//	Record::FileInformation -- レコードファイル管理情報クラス
//
//	NOTES
//	レコードファイル管理情報クラス。
//	管理情報は１つのページに収まるように作る
//
class FileInformation : public Common::Object
{
	friend class File;
public:
	// ファイルのバージョン情報
	struct Vers
	{
		enum Value
		{
			Version1 = 0,
			Version2,
			Version3,
			Version4,					// これ以降はRecord2
			
			// バージョン数
			VersionNum,
			// 現在のバージョン
			CurrentVersion = VersionNum - 1
		};
	};

	// 管理情報に対して行う処理の種類
	struct OpenOperation
	{
		enum Value
		{
			Read,
			Update,
			Verify,
			Batch,
			ValueNum
		};
	};
	// validateで行う処理の種類
	struct ValidateOperation
	{
		enum Value
		{
			Read,
			Insert,
			Update,
			Expunge,
			ValueNum
		};
	};

	// コンストラクタ
	FileInformation(
		const Trans::Transaction&			cTrans_,
		PhysicalFile::File&					cFile_,
		PhysicalFile::PageID				iPageID_,
		OpenOperation::Value				eOperation_,
		Admin::Verification::Progress*		pProgress_ = 0);

	// デストラクタ
	~FileInformation();

	//
	// アクセッサ
	//
	
#ifdef OBSOLETE
	// 最終更新時刻を取得
	const Common::DateTimeData& getLastModifiedTime() const;
#endif //OBSOLETE

	// 挿入されているオブジェクト数を取得
	Tools::ObjectNum getInsertedObjectNum() const;

	// 先頭オブジェクトのオブジェクトIDを取得
	Tools::ObjectID getFirstObjectID() const;

	// 最終オブジェクトのオブジェクトIDを取得
	Tools::ObjectID getLastObjectID() const;

	// 空きオブジェクトIDリストの先頭を取得
	Tools::ObjectID getFirstFreeObjectID() const;

	// 空きオブジェクトIDリストの先頭を取得(可変長)
	Tools::ObjectID getFirstFreeVariableObjectID() const;

	// ファイルのバージョンを取得
	Vers::Value getFileVersion() const;
	
#ifdef OBSOLETE
	// 管理情報をファイルに書き込むのに必要なバイト数を求める
	static Os::Memory::Size getAreaSize();
#endif //OBSOLETE

	//
	// マニピュレータ
	//

#ifdef OBSOLETE
	// 挿入されているオブジェクト数を設定
	void setInsertedObjectNum(const Tools::ObjectNum	InsertedObjectNum_);
#endif //OBSOLETE

#ifdef OBSOLETE
	// 先頭オブジェクトのオブジェクトIDを設定
	void setFirstObjectID(const Tools::ObjectID	FirstObjectID_);
#endif //OBSOLETE

#ifdef OBSOLETE
	// 最終オブジェクトのオブジェクトIDを設定
	void setLastObjectID(const Tools::ObjectID	LastObjectID_);
#endif //OBSOLETE

	// 空きオブジェクトIDリストの先頭を設定
	void setFirstFreeObjectID(const Tools::ObjectID	FirstFreeObjectID_);
	// 空きオブジェクトIDリストの先頭を設定
	void setFirstFreeVariableObjectID(const Tools::ObjectID	FirstFreeObjectID_);

	// ファイルのバージョンを設定
	void setFileVersion(Vers::Value eVersion_) const;

	// オブジェクトに対して行った処理でヘッダー情報が正しい状態を保つように
	// 必要なら修正する
	void validate(DirectIterator& cIterator_,
				  ValidateOperation::Value eOperation_);

	// ファイル上の管理情報のうち正しい方をメンバ変数に読み込む。
	// DoRepair_ が true の場合は修繕も行なう。
	void reload(const bool DoRepair_,
				bool bKeepAttach_ = false);

	// データメンバをファイルに書き込む
	void sync();
	// 書き込んでしまったら元に戻す
	void recover();

	// dirty状態にする
	void touch();
	
	struct SyncProgress {
		typedef unsigned char	Value;
		enum _Value {
			NotWriting = 0x00,
			WritingFirstBlock = 0x01,
			WritingSecondBlock = 0x02
		};
	};

	//管理情報が書き込まれている物理ページの参照を放棄（デタッチしない）
	void releasePage() { m_pPage = 0; }

private:

	// 関数 sync の進み具合をあらわす値にアクセスする
	SyncProgress::Value readSyncProgress();
	void writeSyncProgress(SyncProgress::Value Progress_);

	// 第1ブロック(第2ブロック)にデータメンバを読み書きする
	void readFirstBlock();
	void writeFirstBlock();

	void readSecondBlock();
	void writeSecondBlock();

	// ブロックを読み書きする
	void readBlock(const char* pPointer_);
	void writeBlock(char* pPointer_);

	// 管理情報用ページをアタッチ/デタッチする
	void attachPage(Buffer::Page::FixMode::Value eFixMode_);
	void detachPage(bool bForce_ = false);
	
	/////////////////////////////////////////////////////////
	// ファイルに書き込まれるデータメンバ(ここから)
	//
	// (メンバの総数や型が変化したら以下のメソッドも変更すること)
	//		・initializePhysicalPage
	//		・access
	//		・getAreaSize
	//

	struct Data {
//		// 最終更新時刻 - 年
//		Common::DateTimeData		m_cLastModifiedTime;

		// 挿入されているオブジェクト数
		Tools::ObjectNum			m_InsertedObjectNum;

		// 先頭オブジェクトのオブジェクトID
		Tools::ObjectID				m_FirstObjectID;

		// 最終オブジェクトのオブジェクトID
		Tools::ObjectID				m_LastObjectID;

		// 空きオブジェクトIDリストの先頭
		Tools::ObjectID				m_FirstFreeObjectID;
		Tools::ObjectID				m_FirstFreeVariableObjectID;

		// ファイルのバージョン番号
		Vers::Value					m_eFileVersion;

		// 内容が変化したか
		bool						m_bDirty;

		Data()
			: // m_cLastModifiedTime(),
			  m_InsertedObjectNum(0),
			  m_FirstObjectID(Tools::m_UndefinedObjectID),
			  m_LastObjectID(Tools::m_UndefinedObjectID),
			  m_FirstFreeObjectID(Tools::m_UndefinedObjectID),
			  m_FirstFreeVariableObjectID(Tools::m_UndefinedObjectID),
			  m_eFileVersion(Vers::CurrentVersion),
			  m_bDirty(false)
		{}

		Data& operator= (const Data& cData_)
		{
//			m_cLastModifiedTime = cData_.m_cLastModifiedTime;
			m_InsertedObjectNum = cData_.m_InsertedObjectNum;
			m_FirstObjectID = cData_.m_FirstObjectID;
			m_LastObjectID = cData_.m_LastObjectID;
			m_FirstFreeObjectID = cData_.m_FirstFreeObjectID;
			m_FirstFreeVariableObjectID = cData_.m_FirstFreeVariableObjectID;
			m_eFileVersion = cData_.m_eFileVersion;
			m_bDirty = true;
			return *this;
		}

		void touch()
		{
			m_bDirty = true;
		}
	};
	Data m_cCurrent;
	Data m_cSave;

	//
	// ファイルに書き込まれるデータメンバ(ここまで)
	/////////////////////////////////////////////////////////

	// トランザクション記述子
	const Trans::Transaction&		m_cTrans;

	// 操作モード
	OpenOperation::Value			m_eOperation;
	// 整合性検査時に結果を書き込む変数
	Admin::Verification::Progress*	m_pProgress;

	// 管理情報が書き込まれている物理ファイルの記述子
	PhysicalFile::File&				m_cFile;

	// 管理情報が書き込まれる物理ページの物理ページID
	PhysicalFile::PageID			m_iPageID;

	// 管理情報が書き込まれている物理ページの記述子
	PhysicalFile::Page*				m_pPage;

	// ページにアクセスするアドレスの先頭
	union {
		char*						m_pPageTop;
		const char*					m_pConstPageTop;
	};

	// 書き込みが発生したか、syncしたか
	enum {
		NotDirty,
		Dirty,
		Sync
	}								m_Status;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_FILEINFORMATION_H

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
