// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ValueFile.cpp -- バリューファイルクラスの実現ファイル
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "Btree/ValueFile.h"

#include "Exception/Unexpected.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"
#include "Common/CompressedData.h"

#include "PhysicalFile/Manager.h"

#include "FileCommon/IDNumber.h"
#include "FileCommon/ObjectID.h"

#include "Btree/File.h"
#include "Btree/FileInformation.h"
#include "Btree/UseInfo.h"

#include "Os/File.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PUBLIC CONST
//
//////////////////////////////////////////////////

//
//	CONST public
//	Btree::ValueFile::DirectoryName --
//		バリューファイル格納先ディレクトリ名
//
//	NOTES
//	バリューファイル格納先ディレクトリ名。パスではない。
//
// static
const ModUnicodeString
ValueFile::DirectoryName = _TRMEISTER_U_STRING("Value");

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::ValueFile::ValueFile -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	Btree::FileParameter*		FileParam_
//		ファイルパラメータへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
ValueFile::ValueFile(const Trans::Transaction*	Transaction_,
					 FileParameter*				FileParam_)
	: Common::Object(),
	  m_Transaction(Transaction_),
	  m_FileParam(FileParam_),
	  m_FixMode(Buffer::Page::FixMode::ReadOnly)
{
	// 物理ファイルをアタッチする
	this->m_PhysicalFile =
		PhysicalFile::Manager::attachFile(
			FileParam_->m_ValueFileStorageStrategy,
			FileParam_->m_BufferingStrategy,
			this->m_FileParam->m_IDNumber->getLockName());

	// 物理ページ内で利用可能な領域の最大サイズを取得する
	this->m_PageFreeSizeMax = this->m_PhysicalFile->getPageDataSize(1);
}

//
//	FUNCTION public
//	Btree::ValueFile::ValueFile -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	Btree::FileParameter*				FileParam_
//		ファイルパラメータへのポインタ
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ValueFile::ValueFile(const Trans::Transaction*			Transaction_,
					 FileParameter*						FileParam_,
					 const Buffer::Page::FixMode::Value	FixMode_)
	: Common::Object(),
	  m_Transaction(Transaction_),
	  m_FileParam(FileParam_),
	  m_FixMode(FixMode_)
{
	// 物理ファイルをアタッチする
	this->m_PhysicalFile =
		PhysicalFile::Manager::attachFile(
			FileParam_->m_ValueFileStorageStrategy,
			FileParam_->m_BufferingStrategy,
			this->m_FileParam->m_IDNumber->getLockName());

	// 物理ページ内で利用可能な領域の最大サイズを取得する
	this->m_PageFreeSizeMax = this->m_PhysicalFile->getPageDataSize(1);
}

//
//	FUNCTION public
//	Btree::ValueFile::~ValueFile -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ValueFile::~ValueFile()
{
	// 物理ファイルをデタッチする
	PhysicalFile::Manager::detachFile(this->m_PhysicalFile);
}

//
//	FUNCTION public
//	Btree::ValueFile::create -- バリューファイルを生成する
//
//	NOTES
//	バリューファイルを生成する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	ModLibraryError
//		
//	Unexpected
//		
//	[YET!]
//
void
ValueFile::create()
{
	; _SYDNEY_ASSERT(this->m_Transaction != 0);
	; _SYDNEY_ASSERT(this->m_PhysicalFile != 0);

	//
	// バリューファイル格納先ディレクトリを作成する。
	//

	const Os::Path& valueFileDir =
		m_FileParam->m_ValueFileStorageStrategy.
		m_VersionFileInfo._path._masterData;

	//
	// バリューファイルを生成し、ファイル内部を初期化する。
	//

	bool created = false;
	try
	{
		// 物理ファイルを作成する
		this->m_PhysicalFile->create(*this->m_Transaction);
		created = true;

		// 先頭バリューページを生成する
		PhysicalFile::PageID	topValuePageID =
			this->m_PhysicalFile->allocatePage(*this->m_Transaction);

		; _SYDNEY_ASSERT(topValuePageID == 0);
	}
	catch (Exception::Object&)
	{
		if (created) {
			this->m_PhysicalFile->destroy(*this->m_Transaction);
		}
		// サブディレクトリーを破棄する
		//
		//【注意】	サブディレクトリは
		//			実体である物理ファイルの生成時に
		//			必要に応じて生成されるが、
		//			エラー時には削除されないので、
		//			この関数で削除する必要がある
		File::rmdirOnError(valueFileDir, this->m_FileParam);
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		if (created) {
			this->m_PhysicalFile->destroy(*this->m_Transaction);
		}
		// サブディレクトリーを破棄する
		File::rmdirOnError(valueFileDir, this->m_FileParam);
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION public
//	Btree::ValueFile::destroy -- バリューファイルを破棄する
//
//	NOTES
//	バリューファイルを破棄する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Unexpected
//		
//	ModLibraryError
//		
//	[YET!]
//
void
ValueFile::destroy()
{
	; _SYDNEY_ASSERT(this->m_Transaction != 0);
	; _SYDNEY_ASSERT(this->m_PhysicalFile != 0);

	//
	// バリューファイルを破棄する。
	//

	try
	{
		this->m_PhysicalFile->destroy(*this->m_Transaction);
	}
	catch (Exception::Object&)
	{
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}

	//
	// バリューファイル格納先ディレクトリを削除する。
	//

	File::rmdir(m_FileParam->m_ValueFileStorageStrategy.
				m_VersionFileInfo._path._masterData, m_FileParam);
}

//
//	FUNCTION public
//	Btree::ValueFile::move -- バリューファイルを移動する
//
//	NOTES
//	バリューファイルを移動する。
//
//	ARGUMENTS
//	const ModUnicodeString&	MoveTo_
//		移動先ディレクトリパス
//	bool	accessible
//		trueのとき実体がないことを示す
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	ModLibraryError
//		
//	Unexpected
//		
//	[YET!]
//
void
ValueFile::move(const ModUnicodeString&	MoveTo_, bool accessible)
{
	; _SYDNEY_ASSERT(this->m_Transaction != 0);
	; _SYDNEY_ASSERT(this->m_PhysicalFile != 0);

	//
	// 移動先のディレクトリを作成する。
	//

	Os::Path moveTo(MoveTo_);
	moveTo.addPart(ValueFile::DirectoryName);

	//
	// 物理ファイルを移動する。
	//

	Version::File::StorageStrategy::Path	moveToPath;
	moveToPath._masterData = moveTo;

	if (this->m_FileParam->m_BufferingStrategy.m_VersionFileInfo._category !=
		Buffer::Pool::Category::Temporary)
	{
		moveToPath._versionLog = moveTo;
		moveToPath._syncLog = moveTo;
	}

	try
	{
		this->m_PhysicalFile->move(*this->m_Transaction,
								   moveToPath);
	}
	catch (Exception::Object&)
	{
		if (accessible)

			// サブディレクトリーを破棄する
			//
			//【注意】	サブディレクトリは
			//			実体である物理ファイルの移動時に
			//			必要に応じて生成されるが、
			//			エラー時には削除されないので、
			//			この関数で削除する必要がある

			File::rmdirOnError(moveTo);

		// 移動先のディレクトリだけが残るのならば、
		// まだファイルは利用可能。
		_SYDNEY_RETHROW;
	}
	catch (...)
	{
		if (accessible)
			File::rmdirOnError(moveTo);

		// 移動先のディレクトリだけが残るのならば、
		// まだファイルは利用可能。
		throw Exception::Unexpected(moduleName, srcFile, __LINE__);
	}

	//
	// 移動前のディレクトリを消去する。
	//

	if (accessible)	
		File::rmdir(m_FileParam->m_ValueFileStorageStrategy.
					m_VersionFileInfo._path._masterData);
}

//
//	FUNCTION public
//	Btree::ValueFile::clear -- ファイルを空の状態にする
//
//	NOTES
//	ファイルを空の状態にする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::clear()
{
	// clearでは、バリューファイル側で
	// “ファイルが利用可能かどうか“を意識する必要はない。
	// なので、例外をキャッチする必要もない。

	// 物理ファイルを空の状態にする
	this->m_PhysicalFile->clear(*this->m_Transaction);

	// 先頭バリューページを生成する
	this->m_PhysicalFile->allocatePage(*this->m_Transaction);
}

//
//	FUNCTION public
//	Btree::ValueFile::insert -- バリューフィールドを挿入する
//
//	NOTES
//	バリューフィールドを挿入する。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Object_
//		挿入オブジェクトへのポインタ
//	const PhysicalFile::PageID		LeafPageID_
//		リーフページの物理ページ識別子
//	const ModUInt32					KeyInfoIndex_
//		キー情報インデックス
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	ModUInt64
//		オブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::insert(const Common::DataArrayData*	Object_,
				  const ModUInt64				ObjectNum_,
				  const PhysicalFile::PageID	LeafPageID_,
				  const ModUInt32				KeyInfoIndex_,
				  const bool					CatchMemoryExhaust_,
				  PageVector&					AttachValuePages_,
				  PageIDVector&					AllocateValuePageIDs_)
{
	//
	// 挿入の前処理
	//

	PhysicalFile::Page*	directObjectPage =
		this->searchInsertPage(ObjectNum_,
							   this->m_FileParam->m_DirectValueObjectSize,
							   0,
							   CatchMemoryExhaust_,
							   AttachValuePages_,
							   AllocateValuePageIDs_);

	PhysicalFile::PageID	directObjectPageID = directObjectPage->getID();

	PhysicalFile::AreaID	directObjectAreaID =
		directObjectPage->allocateArea(
			*this->m_Transaction,
			this->m_FileParam->m_DirectValueObjectSize);

	//
	// 外置き可変長バリューフィールドを書き込む
	//

	ModVector<ModUInt64>	variableFieldObjectIDs;

	this->writeOutsideVariableFields(Object_,
									 directObjectPageID,
									 CatchMemoryExhaust_,
									 AttachValuePages_,
									 AllocateValuePageIDs_,
									 variableFieldObjectIDs);

	//
	// 配列フィールドを書き込む
	//

	ModVector<ModUInt64>	arrayFieldObjectIDs;

	this->writeArrayFields(Object_,
						   directObjectPageID,
						   CatchMemoryExhaust_,
						   AttachValuePages_,
						   AllocateValuePageIDs_,
						   arrayFieldObjectIDs);

	//
	// 代表オブジェクトを書き込む
	//

	char* directObjectAreaTop = static_cast<char*>(
		File::getAreaTop(directObjectPage, directObjectAreaID));
	char* p = directObjectAreaTop;
	{
	const File::ObjectType tmp = 
		File::NormalObjectType | File::DirectObjectType;
	(void) Os::Memory::copy(p, &tmp, sizeof(tmp));
	p += sizeof(tmp);
	}
	(void) Os::Memory::copy(p, &LeafPageID_, sizeof(LeafPageID_));
	p += sizeof(LeafPageID_);

	(void) Os::Memory::copy(p, &KeyInfoIndex_, sizeof(KeyInfoIndex_));
	p += sizeof(KeyInfoIndex_);

	NullBitmap::Value* nullBitmapTop = syd_reinterpret_cast<NullBitmap::Value*>(p);
	NullBitmap::clear(nullBitmapTop, m_FileParam->m_ValueNum);

	char* valueWritePos = (m_FileParam->m_ValueNum) ?
		static_cast<char*>(NullBitmap::getTail(
			nullBitmapTop, m_FileParam->m_ValueNum)) : p;

	ModVector<ModUInt64>::Iterator	variableFieldObjectIDIterator =
		variableFieldObjectIDs.begin();
	ModVector<ModUInt64>::Iterator	arrayFieldObjectIDIterator =
		arrayFieldObjectIDs.begin();
#ifdef DEBUG
	ModVector<ModUInt64>::Iterator	variableFieldObjectIDsEnd =
		variableFieldObjectIDs.end();
	ModVector<ModUInt64>::Iterator	arrayFieldObjectIDsEnd =
		arrayFieldObjectIDs.end();
#endif

	for (int i = this->m_FileParam->m_TopValueFieldIndex;
		 i < this->m_FileParam->m_FieldNum;
		 i++)
	{
		Common::Data*	value = Object_->getElement(i).get();

		if (value->isNull())
		{
			valueWritePos = this->writeNullField(nullBitmapTop,
												 valueWritePos,
												 i);
		}
		else if (*(this->m_FileParam->m_IsArrayFieldArray + i))
		{
			// array
#ifdef DEBUG
			; _SYDNEY_ASSERT(arrayFieldObjectIDIterator !=
							 arrayFieldObjectIDsEnd);
#endif
			valueWritePos =
				File::writeObjectID(valueWritePos,
									*arrayFieldObjectIDIterator);

			arrayFieldObjectIDIterator++;
		}
		else if (*(this->m_FileParam->m_IsFixedFieldArray + i))
		{
			// fix

			valueWritePos = File::writeFixedField(valueWritePos, value);
		}
		else
		{
			// var

			if (*(this->m_FileParam->m_FieldOutsideArray + i))
			{
				// outside
#ifdef DEBUG
				; _SYDNEY_ASSERT(variableFieldObjectIDIterator !=
								 variableFieldObjectIDsEnd);
#endif
				// write oid

				valueWritePos =
					File::writeObjectID(valueWritePos,
										*variableFieldObjectIDIterator);

				variableFieldObjectIDIterator++;
			}
			else
			{
				// inside

				// write value

				Os::Memory::Size	maxLen =
					*(this->m_FileParam->m_FieldMaxLengthArray + i);

				valueWritePos =
					File::writeInsideVariableField(valueWritePos,
												   value,
												   maxLen);
			}
		}

	} // end for i

#ifdef DEBUG
	; _SYDNEY_ASSERT(variableFieldObjectIDIterator ==
					 variableFieldObjectIDsEnd);
	; _SYDNEY_ASSERT(arrayFieldObjectIDIterator ==
					 arrayFieldObjectIDsEnd);

	Os::Memory::Size	directObjectAreaSize =
		directObjectPage->getAreaSize(directObjectAreaID);
	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(valueWritePos - directObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == directObjectAreaSize);
#endif

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			directObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}

	return File::makeObjectID(directObjectPageID, directObjectAreaID);
}

