// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ValueFile.h -- バリューファイルクラスのヘッダーファイル
// 
// Copyright (c) 2001, 2003, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_VALUEFILE_H
#define __SYDNEY_BTREE_VALUEFILE_H

#include "Btree/Module.h"
#include "Btree/NullBitmap.h"
#include "Btree/PageVector.h"
#include "Btree/Version.h"

#include "Common/Object.h"
#include "PhysicalFile/Types.h"
#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
}

_SYDNEY_BTREE_BEGIN

class FileInformation;
class FileParameter;
class OpenParameter;
class UseInfo;

//
//	CLASS
//	Btree::ValueFile -- バリューファイルクラス
//
//	NOTES
//	バリューファイルクラス。
//
class ValueFile : public Common::Object
{
public:

	// コンストラクタ
	ValueFile(const Trans::Transaction*	Transaction_,
			  FileParameter*			FileParam_);

	// コンストラクタ
	ValueFile(const Trans::Transaction*				Transaction_,
			  FileParameter*						FileParam_,
			  const Buffer::Page::FixMode::Value	FixMode_);

	// デストラクタ
	~ValueFile();

	// バリューファイルを生成する
	void create();

	// バリューファイルを破棄する
	void destroy();

	// バリューファイルを移動する
	void move(const ModUnicodeString& MoveTo_, bool accessible);

	// ファイルを空の状態にする
	void clear();

	// バリューフィールドを挿入する
	ModUInt64 insert(const Common::DataArrayData*	Object_,
					 const ModUInt64				ObjectNum_,
					 const PhysicalFile::PageID		LeafPageID_,
					 const ModUInt32				KeyInfoIndex_,
					 const bool						CatchMemoryExhaust_,
					 PageVector&					AttachValuePages_,
					 PageIDVector&					AllocateValuePageIDs_);

	// バリューフィールドを更新する
	void update(const ModUInt64					ObjectID_,
				const Common::DataArrayData*	Object_,
				const OpenParameter*			OpenParam_,
				const PhysicalFile::PageID		LeafPageID_,
				const ModUInt32					KeyInfoIndex_,
				const bool						CatchMemoryExhaust_,
				PageVector&						AttachValuePages_,
				PageIDVector&					AllocateValuePageIDs_,
				PageIDVector&					FreeValuePageIDs_);

	// リーフページ情報を更新する
	void update(const ModUInt64				ObjectID_,
				const PhysicalFile::PageID	LeafPageID_,
				const ModUInt32				KeyInfoIndex_,
				const bool					CatchMemoryExhaust_,
				PhysicalFile::Page*&		CurrentObjectPage_,
				PageVector&					AttachValuePages_);

	// バリューフィールドを削除する
	void expunge(const ModUInt64	ObjectID_,
				 FileInformation&	FileInfo_,
				 const bool			CatchMemoryExhaust_,
				 PageVector&		AttachValuePages_,
				 PageIDVector&		FreeValuePageIDs_);

	// バリューフィールドを読み込む
	void read(const ModUInt64			ObjectID_,
			  Common::DataArrayData*	ResultObject_,
			  int&						iElement_,
			  const OpenParameter*		OpenParam_,
			  const bool				DoProjection_,
			  const bool				CatchMemoryExhaust_,
			  PageVector&				AttachValuePages_) const;

	// 配列バリューフィールドを読み込む
	void
		readArrayField(
			PhysicalFile::Page*				DirectObjectPage_,
			const ModUInt64					FieldObjectID_,
			const Common::DataType::Type	ElementDataType_,
			Common::DataArrayData&			ArrayField_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachPages_) const;

	// リーフページ情報を読み込む
	void readLeafInfo(const ModUInt64		ObjectID_,
					  PhysicalFile::PageID&	LeafPageID_,
					  ModUInt32&			KeyInfoIndex_) const;

	// リーフページ情報を読み込む
	void readLeafInfo(const ModUInt64		ObjectID_,
					  PageVector&			AttachValuePages_,
					  const bool			CatchMemoryExhaust_,
					  PhysicalFile::PageID&	LeafPageID_,
					  ModUInt32&			KeyInfoIndex_) const;

	// リーフページ情報を読み込む
	static void readLeafInfo(const void*			ObjectAreaTop_,
							 PhysicalFile::PageID&	LeafPageID_,
							 ModUInt32&				KeyInfoIndex_);

