// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File_Variable.cpp -- Ｂ＋木ファイルクラスの実現ファイル(可変長データ)
// 
// Copyright (c) 2000, 2001, 2004, 2006, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Btree/File.h"

#include "Exception/NotSupported.h"

#include "Common/Assert.h"
#include "Common/CompressedData.h"

#include "FileCommon/DataManager.h"

#include "Btree/NodePageHeader.h"
#include "Btree/ValueFile.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

// Common::Data::copyの仕様が変わった都合で引数を修正

//
//	FUNCTION private
//	Btree::File::readVariableField --
//		ファイルに記録されている可変長フィールドの値を読み込む
//
//	NOTES
//	ファイルに記録されている可変長フィールドの値を読み込む。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	const Common::DataType::Type		FieldType_
//		フィールドデータ型
//	const Os::Memory::Size				FieldMaxLen_
//		フィールド最大長
//	const bool							IsOutside_
//		外置き可変長フィールドかどうか
//			true  : 外置き可変長フィールド
//			false : 外置きではない可変長フィールド
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されているノードページ／バリューページの
//		物理ページ記述子
//	const char*							FieldTop_
//		フィールド値が記録されている領域へのポインタ
//		（ページ内を指している）
//	Common::Data::Pointer&				Field_
//		フィールドデータへの参照
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	const char*
//		次のフィールド値が記録されている領域へポインタ
//
//	EXCEPTIONS
//	[YET!]
//
// static
const char*
File::readVariableField(
	const Trans::Transaction*			Transaction_,
	const Common::DataType::Type		FieldType_,
	const Os::Memory::Size				FieldMaxLen_,
	const bool							IsOutside_,
	PhysicalFile::Page*					DirectObjectPage_,
	const char*							FieldTop_,
	Common::Data::Pointer&				Field_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_)
{
	; _SYDNEY_ASSERT(DirectObjectPage_ != 0);
	; _SYDNEY_ASSERT(FieldTop_ != 0);

	const char*	nextFieldTop = 0;

	if (IsOutside_)
	{
		// 外置き可変長フィールド…

		//
		// 代表オブジェクトには、
		// 外置き可変長フィールドオブジェクトの
		// オブジェクトIDのみが記録されている。
		//

		ModUInt64	objectID = FileCommon::ObjectID::Undefined;

		nextFieldTop = File::readObjectID(FieldTop_, objectID);

		; _SYDNEY_ASSERT(objectID != FileCommon::ObjectID::Undefined);

		// 外置き可変長フィールドオブジェクトから
		// 可変長フィールド値を読み込む
		File::readOutsideVariableField(Transaction_,
									   DirectObjectPage_,
									   objectID,
									   FieldType_,
									   Field_,
									   FixMode_,
									   CatchMemoryExhaust_,
									   AttachPages_);
	}
	else
	{
		// 外置きではない可変長フィールド…

		//
		// 代表オブジェクトに、
		// フィールド長と可変長フィールド値が記録されている。
		//

		const File::InsideVarFieldLen*	fieldLenReadPos =
			syd_reinterpret_cast<const File::InsideVarFieldLen*>(FieldTop_);

		// 代表オブジェクトから可変長フィールド値を読み込む
		File::readInsideVariableField(fieldLenReadPos,
									  FieldType_,
									  Field_);

		nextFieldTop = syd_reinterpret_cast<const char*>(fieldLenReadPos + 1);

		nextFieldTop += FieldMaxLen_;
	}

	; _SYDNEY_ASSERT(nextFieldTop != 0);

	return nextFieldTop;
}