//
//	FUNCTION public
//	Btree::ValueFile::update -- バリューフィールドを更新する
//
//	NOTES
//	バリューフィールドを更新する。
//
//	ARGUMENTS
//	const ModUInt64					ObjectID_
//		更新するオブジェクトのオブジェクトID
//	const Common::DataArrayData*	Object_
//		更新後のオブジェクトへのポインタ
//	const Btree::OpenParameter*		OpenParam_
//		オープンパラメータへのポインタ
//	const PhysicalFile::PageID		LeafPageID_
//		リーフページの物理ページ識別子
//	const ModUInt32					KeyInfoIndex_
//		キー情報インデックス
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//	Btree::PageIDVector&			FreeValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（フリーしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::update(const ModUInt64				ObjectID_,
				  const Common::DataArrayData*	Object_,
				  const OpenParameter*			OpenParam_,
				  const PhysicalFile::PageID	LeafPageID_,
				  const ModUInt32				KeyInfoIndex_,
				  const bool					CatchMemoryExhaust_,
				  PageVector&					AttachValuePages_,
				  PageIDVector&					AllocateValuePageIDs_,
				  PageIDVector&					FreeValuePageIDs_)
{
	; _SYDNEY_ASSERT(ObjectID_ != FileCommon::ObjectID::Undefined);
	; _SYDNEY_ASSERT(Object_ != 0);

	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);
	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	PhysicalFile::Page*	objectPage =
		File::attachPage(this->m_Transaction,
						 this->m_PhysicalFile,
						 objectPageID,
						 this->m_FixMode,
						 CatchMemoryExhaust_,
						 AttachValuePages_);

	//
	// 外置きフィールドオブジェクトが記録されている物理エリアを解放する
	//

	this->freeOutsideFieldObjectArea(objectPage,
									 objectAreaID,
									 CatchMemoryExhaust_,
									 AttachValuePages_,
									 FreeValuePageIDs_,
									 OpenParam_);

	//
	// 外置き可変長バリューフィールドを書き込む
	//

	ModVector<ModUInt64>	variableFieldObjectIDs;

	this->writeOutsideVariableFields(Object_,
									 objectPageID,
									 CatchMemoryExhaust_,
									 AttachValuePages_,
									 AllocateValuePageIDs_,
									 variableFieldObjectIDs,
									 OpenParam_);

	//
	// 配列フィールドを書き込む
	//

	ModVector<ModUInt64>	arrayFieldObjectIDs;

	this->writeArrayFields(Object_,
						   objectPageID,
						   CatchMemoryExhaust_,
						   AttachValuePages_,
						   AllocateValuePageIDs_,
						   arrayFieldObjectIDs,
						   OpenParam_);

	//
	// 可変長フィールドオブジェクトや配列フィールドオブジェクト
	// などの外置きオブジェクトが記録されている物理エリアを解放し、
	// 外置きオブジェクトが記録されている物理ページ内の物理エリアを
	// 再配置した可能性があり、代表オブジェクトと外置きオブジェクトが
	// 同一物理ページ内に記録されていることもあるので
	// 物理エリア先頭へのポインタはここで求める
	//

	char* objectAreaTop = static_cast<char*>(
		File::getAreaTop(objectPage, objectAreaID));
	char* p = objectAreaTop;
	{
	const File::ObjectType tmp =
		File::NormalObjectType | File::DirectObjectType;
	(void) Os::Memory::copy(p, &tmp, sizeof(tmp));
	p += sizeof(tmp);
	}
	(void) Os::Memory::copy(p, &LeafPageID_, sizeof(LeafPageID_));
	p += sizeof(LeafPageID_);

	(void) Os::Memory::copy(p, &KeyInfoIndex_, sizeof(KeyInfoIndex_));
	p += sizeof(KeyInfoIndex_);

	NullBitmap::Value* nullBitmapTop = syd_reinterpret_cast<NullBitmap::Value*>(p);
	NullBitmap nullBitmap(
		nullBitmapTop, m_FileParam->m_ValueNum, NullBitmap::Access::ReadWrite);

	char* valueWritePos = (m_FileParam->m_ValueNum) ?
		static_cast<char*>(nullBitmap.getTail()) : p;

	ModVector<ModUInt64>::Iterator	variableFieldObjectIDIterator =
		variableFieldObjectIDs.begin();
	ModVector<ModUInt64>::Iterator	variableFieldObjectIDsEnd =
		variableFieldObjectIDs.end();

	ModVector<ModUInt64>::Iterator	arrayFieldObjectIDIterator =
		arrayFieldObjectIDs.begin();
	ModVector<ModUInt64>::Iterator	arrayFieldObjectIDsEnd =
		arrayFieldObjectIDs.end();

	for (int i = this->m_FileParam->m_TopValueFieldIndex;
		 i < this->m_FileParam->m_FieldNum;
		 i++)
	{
		if (File::isSelected(OpenParam_, i))
		{
			// 更新対象バリューフィールド…

			Common::Data*	field = Object_->getElement(i).get();

			// まずは必ずヌルビットをOFFしておく。
			nullBitmap.off(i - this->m_FileParam->m_TopValueFieldIndex);

			if (field->isNull())
			{
				valueWritePos = this->writeNullField(nullBitmapTop,
													 valueWritePos,
													 i);
			}
			else if (*(this->m_FileParam->m_IsArrayFieldArray + i))
			{
				// array

				; _SYDNEY_ASSERT(arrayFieldObjectIDIterator !=
								 arrayFieldObjectIDsEnd);

				valueWritePos =
					File::writeObjectID(valueWritePos,
										*arrayFieldObjectIDIterator);

				arrayFieldObjectIDIterator++;
			}
			else if (*(this->m_FileParam->m_IsFixedFieldArray + i))
			{
				// fix

				valueWritePos = File::writeFixedField(valueWritePos,
													  field);
			}
			else
			{
				// var

				if (*(this->m_FileParam->m_FieldOutsideArray + i))
				{
					// outside

					; _SYDNEY_ASSERT(variableFieldObjectIDIterator !=
									 variableFieldObjectIDsEnd);

					// write oid

					valueWritePos =
						File::writeObjectID(valueWritePos,
											*variableFieldObjectIDIterator);

					variableFieldObjectIDIterator++;
				}
				else
				{
					// inside

					// write value

					Os::Memory::Size	maxLen =
						*(this->m_FileParam->m_FieldMaxLengthArray + i);

					valueWritePos =
						File::writeInsideVariableField(valueWritePos,
													   field,
													   maxLen);
				}
			}
		}
		else
		{
			// 更新対象ではないバリューフィールド…

			valueWritePos += this->m_FileParam->getFieldArchiveSize(i);
		}

	} // end for i

	; _SYDNEY_ASSERT(variableFieldObjectIDIterator ==
					 variableFieldObjectIDsEnd);

	; _SYDNEY_ASSERT(arrayFieldObjectIDIterator ==
					 arrayFieldObjectIDsEnd);

#ifdef DEBUG

	Os::Memory::Size	areaSize = objectPage->getAreaSize(objectAreaID);

	Os::Memory::Size	writeSize
		= static_cast<Os::Memory::Size>(valueWritePos - objectAreaTop);

	; _SYDNEY_ASSERT(writeSize == areaSize);

#endif

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}
}

//
//	FUNCTION public
//	Btree::ValueFile::update -- リーフページ情報を更新する
//
//	NOTES
//	リーフページ情報を更新する。
//
//	ARGUMENTS
//	const ModUInt64				ObjectID_
//		更新するオブジェクトのオブジェクトID
//	const PhysicalFile::PageID	LeafPageID_
//		リーフページの物理ページ識別子
//	const ModUInt32				KeyInfoIndex_
//		キー情報インデックス
//	const bool					CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	PhysicalFile::Page*&		CurrentObjectPage_
//		更新するオブジェクトが記録されている物理ページの記述子
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::update(const ModUInt64				ObjectID_,
				  const PhysicalFile::PageID	LeafPageID_,
				  const ModUInt32				KeyInfoIndex_,
				  const bool					CatchMemoryExhaust_,
				  PhysicalFile::Page*&			CurrentObjectPage_,
				  PageVector&					AttachValuePages_)
{
	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);
	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);
	PhysicalFile::Page*	objectPage = 0;

	if (CurrentObjectPage_ != 0 &&
		CurrentObjectPage_->getID() == objectPageID)
	{
		objectPage = CurrentObjectPage_;
	}
	else
	{
		objectPage = File::attachPage(this->m_Transaction,
									  this->m_PhysicalFile,
									  objectPageID,
									  this->m_FixMode,
									  CatchMemoryExhaust_,
									  AttachValuePages_);

		if (CatchMemoryExhaust_ == false)
		{
			CurrentObjectPage_ = objectPage;
		}
	}

	char* p = static_cast<char*>(File::getAreaTop(objectPage, objectAreaID));

	p += sizeof(File::ObjectType);

	(void) Os::Memory::copy(p, &LeafPageID_, sizeof(LeafPageID_));
	p += sizeof(LeafPageID_);

	(void) Os::Memory::copy(p, &KeyInfoIndex_, sizeof(KeyInfoIndex_));

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}
}

//
//	FUNCTION public
//	Btree::ValueFile::expunge -- バリューフィールドを削除する
//
//	NOTES
//	バリューフィールドを削除する。
//
//	ARGUMENTS
//	const ModUInt64			ObjectID_
//		削除するオブジェクトのオブジェクトID
//	FileInformation&		FileInfo_
//		ファイル管理情報への参照
//	const bool				CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	FreeValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（フリーしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::expunge(const ModUInt64	ObjectID_,
				   FileInformation&	FileInfo_,
				   const bool		CatchMemoryExhaust_,
				   PageVector&		AttachValuePages_,
				   PageIDVector&	FreeValuePageIDs_)
{
	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);
	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	PhysicalFile::Page*	objectPage =
		File::attachPage(this->m_Transaction,
						 this->m_PhysicalFile,
						 objectPageID,
						 this->m_FixMode,
						 CatchMemoryExhaust_,
						 AttachValuePages_);

	this->freeObjectArea(objectPage,
						 objectAreaID,
						 CatchMemoryExhaust_,
						 AttachValuePages_,
						 FreeValuePageIDs_);

	if (objectPageID != 0 &&
		objectPage->getTopAreaID(*this->m_Transaction) ==
		PhysicalFile::ConstValue::UndefinedAreaID)
	{
		File::detachPage(this->m_PhysicalFile,
						 AttachValuePages_,
						 objectPage,
						 PhysicalFile::Page::UnfixMode::Dirty,
						 false);

		FreeValuePageIDs_.pushBack(objectPageID);

		this->m_PhysicalFile->freePage(*this->m_Transaction,
									   objectPageID);

		if (FileInfo_.readObjectNum() == 1)
		{
			m_PhysicalFile->unfixVersionPage(true);
			this->clear();
		}
	}
	else if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false);
	}
}

//
//	FUNCTION public
//	Btree::ValueFile::read -- バリューフィールドを読み込む
//
//	NOTES
//	バリューフィールドを読み込む。
//
//	ARGUMENTS
//	const ModUInt64			ObjectID_
//		読み込むオブジェクトのオブジェクトID
//	Common::DataArrayData*	ResultObject_
//		読み込み先のオブジェクトへのポインタ
//	int&					iElement_
//		読み込むオブジェクトのArray中の位置
//	const Btree::OpenParameter*		OpenParam_
//		オープンパラメータへのポインタ
//	const bool				DoProjection_
//		プロジェクション指定がされていれば、指定されたフィールドの値だけを
//		読み込むかどうか
//			true  : プロジェクション指定がされていれば、
//			        指定されたフィールドの値だけを読み込む。
//			false : プロジェクション指定がされていても、
//			        オブジェクト全体を読み込む。
//	const bool				CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::read(const ModUInt64			ObjectID_,
				Common::DataArrayData*	ResultObject_,
				int&					iElement_,
				const OpenParameter*	OpenParam_,
				const bool				DoProjection_,
				const bool				CatchMemoryExhaust_,
				PageVector&				AttachValuePages_) const
{
	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);
	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	PhysicalFile::Page*	objectPage =
		File::attachPage(this->m_Transaction,
						 this->m_PhysicalFile,
						 objectPageID,
						 this->m_FixMode,
						 CatchMemoryExhaust_,
						 AttachValuePages_);

	const char*	objectAreaTop =
		static_cast<const char*>(File::getConstAreaTop(objectPage,
													   objectAreaID));

	const File::ObjectType*	objectType =
		syd_reinterpret_cast<const File::ObjectType*>(objectAreaTop);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	const NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(
			objectAreaTop +
			File::ObjectTypeArchiveSize +
			File::PageIDArchiveSize +
			File::ModUInt32ArchiveSize);

	NullBitmap	nullBitmap(nullBitmapTop,
						   this->m_FileParam->m_ValueNum,
						   NullBitmap::Access::ReadOnly);

	bool	existNull = nullBitmap.existNull();

	const char*	valueTop =
		static_cast<const char*>(nullBitmap.getConstTail());

	// 各バリューフィールドを読み込む…

	bool	doProjection = DoProjection_ && OpenParam_->m_bFieldSelect;

	for (int i = this->m_FileParam->m_TopValueFieldIndex;
		 i < this->m_FileParam->m_FieldNum;
		 i++)
	{
		if (doProjection && File::isSelected(OpenParam_, i) == false)
		{
			valueTop += this->m_FileParam->getFieldArchiveSize(i);

			continue;
		}

		Common::Data::Pointer	field;

		Common::DataType::Type	fieldType =
			*(this->m_FileParam->m_FieldTypeArray + i);

		int	bitIndex = i - this->m_FileParam->m_TopValueFieldIndex;

		if (existNull && nullBitmap.isNull(bitIndex))
		{
			field = Common::NullData::getInstance();

			valueTop += this->m_FileParam->getFieldArchiveSize(i);
		}
		else if (*(this->m_FileParam->m_IsArrayFieldArray + i))
		{
			// 配列フィールド…

			ModAutoPointer<Common::DataArrayData> arrayField =
				new Common::DataArrayData();

			valueTop = this->readArrayField(i,
											objectPage,
											valueTop,
											*arrayField,
											CatchMemoryExhaust_,
											AttachValuePages_);

			field = arrayField.release();
		}
		else if (*(this->m_FileParam->m_IsFixedFieldArray + i))
		{
			// 固定長バリューフィールド…

			valueTop = File::readFixedField(fieldType,
											valueTop,
											field);
		}
		else
		{
			// 可変長バリューフィールド…

			bool	isOutside =
				*(this->m_FileParam->m_FieldOutsideArray + i);

			Os::Memory::Size	fieldMaxLen =
				isOutside ?
					0 :
					*(this->m_FileParam->m_FieldMaxLengthArray + i);

			valueTop = File::readVariableField(this->m_Transaction,
											   fieldType,
											   fieldMaxLen,
											   isOutside,
											   objectPage,
											   valueTop,
											   field,
											   this->m_FixMode,
											   CatchMemoryExhaust_,
											   AttachValuePages_);
		}

		//ResultObject_->setElement(i, field);
		ResultObject_->getElement(iElement_++)->assign(field.get());
	}

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false);
	}
}