	//
	// 整合性検査用メソッド
	//

	// バリューフィールドを記録するために使用している
	// すべての物理ページと物理エリアを登録する
	void setUseInfo(const ModUInt64					ObjectID_,
					UseInfo&						UseInfo_,
					Admin::Verification::Progress&	Progress_) const;

	//
	// 運用管理用メソッド
	//

	// ファイルをマウントする
	void mount();

	// ファイルをアンマウントする
	void unmount();

	// ファイルをフラッシュする
	void flush();

	// バックアップ開始を通知する
	void startBackup(const bool	Restorable_);

	// バックアップ終了を通知する
	void endBackup();

	// 障害回復する
	void recover(const Trans::TimeStamp&	Point_);

	// ある時点に開始された読取専用トランザクションが
	// 参照する版を最新版とする
	void restore(const Trans::TimeStamp&	Point_);

	// 同期をとる
	void sync(bool& incomplete, bool& modified);

	//
	// 静的データメンバ
	//

	// バリューファイル格納先ディレクトリ名
	static const ModUnicodeString	DirectoryName;

	//
	// 非静的データメンバ
	//

	// トランザクション記述子
	const Trans::Transaction*	m_Transaction;

	// 物理ファイル記述子
	PhysicalFile::File*			m_PhysicalFile;

private:

	// オブジェクトを挿入可能なバリューページを検索する
	PhysicalFile::Page*
		searchInsertPage(
			const ModUInt64					ObjectNum_,
			const PhysicalFile::AreaSize	AreaSize_,
			const PhysicalFile::PageID		StartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_) const;

	// 
	ModUInt32 getObjectPerPage() const;

	// 外置き可変長バリューフィールドを書き込む
	void
		writeOutsideVariableFields(
			const Common::DataArrayData*	Object_,
			const PhysicalFile::PageID		SearchStartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_,
			ModVector<ModUInt64>&			ObjectIDs_,
			const OpenParameter*			OpenParam_ = 0) const;

	// 外置き可変長バリューフィールドを書き込む
	ModUInt64
		writeOutsideVariableValue(
			const Common::Data*			VariableField_,
			const PhysicalFile::PageID	SearchStartPageID_,
			const bool					CatchMemoryExhaust_,
			PageVector&					AttachValuePages_,
			PageIDVector&				AllocateValuePageIDs_) const;

	// 圧縮されている外置き可変長バリューフィールドを書き込む
	ModUInt64
		writeCompressedOutsideVariableValue(
			const char*					FieldBuffer_,
			const Os::Memory::Size		UncompressedSize_,
			const Os::Memory::Size		CompressedSize_,
			const int					BufferIndex_,
			const PhysicalFile::PageID	SearchStartPageID_,
			const bool					CatchMemoryExhaust_,
			PageVector&					AttachValuePages_,
			PageIDVector&				AllocateValuePageIDs_,
			const bool					IsRecur_ = false) const;

	// 外置き可変長バリューフィールドを書き込む
	ModUInt64
		writeOutsideVariableValue(
			const Common::DataType::Type	FieldType_,
			const char*						FieldBuffer_,
			const Os::Memory::Size			FieldLength_,
			const int						BufferIndex_,
			const PhysicalFile::PageID		SearchStartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_) const;

	// 配列バリューフィールドを書き込む
	void
		writeArrayFields(
			const Common::DataArrayData*	Object_,
			const PhysicalFile::PageID		SearchStartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_,
			ModVector<ModUInt64>&			ObjectIDs_,
			const OpenParameter*			OpenParam_ = 0) const;

	// 配列バリューフィールドを書き込む
	ModUInt64
		writeArrayField(
			const Common::Data*				ArrayField_,
			const Common::DataType::Type	ElementType_,
			const PhysicalFile::PageID		SearchStartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_) const;

	// 1つの物理エリアに記録可能な要素数を返す
	int
		getPutableElementNumPerArea(
			const Os::Memory::Size	ElementSize_) const;

	// 可変長要素の配列バリューフィールドを書き込む
	ModUInt64
		writeVariableElementArrayField(
			const Common::DataArrayData*	ArrayField_,
			const PhysicalFile::PageID		SearchStartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_) const;

	// 
	ModUInt64
		writeVariableElementArrayField(
			ModVector<ModUInt64>&			ObjectIDs_,
			const int						ElementNum_,
			const int						StartElementIndex_,
			const PhysicalFile::PageID		SearchStartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_) const;