//
//	FUNCTION private
//	Btree::File::readInsideVariableField --
//		ファイルに記録されている外置きではない可変長フィールドの値を読み込む
//
//	NOTES
//	ファイルに記録されている外置きではない可変長フィールドの値を読み込む。
//
//	ARGUMENTS
//	const Btree::File::InsideVarFieldLen*	FieldLenReadPos_
//		可変長フィールド長へのポインタ
//	const Common::DataType::Type			FieldType_
//		フィールドデータ型
//	Common::Data::Pointer&					Field_
//		フィールドデータへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::readInsideVariableField(
	const File::InsideVarFieldLen*	FieldLenReadPos_,
	const Common::DataType::Type	FieldType_,
	Common::Data::Pointer&			Field_)
{

	; _SYDNEY_ASSERT(FieldLenReadPos_ != 0);

	// Field_はこの中でコンストラクトする！
	; _SYDNEY_ASSERT(Field_.get() == 0);

/*
	[YET!]
	なぜだか、少し前のFileCommon::DataManager::createCommonData()は、
	とても遅かったのでコメントアウトしてある。
	近い将来、Quantifyで計測してみて、問題ないようならば
	元に戻す。

	const void*	fieldTop = FieldLenReadPos_ + 1;

	Field_ = FileCommon::DataManager::createCommonData(FieldType_,
													   fieldTop,
													   *FieldLenReadPos_);
*/
	const void*	fieldTop = FieldLenReadPos_ + 1;

	if (FieldType_ == Common::DataType::String)
	{
		ModSize	numChar = *FieldLenReadPos_ / sizeof(ModUnicodeChar);

		ModUnicodeChar	forZeroByte = 0;

		if (numChar == 0)
		{
			fieldTop = &forZeroByte;
		}

		ModUnicodeString
			stringValue(static_cast<const ModUnicodeChar*>(fieldTop),
						numChar);

		Field_ = new Common::StringData(stringValue);
	}
	else if (FieldType_ == Common::DataType::Binary)
	{
		Field_ = new Common::BinaryData(fieldTop, *FieldLenReadPos_);
	}
	else
	{
		throw Exception::NotSupported(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION private
//	Btree::File::readOutsideVariableField --
//		ファイルに記録されている外置き可変長フィールドの値を読み込む
//
//	NOTES
//	ファイルに記録されている外置き可変長フィールドの値を読み込む。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されている
//		ノードページ／バリューページの物理ページ記述子
//	const ModUInt64						FieldObjectID_
//		外置き可変長フィールドオブジェクトのオブジェクトID
//	const Common::DataType::Type		FieldType_
//		フィールドデータ型
//	Common::Data::Pointer&				Field_
//		フィールドデータへの参照
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::readOutsideVariableField(
	const Trans::Transaction*			Transaction_,
	PhysicalFile::Page*					DirectObjectPage_,
	const ModUInt64						FieldObjectID_,
	const Common::DataType::Type		FieldType_,
	Common::Data::Pointer&				Field_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_)
{
	if (File::isCompressedFieldObject(Transaction_,
									  DirectObjectPage_,
									  FieldObjectID_,
									  FixMode_,
									  CatchMemoryExhaust_,
									  AttachPages_))
	{
		char*	fieldBuffer = 0;

		// 圧縮されている…
		Os::Memory::Size	uncompressedSize = 0;
		Os::Memory::Size	compressedSize = 0;

		try {
			File::readCompressedOutsideVariableField(Transaction_,
													 DirectObjectPage_,
													 FieldObjectID_,
													 fieldBuffer,
													 uncompressedSize,
													 compressedSize,
													 0, // バッファインデックス
													 FixMode_,
													 CatchMemoryExhaust_,
													 AttachPages_);

			; _SYDNEY_ASSERT(fieldBuffer != 0);
	 		; _SYDNEY_ASSERT(uncompressedSize > 0 && compressedSize > 0);

			FileCommon::DataManager::DataType type;
			type._name = FieldType_;
			Field_ =
				FileCommon::DataManager::createCommonData(
					type,
					Common::Data::Function::Compressed,
					&uncompressedSize);

			FileCommon::DataManager::readCommonData(*Field_,
													fieldBuffer,
													compressedSize);

		} catch (...) {
			if (fieldBuffer) ModDefaultManager::free(fieldBuffer, compressedSize);
			_SYDNEY_RETHROW;
		}
		ModDefaultManager::free(fieldBuffer, compressedSize);
	}
	else
	{
		// 圧縮されていない…

		if (FieldType_ == Common::DataType::String)
		{
			// String型のフィールド…

			ModUnicodeString	stringField;

			File::readOutsideStringField(Transaction_,
										 DirectObjectPage_,
										 FieldObjectID_,
										 stringField,
										 FixMode_,
										 CatchMemoryExhaust_,
										 AttachPages_);

			Field_ = new Common::StringData(stringField);
		}
		else if (FieldType_ == Common::DataType::Binary)
		{
			// Binary型のフィールド…

			//
			// （ということは、バリューフィールド）
			//

			Os::Memory::Size	fieldLen =
				File::getOutsideBinaryFieldLength(Transaction_,
												  DirectObjectPage_,
												  FieldObjectID_,
												  FixMode_,
												  CatchMemoryExhaust_,
												  AttachPages_);

			void*	binaryBuffer = 0;
			try {
				binaryBuffer = ModDefaultManager::allocate(fieldLen);

				File::readOutsideBinaryField(Transaction_,
											 DirectObjectPage_,
											 FieldObjectID_,
											 static_cast<char*>(binaryBuffer),
											 0, // バッファのインデックス
											 FixMode_,
											 CatchMemoryExhaust_,
											 AttachPages_);

				Field_ = new Common::BinaryData(binaryBuffer, fieldLen);

			} catch (...) {
				if (binaryBuffer) ModDefaultManager::free(binaryBuffer, fieldLen);
				_SYDNEY_RETHROW;
			}
			ModDefaultManager::free(binaryBuffer, fieldLen);
		}
		else
		{
			throw Exception::NotSupported(moduleName, srcFile, __LINE__);
		}
	}
}

//
//	FUNCTION private
//	Btree::File::readCompressedOutsideVariableField --
//		ファイルに記録されている
//		圧縮された外置き可変長フィールドの値を読み込む
//
//	NOTES
//	ファイルに記録されている圧縮された外置き可変長フィールドの値を読み込む。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されている
//		ノードページ／バリューページの物理ページ記述子
//	const ModUInt64						FieldObjectID_
//		外置き可変長フィールドオブジェクトのオブジェクトID
//	char*&								FieldBuffer_
//		フィールド値を読み込むためのバッファへのポインタ
//	Os::Memory::Size&					UncompressedSize_
//		圧縮前のフィールド長 [byte]
//	Os::Memory::Size&					CompressedSize_
//		圧縮後のフィールド長 [byte]
//	const int							BufferIndex_
//		バッファ内のインデックス（何バイト目以降に読み込むか）
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	PageVector&							AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//	const bool							IsRecur_ = false
//		再帰呼び出しかどうか
//			true  : 再帰呼び出し
//			false : 再帰呼び出しではない
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::readCompressedOutsideVariableField(
	const Trans::Transaction*			Transaction_,
	PhysicalFile::Page*					DirectObjectPage_,
	const ModUInt64						FieldObjectID_,
	char*&								FieldBuffer_,
	Os::Memory::Size&					UncompressedSize_,
	Os::Memory::Size&					CompressedSize_,
	const int							BufferIndex_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_,
	const bool							IsRecur_ // = false
	)
{
	PhysicalFile::Page*	fieldObjectPage = 0;
	bool	attached = File::attachObjectPage(Transaction_,
											  FieldObjectID_,
											  DirectObjectPage_,
											  fieldObjectPage,
											  FixMode_,
											  CatchMemoryExhaust_,
											  AttachPages_);

	const PhysicalFile::AreaID fieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(fieldObjectPage, fieldObjectAreaID));

	const char*	fieldTop = 0;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (*objectTypeReadPos == File::CompressedObjectType)
	{
		// リンクオブジェクトではない…

		fieldTop = syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);
	}
	else
	{
		// リンクオブジェクト（のはず）…

		; _SYDNEY_ASSERT(
			*objectTypeReadPos == File::DivideCompressedObjectType);

		//
		// 次のオブジェクトのIDを読み込む。
		//

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);

		fieldTop =
			syd_reinterpret_cast<const char*>(
				nextLinkObjectIDReadPos + File::ObjectIDArchiveSize);
	}

	if (IsRecur_ == false)
	{
		// 再帰呼び出しではない…

		//
		// ということは、圧縮前後のフィールド長が記録されている。
		//

		const Os::Memory::Size*	fieldLenReadPos =
			syd_reinterpret_cast<const Os::Memory::Size*>(fieldTop);

		UncompressedSize_ = *fieldLenReadPos++;
		CompressedSize_ = *fieldLenReadPos;

		// フィールド値を読み込む先のバッファを確保する。
		FieldBuffer_ =
			static_cast<char*>(ModDefaultManager::allocate(CompressedSize_));

		fieldTop =
			syd_reinterpret_cast<const char*>(fieldLenReadPos + 1);
	}

	; _SYDNEY_ASSERT(FieldBuffer_ != 0);

	Os::Memory::Size	currentReadSize = 0;

	if (*objectTypeReadPos == File::CompressedObjectType)
	{
		// リンクオブジェクトではない…

		if (IsRecur_)
		{
			// 再帰呼び出し…

			currentReadSize =
				fieldObjectPage->getAreaSize(fieldObjectAreaID) -
				File::ObjectTypeArchiveSize;
		}
		else
		{
			// 再帰呼び出しではない…

			currentReadSize = CompressedSize_;
		}
	}
	else
	{
		// リンクオブジェクト…

		currentReadSize =
			fieldObjectPage->getAreaSize(fieldObjectAreaID) -
			File::ObjectTypeArchiveSize -
			File::ObjectIDArchiveSize;

		if (IsRecur_ == false)
		{
			currentReadSize -= (sizeof(Os::Memory::Size) << 1);
		}
	}

	ModOsDriver::Memory::copy(FieldBuffer_ + BufferIndex_,
							  fieldTop,
							  currentReadSize);

	if (CatchMemoryExhaust_ && attached)
	{
		fieldObjectPage->getFile()->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し

		File::readCompressedOutsideVariableField(
			Transaction_,
			DirectObjectPage_,
			nextLinkObjectID,
			FieldBuffer_,
			UncompressedSize_,
			CompressedSize_,
			BufferIndex_ + currentReadSize,
			FixMode_,
			CatchMemoryExhaust_,
			AttachPages_,
			true); // 再帰呼び出し
	}
}