//
//	FUNCTION public
//	Btree::ValueFile::readArrayField -- 配列バリューフィールドを読み込む
//
//	NOTES
//	配列バリューフィールドを読み込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*				DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const ModUInt64					FieldObjectID_
//		配列バリューフィールドオブジェクトのオブジェクトID
//	const Common::DataType::Type	ElementType_
//		要素のデータ型
//	Common::DataArrayData&			ArrayField_
//		配列バリューフィールド読込先への参照
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::readArrayField(
	PhysicalFile::Page*				DirectObjectPage_,
	const ModUInt64					FieldObjectID_,
	const Common::DataType::Type	ElementType_,
	Common::DataArrayData&			ArrayField_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_) const
{
	if (FileCommon::DataManager::isVariable(ElementType_))
	{
		// 可変長要素…

		this->readVariableElementArrayField(DirectObjectPage_,
											FieldObjectID_,
											ElementType_,
											ArrayField_,
											CatchMemoryExhaust_,
											AttachValuePages_);
	}
	else
	{
		// 固定長要素…

		this->readFixedElementArrayField(DirectObjectPage_,
										 FieldObjectID_,
										 ElementType_,
										 ArrayField_,
										 CatchMemoryExhaust_,
										 AttachValuePages_);
	}
}

//
//	FUNCTION public
//	Btree::ValueFile::readLeafInfo -- リーフページ情報を読み込む
//
//	NOTES
//	リーフページ情報を読み込む。
//
//	ARGUMENTS
//	const ModUInt64			ObjectID_
//		読み込むオブジェクトのオブジェクトID
//	PhysicalFile::PageID&	LeafPageID_
//		リーフページの物理ページ識別子への参照
//	ModUInt32&				KeyInfoIndex_
//		キー情報インデックスへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::readLeafInfo(const ModUInt64			ObjectID_,
						PhysicalFile::PageID&	LeafPageID_,
						ModUInt32&				KeyInfoIndex_) const
{
	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);
	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	PhysicalFile::Page*	objectPage =
		this->m_PhysicalFile->attachPage(*this->m_Transaction,
										 objectPageID,
										 this->m_FixMode);

	const char*	objectAreaTop =
		static_cast<const char*>(File::getConstAreaTop(objectPage,
													   objectAreaID));

	ValueFile::readLeafInfo(objectAreaTop,
							LeafPageID_,
							KeyInfoIndex_);

	this->m_PhysicalFile->detachPage(
		objectPage,
		PhysicalFile::Page::UnfixMode::NotDirty,
		false);
}

//
//	FUNCTION public
//	Btree::ValueFile::readLeafInfo -- リーフページ情報を読み込む
//
//	NOTES
//	リーフページ情報を読み込む。
//
//	ARGUMENTS
//	const ModUInt64			ObjectID_
//		読み込むオブジェクトのオブジェクトID
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	const bool				CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	PhysicalFile::PageID&	LeafPageID_
//		リーフページの物理ページ識別子への参照
//	ModUInt32&				KeyInfoIndex_
//		キー情報インデックスへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::readLeafInfo(const ModUInt64			ObjectID_,
						PageVector&				AttachValuePages_,
						const bool				CatchMemoryExhaust_,
						PhysicalFile::PageID&	LeafPageID_,
						ModUInt32&				KeyInfoIndex_) const
{
	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);
	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	PhysicalFile::Page*	objectPage =
		File::attachPage(this->m_Transaction,
						 this->m_PhysicalFile,
						 objectPageID,
						 this->m_FixMode,
						 CatchMemoryExhaust_,
						 AttachValuePages_);

	const void*	objectAreaTop = File::getConstAreaTop(objectPage,
													  objectAreaID);

	ValueFile::readLeafInfo(objectAreaTop,
							LeafPageID_,
							KeyInfoIndex_);

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false);
	}
}

//	FUNCTION public
//	Btree::ValueFile::readLeafInfo -- リーフページ情報を読み込む
//
//	NOTES
//	リーフページ情報を読み込む。
//
//	ARGUMENTS
//	const void*				ObjectAreaTop_
//		オブジェクトが記録されている物理エリア先頭へのポインタ
//	PhysicalFile::PageID&	LeafPageID_
//		リーフページの物理ページ識別子への参照
//	ModUInt32&				KeyInfoIndex_
//		キー情報インデックスへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

// static
void
ValueFile::readLeafInfo(const void*				ObjectAreaTop_,
						PhysicalFile::PageID&	LeafPageID_,
						ModUInt32&				KeyInfoIndex_)
{
	const char* p =
		static_cast<const char*>(ObjectAreaTop_) + sizeof(File::ObjectType);

	(void) Os::Memory::copy(&LeafPageID_, p, sizeof(LeafPageID_));
	p += sizeof(LeafPageID_);

	(void) Os::Memory::copy(&KeyInfoIndex_, p, sizeof(KeyInfoIndex_));

	; _SYDNEY_ASSERT(LeafPageID_ != PhysicalFile::ConstValue::UndefinedPageID);
	; _SYDNEY_ASSERT(KeyInfoIndex_ != ModUInt32Max);
}

//
//	FUNCTION public
//	Btree::ValueFile::setUseInfo --
//		バリューフィールドを記録するために使用している
//		すべての物理ページと物理エリアを登録する
//
//	NOTES
//	バリューフィールドを記録するために使用している
//	すべての物理ページと物理エリアを登録する。
//
//	ARGUMENTS
//	const ModUInt64					ObjectID_
//		オブジェクトID
//	Btree::UseInfo&					UseInfo_
//		登録情報への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::setUseInfo(const ModUInt64					ObjectID_,
					  UseInfo&							UseInfo_,
					  Admin::Verification::Progress&	Progress_) const
{
	const PhysicalFile::PageID objectPageID =
		Common::ObjectIDData::getFormerValue(ObjectID_);
	const PhysicalFile::AreaID objectAreaID =
		Common::ObjectIDData::getLatterValue(ObjectID_);

	UseInfo_.append(objectPageID, objectAreaID);

	if (this->m_FileParam->m_ExistOutsideFieldInValue)
	{
		PhysicalFile::Page*	objectPage =
			this->m_PhysicalFile->verifyPage(
				*this->m_Transaction,
				objectPageID,
				Buffer::Page::FixMode::ReadOnly,
				Progress_);

		if (Progress_.isGood() == false)
		{
			return;
		}

		; _SYDNEY_ASSERT(objectPage != 0);

		const void*	objectAreaTop = File::getConstAreaTop(objectPage,
														  objectAreaID);

		const File::ObjectType*	objectType =
			static_cast<const File::ObjectType*>(objectAreaTop);

		; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

		for (int i = this->m_FileParam->m_TopValueFieldIndex;
			 i < this->m_FileParam->m_FieldNum;
			 i++)
		{
			if (*(this->m_FileParam->m_FieldOutsideArray + i))
			{
				char*	objectIDReadPos =
					static_cast<char*>(this->getFieldPointer(objectPage,
															 objectAreaID,
															 i));

				if (objectIDReadPos != 0)
				{
					ModUInt64	objectID;
					File::readObjectID(objectIDReadPos, objectID);

					try
					{
						if (*(this->m_FileParam->m_IsArrayFieldArray + i))
						{
							bool	elementIsFixed =
								*(this->m_FileParam->m_IsFixedElementArray +
								  i);

							this->setArrayFieldUseInfo(objectPage,
													   objectID,
													   elementIsFixed,
													   UseInfo_,
													   Progress_);
						}
						else
						{
							File::setOutsideVariableFieldUseInfo(
								*this->m_Transaction,
								this->m_PhysicalFile,
								objectPage,
								objectID,
								UseInfo_,
								Progress_);
						}

						if (Progress_.isGood() == false)
						{
							this->m_PhysicalFile->detachPage(
								objectPage,
								PhysicalFile::Page::UnfixMode::NotDirty,
								false);

							return;
						}
					}
					catch (...)
					{
						this->m_PhysicalFile->detachPage(
							objectPage,
							PhysicalFile::Page::UnfixMode::NotDirty,
							false);

						_SYDNEY_RETHROW;
					}
				}

			} // end if

		} // end for i

		this->m_PhysicalFile->detachPage(
			objectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false);
	}
}

//
// 運用管理用メソッド
//

//
//	FUNCTION public
//	Btree::ValueFile::mount -- ファイルをマウントする
//
//	NOTES
//	ファイルをマウントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::mount()
{
	this->m_PhysicalFile->mount(*this->m_Transaction);
}

//
//	FUNCTION public
//	Btree::ValueFile::unmount -- ファイルをアンマウントする
//
//	NOTES
//	ファイルをアンマウントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::unmount()
{
	this->m_PhysicalFile->unmount(*this->m_Transaction);
}

//
//	FUNCTION public
//	Btree::ValueFile::flush -- ファイルをフラッシュする
//
//	NOTES
//	ファイルをフラッシュする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::flush()
{
	this->m_PhysicalFile->flush(*this->m_Transaction);
}

//
//	FUNCTION public
//	Btree::ValueFile::startBackup -- バックアップ開始を通知する
//
//	NOTES
//	バックアップ開始を通知する。
//
//	ARGUMENTS
//	const bool	Restorable_
//		版が最新版になるように変更可能とするかどうか
//			true  : バックアップされた内容をリストアしたとき、
//			        あるタイムスタンプの表す時点に開始された
//			        読取専用トランザクションの参照する版が
//			        最新版になるように変更可能にする。
//			false : バックアップされた内容をリストアしたとき、
//			        バックアップ開始時点に障害回復可能にする。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::startBackup(const bool	Restorable_)
{
	this->m_PhysicalFile->startBackup(*this->m_Transaction, Restorable_);
}

//
//	FUNCTION public
//	Btree::ValueFile::endBackup -- バックアップ終了を通知する
//
//	NOTES
//	バックアップ終了を通知する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::endBackup()
{
	this->m_PhysicalFile->endBackup(*this->m_Transaction);
}

//
//	FUNCTION public
//	Btree::ValueFile::recover -- 障害回復する
//
//	NOTES
//	障害回復する。
//
//	ARGUMENTS
//	const Trans::TimeStamp&	Point_
//		バージョンファイルを戻す時点のタイムスタンプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::recover(const Trans::TimeStamp&	Point_)
{
	this->m_PhysicalFile->recover(*this->m_Transaction, Point_);
}

//
//	FUNCTION public
//	Btree::ValueFile::restore --
//		ある時点に開始された読取専用トランザクションが
//		参照する版を最新版とする
//
//	NOTES
//	ある時点に開始された読取専用トランザクションが
//	参照する版を最新版とする。
//
//	ARGUMENTS
//	const Trans::TimeStamp&	Point_
//		このタイムスタンプの表す時点に開始された
//		読取専用トランザクションの参照する版が
//		最新版になるようにバージョンファイルを変更する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::restore(const Trans::TimeStamp&	Point_)
{
	this->m_PhysicalFile->restore(*this->m_Transaction, Point_);
}