	// 固定長要素の配列バリューフィールドを書き込む
	ModUInt64
		writeFixedElementArrayField(
			const Common::DataArrayData*	ArrayField_,
			const int						ElementNum_,
			const int						StartElementIndex_,
			const Common::DataType::Type	ElementType_,
			const PhysicalFile::PageID		SearchStartPageID_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachValuePages_,
			PageIDVector&					AllocateValuePageIDs_) const;

	// 配列バリューフィールドを読み込む
	// Common::Data::copyの仕様が変わった都合で引数を修正
	const char*
		readArrayField(
			const int				FieldIndex_,
			PhysicalFile::Page*		DirectObjectPage_,
			const char*				ValueTop_,
			Common::DataArrayData&	ArrayField_,
			const bool				CatchMemoryExhaust_,
			PageVector&				AttachValuePages_) const;

	// 要素が固定長の配列バリューフィールドを読み込む
	void
		readFixedElementArrayField(
			PhysicalFile::Page*				DirectObjectPage_,
			const ModUInt64					ArrayFieldObjectID_,
			const Common::DataType::Type	ElementDataType_,
			Common::DataArrayData&			ArrayField_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachPages_) const;

	// 要素が可変長の配列バリューフィールドを読み込む
	void
		readVariableElementArrayField(
			PhysicalFile::Page*				DirectObjectPage_,
			const ModUInt64					ArrayFieldObjectID_,
			const Common::DataType::Type	ElementDataType_,
			Common::DataArrayData&			ArrayField_,
			const bool						CatchMemoryExhaust_,
			PageVector&						AttachPages_) const;


	// ヌル値を書き込む
	char* writeNullField(NullBitmap::Value*	NullBitmapTop_,
						 char*				ValueTop_,
						 const int			ValueFieldIndex_) const;

	// オブジェクトが記録されている物理エリアを解放する
	void freeObjectArea(PhysicalFile::Page*			DirectObjectPage_,
						const PhysicalFile::AreaID	DirectObjectAreaID_,
						const bool					CatchMemoryExhaust_,
						PageVector&					AttachValuePages_,
						PageIDVector&				FreeValuePageIDs_) const;

	// 外置きバリューフィールドオブジェクトが記録されている
	// 物理エリアを解放する
	void
		freeOutsideFieldObjectArea(
			PhysicalFile::Page*			DirectObjectPage_,
			const PhysicalFile::AreaID	DirectObjectAreaID_,
			const bool					CatchMemoryExhaust_,
			PageVector&					AttachValuePages_,
			PageIDVector&				FreeValuePageIDs_,
			const OpenParameter*		OpenParam_ = 0) const;

	// 配列バリューフィールドオブジェクトが記録されている
	// 物理エリアを解放する
	void
		freeArrayFieldObjectArea(
			const ModUInt64		ArrayFieldObjectID_,
			PhysicalFile::Page*	DirectObjectPage_,
			const bool			ElementIsFixed_,
			const bool			CatchMemoryExhaust_,
			PageVector&			AttachValuePages_,
			PageIDVector&		FreeValuePageIDs_) const;

	// フィールドの値が記録されている領域へのポインタを返す
	void* getFieldPointer(PhysicalFile::Page*			ObjectPage_,
						  const PhysicalFile::AreaID	ObjectAreaID_,
						  const int						FieldIndex_) const;

	//
	// 整合性検査のためのメソッド
	//

	// 配列フィールドオブジェクトのために使用している
	// 物理ページと物理エリアを登録する
	void
		setArrayFieldUseInfo(
			PhysicalFile::Page*				DirectObjectPage_,
			const ModUInt64					FieldObjectID_,
			const bool						ElementIsFixed_,
			UseInfo&						UseInfo_,
			Admin::Verification::Progress&	Progress_) const;

	//
	// 非静的データメンバ
	//

	// ファイルパラメータへのポインタ
	FileParameter*					m_FileParam;

	// フィックスモード
	Buffer::Page::FixMode::Value	m_FixMode;

	// バリューページの最大空き領域サイズ [byte]
	Os::Memory::Size				m_PageFreeSizeMax;

}; // end of class Btree::ValueFile

_SYDNEY_BTREE_END
_SYDNEY_END

#endif //__SYDNEY_BTREE_VALUEFILE_H

//
//	Copyright (c) 2001, 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