//
//	FUNCTION private
//	Btree::File::isCompressedFieldObject --
//		圧縮されたフィールド値が記録されている
//		外置き可変長フィールドオブジェクトかどうかを知らせる
//
//	NOTES
//	引数FieldObjectID_で示される外置き可変長フィールドオブジェクトに、
//	圧縮されたフィールド値が記録されているかどうかを知らせる。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されている
//		ノードページ／バリューページの物理ページ記述子
//	const ModUInt64						FieldObjectID_
//		外置きString型フィールドオブジェクトのオブジェクトID
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	bool
//		引数FieldObjectID_で示される外置き可変長フィールドオブジェクトに
//		圧縮されたフィールド値が記録されているかどうか
//			true  : 圧縮されたフィールド値が記録されている
//			false : 圧縮されていないフィールド値が記録されている
//
//	EXCEPTIONS
//	[YET!]
//
// static
bool
File::isCompressedFieldObject(
	const Trans::Transaction*			Transaction_,
	PhysicalFile::Page*					DirectObjectPage_,
	const ModUInt64						FieldObjectID_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_)
{
	PhysicalFile::Page*	fieldObjectPage = 0;
	bool	attached = File::attachObjectPage(Transaction_,
											  FieldObjectID_,
											  DirectObjectPage_,
											  fieldObjectPage,
											  FixMode_,
											  CatchMemoryExhaust_,
											  AttachPages_);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(
				fieldObjectPage,
				Common::ObjectIDData::getLatterValue(FieldObjectID_)));

	bool	isCompressed =
		(*objectTypeReadPos == File::CompressedObjectType) ||
		(*objectTypeReadPos == File::DivideCompressedObjectType);

	if (CatchMemoryExhaust_ && attached)
	{
		fieldObjectPage->getFile()->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return isCompressed;
}

//
//	FUNCTION private
//	Btree::File::readOutsideStringField --
//		ファイルに記録されている外置きString型フィールドの値を読み込む
//
//	NOTES
//	ファイルに記録されている外置きString型フィールドの値を読み込む。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されている
//		ノードページ／バリューページの物理ページ記述子
//	const ModUInt64						FieldObjectID_
//		外置きString型フィールドオブジェクトのオブジェクトID
//	ModUnicodeString&					StringField_
//		フィールド値読み込み先
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::readOutsideStringField(
	const Trans::Transaction*			Transaction_,
	PhysicalFile::Page*					DirectObjectPage_,
	const ModUInt64						FieldObjectID_,
	ModUnicodeString&					StringField_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_)
{
	PhysicalFile::Page*	fieldObjectPage = 0;
	bool	attached = File::attachObjectPage(Transaction_,
											  FieldObjectID_,
											  DirectObjectPage_,
											  fieldObjectPage,
											  FixMode_,
											  CatchMemoryExhaust_,
											  AttachPages_);

	const PhysicalFile::AreaID fieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(fieldObjectPage, fieldObjectAreaID));

	const ModUnicodeChar*	fieldTop = 0;
	Os::Memory::Size		fieldLen = 0;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (*objectTypeReadPos == File::NormalObjectType)
	{
		// リンクオブジェクトではない…

		fieldTop =
			syd_reinterpret_cast<const ModUnicodeChar*>(objectTypeReadPos + 1);

		fieldLen =
			fieldObjectPage->getAreaSize(fieldObjectAreaID) -
			File::ObjectTypeArchiveSize;
	}
	else
	{
		// リンクオブジェクト（のはず）…

		; _SYDNEY_ASSERT(*objectTypeReadPos == File::DivideObjectType);

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);

		fieldTop =
			syd_reinterpret_cast<const ModUnicodeChar*>(
				nextLinkObjectIDReadPos + File::ObjectIDArchiveSize);

		fieldLen =
			fieldObjectPage->getAreaSize(fieldObjectAreaID) -
			File::ObjectTypeArchiveSize -
			File::ObjectIDArchiveSize;
	}

	; _SYDNEY_ASSERT(fieldTop != 0);

	//
	// フィールド値の長さは、ModUnicodeCharのサイズの倍数のはず。
	//

	; _SYDNEY_ASSERT(fieldLen % sizeof(ModUnicodeChar) == 0);

	ModSize	numChar = fieldLen / sizeof(ModUnicodeChar);

	ModUnicodeChar	forZeroByte = 0;

	if (numChar == 0)
	{
		fieldTop = &forZeroByte;
	}

	StringField_.append(fieldTop, numChar);

	if (CatchMemoryExhaust_ && attached)
	{
		fieldObjectPage->getFile()->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		File::readOutsideStringField(Transaction_,
									 DirectObjectPage_,
									 nextLinkObjectID,
									 StringField_,
									 FixMode_,
									 CatchMemoryExhaust_,
									 AttachPages_);
	}
}