//	FUNCTION public
//	Btree::ValueFile::sync -- 同期をとる
//
//	NOTES
//
//	ARGUMENTS
//		bool&				incomplete
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、ファイルを処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でファイルを持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でファイルを持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、ファイルが更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ValueFile::sync(bool& incomplete, bool& modified)
{
	; _SYDNEY_ASSERT(m_PhysicalFile);
	; _SYDNEY_ASSERT(m_Transaction);

	m_PhysicalFile->sync(*m_Transaction, incomplete, modified);
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::ValueFile::searchInsertPage --
//		オブジェクトを挿入可能なバリューページを検索する
//
//	NOTES
//	オブジェクトを挿入可能なバリューページを検索する。
//
//	ARGUMENTS
//	const PhysicalFile::AreaSize	AreaSize_
//		オブジェクトを記録するために必要な物理エリアサイズ [byte]
//	const PhysicalFile::PageID		StartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	PhysicalFile::Page*
//		検索結果である、オブジェクトを挿入可能なバリューページの
//		物理ページ記述子
//
//	EXCEPTIONS
//	[YET!]
//
PhysicalFile::Page*
ValueFile::searchInsertPage(
	const ModUInt64					ObjectNum_,
	const PhysicalFile::AreaSize	AreaSize_,
	const PhysicalFile::PageID		StartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_) const
{
	PhysicalFile::PageID	insertPageID =
		PhysicalFile::ConstValue::UndefinedPageID;

	if (AttachValuePages_.isEmpty() == ModFalse)
	{
		PageVector::Iterator	pagesIterator = AttachValuePages_.end();

		pagesIterator--;

		if ((*pagesIterator)->getFreeAreaSize(*this->m_Transaction) >
			AreaSize_)
		{
			insertPageID = (*pagesIterator)->getID();
		}
		else if (pagesIterator != AttachValuePages_.begin())
		{
			pagesIterator--;

			if ((*pagesIterator)->getFreeAreaSize(*this->m_Transaction) >
				AreaSize_)
			{
				insertPageID = (*pagesIterator)->getID();
			}
		}
	}

	if (insertPageID == PhysicalFile::ConstValue::UndefinedPageID &&
		this->m_FileParam->m_ExistOutsideFieldInValue == false)
	{
		//
		// 挿入されているオブジェクト数と、
		// ページ内の使用状況などによって、
		// 空きのあるページが存在するかどうかの見当をつけて、
		// PhysicalFile::File::searchFreePage()を呼び出しても、
		// 該当する物理ページが見つけ出せないようならば、
		// 最後のバリューページに挿入するようにする。
		//

		//
		// 現在挿入されているオブジェクト数分のバリューオブジェクトを
		// 記録するために必要なバリューページ数を求める。
		//

		ModUInt32	objectPerPage = this->getObjectPerPage();

		PhysicalFile::PageNum	needPageNum =
			static_cast<PhysicalFile::PageNum>(ObjectNum_ / objectPerPage);
		if (ObjectNum_ % objectPerPage != 0)
		{
			needPageNum++;
		}

		PhysicalFile::PageNum	usedPageNum;
		PhysicalFile::PageNum	unusePageNum;
		this->m_PhysicalFile->fetchOutPageNum(*this->m_Transaction,
											  usedPageNum,
											  unusePageNum);

		if (unusePageNum == 0 && needPageNum + 2 >= usedPageNum)
		{//                                  ~~~ きっちり、必要な分の
		 //                                      ページに収まっているとは
		 //                                      限らないので、
		 //                                      若干、幅を持たせる。

			PhysicalFile::PageID	lastValuePageID = usedPageNum - 1;

			PhysicalFile::Page*	lastValuePage =
				File::attachPage(this->m_Transaction,
								 this->m_PhysicalFile,
								 lastValuePageID,
								 this->m_FixMode,
								 CatchMemoryExhaust_,
								 AttachValuePages_);

			if (lastValuePage->getFreeAreaSize(*this->m_Transaction) >=
				AreaSize_)
			{
				return lastValuePage;
			}

			if (CatchMemoryExhaust_)
			{
				this->m_PhysicalFile->detachPage(
					lastValuePage,
					PhysicalFile::Page::UnfixMode::NotDirty,
					false); // 本当にデタッチ（アンフィックス）してしまい、
					        // 物理ファイルマネージャがキャッシュ
					        // しないようにする。
			}

			insertPageID =
				this->m_PhysicalFile->allocatePage(*this->m_Transaction);

			AllocateValuePageIDs_.pushBack(insertPageID);
		}
	}

	if (insertPageID == PhysicalFile::ConstValue::UndefinedPageID)
	{
		//
		// 指定された物理エリアサイズが、
		// 物理ページを高速検索できないような大きさの場合には、
		// 物理ページを追加するだけにする。
		//

		if (AreaSize_ <= this->m_PhysicalFile->getPageSearchableThreshold())
		{
			insertPageID =
				this->m_PhysicalFile->searchFreePage(
					*this->m_Transaction,
					AreaSize_,
					StartPageID_,
					false); // 空き領域で検索（未使用領域ではない）
		}

		if (insertPageID == PhysicalFile::ConstValue::UndefinedPageID)
		{
			insertPageID =
				this->m_PhysicalFile->allocatePage(*this->m_Transaction);

			AllocateValuePageIDs_.pushBack(insertPageID);
		}
	}

	; _SYDNEY_ASSERT(
		insertPageID != PhysicalFile::ConstValue::UndefinedPageID);

	return File::attachPage(this->m_Transaction,
							this->m_PhysicalFile,
							insertPageID,
							this->m_FixMode,
							CatchMemoryExhaust_,
							AttachValuePages_);
}

ModUInt32
ValueFile::getObjectPerPage() const
{
	ModUInt32	objectPerPage = 10;

	PhysicalFile::PageSize	pageDataSizeMax =
		this->m_PhysicalFile->getPageDataSize(0);

	while (true)
	{
		PhysicalFile::PageSize	physicalFileUseSize =
			pageDataSizeMax -
			this->m_PhysicalFile->getPageDataSize(objectPerPage);

		Os::Memory::Size	totalSize =
			this->m_FileParam->m_DirectValueObjectSize * objectPerPage +
			physicalFileUseSize;

		if (pageDataSizeMax < totalSize)
		{
			objectPerPage -= 10;

			break;
		}

		objectPerPage += 10;
	}

	for (int i = 0; i < 10; i++, objectPerPage++)
	{
		PhysicalFile::PageSize	physicalFileUseSize =
			pageDataSizeMax -
			this->m_PhysicalFile->getPageDataSize(objectPerPage);

		Os::Memory::Size	totalSize =
			this->m_FileParam->m_DirectValueObjectSize * objectPerPage +
			physicalFileUseSize;

		if (pageDataSizeMax < totalSize)
		{
			objectPerPage--;

			break;
		}
	}

	return objectPerPage;
}

//
//	FUNCTION private
//	Btree::ValueFile::writeOutsideVariableFields --
//		外置き可変長バリューフィールドを書き込む
//
//	NOTES
//	外置き可変長バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Object_
//		オブジェクトへのポインタ
//	const PhysicalFile::PageID		SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//	ModVector<ModUInt64>&			ObjectIDs_
//		外置き可変長バリューフィールドオブジェクトの
//		オブジェクトIDを格納するベクターへの参照
//	const Btree::OpenParameter*		OpenParam_ = 0
//		オープンパラメータへのポインタ
//		※ オブジェクト更新時に設定する。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::writeOutsideVariableFields(
	const Common::DataArrayData*	Object_,
	const PhysicalFile::PageID		SearchStartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_,
	ModVector<ModUInt64>&			ObjectIDs_,
	const OpenParameter*			OpenParam_ // = 0
	) const
{
	bool	selected = true;

	if (this->m_FileParam->m_ExistVariableFieldInValue &&
		this->m_FileParam->m_ExistOutsideFieldInValue)
	{
		for (int i = this->m_FileParam->m_TopValueFieldIndex;
			 i < this->m_FileParam->m_FieldNum;
			 i++)
		{
			if (OpenParam_ != 0)
			{
				// オブジェクト更新時に呼ばれた…

				selected = File::isSelected(OpenParam_, i);
			}

			if (selected &&
				*(this->m_FileParam->m_IsFixedFieldArray + i) == false &&
				*(this->m_FileParam->m_IsArrayFieldArray + i) == false &&
				*(this->m_FileParam->m_FieldOutsideArray + i))
			{
				const Common::Data*	variableField =
					Object_->getElement(i).get();

				; _SYDNEY_ASSERT(variableField != 0);

				if (!variableField->isNull())
				{
					ModUInt64	objectID =
						this->writeOutsideVariableValue(
							variableField,
							SearchStartPageID_,
							CatchMemoryExhaust_,
							AttachValuePages_,
							AllocateValuePageIDs_);

					ObjectIDs_.pushBack(objectID);
				}
			}
		}
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::writeOutsideVariableValue --
//		外置き可変長バリューフィールドを書き込む
//
//	NOTES
//	外置き可変長バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const Common::Data*			VariableField_
//		可変長バリューフィールドデータへのポインタ
//	const PhysicalFile::PageID	SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool					CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&		AlloocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	ModUInt64
//		外置き可変長バリューフィールドオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::writeOutsideVariableValue(
	const Common::Data*			VariableField_,
	const PhysicalFile::PageID	SearchStartPageID_,
	const bool					CatchMemoryExhaust_,
	PageVector&					AttachValuePages_,
	PageIDVector&				AllocateValuePageIDs_) const
{
	const void*			fieldBuffer = 0;
	Os::Memory::Size	fieldLen = 0;
	FileCommon::DataManager::getCommonDataBuffer(*VariableField_,
												 fieldBuffer,
												 fieldLen);

	bool	isCompressed = false;

	Os::Memory::Size	uncompressedSize = 0;

	if ((VariableField_->getFunction() &
		 Common::Data::Function::Compressed)
		!= 0)
	{
		// 圧縮されているかもしれない…

		const Common::CompressedData*	compressedVariableField =
			dynamic_cast<const Common::CompressedData*>(VariableField_);

		; _SYDNEY_ASSERT(compressedVariableField != 0);

		if (compressedVariableField->isCompressed())
		{
			isCompressed = true;

			uncompressedSize = compressedVariableField->getValueSize();
		}
	}

	if (isCompressed)
	{
		// 圧縮されている…

		return
			this->writeCompressedOutsideVariableValue(
				static_cast<const char*>(fieldBuffer),
				uncompressedSize,
				fieldLen, // これが圧縮後のフィールド長
				0,        // バッファのインデックス
				SearchStartPageID_,
				CatchMemoryExhaust_,
				AttachValuePages_,
				AllocateValuePageIDs_);
	}
	else
	{
		// 圧縮されていない…

		return
			this->writeOutsideVariableValue(VariableField_->getType(),
											static_cast<const char*>(fieldBuffer),
											fieldLen,
											0, // バッファのインデックス
											SearchStartPageID_,
											CatchMemoryExhaust_,
											AttachValuePages_,
											AllocateValuePageIDs_);
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::writeCompressedOutsideVariableValue --
//		圧縮されている外置き可変長バリューフィールドを書き込む
//
//	NOTES
//	圧縮されている外置き可変長バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const char*					FieldBuffer_
//		圧縮されている可変長バリューフィールドの値が記録されている
//		バッファへのポインタ
//	const Os::Memory::Size		UncompressedSize_
//		圧縮前のフィールド長
//	const Os::Memory::Size		CompressedSize_
//		圧縮後のフィールド長
//	const int					BufferIndex_
//		引数FieldBuffer_の書き込み開始インデックス
//	const PhysicalFile::PageID	SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool					CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&		AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//	const bool					IsRecur_ = false
//		再帰呼び出しかどうか
//			true  : 再帰呼び出し
//			false : 再帰呼び出しではない
//
//	RETURN
//	ModUInt64
//		外置き可変長バリューフィールドオブジェクトのID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::writeCompressedOutsideVariableValue(
	const char*					FieldBuffer_,
	const Os::Memory::Size		UncompressedSize_,
	const Os::Memory::Size		CompressedSize_,
	const int					BufferIndex_,
	const PhysicalFile::PageID	SearchStartPageID_,
	const bool					CatchMemoryExhaust_,
	PageVector&					AttachValuePages_,
	PageIDVector&				AllocateValuePageIDs_,
	const bool					IsRecur_ // = false
	) const
{
	; _SYDNEY_ASSERT(FieldBuffer_ != 0);

	Os::Memory::Size	writeBufferLen = CompressedSize_ - BufferIndex_;

	// 圧縮前後のフィールド長を記録するために必要なサイズ
	Os::Memory::Size	fieldLenArchiveSize = sizeof(Os::Memory::Size) << 1;

	// フィールド値以外に必要なサイズ
	Os::Memory::Size	otherSize =
		File::ObjectTypeArchiveSize; // オブジェクトタイプ

	if (IsRecur_ == false)
	{
		// 再帰呼び出しではない（先頭のフィールドオブジェクト）…

		//
		// フィールド値以外に必要なサイズに、
		// 圧縮前後のフィールド長の記録サイズを加算する。
		//

		otherSize += fieldLenArchiveSize;
	}

	Os::Memory::Size	demandAreaSize = writeBufferLen + otherSize;

	File::ObjectType	objectType = File::CompressedObjectType;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (demandAreaSize > this->m_PageFreeSizeMax)
	{
		// 1つの物理エリアでは収まらない（＝1つの物理ページでは収まらない）…

		//
		// では、リンクオブジェクトにして複数の物理エリアに記録する。
		//

		objectType = File::DivideCompressedObjectType;

		otherSize += File::ObjectIDArchiveSize;

		// 1つの物理エリアに記録可能なフィールド長を求める
		writeBufferLen = this->m_PageFreeSizeMax - otherSize;

		demandAreaSize = writeBufferLen + otherSize;

		// 再帰呼び出し
		nextLinkObjectID =
			this->writeCompressedOutsideVariableValue(
				FieldBuffer_,
				UncompressedSize_,
				CompressedSize_,
				BufferIndex_ + writeBufferLen,
				SearchStartPageID_,
				CatchMemoryExhaust_,
				AttachValuePages_,
				AllocateValuePageIDs_,
				true); // 再帰呼び出し
	}

	PhysicalFile::Page*	fieldObjectPage =
		this->searchInsertPage(0, // dummy number ob objects in file
							   demandAreaSize,
							   SearchStartPageID_,
							   CatchMemoryExhaust_,
							   AttachValuePages_,
							   AllocateValuePageIDs_);

	PhysicalFile::PageID	fieldObjectPageID = fieldObjectPage->getID();

	PhysicalFile::AreaID	fieldObjectAreaID =
		fieldObjectPage->allocateArea(*this->m_Transaction,
									  demandAreaSize);

	//
	// オブジェクトタイプを書き込む
	//

	File::ObjectType*	objectTypeWritePos =
		static_cast<File::ObjectType*>(File::getAreaTop(fieldObjectPage,
														fieldObjectAreaID));

#ifdef DEBUG

	char*	fieldObjectAreaTop =
		syd_reinterpret_cast<char*>(objectTypeWritePos);

#endif

	*objectTypeWritePos = objectType;

	char*	fieldWritePos = 0;

	if (objectType == File::DivideCompressedObjectType)
	{
		// リンクオブジェクト…

		; _SYDNEY_ASSERT(
			nextLinkObjectID != FileCommon::ObjectID::Undefined);

		char*	nextLinkObjectIDWritePos =
			syd_reinterpret_cast<char*>(objectTypeWritePos + 1);

		File::writeObjectID(nextLinkObjectIDWritePos, nextLinkObjectID);

		fieldWritePos =
			nextLinkObjectIDWritePos + File::ObjectIDArchiveSize;
	}
	else
	{
		// リンクオブジェクトではない…

		fieldWritePos =
			syd_reinterpret_cast<char*>(objectTypeWritePos + 1);
	}

	if (IsRecur_ == false)
	{
		// 再帰呼び出しではない（先頭のフィールドオブジェクト）…

		//
		// 圧縮前後のフィールド長を書き込む
		//

		Os::Memory::Size*	fieldLenWritePos =
			syd_reinterpret_cast<Os::Memory::Size*>(fieldWritePos);

		*fieldLenWritePos++ = UncompressedSize_;
		*fieldLenWritePos = CompressedSize_;

		fieldWritePos += fieldLenArchiveSize;
	}

	; _SYDNEY_ASSERT(fieldWritePos != 0);

	; _SYDNEY_ASSERT(writeBufferLen > 0);

	Os::Memory::copy(fieldWritePos,
					 FieldBuffer_ + BufferIndex_, writeBufferLen);

#ifdef DEBUG

	Os::Memory::Size	fieldObjectAreaSize =
		fieldObjectPage->getAreaSize(fieldObjectAreaID);

	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(
			fieldWritePos + writeBufferLen - fieldObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == fieldObjectAreaSize);

#endif

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return File::makeObjectID(fieldObjectPageID, fieldObjectAreaID);
}

//
//	FUNCTION private
//	Btree::ValueFile::writeOutsideVariableValue -- 
//		外置き可変長バリューフィールドを書き込む
//
//	NOTES
//	外置き可変長バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const Common::DataType::Type	FieldType_
//		バリューフィールドのデータ型
//	const char*						FieldBuffer_
//		バリューフィールドの値が記録されているバッファへのポインタ
//	const Os::Memory::Size			FieldLength_
//		バリューフィールドの値のバイト数 [byte]
//	const int						BufferIndex_
//		バッファのインデックス
//	const PhysicalFile::PageID		SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	PageVector&						AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	PageIDVector&					AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	ModUInt64
//		外置き可変長バリューフィールドオブジェクトのID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::writeOutsideVariableValue(
	const Common::DataType::Type	FieldType_,
	const char*						FieldBuffer_,
	const Os::Memory::Size			FieldLength_,
	const int						BufferIndex_,
	const PhysicalFile::PageID		SearchStartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_) const
{
	; _SYDNEY_ASSERT(FieldBuffer_ != 0);

	Os::Memory::Size	writeBufferLen = FieldLength_ - BufferIndex_;

	// フィールド値以外に必要なサイズ
	Os::Memory::Size	otherSize =
		File::ObjectTypeArchiveSize; // オブジェクトタイプ

	Os::Memory::Size	demandAreaSize = writeBufferLen + otherSize;

	File::ObjectType	objectType = File::NormalObjectType;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (demandAreaSize > this->m_PageFreeSizeMax)
	{
		// 1つの物理エリアでは収まらない（＝1つの物理ページでは収まらない）…

		//
		// では、リンクオブジェクトにして複数の物理エリアに記録する。
		//

		objectType = File::DivideObjectType;

		otherSize += File::ObjectIDArchiveSize;

		// 1つの物理エリアに記録可能なフィールド長を求める
		writeBufferLen = this->m_PageFreeSizeMax - otherSize;

		//
		// String型のフィールドの場合には偶数バイトずつ分割する。
		//

		if (FieldType_ == Common::DataType::String &&
			writeBufferLen & 0x01)
		{
			writeBufferLen--;
		}

		demandAreaSize = writeBufferLen + otherSize;

		// 再帰呼び出し
		nextLinkObjectID =
			this->writeOutsideVariableValue(FieldType_,
											FieldBuffer_,
											FieldLength_,
											BufferIndex_ + writeBufferLen,
											SearchStartPageID_,
											CatchMemoryExhaust_,
											AttachValuePages_,
											AllocateValuePageIDs_);
	}

	PhysicalFile::Page*	fieldObjectPage =
		this->searchInsertPage(0, // dummy number ob objects in file
							   demandAreaSize,
							   SearchStartPageID_,
							   CatchMemoryExhaust_,
							   AttachValuePages_,
							   AllocateValuePageIDs_);

	PhysicalFile::PageID	fieldObjectPageID = fieldObjectPage->getID();

	PhysicalFile::AreaID	fieldObjectAreaID =
		fieldObjectPage->allocateArea(*this->m_Transaction,
									  demandAreaSize);

	//
	// オブジェクトタイプを書き込む
	//

	File::ObjectType*	objectTypeWritePos =
		static_cast<File::ObjectType*>(File::getAreaTop(fieldObjectPage,
														fieldObjectAreaID));

#ifdef DEBUG

	char*	fieldObjectAreaTop =
		syd_reinterpret_cast<char*>(objectTypeWritePos);

#endif

	*objectTypeWritePos = objectType;

	char*	fieldWritePos = 0;

	if (objectType == File::DivideObjectType)
	{
		// リンクオブジェクト…

		; _SYDNEY_ASSERT(
			nextLinkObjectID != FileCommon::ObjectID::Undefined);

		//
		// 次のオブジェクトのオブジェクトIDを書き込む
		//

		char*	nextLinkObjectIDWritePos =
			syd_reinterpret_cast<char*>(objectTypeWritePos + 1);

		File::writeObjectID(nextLinkObjectIDWritePos, nextLinkObjectID);

		fieldWritePos =
			nextLinkObjectIDWritePos + File::ObjectIDArchiveSize;
	}
	else
	{
		// リンクオブジェクトではない…

		fieldWritePos =
			syd_reinterpret_cast<char*>(objectTypeWritePos + 1);
	}

	if (writeBufferLen > 0)
	{
		Os::Memory::copy(fieldWritePos,
						 FieldBuffer_ + BufferIndex_, writeBufferLen);
	}

#ifdef DEBUG

	Os::Memory::Size	fieldObjectAreaSize =
		fieldObjectPage->getAreaSize(fieldObjectAreaID);

	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(
			fieldWritePos + writeBufferLen - fieldObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == fieldObjectAreaSize);

#endif

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return File::makeObjectID(fieldObjectPageID, fieldObjectAreaID);
}

//
//	FUNCTION private
//	Btree::ValueFile::writeArrayFields -- 配列バリューフィールドを書き込む
//
//	NOTES
//	配列バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const Common::DataArrayData*	Object_
//		オブジェクトへのポインタ
//	const PhysicalFile::PageID		SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//	ModVector<ModUInt64>&			ObjectIDs_
//		配列バリューフィールドオブジェクトの
//		オブジェクトIDを格納するベクターへの参照
//	const OpenParameter*			OpenParam_ = 0
//		オープンパラメータへのポインタ
//		※ オブジェクト更新時に設定する。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::writeArrayFields(
	const Common::DataArrayData*	Object_,
	const PhysicalFile::PageID		SearchStartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_,
	ModVector<ModUInt64>&			ObjectIDs_,
	const OpenParameter*			OpenParam_ // = 0
	) const
{
	bool	selected = true;

	if (this->m_FileParam->m_ExistArrayFieldInValue)
	{
		for (int i = this->m_FileParam->m_TopValueFieldIndex;
			 i < this->m_FileParam->m_FieldNum;
			 i++)
		{
			if (OpenParam_ != 0)
			{
				// オブジェクト更新時に呼ばれた…

				selected = File::isSelected(OpenParam_, i);
			}

			if (selected &&
				*(this->m_FileParam->m_IsArrayFieldArray + i))
			{
				Common::Data*	arrayField = Object_->getElement(i).get();

				; _SYDNEY_ASSERT(arrayField != 0);

				if (!arrayField->isNull())
				{
					Common::DataType::Type	elementType =
						*(this->m_FileParam->m_ElementTypeArray + i);

					ModUInt64	objectID =
						this->writeArrayField(arrayField,
											  elementType,
											  SearchStartPageID_,
											  CatchMemoryExhaust_,
											  AttachValuePages_,
											  AllocateValuePageIDs_);

					ObjectIDs_.pushBack(objectID);
				}
			}
		}
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::writeArrayField --
//		配列バリューフィールドを書き込む
//
//	NOTES
//	配列バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const Common::Data*				ArrayField_
//		配列バリューフィールドデータへのポインタ
//	const Common::DataType::Type	ElementType_
//		要素のデータ型
//	const PhysicalFile::PageID		SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	ModUInt64
//		配列バリューフィールドオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::writeArrayField(
	const Common::Data*				ArrayField_,
	const Common::DataType::Type	ElementType_,
	const PhysicalFile::PageID		SearchStartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_) const
{
	; _SYDNEY_ASSERT(ArrayField_->getType() == Common::DataType::Array);

	const Common::DataArrayData*	arrayField =
		_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, ArrayField_);

	; _SYDNEY_ASSERT(arrayField != 0);

	if (FileCommon::DataManager::isVariable(ElementType_))
	{
		// 可変長要素…

		return
			this->writeVariableElementArrayField(
				arrayField,
				SearchStartPageID_,
				CatchMemoryExhaust_,
				AttachValuePages_,
				AllocateValuePageIDs_);
	}
	else
	{
		// 固定長要素…

		return
			this->writeFixedElementArrayField(
				arrayField,
				arrayField->getCount(),
				0, // 書き込み開始インデックス
				ElementType_,
				SearchStartPageID_,
				CatchMemoryExhaust_,
				AttachValuePages_,
				AllocateValuePageIDs_);
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::getPutableElementNumPerArea --
//		1つの物理エリアに記録可能な要素数を返す
//
//	NOTES
//	1つの物理エリアに記録可能な要素数を返す。
//
//	ARGUMENTS
//	const Os::Memory::Size	ElementSize_
//		1要素のサイズ [byte]
//
//	RETURN
//	int
//		1つの物理エリアに記録可能な要素数
//
//	EXCEPTIONS
//	なし
//
int
ValueFile::getPutableElementNumPerArea(
	const Os::Memory::Size	ElementSize_) const
{
	// 配列フィールドオブジェクト内の
	// 要素の値の列と、ヌルビットマップ以外に
	// 必要なサイズ
	Os::Memory::Size	otherSize =
		File::ObjectTypeArchiveSize + // オブジェクトタイプ
		File::ObjectIDArchiveSize +   // 次のオブジェクトのID
		File::ModUInt32ArchiveSize;   // 要素数

	int	elementNumPerArea = 10;

	while (true)
	{
		Os::Memory::Size	totalSize =
			ElementSize_ * elementNumPerArea +       // 要素の値の列
			NullBitmap::getSize(elementNumPerArea) + // ヌルビットマップ
			otherSize;                               // その他

		if (totalSize > this->m_PageFreeSizeMax)
		{
			elementNumPerArea -= 10;

			break;
		}

		elementNumPerArea += 10;
	}

	for (int i = 0; i < 10; i++, elementNumPerArea++)
	{
		Os::Memory::Size	totalSize =
			ElementSize_ * elementNumPerArea +       // 要素の値の列
			NullBitmap::getSize(elementNumPerArea) + // ヌルビットマップ
			otherSize;                               // その他

		if (totalSize > this->m_PageFreeSizeMax)
		{
			elementNumPerArea--;

			break;
		}
	}

	return elementNumPerArea;
}

//
//	FUNCTION private
//	Btree::ValueFile::writeVariableElementArrayField --
//		可変長要素の配列バリューフィールドを書き込む
//
//	NOTES
//	可変長要素の配列バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const Common::DataArrayData*	ArrayField_
//		配列バリューフィールドデータへのポインタ
//	const PhysicalFile::PageID		SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	ModUInt64
//		配列バリューフィールドオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::writeVariableElementArrayField(
	const Common::DataArrayData*	ArrayField_,
	const PhysicalFile::PageID		SearchStartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_) const
{
	; _SYDNEY_ASSERT(ArrayField_ != 0);

	//
	// まずは、ひとつひとつの要素ごとに、
	// 可変長フィールドオブジェクトとして、
	// 個別の物理エリアに要素の値を書き込み、
	// それぞれのオブジェクトIDを取得して、
	// ベクターを形成する。
	//

	int	elementNum = ArrayField_->getCount();

	ModVector<ModUInt64>	elementObjectIDs;

	elementObjectIDs.reserve(elementNum);

	for (int i = 0; i < elementNum; i++)
	{
		Common::Data*	element = ArrayField_->getElement(i).get();

		ModUInt64	elementObjectID = FileCommon::ObjectID::Undefined;

		if (!element->isNull())
		{
			elementObjectID =
				this->writeOutsideVariableValue(element,
												SearchStartPageID_,
												CatchMemoryExhaust_,
												AttachValuePages_,
												AllocateValuePageIDs_);
		}

		elementObjectIDs.pushBack(elementObjectID);
	}

	; _SYDNEY_ASSERT(
		static_cast<ModSize>(elementNum) == elementObjectIDs.getSize());

	//
	// 次に、各要素ごとのフィールドオブジェクトIDの列を書き込む。
	//

	return
		this->writeVariableElementArrayField(
			elementObjectIDs,
			elementNum,
			0, // 書き込み開始インデックス
			SearchStartPageID_,
			CatchMemoryExhaust_,
			AttachValuePages_,
			AllocateValuePageIDs_);
}

//
//	FUNCTION private
//	Btree::ValueFile::writeVariableElementArrayField --
//		可変長要素の配列バリューフィールドを書き込む
//
//	NOTES
//	可変長要素の配列バリューフィールドを書き込む。
//	（各要素のフィールドオブジェクトIDの列を書き込む。）
//
//	ARGUMENTS
//	ModVector<ModUInt64>&			ObjectIDs_
//		各要素のフィールドオブジェクトIDの列
//	const int						ElementNum_
//		引数ObjectIDs_の要素数
//	const int						StartElementIndex_
//		引数ObjectIDs_の書き込み開始インデックス
//	const PhysicalFile::PageID		SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	ModUInt64
//		配列バリューフィールドオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::writeVariableElementArrayField(
	ModVector<ModUInt64>&			ObjectIDs_,
	const int						ElementNum_,
	const int						StartElementIndex_,
	const PhysicalFile::PageID		SearchStartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_) const
{
	; _SYDNEY_ASSERT(ElementNum_ > 0);
	; _SYDNEY_ASSERT(
		StartElementIndex_ >= 0 && StartElementIndex_ < ElementNum_);

	//
	// 配列フィールドオブジェクトを記録するための
	// 物理エリアサイズを求める。
	//

	int	writeElementNum = ElementNum_ - StartElementIndex_;

	NullBitmap::Size	nullBitmapSize =
		NullBitmap::getSize(writeElementNum);

	// オブジェクトID列以外に必要なサイズ
	Os::Memory::Size	otherSize =
		File::ObjectTypeArchiveSize + // オブジェクトタイプ
		File::ModUInt32ArchiveSize +  // 要素数
		nullBitmapSize;               // ヌルビットマップ

	Os::Memory::Size	demandAreaSize =
		File::ObjectIDArchiveSize * writeElementNum + otherSize;

	File::ObjectType	objectType = File::ArrayObjectType;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (demandAreaSize > this->m_PageFreeSizeMax)
	{
		// 1つの物理エリアでは収まらない…

		//
		// では、リンクオブジェクトにして複数の物理エリアに記録する。
		//

		objectType = File::DivideArrayObjectType;

		// 1つの物理エリアに記録可能な要素数を求める
		writeElementNum =
			this->getPutableElementNumPerArea(File::ObjectIDArchiveSize);

		//
		// 必要なサイズを再計算する
		//

		//
		// リンクオブジェクトには、“次のオブジェクトのID”を
		// 別途記録する必要がある。
		//

		nullBitmapSize = NullBitmap::getSize(writeElementNum);

		otherSize =
			File::ObjectTypeArchiveSize + // オブジェクトタイプ
			File::ObjectIDArchiveSize +   // 次のオブジェクトのID
			File::ModUInt32ArchiveSize +  // 要素数
			nullBitmapSize;               // ヌルビットマップ

		demandAreaSize =
			File::ObjectIDArchiveSize * writeElementNum + otherSize;

		// 再帰呼び出し
		nextLinkObjectID =
			this->writeVariableElementArrayField(
				ObjectIDs_,
				ElementNum_,
				StartElementIndex_ + writeElementNum,
				SearchStartPageID_,
				CatchMemoryExhaust_,
				AttachValuePages_,
				AllocateValuePageIDs_);
	}

	PhysicalFile::Page*	fieldObjectPage =
		this->searchInsertPage(0, // dummy number ob objects in file
							   demandAreaSize,
							   SearchStartPageID_,
							   CatchMemoryExhaust_,
							   AttachValuePages_,
							   AllocateValuePageIDs_);

	PhysicalFile::PageID	fieldObjectPageID = fieldObjectPage->getID();

	PhysicalFile::AreaID	fieldObjectAreaID =
		fieldObjectPage->allocateArea(*this->m_Transaction,
									  demandAreaSize);

	//
	// オブジェクトタイプを書き込む
	//

	File::ObjectType*	objectTypeWritePos =
		static_cast<File::ObjectType*>(File::getAreaTop(fieldObjectPage,
														fieldObjectAreaID));

#ifdef DEBUG

	char*	fieldObjectAreaTop =
		syd_reinterpret_cast<char*>(objectTypeWritePos);

#endif

	*objectTypeWritePos = objectType;

	int*	elementNumWritePos = 0;

	if (objectType == File::DivideArrayObjectType)
	{
		// リンクオブジェクト…

		; _SYDNEY_ASSERT(
			nextLinkObjectID != FileCommon::ObjectID::Undefined);

		//
		// 次のオブジェクトのオブジェクトIDを書き込む
		//

		char*	nextLinkObjectIDWritePos =
			syd_reinterpret_cast<char*>(objectTypeWritePos + 1);

		File::writeObjectID(nextLinkObjectIDWritePos, nextLinkObjectID);

		elementNumWritePos =
			syd_reinterpret_cast<int*>(
				nextLinkObjectIDWritePos + File::ObjectIDArchiveSize);
	}
	else
	{
		elementNumWritePos =
			syd_reinterpret_cast<int*>(objectTypeWritePos + 1);
	}

	//
	// 要素数を書き込む
	//

	; _SYDNEY_ASSERT(elementNumWritePos != 0);

	*elementNumWritePos = writeElementNum;

	NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(elementNumWritePos + 1);

	NullBitmap	nullBitmap(nullBitmapTop,
						   writeElementNum,
						   NullBitmap::Access::ReadWrite);

	nullBitmap.clear();

	char*	objectIDWritePos = static_cast<char*>(nullBitmap.getTail());

	//
	// 可変長要素オブジェクトのオブジェクトIDを書き込む
	// （必要に応じてヌルビットマップのビットもONする）
	//

	int	objectIDIndex = StartElementIndex_;

	for (int i = 0; i < writeElementNum; i++, objectIDIndex++)
	{
		ModUInt64	objectID = ObjectIDs_[objectIDIndex];

		if (objectID == FileCommon::ObjectID::Undefined)
		{
			nullBitmap.on(i);
		}

		objectIDWritePos = File::writeObjectID(objectIDWritePos, objectID);
	}

#ifdef DEBUG

	Os::Memory::Size	fieldObjectAreaSize =
		fieldObjectPage->getAreaSize(fieldObjectAreaID);

	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(
			objectIDWritePos - fieldObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == fieldObjectAreaSize);

#endif

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
				fieldObjectPage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
	}

	return File::makeObjectID(fieldObjectPageID, fieldObjectAreaID);
}

//
//	FUNCTION private
//	Btree::ValueFile::writeFixedElementArrayField --
//		固定長要素の配列バリューフィールドを書き込む
//
//	NOTES
//	固定長要素の配列バリューフィールドを書き込む。
//
//	ARGUMENTS
//	const Common::DataArrayData*	ArrayField_
//		配列バリューフィールドデータへのポインタ
//	const int						ElementNum_
//		引数ArrayField_の要素数
//	const int						StartElementIndex_
//		引数ArrayField_の書き込み開始インデックス
//	const Common::DataType::Type	ElementType_
//		要素のデータ型
//	const PhysicalFile::PageID		SearchStartPageID_
//		検索開始バリューページの物理ページ識別子
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&			AllocateValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（アロケートしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	ModUInt64
//		配列バリューフィールドオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
ValueFile::writeFixedElementArrayField(
	const Common::DataArrayData*	ArrayField_,
	const int						ElementNum_,
	const int						StartElementIndex_,
	const Common::DataType::Type	ElementType_,
	const PhysicalFile::PageID		SearchStartPageID_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_,
	PageIDVector&					AllocateValuePageIDs_) const
{
	; _SYDNEY_ASSERT(ArrayField_ != 0);
	; _SYDNEY_ASSERT(ElementNum_ > 0);
	; _SYDNEY_ASSERT(
		StartElementIndex_ >= 0 && StartElementIndex_ < ElementNum_);

	//
	// 配列フィールドオブジェクトを記録するための
	// 物理エリアサイズを求める。
	//

	int	writeElementNum = ElementNum_ - StartElementIndex_;

	NullBitmap::Size	nullBitmapSize =
		NullBitmap::getSize(writeElementNum);

	// フィールド値列以外に必要なサイズ
	Os::Memory::Size	otherSize =
		File::ObjectTypeArchiveSize + // オブジェクトタイプ
		File::ModUInt32ArchiveSize +  // 要素数
		nullBitmapSize;               // ヌルビットマップ

	Os::Memory::Size	elementSize =
		FileCommon::DataManager::getFixedCommonDataArchiveSize(ElementType_);

	Os::Memory::Size	demandAreaSize =
		elementSize * writeElementNum + otherSize;

	File::ObjectType	objectType = File::ArrayObjectType;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (demandAreaSize > this->m_PageFreeSizeMax)
	{
		// 1つの物理エリアでは収まらない…

		//
		// では、リンクオブジェクトにして複数の物理エリアに記録する。
		//

		objectType = File::DivideArrayObjectType;

		// 1つの物理エリアに記録可能な要素数を求める
		writeElementNum = this->getPutableElementNumPerArea(elementSize);

		//
		// 必要なサイズを再計算する
		//

		//
		// リンクオブジェクトには、“次のオブジェクトのID”を
		// 別途記録する必要がある。
		//

		nullBitmapSize = NullBitmap::getSize(writeElementNum);

		otherSize =
			File::ObjectTypeArchiveSize + // オブジェクトタイプ
			File::ObjectIDArchiveSize +   // 次のオブジェクトのID
			File::ModUInt32ArchiveSize +  // 要素数
			nullBitmapSize;               // ヌルビットマップ

		demandAreaSize =
			elementSize * writeElementNum + otherSize;

		// 再帰呼び出し
		nextLinkObjectID =
			this->writeFixedElementArrayField(
				ArrayField_,
				ElementNum_,
				StartElementIndex_ + writeElementNum,
				ElementType_,
				SearchStartPageID_,
				CatchMemoryExhaust_,
				AttachValuePages_,
				AllocateValuePageIDs_);
	}

	PhysicalFile::Page*	fieldObjectPage =
		this->searchInsertPage(0, // dummy number ob objects in file
							   demandAreaSize,
							   SearchStartPageID_,
							   CatchMemoryExhaust_,
							   AttachValuePages_,
							   AllocateValuePageIDs_);

	PhysicalFile::PageID	fieldObjectPageID = fieldObjectPage->getID();

	PhysicalFile::AreaID	fieldObjectAreaID =
		fieldObjectPage->allocateArea(*this->m_Transaction,
									  demandAreaSize);

	//
	// オブジェクトタイプを書き込む
	//

	File::ObjectType*	objectTypeWritePos =
		static_cast<File::ObjectType*>(File::getAreaTop(fieldObjectPage,
														fieldObjectAreaID));

#ifdef DEBUG

	char*	fieldObjectAreaTop =
		syd_reinterpret_cast<char*>(objectTypeWritePos);

#endif

	*objectTypeWritePos = objectType;

	int*	elementNumWritePos = 0;

	if (objectType == File::DivideArrayObjectType)
	{
		// リンクオブジェクト…

		; _SYDNEY_ASSERT(
			nextLinkObjectID != FileCommon::ObjectID::Undefined);

		//
		// 次のオブジェクトのオブジェクトIDを書き込む
		//

		char*	nextLinkObjectIDWritePos =
			syd_reinterpret_cast<char*>(objectTypeWritePos + 1);

		File::writeObjectID(nextLinkObjectIDWritePos, nextLinkObjectID);

		elementNumWritePos =
			syd_reinterpret_cast<int*>(
				nextLinkObjectIDWritePos + File::ObjectIDArchiveSize);
	}
	else
	{
		elementNumWritePos =
			syd_reinterpret_cast<int*>(objectTypeWritePos + 1);
	}

	//
	// 要素数を書き込む
	//

	; _SYDNEY_ASSERT(elementNumWritePos != 0);

	*elementNumWritePos = writeElementNum;

	NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<NullBitmap::Value*>(elementNumWritePos + 1);

	NullBitmap	nullBitmap(nullBitmapTop,
						   writeElementNum,
						   NullBitmap::Access::ReadWrite);

	nullBitmap.clear();

	char*	elementWritePos = static_cast<char*>(nullBitmap.getTail());

	//
	// 固定長要素の値を書き込む
	// （必要に応じてヌルビットマップのビットもONする）
	//

	Os::Memory::Size	nullSkipSize =
		FileCommon::DataManager::getFixedCommonDataArchiveSize(ElementType_);

	int	elementIndex = StartElementIndex_;

	for (int i = 0; i < writeElementNum; i++, elementIndex++)
	{
		Common::Data*	element =
			ArrayField_->getElement(elementIndex).get();

		if (element->isNull())
		{
			nullBitmap.on(i);

			elementWritePos += nullSkipSize;
		}
		else
		{
			elementWritePos = File::writeFixedField(elementWritePos,
													element);
		}
	}

#ifdef DEBUG

	Os::Memory::Size	fieldObjectAreaSize =
		fieldObjectPage->getAreaSize(fieldObjectAreaID);

	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(elementWritePos - fieldObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == fieldObjectAreaSize);

#endif

	if (CatchMemoryExhaust_)
	{
		this->m_PhysicalFile->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return File::makeObjectID(fieldObjectPageID, fieldObjectAreaID);
}

// Common::Data::copyの仕様が変わった都合で引数を修正

//
//	FUNCTION private
//	Btree::ValueFile::readArrayField -- 配列バリューフィールドを読み込む
//
//	NOTES
//	配列バリューフィールドを読み込む。
//
//	ARGUMENTS
//	const int				FieldIndex_
//		フィールドインデックス
//	PhysicalFile::Page*		DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const char*				ValueTop_
//		代表オブジェクトが記録されている物理エリア内の
//		配列バリューフィールドオブジェクトのオブジェクトIDが
//		記録されている領域へのポインタ
//	Common::DataArrayData&	ArrayField_
//		配列バリューフィールド読み込み先への参照
//	const bool				CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	const char*
//		代表オブジェクトが記録されている物理エリア内の、
//		次のバリューフィールドが記録されている領域先頭へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
const char*
ValueFile::readArrayField(const int					FieldIndex_,
						  PhysicalFile::Page*		DirectObjectPage_,
						  const char*				ValueTop_,
						  Common::DataArrayData&	ArrayField_,
						  const bool				CatchMemoryExhaust_,
						  PageVector&				AttachValuePages_) const
{
	; _SYDNEY_ASSERT(DirectObjectPage_ != 0);
	; _SYDNEY_ASSERT(ValueTop_ != 0);

	const char*	nextFieldTop = 0;

	Common::DataType::Type	elementType =
		*(this->m_FileParam->m_ElementTypeArray + FieldIndex_);

	ModUInt64 objectID;
	nextFieldTop = File::readObjectID(ValueTop_, objectID);
	; _SYDNEY_ASSERT(objectID != FileCommon::ObjectID::Undefined);

	this->readArrayField(DirectObjectPage_,
						 objectID,
						 elementType,
						 ArrayField_,
						 CatchMemoryExhaust_,
						 AttachValuePages_);

	return nextFieldTop;
}

//
//	FUNCTION private
//	Btree::ValueFile::readFixedElementArrayField --
//		固定長要素の配列バリューフィールドを読み込む
//
//	NOTES
//	固定長要素の配列バリューフィールドを読み込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*				DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const ModUInt64					ArrayFieldObjectID_
//		配列バリューフィールドオブジェクトのオブジェクトID
//	const Common::DataType::Type	ElementType_
//		要素のデータ型
//	Common::DataArrayData&			ArrayField_
//		配列バリューフィールド読込先への参照
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::readFixedElementArrayField(
	PhysicalFile::Page*				DirectObjectPage_,
	const ModUInt64					ArrayFieldObjectID_,
	const Common::DataType::Type	ElementType_,
	Common::DataArrayData&			ArrayField_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_) const
{
	; _SYDNEY_ASSERT(DirectObjectPage_ != 0);
	; _SYDNEY_ASSERT(
		ArrayFieldObjectID_ != FileCommon::ObjectID::Undefined);

	PhysicalFile::Page*	arrayFieldObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_Transaction,
											  ArrayFieldObjectID_,
											  DirectObjectPage_,
											  arrayFieldObjectPage,
											  this->m_FixMode,
											  CatchMemoryExhaust_,
											  AttachValuePages_);
	; _SYDNEY_ASSERT(arrayFieldObjectPage != 0);

	const PhysicalFile::AreaID arrayFieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(ArrayFieldObjectID_);

	const char* p = static_cast<const char*>(
		File::getConstAreaTop(arrayFieldObjectPage, arrayFieldObjectAreaID));

	File::ObjectType objectType;
	(void) Os::Memory::copy(&objectType, p, sizeof(objectType));
	p += sizeof(objectType);

	ModUInt64 nextLinkObjectID = FileCommon::ObjectID::Undefined;
	if (objectType == File::DivideArrayObjectType)
		p = File::readObjectID(p, nextLinkObjectID);

	int elementNum;
	(void) Os::Memory::copy(&elementNum, p, sizeof(elementNum));
	p += sizeof(elementNum);

	ArrayField_.reserve(ArrayField_.getCount() + elementNum);

	const NullBitmap::Value* nullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(p);

	NullBitmap nullBitmap(
		nullBitmapTop, elementNum, NullBitmap::Access::ReadOnly);

	bool	existNull = nullBitmap.existNull();

	const char*	elementReadPos =
		static_cast<const char*>(nullBitmap.getConstTail());

	Os::Memory::Size	nullSkipSize =
		FileCommon::DataManager::getFixedCommonDataArchiveSize(ElementType_);

	for (int i = 0; i < elementNum; i++)
	{
		Common::Data::Pointer	element;

		if (existNull && nullBitmap.isNull(i))
		{
			element = Common::NullData::getInstance();

			elementReadPos += nullSkipSize;
		}
		else
		{
			elementReadPos = File::readFixedField(ElementType_,
												  elementReadPos,
												  element);

			; _SYDNEY_ASSERT(element.get() != 0);
		}

		ArrayField_.pushBack(element);
	}

	if (CatchMemoryExhaust_ && attached)
	{
		this->m_PhysicalFile->detachPage(
			arrayFieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		this->readFixedElementArrayField(DirectObjectPage_,
										 nextLinkObjectID,
										 ElementType_,
										 ArrayField_,
										 CatchMemoryExhaust_,
										 AttachValuePages_);
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::readVariableElementArrayField --
//		可変長要素の配列バリューフィールドを読み込む
//
//	NOTES
//	可変長要素の配列バリューフィールドを読み込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*				DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const ModUInt64					ArrayFieldObjectID_
//		配列バリューフィールドオブジェクトのオブジェクトID
//	const Common::DataType::Type	ElementType_
//		要素のデータ型
//	Common::DataArrayData&			ArrayField_
//		配列バリューフィールド読込先への参照
//	const bool						CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&				AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::readVariableElementArrayField(
	PhysicalFile::Page*				DirectObjectPage_,
	const ModUInt64					ArrayFieldObjectID_,
	const Common::DataType::Type	ElementType_,
	Common::DataArrayData&			ArrayField_,
	const bool						CatchMemoryExhaust_,
	PageVector&						AttachValuePages_) const
{
	; _SYDNEY_ASSERT(DirectObjectPage_ != 0);
	; _SYDNEY_ASSERT(
		ArrayFieldObjectID_ != FileCommon::ObjectID::Undefined);

	PhysicalFile::Page*	arrayFieldObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_Transaction,
											  ArrayFieldObjectID_,
											  DirectObjectPage_,
											  arrayFieldObjectPage,
											  this->m_FixMode,
											  CatchMemoryExhaust_,
											  AttachValuePages_);

	; _SYDNEY_ASSERT(arrayFieldObjectPage != 0);

	const PhysicalFile::AreaID arrayFieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(ArrayFieldObjectID_);

	const char* p = static_cast<const char*>(
		File::getConstAreaTop(arrayFieldObjectPage, arrayFieldObjectAreaID));

	File::ObjectType objectType;
	(void) Os::Memory::copy(&objectType, p, sizeof(objectType));
	p += sizeof(objectType);

	ModUInt64 nextLinkObjectID = FileCommon::ObjectID::Undefined;
	if (objectType == File::DivideArrayObjectType)
		p = File::readObjectID(p, nextLinkObjectID);

	int elementNum;
	(void) Os::Memory::copy(&elementNum, p, sizeof(elementNum));
	p += sizeof(elementNum);

	ArrayField_.reserve(ArrayField_.getCount() + elementNum);

	const NullBitmap::Value* nullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(p);

	NullBitmap nullBitmap(
		nullBitmapTop, elementNum, NullBitmap::Access::ReadOnly);

	bool	existNull = nullBitmap.existNull();

	const char*	elementObjectIDReadPos =
		static_cast<const char*>(nullBitmap.getConstTail());

	for (int i = 0; i < elementNum; i++)
	{
		Common::Data::Pointer	element;

		ModUInt64	elementObjectID;

		elementObjectIDReadPos =
			File::readObjectID(elementObjectIDReadPos, elementObjectID);

		if (existNull && nullBitmap.isNull(i))
		{
			element = Common::NullData::getInstance();
		}
		else
		{
			File::readOutsideVariableField(this->m_Transaction,
										   arrayFieldObjectPage,
										   elementObjectID,
										   ElementType_,
										   element,
										   this->m_FixMode,
										   CatchMemoryExhaust_,
										   AttachValuePages_);
		}

		; _SYDNEY_ASSERT(element.get() != 0);

		ArrayField_.pushBack(element);
	}

	if (CatchMemoryExhaust_ && attached)
	{
		this->m_PhysicalFile->detachPage(
			arrayFieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		this->readVariableElementArrayField(DirectObjectPage_,
											nextLinkObjectID,
											ElementType_,
											ArrayField_,
											CatchMemoryExhaust_,
											AttachValuePages_);
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::writeNullField -- ヌル値を書き込む
//
//	NOTES
//	ヌル値を書き込む。
//
//	ARGUMENTS
//	NullBitmap::Value*	NullBitmapTop_
//		ヌルビットマップへのポインタ
//	char*				ValueTop_
//		バリューフィールド書き込み先へのポインタ
//	const int			ValueFieldIndex_
//		バリューフィールドインデックス
//
//	RETURN
//	char*
//		次のバリューフィールド書き込み先へのポインタ
//
//	EXCEPTIONS
//	なし
//
char*
ValueFile::writeNullField(NullBitmap::Value*	NullBitmapTop_,
						  char*					ValueTop_,
						  const int				ValueFieldIndex_) const
{
	; _SYDNEY_ASSERT(NullBitmapTop_ != 0);
	; _SYDNEY_ASSERT(ValueTop_ != 0);
	; _SYDNEY_ASSERT(
		ValueFieldIndex_ >= this->m_FileParam->m_TopValueFieldIndex &&
		ValueFieldIndex_ < this->m_FileParam->m_FieldNum);

	//
	// ヌルビットマップのビットをONする。
	//

	int	bitIndex =
		ValueFieldIndex_ - this->m_FileParam->m_TopValueFieldIndex;

	NullBitmap::on(NullBitmapTop_,
				   this->m_FileParam->m_ValueNum,
				   bitIndex);

	//
	// バリューフィールドの値の書き込みサイズ分をスキップする。
	//

	char*	nextValueTop = 0;

	if (*(this->m_FileParam->m_IsArrayFieldArray +
		  ValueFieldIndex_))
	{
		// 配列フィールド…

		//
		// 外置きオブジェクトのオブジェクトIDの書き込みサイズ分を
		// スキップする。
		//

		nextValueTop = ValueTop_ + File::ObjectIDArchiveSize;
	}
	else if (*(this->m_FileParam->m_IsFixedFieldArray + ValueFieldIndex_))
	{
		// 固定長フィールド…

		//
		// バリューフィールドの値の書き込みサイズ分をスキップする。
		//

		Common::DataType::Type	valueDataType =
			*(this->m_FileParam->m_FieldTypeArray + ValueFieldIndex_);

		Os::Memory::Size	valueSize =
			FileCommon::DataManager::getFixedCommonDataArchiveSize(
				valueDataType);

		nextValueTop = ValueTop_ + valueSize;
	}
	else
	{
		// 可変長フィールド…

		if (*(this->m_FileParam->m_FieldOutsideArray + ValueFieldIndex_))
		{
			// 外置き可変長フィールド…

			//
			// 外置きオブジェクトのオブジェクトIDの書き込みサイズ分を
			// スキップする。
			//

			nextValueTop = ValueTop_ + File::ObjectIDArchiveSize;
		}
		else
		{
			// 外置きではない可変長フィールド…

			//
			// キー値のサイズの書き込みサイズ分と
			// フィールド最大長分、スキップする。
			//

			//
			// 外置きではない可変長フィールドは、
			// 代表オブジェクト内で、
			// 下図のような物理構成となっている。
			//
			//     ┌───────────┐
			//     │　　フィールド長　　　│ 1[byte]
			//     ├───────────┤
			//     │　　フィールド値　　　│ *
			//     └───────────┘
			//

			//
			// ※ 外置きではない可変長フィールドでは、
			// 　 フィールド値を記録する領域として、
			// 　 フィールド最大長の分だけ確保してある。
			//

			nextValueTop =
				ValueTop_ +
				File::InsideVarFieldLenArchiveSize +
				*(this->m_FileParam->m_FieldMaxLengthArray +
				  ValueFieldIndex_);
		}
	}

	; _SYDNEY_ASSERT(nextValueTop != 0);

	return nextValueTop;
}

//
//	FUNCTION private
//	Btree::ValueFile::freeObjectArea --
//		オブジェクトが記録されている物理エリアを解放する
//
//	NOTES
//	オブジェクトが記録されている物理エリアを解放する。
//
//	ARGUMENTS
//	PhysicalFile::Page*			DirectObjectPage_
//		代表オブジェクトが記録されているバリューページの
//		物理ページ記述子
//	const PhysicalFile::AreaID	DirectObjectAreaID_
//		代表オブジェクトが記録されている物理エリアの識別子
//	const bool					CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDvector&		FreeValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（フリーしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::freeObjectArea(
	PhysicalFile::Page*			DirectObjectPage_,
	const PhysicalFile::AreaID	DirectObjectAreaID_,
	const bool					CatchMemoryExhaust_,
	PageVector&					AttachValuePages_,
	PageIDVector&				FreeValuePageIDs_) const
{
	// 外置きフィールドオブジェクトが記録されている物理エリアを解放する
	this->freeOutsideFieldObjectArea(DirectObjectPage_,
									 DirectObjectAreaID_,
									 CatchMemoryExhaust_,
									 AttachValuePages_,
									 FreeValuePageIDs_);

	DirectObjectPage_->freeArea(*this->m_Transaction,
								DirectObjectAreaID_);

	if (DirectObjectPage_->getTopAreaID(*this->m_Transaction) !=
		PhysicalFile::ConstValue::UndefinedAreaID)
	{
		DirectObjectPage_->compaction(*this->m_Transaction);
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::freeOutsideFieldObjectArea --
//		外置きバリューフィールドオブジェクトが記録されている
//		物理エリアを解放する
//
//	NOTES
//	外置きバリューフィールドオブジェクトが記録されている
//	物理エリアを解放する。
//
//	ARGUMENTS
//	PhysicalFile::Page*			DirectObjectPage_
//		代表オブジェクトが記録されているバリューページの
//		物理ページ記述子
//	const PhysicalFile::AreaID	DirectObjectAreaID_
//		代表オブジェクトが記録されている物理エリアの識別子
//	const bool					CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&			AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&		FreeValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（フリーしたバリューページの物理ページ識別子をつむ）
//	const OpenParameter*		OpenParam_ = 0
//		オープンパラメータへのポインタ
//		※ オブジェクト更新時に設定する。
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::freeOutsideFieldObjectArea(
	PhysicalFile::Page*			DirectObjectPage_,
	const PhysicalFile::AreaID	DirectObjectAreaID_,
	const bool					CatchMemoryExhaust_,
	PageVector&					AttachValuePages_,
	PageIDVector&				FreeValuePageIDs_,
	const OpenParameter*		OpenParam_ // = 0
	) const
{
	if (this->m_FileParam->m_ExistOutsideFieldInValue)
	{
		bool	selected = true;

		for (int i = this->m_FileParam->m_TopValueFieldIndex;
			 i < this->m_FileParam->m_FieldNum;
			 i++)
		{
			if (OpenParam_ != 0)
			{
				selected = File::isSelected(OpenParam_, i);
			}

			if (*(this->m_FileParam->m_FieldOutsideArray + i) && selected)
			{
				char*	objectIDReadPos =
					static_cast<char*>(
						this->getFieldPointer(DirectObjectPage_,
											  DirectObjectAreaID_,
											  i));

				if (objectIDReadPos == 0)
				{
					continue;
				}

				ModUInt64	objectID;
				File::readObjectID(objectIDReadPos, objectID);

				if (*(this->m_FileParam->m_IsArrayFieldArray + i))
				{
					// 配列フィールド…

					bool	elementIsFixed =
						*(this->m_FileParam->m_IsFixedElementArray + i);

					this->freeArrayFieldObjectArea(
						objectID,
						DirectObjectPage_,
						elementIsFixed,
						CatchMemoryExhaust_,
						AttachValuePages_,
						FreeValuePageIDs_);
				}
				else
				{
					// 外置き可変長フィールド…

					File::freeVariableFieldObjectArea(
						this->m_Transaction,
						objectID,
						DirectObjectPage_,
						this->m_FixMode,
						CatchMemoryExhaust_,
						AttachValuePages_,
						FreeValuePageIDs_);
				}
			}
		}
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::freeArrayFieldObjectArea --
//		配列バリューフィールドオブジェクトが記録されている
//		物理エリアを解放する
//
//	NOTES
//	配列バリューフィールドオブジェクトが記録されている
//	物理エリアを解放する。
//
//	ARGUMENTS
//	const ModUInt64			ArrayFieldObjectID_
//		配列バリューフィールドオブジェクトが記録されている物理エリアの
//		識別子
//	PhysicalFile::Page*		DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const bool				ElementIsFixed_
//		要素が固定長かどうか
//			true  : 要素が固定長
//			false : 要素が可変長
//	const bool				CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&		AttachValuePages_
//		バリューページ記述子ベクターへの参照
//		（アタッチしたバリューページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	FreeValuePageIDs_
//		バリューページ識別子ベクターへの参照
//		（フリーしたバリューページの物理ページ識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::freeArrayFieldObjectArea(
	const ModUInt64		ArrayFieldObjectID_,
	PhysicalFile::Page*	DirectObjectPage_,
	const bool			ElementIsFixed_,
	const bool			CatchMemoryExhaust_,
	PageVector&			AttachValuePages_,
	PageIDVector&		FreeValuePageIDs_) const
{
	//
	// 配列バリューフィールドオブジェクトが記録されている
	// 物理ページをアタッチする
	//

	PhysicalFile::Page*		arrayFieldObjectPage = 0;

	bool	attached = File::attachObjectPage(this->m_Transaction,
											  ArrayFieldObjectID_,
											  DirectObjectPage_,
											  arrayFieldObjectPage,
											  this->m_FixMode,
											  CatchMemoryExhaust_,
											  AttachValuePages_);

	// 配列バリューフィールドオブジェクトが記録されている
	// 物理エリアの識別子を設定する
	const PhysicalFile::AreaID arrayFieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(ArrayFieldObjectID_);

	// 配列バリューフィールドオブジェクトが記録されている
	// 物理エリア先頭へのポインタを取得する
	const void*	arrayFieldObjectAreaTop =
		File::getConstAreaTop(arrayFieldObjectPage,
							  arrayFieldObjectAreaID);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(arrayFieldObjectPage,
								  arrayFieldObjectAreaID));

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	const int*	elementNumReadPos = 0;

	// check object type
	if (*objectTypeReadPos != File::ArrayObjectType)
	{
		// 配列バリューフィールドオブジェクトが
		// リンクオブジェクト（のはず）…

		; _SYDNEY_ASSERT(*objectTypeReadPos == File::DivideArrayObjectType);

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);

		elementNumReadPos =
			syd_reinterpret_cast<const int*>(
				nextLinkObjectIDReadPos + File::ObjectIDArchiveSize);
	}
	else
	{
		elementNumReadPos =
			syd_reinterpret_cast<const int*>(objectTypeReadPos + 1);
	}

	; _SYDNEY_ASSERT(elementNumReadPos != 0);

	if (ElementIsFixed_ == false)
	{
		// 要素が可変長…

		// ヌルビットマップ先頭へのポインタを設定する
		const NullBitmap::Value*	nullBitmapTop =
			syd_reinterpret_cast<const NullBitmap::Value*>(
				elementNumReadPos + 1);

		NullBitmap	nullBitmap(nullBitmapTop,
							   *elementNumReadPos,
							   NullBitmap::Access::ReadOnly);

		bool	existNull = nullBitmap.existNull();

		// 先頭の可変長要素オブジェクトのオブジェクトIDへの
		// ポインタを取得する
		const char*	objectIDReadPos =
			static_cast<const char*>(nullBitmap.getConstTail());

		//
		// 可変長要素オブジェクトが記録されている
		// 物理エリアを解放する
		//

		for (int i = 0; i < *elementNumReadPos; i++)
		{
			ModUInt64	objectID = FileCommon::ObjectID::Undefined;

			// 可変長要素オブジェクトのオブジェクトIDを読み込む
			objectIDReadPos = File::readObjectID(objectIDReadPos,
												 objectID);

			if (existNull == false || nullBitmap.isNull(i) == false)
			{
				// 可変長要素がヌル値ではない…

				; _SYDNEY_ASSERT(
					objectID != FileCommon::ObjectID::Undefined);

				// 可変長要素オブジェクトが記録されている
				// 物理エリアを解放する
				File::freeVariableFieldObjectArea(
					this->m_Transaction,
					objectID,
					arrayFieldObjectPage,
					this->m_FixMode,
					CatchMemoryExhaust_,
					AttachValuePages_,
					FreeValuePageIDs_);
			}
		}
	}

	// 配列バリューフィールドオブジェクトが記録されていた
	// 物理エリアを解放する
	arrayFieldObjectPage->freeArea(*this->m_Transaction,
								   arrayFieldObjectAreaID);

	if (attached)
	{
		// 配列バリューフィールドオブジェクトが記録されている
		// 物理ページをアタッチした…

		//
		// ということは、配列バリューフィールドオブジェクトが
		// 記録されていた物理ページは、
		// 代表オブジェクトが記録されている物理ページとは異なる。
		//

		//
		// この場合、配列バリューフィールドオブジェクトが
		// 記録されていた物理ページ内の物理エリアの再配置を行う。
		//

		PhysicalFile::PageID	arrayFieldObjectPageID =
			arrayFieldObjectPage->getID();

		if (arrayFieldObjectPageID != 0 &&
			arrayFieldObjectPage->getTopAreaID(*this->m_Transaction) ==
			PhysicalFile::ConstValue::UndefinedAreaID)
		{
			File::detachPage(this->m_PhysicalFile,
							 AttachValuePages_,
							 arrayFieldObjectPage,
							 PhysicalFile::Page::UnfixMode::Dirty,
							 false);

			attached = false;

			FreeValuePageIDs_.pushBack(arrayFieldObjectPageID);

			this->m_PhysicalFile->freePage(*this->m_Transaction,
										   arrayFieldObjectPageID);

			arrayFieldObjectPage = 0;
		}
		else 
		{
			arrayFieldObjectPage->compaction(*this->m_Transaction);
		}
	}

	if (CatchMemoryExhaust_ && attached)
	{
		this->m_PhysicalFile->detachPage(
			arrayFieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチしてしまう
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		this->freeArrayFieldObjectArea(nextLinkObjectID,
									   DirectObjectPage_,
									   ElementIsFixed_,
									   CatchMemoryExhaust_,
									   AttachValuePages_,
									   FreeValuePageIDs_);
	}
}

//
//	FUNCTION private
//	Btree::ValueFile::getFieldPointer --
//		フィールドの値が記録されている領域へのポインタを返す
//
//	NOTES
//	フィールドの値が記録されている領域へのポインタを返す。
//
//	ARGUMENTS
//	PhysicalFile::Page*			ObjectPage_
//		オブジェクトが記録されている物理ページの記述子
//	const PhysicalFile::AreaID	ObjectAreaID_
//		オブジェクトが記録されている物理エリアの識別子
//	const int					FieldIndex_
//		バリューフィールドインデックス
//
//	RETURN
//	void*
//		フィールドの値が記録されている領域へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
void*
ValueFile::getFieldPointer(PhysicalFile::Page*			ObjectPage_,
						   const PhysicalFile::AreaID	ObjectAreaID_,
						   const int					FieldIndex_) const
{
	; _SYDNEY_ASSERT(ObjectPage_ != 0);

	// オブジェクトが記録されている物理エリア先頭への
	// ポインタを取得する
	const char*	objectAreaTop =
		static_cast<const char*>(File::getConstAreaTop(ObjectPage_,
													   ObjectAreaID_));

	//
	// オブジェクトタイプのチェック
	//

	const File::ObjectType*	objectType =
		syd_reinterpret_cast<const File::ObjectType*>(objectAreaTop);

	; _SYDNEY_ASSERT((*objectType & File::NormalObjectType) != 0);

	// ヌルビットマップ先頭へのポインタを設定する
	const NullBitmap::Value*	nullBitmapTop =
		syd_reinterpret_cast<const NullBitmap::Value*>(
			objectAreaTop +
			File::ObjectTypeArchiveSize +
			File::PageIDArchiveSize +
			File::ModUInt32ArchiveSize);

	//
	// フィールド値がヌル値かどうかをチェックする
	//

	int	fieldNum = this->m_FileParam->m_ValueNum;
	int	bitIndex = FieldIndex_ - this->m_FileParam->m_TopValueFieldIndex;

	if (NullBitmap::isNull(nullBitmapTop, fieldNum, bitIndex))
	{
		// フィールド値がヌル値…

		// この場合にはヌルポインタを返す
		return 0;
	}

	// 先頭フィールドの値へのポインタを取得する
	const char*	field =
		static_cast<const char*>(NullBitmap::getConstTail(nullBitmapTop,
														  fieldNum));

	//
	// 指定されたフィールドの値が記録されている位置まで
	// ポインタを移動し、返す。
	//

	for (int i = this->m_FileParam->m_TopValueFieldIndex;
		 i < FieldIndex_;
		 i++)
	{
		field += this->m_FileParam->getFieldArchiveSize(i);
	}

	return const_cast<char*>(field);
}

//
//	FUNCTION private
//	Btree::ValueFile::setArrayFieldUseInfo --
//		配列フィールドオブジェクトのために使用している
//		物理ページと物理エリアを登録する
//
//	NOTES
//	配列フィールドオブジェクトのために使用している
//	物理ページと物理エリアを登録する。
//
//	ARGUMENTS
//	PhysicalFile::Page*				DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const ModUInt64					FieldObjectID_
//		配列バリューフィールドオブジェクトのオブジェクトID
//	const bool						ElementIsFixed_
//		要素が固定長かどうか
//			true  : 要素が固定長
//			false : 要素が可変長
//	Btree::UseInfo&					UseInfo_
//		登録情報への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
ValueFile::setArrayFieldUseInfo(
	PhysicalFile::Page*				DirectObjectPage_,
	const ModUInt64					FieldObjectID_,
	const bool						ElementIsFixed_,
	UseInfo&						UseInfo_,
	Admin::Verification::Progress&	Progress_) const
{
	//
	// 配列バリューフィールドオブジェクトのオブジェクトIDから、
	// 配列バリューフィールドオブジェクトが記録されている
	//   ① 物理ページの識別子
	//   ② 物理エリアの識別子
	// を設定する。
	//

	const PhysicalFile::PageID fieldObjectPageID =
		Common::ObjectIDData::getFormerValue(FieldObjectID_);
	const PhysicalFile::AreaID fieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	// 登録情報へ登録する
	UseInfo_.append(fieldObjectPageID, fieldObjectAreaID);

	bool	attached = false;

	PhysicalFile::Page*	fieldObjectPage = 0;

	if (fieldObjectPageID != DirectObjectPage_->getID())
	{
		// 代表オブジェクトが記録されている物理ページとは
		// 異なる物理ページに記録されている…

		attached = true;

		// 配列バリューフィールドが記録されている物理ページをアタッチする
		fieldObjectPage =
			this->m_PhysicalFile->verifyPage(
				*this->m_Transaction,
				fieldObjectPageID,
				Buffer::Page::FixMode::ReadOnly,
				Progress_);

		if (Progress_.isGood() == false)
		{
			return;
		}
	}
	else
	{
		// 代表オブジェクトと同じ物理ページ内に記録されている…

		fieldObjectPage = DirectObjectPage_;
	}

	; _SYDNEY_ASSERT(fieldObjectPage != 0);

	// 配列バリューフィールドが記録されている物理エリア先頭への
	// ポインタを取得する
	const void*	fieldObjectAreaTop =
		File::getConstAreaTop(fieldObjectPage,
							  fieldObjectAreaID);

 	//
	// オブジェクトタイプのチェック
	//

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(fieldObjectAreaTop);

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	const int*	elementNumReadPos = 0;

	if (*objectTypeReadPos != File::ArrayObjectType)
	{
		// リンクオブジェクト…

		; _SYDNEY_ASSERT(*objectTypeReadPos == File::DivideArrayObjectType);

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);

		elementNumReadPos =
			syd_reinterpret_cast<const int*>(
				nextLinkObjectIDReadPos + File::ObjectIDArchiveSize);
	}
	else
	{
		elementNumReadPos =
			syd_reinterpret_cast<const int*>(objectTypeReadPos + 1);
	}

	; _SYDNEY_ASSERT(elementNumReadPos != 0);

	if (ElementIsFixed_ == false)
	{
		// 要素が可変長…

		// ヌルビットマップ先頭へのポインタを設定する
		const NullBitmap::Value*	nullBitmapTop =
			syd_reinterpret_cast<const NullBitmap::Value*>(
				elementNumReadPos + 1);

		NullBitmap	nullBitmap(nullBitmapTop,
							   *elementNumReadPos,
							   NullBitmap::Access::ReadOnly);

		bool	existNull = nullBitmap.existNull();

		// 先頭の可変長要素オブジェクトのオブジェクトIDへの
		// ポインタを取得する
		const char*	elementObjectIDReadPos =
			static_cast<const char*>(nullBitmap.getConstTail());

		//
		// 可変長要素オブジェクトが記録されている
		// 物理ページと物理エリアを登録する
		//

		for (int i = 0; i < *elementNumReadPos; i++)
		{
			// 可変長要素オブジェクトのオブジェクトIDを読み込む

			ModUInt64	elementObjectID;

			elementObjectIDReadPos =
				File::readObjectID(elementObjectIDReadPos,
								   elementObjectID);

			try
			{
				if (existNull == false || nullBitmap.isNull(i) == false)
				{
					// 要素値がヌル値ではない…

					// 可変長要素オブジェクトが記録されている
					// 物理ページと物理エリアを登録する
					File::setOutsideVariableFieldUseInfo(
						*this->m_Transaction,
						this->m_PhysicalFile,
						fieldObjectPage,
						elementObjectID,
						UseInfo_,
						Progress_);
				}

				if (Progress_.isGood() == false)
				{
					if (attached)
					{
						this->m_PhysicalFile->detachPage(
							fieldObjectPage,
							PhysicalFile::Page::UnfixMode::NotDirty);
					}

					return;
				}
			}
			catch (...)
			{
				if (attached)
				{
					this->m_PhysicalFile->detachPage(
						fieldObjectPage,
						PhysicalFile::Page::UnfixMode::NotDirty);
				}

				_SYDNEY_RETHROW;
			}
		}
	}

	if (attached)
	{
		this->m_PhysicalFile->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty);
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// 再帰呼び出し
		this->setArrayFieldUseInfo(DirectObjectPage_,
								   nextLinkObjectID,
								   ElementIsFixed_,
								   UseInfo_,
								   Progress_);
	}
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