//
//	FUNCTION private
//	Btree::File::getOutsideBinaryFieldLength --
//		ファイルに記録されている外置きBinary型フィールドの
//		フィールド長を返す
//
//	NOTES
//	ファイルに記録されている外置きBinary型フィールドの
//	フィールド長を返す。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されている
//		ノードページ／バリューページの物理ページ記述子
//	const ModUInt64						FieldObjectID_
//		外置きBinary型フィールドオブジェクトのオブジェクトID
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	Os::Memory::Size
//		外置きBinary型フィールドのフィールド長
//
//	EXCEPTIONS
//	[YET!]
//
// static
Os::Memory::Size
File::getOutsideBinaryFieldLength(
	const Trans::Transaction*			Transaction_,
	PhysicalFile::Page*					DirectObjectPage_,
	const ModUInt64						FieldObjectID_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_)
{
	PhysicalFile::Page*	fieldObjectPage = 0;
	bool	attached = File::attachObjectPage(Transaction_,
											  FieldObjectID_,
											  DirectObjectPage_,
											  fieldObjectPage,
											  FixMode_,
											  CatchMemoryExhaust_,
											  AttachPages_);

	const PhysicalFile::AreaID fieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(fieldObjectPage, fieldObjectAreaID));

	Os::Memory::Size	fieldLen = 0;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (*objectTypeReadPos == File::NormalObjectType)
	{
		fieldLen =
			fieldObjectPage->getAreaSize(fieldObjectAreaID) -
			File::ObjectTypeArchiveSize;
	}
	else
	{
		// リンクオブジェクト（のはず）…

		; _SYDNEY_ASSERT(*objectTypeReadPos == File::DivideObjectType);

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);

		fieldLen =
			fieldObjectPage->getAreaSize(fieldObjectAreaID) -
			File::ObjectTypeArchiveSize -
			File::ObjectIDArchiveSize;
	}

	if (CatchMemoryExhaust_ && attached)
	{
		fieldObjectPage->getFile()->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// 再帰呼び出し
		fieldLen += File::getOutsideBinaryFieldLength(Transaction_,
													  DirectObjectPage_,
													  nextLinkObjectID,
													  FixMode_,
													  CatchMemoryExhaust_,
													  AttachPages_);
	}

	return fieldLen;
}

//
//	FUNCTION private
//	Btree::File::readOutsideBinaryField --
//		ファイルに記録されている外置きBinary型フィールドの値を読み込む
//
//	NOTES
//	ファイルに記録されている外置きBinary型フィールドの値を読み込む。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されている
//		ノードページ／バリューページの物理ページ記述子
//	const ModUInt64						FieldObjectID_
//		外置きBinary型フィールドオブジェクトのオブジェクトID
//	char*								BinaryBuffer_
//		フィールド値読み込み先バッファへのポインタ
//	const int							BufferIndex_
//		バッファ内のインデックス（何バイト目以降にコピーするか）
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	Btree::PageVector&					AttachPages_
//		ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::readOutsideBinaryField(
	const Trans::Transaction*			Transaction_,
	PhysicalFile::Page*					DirectObjectPage_,
	const ModUInt64						FieldObjectID_,
	char*								BinaryBuffer_,
	const int							BufferIndex_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_)
{
	PhysicalFile::Page*	fieldObjectPage = 0;
	bool	attached = File::attachObjectPage(Transaction_,
											  FieldObjectID_,
											  DirectObjectPage_,
											  fieldObjectPage,
											  FixMode_,
											  CatchMemoryExhaust_,
											  AttachPages_);

	const PhysicalFile::AreaID fieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(fieldObjectPage, fieldObjectAreaID));

	const void*			fieldTop = 0;
	Os::Memory::Size	fieldLen = 0;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (*objectTypeReadPos == File::NormalObjectType)
	{
		fieldTop = objectTypeReadPos + 1;

		fieldLen =
			fieldObjectPage->getAreaSize(fieldObjectAreaID) -
			File::ObjectTypeArchiveSize;
	}
	else
	{
		// リンクオブジェクト（のはず）…

		; _SYDNEY_ASSERT(*objectTypeReadPos == File::DivideObjectType);

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);

		fieldTop = nextLinkObjectIDReadPos + File::ObjectIDArchiveSize;

		fieldLen =
			fieldObjectPage->getAreaSize(fieldObjectAreaID) -
			File::ObjectTypeArchiveSize -
			File::ObjectIDArchiveSize;
	}

	; _SYDNEY_ASSERT(fieldTop != 0);

	ModOsDriver::Memory::copy(BinaryBuffer_ + BufferIndex_,
							  fieldTop,
							  fieldLen);

	if (CatchMemoryExhaust_ && attached)
	{
		fieldObjectPage->getFile()->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::NotDirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		File::readOutsideBinaryField(Transaction_,
									 DirectObjectPage_,
									 nextLinkObjectID,
									 BinaryBuffer_,
									 BufferIndex_ + fieldLen,
									 FixMode_,
									 CatchMemoryExhaust_,
									 AttachPages_);
	}
}


//
//	FUNCTION private
//	Btree::File::copyVariableKeyField -- 
//		外置き可変長キーフィールドオブジェクトをコピーする
//
//	NOTES
//	既に記録されている外置き可変長キーフィールドオブジェクトを
//	異なる領域にコピーする。
//	※ バリューフィールドでは、このような操作はしない。
//
//	ARGUMENTS
//	PhysicalFile::Page*		DstTopNodePage_
//		コピー先ノードページの物理ページ記述子
//	const ModUInt64			SrcVariableFieldObjectID_
//		コピー元可変長キーフィールドオブジェクトのオブジェクトID
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		コピー先ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const bool				IsMove_
//		移動するかどうか
//			true  : 移動する
//			false : コピーする
//
//	RETURN
//	ModUInt64
//		コピー先可変長キーフィールドオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::copyVariableKeyField(PhysicalFile::Page*	DstTopNodePage_,
						   const ModUInt64		SrcVariableFieldObjectID_,
						   PageVector&			AttachNodePages_,
						   PageIDVector&		AllocateNodePageIDs_,
						   const bool			IsLeafPage_,
						   const bool			IsMove_) const
{
	PhysicalFile::Page* srcVariableFieldObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(SrcVariableFieldObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	const PhysicalFile::AreaID srcVariableFieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(SrcVariableFieldObjectID_);

	const char*	srcVariableFieldObjectAreaTop =
		static_cast<const char*>(
			File::getConstAreaTop(srcVariableFieldObjectPage,
								  srcVariableFieldObjectAreaID));

	Os::Memory::Size	areaSize =
		srcVariableFieldObjectPage->getAreaSize(
			srcVariableFieldObjectAreaID);

	PhysicalFile::Page*	dstVariableFieldObjectPage =
		this->searchInsertNodePage(DstTopNodePage_,
								   AttachNodePages_,
								   AllocateNodePageIDs_,
								   areaSize,
								   IsLeafPage_);

	; _SYDNEY_ASSERT(dstVariableFieldObjectPage != 0);

	PhysicalFile::PageID	dstVariableFieldObjectPageID =
		dstVariableFieldObjectPage->getID();

	PhysicalFile::AreaID	dstVariableFieldObjectAreaID =
		dstVariableFieldObjectPage->allocateArea(*this->m_pTransaction,
												 areaSize);

	char*	dstVariableFieldObjectAreaTop =
		static_cast<char*>(File::getAreaTop(dstVariableFieldObjectPage,
											dstVariableFieldObjectAreaID));

	const char*	srcObjectTypeReadPos = srcVariableFieldObjectAreaTop;

	if (*srcObjectTypeReadPos == File::NormalObjectType ||
		*srcObjectTypeReadPos == File::CompressedObjectType)
	{
		ModOsDriver::Memory::copy(dstVariableFieldObjectAreaTop,
								  srcVariableFieldObjectAreaTop,
								  areaSize);
	}
	else
	{
		// リンクオブジェクト（のはず）…

		; _SYDNEY_ASSERT(
			*srcObjectTypeReadPos == File::DivideObjectType ||
			*srcObjectTypeReadPos == File::DivideCompressedObjectType);

		char*	dstObjectTypeWritePos = dstVariableFieldObjectAreaTop;

		*dstObjectTypeWritePos = *srcObjectTypeReadPos;

		const char*	srcNextLinkObjectIDReadPos = srcObjectTypeReadPos + 1;

		ModUInt64	srcNextLinkObjectID;
		File::readObjectID(srcNextLinkObjectIDReadPos, srcNextLinkObjectID);

		// 再帰呼び出し
		ModUInt64	dstNextLinkObjectID =
			this->copyVariableKeyField(DstTopNodePage_,
									   srcNextLinkObjectID,
									   AttachNodePages_,
									   AllocateNodePageIDs_,
									   IsLeafPage_,
									   IsMove_);

		char*	dstNextLinkObjectIDWritePos = dstObjectTypeWritePos + 1;

		File::writeObjectID(dstNextLinkObjectIDWritePos,
							dstNextLinkObjectID);

		ModSize	divideFieldLen =
			areaSize -
			File::ObjectTypeArchiveSize -
			File::ObjectIDArchiveSize;

		const char*	srcFieldTop =
			srcNextLinkObjectIDReadPos + File::ObjectIDArchiveSize;

		char*	dstFieldTop =
			dstNextLinkObjectIDWritePos + File::ObjectIDArchiveSize;

		ModOsDriver::Memory::copy(dstFieldTop, srcFieldTop, divideFieldLen);
	}

	if (IsMove_)
	{
		srcVariableFieldObjectPage->freeArea(
			*this->m_pTransaction,
			srcVariableFieldObjectAreaID);
	}

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			srcVariableFieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう

		if (dstVariableFieldObjectPage != DstTopNodePage_)
		{
			this->m_pPhysicalFile->detachPage(
				dstVariableFieldObjectPage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false); // 本当にデタッチ（アンフィックス）してしまう
		}
	}

	return File::makeObjectID(dstVariableFieldObjectPageID,
							  dstVariableFieldObjectAreaID);
}

//
//	FUNCTION private
//	Btree::File::writeOutsideVariableKey --
//		外置き可変長キーフィールドの値を書き込む
//
//	NOTES
//	外置き可変長キーフィールドの値を書き込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*		TopNodePage_
//		書き込み先ノードページの物理ページ記述子
//	const Common::Data*		KeyField_
//		可変長キーフィールドデータへのポインタ
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		書き込み先ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	ModUInt64
//		外置き可変長キーフィールドオブジェクトのID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::writeOutsideVariableKey(PhysicalFile::Page*	TopNodePage_,
							  const Common::Data*	KeyField_,
							  PageVector&			AttachNodePages_,
							  PageIDVector&			AllocateNodePageIDs_,
							  const bool			IsLeafPage_) const
{
	const void*			fieldBuffer	= 0;
	Os::Memory::Size	fieldLen = 0;
	FileCommon::DataManager::getCommonDataBuffer(*KeyField_,
												 fieldBuffer,
												 fieldLen);

	bool	isCompressed = false;

	Os::Memory::Size	uncompressedSize = 0;

	if ((KeyField_->getFunction() & Common::Data::Function::Compressed) != 0)
	{
		// 圧縮されているかもしれない…

		const Common::CompressedData*	compressedKeyField =
			dynamic_cast<const Common::CompressedData*>(KeyField_);

		; _SYDNEY_ASSERT(compressedKeyField != 0);

		if (compressedKeyField->isCompressed())
		{
			isCompressed = true;

			uncompressedSize = compressedKeyField->getValueSize();
		}
	}

	if (isCompressed)
	{
		// 圧縮されている…

		return
			this->writeCompressedOutsideVariableKey(
				TopNodePage_,
				static_cast<const char*>(fieldBuffer),
				uncompressedSize,
				fieldLen, // これが圧縮後のフィールド長
				0,        // バッファのインデックス
				AttachNodePages_,
				AllocateNodePageIDs_,
				IsLeafPage_);
	}
	else
	{
		// 圧縮されていない…

		; _SYDNEY_ASSERT(fieldLen % sizeof(ModUnicodeChar) == 0);

		return this->writeOutsideVariableKey(TopNodePage_,
											 static_cast<const char*>(fieldBuffer),
											 fieldLen,
											 0, // バッファのインデックス
											 AttachNodePages_,
											 AllocateNodePageIDs_,
											 IsLeafPage_);
	}
}

//
//	FUNCTION private
//	Btree::File::writeCompressedOutsideVariableKey --
//		圧縮されている外置き可変長キーフィールドの値を書き込む
//
//	NOTES
//	圧縮されている外置き可変長キーフィールドの値を書き込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*		TopNodePage_
//		書き込み先ノードページの物理ページ記述子
//	const char*				FieldBuffer_
//		圧縮されている可変長キーフィールドの値が記録されている
//		バッファへのポインタ
//	const Os::Memory::Size	UncompressedSize_
//		圧縮前のフィールド長
//	const Os::Memory::Size	CompressedSize_
//		圧縮後のフィールド長
//	const int				BufferIndex_
//		引数FieldBuffer_の書き込み開始インデックス
//	PageVector&				AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	PageIDVector&			AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		書き込み先ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//	const bool				IsRecur_ = false
//		再帰呼び出しかどうか
//			true  : 再帰呼び出し
//			false : 再帰呼び出しではない
//
//	RETURN
//	ModUInt64
//		外置き可変長キーフィールドオブジェクトのID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::writeCompressedOutsideVariableKey(
	PhysicalFile::Page*		TopNodePage_,
	const char*				FieldBuffer_,
	const Os::Memory::Size	UncompressedSize_,
	const Os::Memory::Size	CompressedSize_,
	const int				BufferIndex_,
	PageVector&				AttachNodePages_,
	PageIDVector&			AllocateNodePageIDs_,
	const bool				IsLeafPage_,
	const bool				IsRecur_ // = false
	) const
{
	; _SYDNEY_ASSERT(TopNodePage_ != 0);
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
		// フィールド値位外に必要なサイズに、
		// 圧縮前後のフィールド長の記録サイズを加算する。
		//

		otherSize += fieldLenArchiveSize;
	}

	Os::Memory::Size	demandAreaSize = writeBufferLen + otherSize;

	Os::Memory::Size	freeSizeMax =
		this->m_NodePageFreeSizeMax -
		NodePageHeader::getArchiveSize(IsLeafPage_);

	File::ObjectType	objectType = File::CompressedObjectType;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (demandAreaSize > freeSizeMax)
	{
		// 1つの物理エリアでは収まらない（＝1つの物理ページでは収まらない）…

		//
		// では、リンクオブジェクトにして複数の物理エリアに記録する。
		//

		objectType = File::DivideCompressedObjectType;

		otherSize += File::ObjectIDArchiveSize;

		writeBufferLen = freeSizeMax - otherSize;

		// 1つの物理エリアに記録可能なフィールド長を求める
		writeBufferLen = freeSizeMax - otherSize;

		demandAreaSize = writeBufferLen + otherSize;

		// 再帰呼び出し
		nextLinkObjectID =
			this->writeCompressedOutsideVariableKey(
				TopNodePage_,
				FieldBuffer_,
				UncompressedSize_,
				CompressedSize_,
				BufferIndex_ + writeBufferLen,
				AttachNodePages_,
				AllocateNodePageIDs_,
				IsLeafPage_,
				true); // 再帰呼び出し
	}

	PhysicalFile::Page*	fieldObjectPage =
		this->searchInsertNodePage(TopNodePage_,
								   AttachNodePages_,
								   AllocateNodePageIDs_,
								   demandAreaSize,
								   IsLeafPage_);

	PhysicalFile::PageID	fieldObjectPageID = fieldObjectPage->getID();

	PhysicalFile::AreaID	fieldObjectAreaID =
		fieldObjectPage->allocateArea(*this->m_pTransaction,
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

	; _SYDNEY_ASSERT(fieldWritePos != 0);

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

	ModOsDriver::Memory::copy(fieldWritePos,
							  FieldBuffer_ + BufferIndex_,
							  writeBufferLen);

#ifdef DEBUG

	Os::Memory::Size	fieldObjectAreaSize =
		fieldObjectPage->getAreaSize(fieldObjectAreaID);

	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(
			fieldWritePos + writeBufferLen - fieldObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == fieldObjectAreaSize);

#endif

	if (this->m_CatchMemoryExhaust && (fieldObjectPage != TopNodePage_))
	{
		this->m_pPhysicalFile->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return File::makeObjectID(fieldObjectPageID, fieldObjectAreaID);
}

//
//	FUNCTION private
//	Btree::File::writeOutsideVariableKey --
//		外置き可変長キーフィールドの値を書き込む
//
//	NOTES
//	外置き可変長キーフィールドの値を書き込む。
//
//	ARGUMENTS
//	PhysicalFile::Page*		TopNodePage_
//		書き込み先ノードページの物理ページ記述子
//	const char*				FieldBuffer_
//		可変長キーフィールドの値が記録されているバッファへのポインタ
//	const Os::Memory::Size	FieldLength_
//		可変長フィールド長
//	const int				BufferIndex_
//		引数FieldBuffer_の書き込み開始インデックス
//	Btree::PageVector&		AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//	Btree::PageIDVector&	AllocateNodePageIDs_
//		ノードページ識別子ベクターへの参照
//		（アロケートしたノードページの物理ページ識別子をつむ）
//	const bool				IsLeafPage_
//		書き込み先ノードページがリーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	ModUInt64
//		外置き可変長キーフィールドオブジェクトのID
//
//	EXCEPTIONS
//	[YET!]
//
ModUInt64
File::writeOutsideVariableKey(PhysicalFile::Page*		TopNodePage_,
							  const char*				FieldBuffer_,
							  const Os::Memory::Size	FieldLength_,
							  const int					BufferIndex_,
							  PageVector&				AttachNodePages_,
							  PageIDVector&				AllocateNodePageIDs_,
							  const bool				IsLeafPage_) const
{
	; _SYDNEY_ASSERT(TopNodePage_ != 0);
	; _SYDNEY_ASSERT(FieldBuffer_ != 0);

	Os::Memory::Size	writeBufferLen = FieldLength_ - BufferIndex_;

	// フィールド値以外に必要なサイズ
	Os::Memory::Size	otherSize =
		File::ObjectTypeArchiveSize; // オブジェクトタイプ

	Os::Memory::Size	demandAreaSize = writeBufferLen + otherSize;

	Os::Memory::Size	freeSizeMax =
		this->m_NodePageFreeSizeMax -
		NodePageHeader::getArchiveSize(IsLeafPage_);

	File::ObjectType	objectType = File::NormalObjectType;

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (demandAreaSize > freeSizeMax)
	{
		// 1つの物理エリアでは収まらない（＝1つの物理ページでは収まらない）…

		//
		// では、リンクオブジェクトにして複数の物理エリアに記録する。
		//

		objectType = File::DivideObjectType;

		otherSize += File::ObjectIDArchiveSize;

		// 1つの物理エリアに記録可能なフィールド長を求める
		writeBufferLen = freeSizeMax - otherSize;

		//
		// キーフィールドにできる可変長フィールドは
		// String型のフィールドのみ。
		// なので、偶数バイトずつ分割する。
		//
		
		if (writeBufferLen & 0x1)
		{
			writeBufferLen--;
		}

		demandAreaSize = writeBufferLen + otherSize;

		// 再帰呼び出し
		nextLinkObjectID =
			this->writeOutsideVariableKey(TopNodePage_,
										  FieldBuffer_,
										  FieldLength_,
										  BufferIndex_ + writeBufferLen,
										  AttachNodePages_,
										  AllocateNodePageIDs_,
										  IsLeafPage_);
	}

	PhysicalFile::Page*	fieldObjectPage =
		this->searchInsertNodePage(TopNodePage_,
								   AttachNodePages_,
								   AllocateNodePageIDs_,
								   demandAreaSize,
								   IsLeafPage_);

	PhysicalFile::PageID	fieldObjectPageID = fieldObjectPage->getID();

	PhysicalFile::AreaID	fieldObjectAreaID =
		fieldObjectPage->allocateArea(*this->m_pTransaction,
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
		fieldWritePos =
			syd_reinterpret_cast<char*>(objectTypeWritePos + 1);
	}

	; _SYDNEY_ASSERT(fieldWritePos != 0);

	if (writeBufferLen > 0)
	{
		ModOsDriver::Memory::copy(fieldWritePos,
								  FieldBuffer_ + BufferIndex_,
								  writeBufferLen);
	}

#ifdef DEBUG

	Os::Memory::Size	fieldObjectAreaSize =
		fieldObjectPage->getAreaSize(fieldObjectAreaID);

	Os::Memory::Size	writeSize =
		static_cast<Os::Memory::Size>(
			fieldWritePos + writeBufferLen - fieldObjectAreaTop);

	; _SYDNEY_ASSERT(writeSize == fieldObjectAreaSize);

#endif

	if (this->m_CatchMemoryExhaust && (fieldObjectPage != TopNodePage_))
	{
		this->m_pPhysicalFile->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまい、
			        // 物理ファイルマネージャがキャッシュしないようにする。
	}

	return File::makeObjectID(fieldObjectPageID, fieldObjectAreaID);
}

//
//	FUNCTION private
//	Btree::writeInsideVariableField --
//		外置きではない可変長フィールドの値を書き込む
//
//	NOTES
//	外置きではない可変フィールドの値を書き込む。
//
//	ARGUMENTS
//	char*					FieldTop_
//		フィールド値を書き込む領域へのポインタ
//	const Common::Data*		Field_
//		可変長フィールドデータへのポインタ
//	const Os::Memory::Size	MaxLen_
//		可変長フィールドの最大長
//
//	RETURN
//	char*
//		次のフィールド値が書き込まれている領域へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
// static
char*
File::writeInsideVariableField(char*					FieldTop_,
							   const Common::Data*		Field_,
							   const Os::Memory::Size	MaxLen_)
{
	; _SYDNEY_ASSERT(
		MaxLen_ > 0 &&
		MaxLen_ <= FileParameter::VariableFieldInsideThreshold);

	const void*			fieldBuffer = 0;
	Os::Memory::Size	fieldLen = 0;
	FileCommon::DataManager::getCommonDataBuffer(*Field_,
												 fieldBuffer,
												 fieldLen);

	; _SYDNEY_ASSERT(fieldLen <= MaxLen_);

	File::InsideVarFieldLen*	lengthWritePos =
		syd_reinterpret_cast<File::InsideVarFieldLen*>(FieldTop_);

	*lengthWritePos = static_cast<File::InsideVarFieldLen>(fieldLen);

	char*	fieldWritePos = syd_reinterpret_cast<char*>(lengthWritePos + 1);

	if (fieldLen > 0)
	{
		ModOsDriver::Memory::copy(fieldWritePos, fieldBuffer, fieldLen);

		fieldWritePos += fieldLen;
	}

	if (fieldLen < MaxLen_)
	{
		// 外置きでない場合、
		// フィールド値を記録するための領域サイズは
		// フィールド最大長固定となっている。
		// そのため、今回書き込んだフィールド長が
		// 指定されているフィールド最大長未満の場合、
		// その差分だけポインタを進める必要がある。

		Os::Memory::Size	diffLen = MaxLen_ - fieldLen;

		fieldWritePos += diffLen;
	}

	return fieldWritePos;
}

//
//	FUNCTION private
//	Btree::File::freeOutsideVariableKeyArea --
//		外置き可変長キーフィールドオブジェクトが記録されている
//		物理エリアを解放する
//
//	NOTES
//	外置き可変長キーフィールドオブジェクトが記録されている
//	物理エリアを解放する。
//
//	ARGUMENTS
//	const ModUInt64		FieldObjectID_
//		外置き可変長キーフィールドオブジェクトのオブジェクトID
//	Btree::PageVector&	AttachNodePages_
//		ノードページ記述子ベクターへの参照
//		（アタッチしたノードページの物理ページ記述子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
File::freeOutsideVariableKeyArea(const ModUInt64	FieldObjectID_,
								 PageVector&		AttachNodePages_) const
{
	PhysicalFile::Page*	fieldObjectPage = File::attachPage(
		m_pTransaction, m_pPhysicalFile,
		Common::ObjectIDData::getFormerValue(FieldObjectID_),
		m_FixMode, m_CatchMemoryExhaust, AttachNodePages_);

	const PhysicalFile::AreaID fieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(FieldObjectID_);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(fieldObjectPage, fieldObjectAreaID));

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (*objectTypeReadPos == File::DivideObjectType ||
		*objectTypeReadPos == File::DivideCompressedObjectType)
	{
		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);
	}

	fieldObjectPage->freeArea(*this->m_pTransaction,
							  fieldObjectAreaID);

	if (this->m_CatchMemoryExhaust)
	{
		this->m_pPhysicalFile->detachPage(
			fieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチ（アンフィックス）してしまう
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		this->freeOutsideVariableKeyArea(nextLinkObjectID,
										 AttachNodePages_);
	}
}

//
//	FUNCTION private
//	Btree::File::freeVariableFieldObjectArea --
//		可変長フィールドオブジェクトが記録されている物理エリアを解放する
//
//	NOTES
//	可変長フィールドオブジェクトが記録されている物理エリアを解放する。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	const ModUInt64						VariableFieldObjectID_
//		可変長フィールドオブジェクトのオブジェクトID
//	PhysicalFile::Page*					DirectObjectPage_
//		代表オブジェクトが記録されている物理ページの記述子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const bool							CatchMemoryExhaust_
//		例外MemoryExhaustをキャッチしたかどうか
//			true  : キャッチした
//			false : キャッチしていない
//	PageVector&							AttachPages_
//		物理ページ記述子ベクターへの参照
//		（アタッチした物理ページ記述子をつむ）
//	PageIDVector&						FreePageIDs_
//		物理ページ識別子ベクターへの参照
//		（フリーした物理ページの識別子をつむ）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
// static
void
File::freeVariableFieldObjectArea(
	const Trans::Transaction*			Transaction_,
	const ModUInt64						VariableFieldObjectID_,
	PhysicalFile::Page*					DirectObjectPage_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const bool							CatchMemoryExhaust_,
	PageVector&							AttachPages_,
	PageIDVector&						FreePageIDs_)
{
	PhysicalFile::Page*		variableFieldObjectPage = 0;

	bool	attached =
		File::attachObjectPage(
			Transaction_,
			VariableFieldObjectID_,
			DirectObjectPage_,
			variableFieldObjectPage,
			FixMode_,
			CatchMemoryExhaust_,
			AttachPages_);

	const PhysicalFile::AreaID variableFieldObjectAreaID =
		Common::ObjectIDData::getLatterValue(VariableFieldObjectID_);

	const File::ObjectType*	objectTypeReadPos =
		static_cast<const File::ObjectType*>(
			File::getConstAreaTop(variableFieldObjectPage,
								  variableFieldObjectAreaID));

	ModUInt64	nextLinkObjectID = FileCommon::ObjectID::Undefined;

	if (*objectTypeReadPos == File::DivideObjectType ||
		*objectTypeReadPos == File::DivideCompressedObjectType)
	{
		// リンクオブジェクト…

		const char*	nextLinkObjectIDReadPos =
			syd_reinterpret_cast<const char*>(objectTypeReadPos + 1);

		File::readObjectID(nextLinkObjectIDReadPos, nextLinkObjectID);
	}

	variableFieldObjectPage->freeArea(*Transaction_,
									  variableFieldObjectAreaID);

	if (attached)
	{
		if (variableFieldObjectPage->getTopAreaID(*Transaction_) ==
			PhysicalFile::ConstValue::UndefinedAreaID)
		{
			PhysicalFile::File*	physicalFile =
				variableFieldObjectPage->getFile();

			PhysicalFile::PageID	variableFieldObjectPageID =
				variableFieldObjectPage->getID();

			File::detachPage(
				physicalFile,
				AttachPages_,
				variableFieldObjectPage,
				PhysicalFile::Page::UnfixMode::Dirty,
				false);

			FreePageIDs_.pushBack(variableFieldObjectPageID);

			physicalFile->freePage(*Transaction_,
								   variableFieldObjectPageID);

			variableFieldObjectPage = 0;
		}
		else
		{
			variableFieldObjectPage->compaction(*Transaction_);
		}
	}

	if (CatchMemoryExhaust_ && attached && variableFieldObjectPage != 0)
	{
		variableFieldObjectPage->getFile()->detachPage(
			variableFieldObjectPage,
			PhysicalFile::Page::UnfixMode::Dirty,
			false); // 本当にデタッチしてしまう
	}

	if (nextLinkObjectID != FileCommon::ObjectID::Undefined)
	{
		// リンクオブジェクト…

		// 再帰呼び出し
		File::freeVariableFieldObjectArea(Transaction_,
										  nextLinkObjectID,
										  DirectObjectPage_,
										  FixMode_,
										  CatchMemoryExhaust_,
										  AttachPages_,
										  FreePageIDs_);
	}
}

//
//	Copyright (c) 2000, 2001, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
