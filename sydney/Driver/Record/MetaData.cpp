// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MetaData.cpp -- メタデータクラス
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Record";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"
#include "Record/MetaData.h"
#include "Record/FileInformation.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/DecimalData.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "FileCommon/DataManager.h"
#include "FileCommon/HintArray.h"
#include "LogicalFile/ObjectID.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Limits.h"
#include "PhysicalFile/File.h"
#include "Record/File.h"
#include "Record/FileOption.h"
#include "Record/Module.h"
#include "Record/TargetFields.h"

_SYDNEY_USING
_SYDNEY_RECORD_USING

namespace
{
	// ヒントのキーワード
	const ModUnicodeString _HintCompressed("compressed");
	const ModUnicodeString _HintFixedSize("fixed");

	// DirectFileの種別
	const PhysicalFile::Type _eDirectFileType = PhysicalFile::PageManageType;
	const PhysicalFile::Type _eVariableFileType = PhysicalFile::AreaManageType;

	// バイトあたりのビットマップに使用するビット数
	const int _iBitsPerByte = 8;
}

//
//	FUNCTION public
//	Record::MetaData::MetaData -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	
//
MetaData::
MetaData(const LogicalFile::FileID&	cFileID_)
	: m_FieldInfoArray(0),
	  m_RecordArchiveSize(0),
	  m_UseRatePerPage(100),		// デフォルト値
	  m_ObjectSize(0),
	  m_ObjectPerPage(0),
	  m_DirectPageSize(0),
	  m_VariablePageSize(0),
	  m_IsTemporary(false),
	  m_ReadOnly(false),
	  m_pDirectFields(0),
	  m_pVariableFields(0),
	  m_HasCompress(false),
	  m_FileID(cFileID_),//コピー
	  m_IDNumber(m_FileID)//コピーを参照させる
{
}

//
//	FUNCTION public
//	Record::MetaData::~MetaData -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	
//
MetaData::
~MetaData()
{
	if (m_FieldInfoArray != 0)
	{
		delete [] m_FieldInfoArray;
		m_FieldInfoArray = 0;
	}
	delete m_pDirectFields, m_pDirectFields = 0;
	delete m_pVariableFields, m_pVariableFields = 0;
}

//
//	FUNCTION public
//	Record::MetaData::create -- FileIDを作成する
//
void
MetaData::create()
{
	// ヒントを解釈し、FileIDに設定する

	m_FieldNum = m_FileID.getInteger(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key));

	for (Tools::FieldNum i = 0; i < m_FieldNum; ++i)
	{
		// フィールドヒントを解釈する
		bool compress = false;
		bool fixed = false;
		
		{
			ModUnicodeString	fieldHint;
			if (m_FileID.getString(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						FileCommon::FileOption::FieldHint::Key,i), fieldHint))
			{
				FileCommon::HintArray hints(fieldHint);
				FileCommon::HintArray::Iterator iterator = hints.begin();
				const FileCommon::HintArray::Iterator& end = hints.end();
				for (; iterator != end; ++iterator)
				{
					if ((*iterator)->CompareToKey(fieldHint, _HintCompressed,
												  _HintCompressed.getLength()))
					{
						compress = true;
					}
					else if ((*iterator)->CompareToKey(
								 fieldHint,
								 _HintFixedSize,
								 _HintFixedSize.getLength()))
					{
						fixed = true;
					}
				}
			}
		}

		// フィールドチェック
		
		Common::DataType::Type	fieldType;
		{
			int	fieldTypeDummy;
			bool	findType = m_FileID.getInteger(
				_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldType::Key, i),
				fieldTypeDummy);
			if (!findType)
			{
				// フィールド型にデフォルト値はない。
				// 設定されていなければ例外送出

				SydErrorMessage
					<< "FIELD[" << i << "] : must be set FieldType"
					<< ModEndl;
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			fieldType = static_cast<Common::DataType::Type>(fieldTypeDummy);
		}

		if (i == 0)
		{
			// 0 番フィールドは絶対にオブジェクトIDであること。

			if (fieldType != LogicalFile::ObjectID().getType())
			{
				// データタイプが変だ
				SydErrorMessage
					<< "FIELD[" << i
					<< "] : ObjectID must be LogicalFile::ObjectID"
					<< ModEndl;

				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}

		// フィールド長を取得
		// 設定されていない場合はゼロにする。この値が "ゼロ" という
		// ことは、"無制限" であるという意味である。
		// ※ 配列フィールドの場合は要素数となる！

		Tools::FieldLength	fieldLength = 0;
		{
			int	fieldLengthDummy;
			if (m_FileID.getInteger(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						FileCommon::FileOption::FieldLength::Key, i),
					fieldLengthDummy)) {
				fieldLength = static_cast<Tools::FieldLength>(fieldLengthDummy);
			}
		}

		if (Common::Data::isFixedSize(fieldType))
		{
			// 固定長
			fixed = true;

			Tools::FieldLength requestedLength =
				FileCommon::DataManager::getFixedCommonDataArchiveSize(
					fieldType);

			if (fieldType == Common::DataType::Decimal)
			{
				// Decimal data should have fieldLength specified
				if (fieldLength == 0) {
					_SYDNEY_THROW0(Exception::NotSupported);
				}
			}
			else if (fieldLength == 0)
			{
				fieldLength = requestedLength;
			}
			else if (fieldLength != requestedLength)
			{
				SydErrorMessage
					<< "FIELD[" << i << "] : fieldType = " << fieldType
					<< ", length = " << fieldLength
					<< ". length must be" << requestedLength
					<< ModEndl;

				_SYDNEY_THROW0(Exception::NotSupported);
			}
		}
		else if (fixed &&
				 (fieldLength == 0 || fieldType == Common::DataType::Array))
		{
			// 無制限可変長、または配列の場合はヒントを無視する
			fixed = false;
		}
		
		// FileIDに設定する
		
		m_FileID.setBoolean(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldFixed::Key, i), fixed);

		m_FileID.setBoolean(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				Record::FileOption::Compressed::Key, i), compress);

		m_FileID.setInteger(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileCommon::FileOption::FieldLength::Key, i), fieldLength);

	}

	// 初期化する(ページサイズ等がファイルIDに設定される)
	initialize();
}

void
MetaData::
setDirectoryPath(const LogicalFile::FileID&	cFileID_) const
{
	cFileID_.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						   FileCommon::FileOption::Area::Key,0),
					   m_cstrPathName);
}

//	FUNCTION public
//	Record::MetaData::setDirectoryPath --
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	cstrPath_
//			設定する絶対パス名
//
//			引数を const Os::Path& にすると、
//			引数に ModUnicodeString を与えたとき、
//			内部で代入するときの都合 2 回コピーが起きてしまうので、
//			引数は const ModUnicodeString& にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
MetaData::
setDirectoryPath(const ModUnicodeString& cstrPath_)
{
	m_cstrPathName = cstrPath_;
}

//
//	FUNCTION public
//	Record::MetaData::getFieldSize -- 
//
//	NOTES
//	フィールドをファイルに書き込む場合に必要なサイズ。
//	フィールドのサイズとは「フィールド値」である。
//
//	フィールドが固定長の場合
//		フィールド値のためのサイズ = フィールド値の最大サイズ
//	フィールドが可変長の場合
//		フィールド値のためのサイズ = データサイズ
//									+ ファイル上のサイズ(圧縮)
//									+ データオブジェクトのオブジェクトID
//	フィールドが配列の場合
//		フィールド値のためのサイズ = データオブジェクトのオブジェクトID
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	
//

Os::Memory::Size
MetaData::
getFieldSize(const int	FieldID_) const
{
	FieldInfo&			info = m_FieldInfoArray[FieldID_];
	Os::Memory::Size&	fieldSize = info.m_FieldSize;

	if (!fieldSize) {

		// フィールド種を格納するのはやめた

		if (!info.m_Variable) {

			// フィールド値の型が固定長のものならば
			// フィールド値がファイル上に占めるサイズと
			// フィールド値の最大サイズは等しい

			if (info.m_DataType._name != Common::DataType::Decimal)
				fieldSize += info.m_DataType._length;
			else
				fieldSize += Common::DecimalData::getDumpSizeBy(info.m_DataType._length, info.m_DataType._scale);

			if (info.m_DataType.isVariable())

				// 型がFixedSizeでないなら実際のサイズを格納する部分も必要

				fieldSize += Tools::m_FieldLengthArchiveSize;

		} else if (info.m_DataType._name != Common::DataType::Array) {

			// 可変長のものについてデータサイズを
			// 代表オブジェクトに格納するのはやめた
			// オブジェクトIDだけを保持している

			fieldSize += Tools::m_ObjectIDArchiveSize;
		} else {

			// 配列フィールドの場合

			//	オブジェクトID(ModUInt64)

			fieldSize += Tools::m_ObjectIDArchiveSize;
		}

		; _SYDNEY_ASSERT(fieldSize);
	}

	return fieldSize;
}

//
//	FUNCTION public
//	Record::MetaData::getHeaderSize -- 
//
//	NOTES
//	フィールドを可変長ファイルに書き込むときにヘッダーとして
//	書き込むデータのサイズ
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//	
//

Os::Memory::Size
MetaData::
getHeaderSize(const int	FieldID_) const
{
	FieldInfo&			info = m_FieldInfoArray[FieldID_];
	Os::Memory::Size&	headerSize = info.m_HeaderSize;

	if (!headerSize) {

		if (!info.m_Variable) {

			// フィールド値の型が固定長のものならばヘッダーは要らない

			headerSize = 0;

		} else if (info.m_DataType._name != Common::DataType::Array) {

			// 可変長のものは圧縮前後のデータサイズ

			headerSize = Tools::m_FieldLengthArchiveSize * 2;

		} else {

			// 配列フィールドの場合
			// 要素が固定長なら要素数
			// 要素が可変長なら要素数とデータサイズ

			headerSize = Tools::m_ElementNumArchiveSize;

			if (info.m_ElementType.isVariable()) {
				headerSize += Tools::m_FieldLengthArchiveSize;
			}
		}

		; _SYDNEY_ASSERT(headerSize);
	}

	return headerSize;
}

// 同値性の検査
// (メタデータだけ比較する。従って論理ファイルIDは比較しない)
bool
MetaData::
equals(const MetaData& MetaData_) const
{
	// ディレクトリパス名だけ比較する
	return m_cstrPathName.compare(
		MetaData_.getDirectoryPath()) == Os::Path::CompareResult::Identical;
}

//
//	FUNCTION public
//	Record::MetaData::getVersion -- バージョン番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		バージョン番号
//
//	EXCEPTIONS
//
int
MetaData::getVersion() const
{
	int v;
	if (m_FileID.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key), v) == false)
		v = FileInformation::Vers::CurrentVersion;
	return v;
}

//
// PRIVATE
//

//
//	FUNCTION private
//	Record::MetaData::initialize -- 初期化
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MetaData::initialize() const
{
	if (m_FieldInfoArray == 0)
	{
		// フィールド情報の配列を作成
		initializeFieldInfoArray(m_FileID);

		// フィールドを固定長と可変長に分ける
		divideFields();

		// ObjectSize
		setObjectSize();

		// PageSize
		setDirectPageSize();
		setVariablePageSize();

		// YET YET YET
		// m_UseRatePerPage をチューニングできる機構を入れる

		setTemporary(m_FileID);
		setReadOnly(m_FileID);

		// 物理ファイルに関する情報
		// パスの存在確認は create で行う
		setDirectoryPath(m_FileID);
	}
}


// フィールド情報をベクタに読み込む
// (フィールド数が本当にゼロの場合は無駄な処理を通るが害はない)
void
MetaData::
initializeFieldInfoArray(
	const LogicalFile::FileID&	cFileID_) const
{
	m_FieldNum = cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::FieldNumber::Key));

	m_VariableFieldNum = 0;

	// Version2かどうか
	int iVersion = getVersion();

	// フィールド情報を格納するための配列を確保
	// (先頭要素はオブジェクトIDのためのフィールド)
	m_FieldInfoArray = new FieldInfo[m_FieldNum];

	// フィールド数とはオブジェクトIDを含むフィールドの総数である
	for (Tools::FieldNum i = 0; i < m_FieldNum; ++i)
	{
		//
		// フィールド情報を取得
		//

		// フィールドヒント
		bool compress = false;
		bool fixed = false;

		if (iVersion < FileInformation::Vers::Version3)
		{
			// 昔のバージョンは毎回ヒントを解釈しなくてはならない
			
			ModUnicodeString	fieldHint;
			if (cFileID_.getString(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldHint::Key,i), fieldHint)) {
				FileCommon::HintArray hints(fieldHint);
				FileCommon::HintArray::Iterator iterator = hints.begin();
				const FileCommon::HintArray::Iterator& end = hints.end();
				for (; iterator != end; ++iterator) {
					if ((*iterator)->CompareToKey(fieldHint, _HintCompressed, _HintCompressed.getLength()))
						compress = true;
					else if ((*iterator)->CompareToKey(fieldHint, _HintFixedSize, _HintFixedSize.getLength()))
						fixed = true;
				}
			}
		}
		else
		{
			bool tmp0;
			if (cFileID_.getBoolean(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						Record::FileOption::Compressed::Key, i), tmp0) == true)
			{
				compress = tmp0;
			}
		}
		
		m_FieldInfoArray[i].m_Compress = compress;
		if (compress) m_HasCompress = true;

		// FIXEDかどうか -- バージョン2(数値としては1)以上のときのみ
		if (iVersion >= FileInformation::Vers::Version2)
		{
			bool tmp0;
			if (cFileID_.getBoolean(
					_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						FileCommon::FileOption::FieldFixed::Key,i),	tmp0)
				== true)
			{
				fixed = tmp0;
			}
		}
		
		// フィールド型を取得

		Common::DataType::Type	fieldType;
		{
			int	fieldTypeDummy;
			cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldType::Key,i), fieldTypeDummy); // falseを返すことはない
			fieldType = static_cast<Common::DataType::Type>(fieldTypeDummy);
		}

		// フィールド長を取得
		// 設定されていない場合はゼロにする。この値が "ゼロ" という
		// ことは、"無制限" であるという意味である。
		// ※ 配列フィールドの場合は要素数となる！

		Tools::FieldLength	fieldLength = 0;
		{
			int	fieldLengthDummy;
			if (cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldLength::Key,i), fieldLengthDummy)) {
				fieldLength = static_cast<Tools::FieldLength>(fieldLengthDummy);
			}
		}

		// Fraction を取得
		int fieldScale = 0;
		(void) cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::FieldFraction::Key,i),
								   fieldScale);

		// フィールドの符号化方式を取得する
		{
		int tmp;
		m_FieldInfoArray[i].m_DataType._encodingForm =
			(cFileID_.getInteger(
				_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldEncodingForm::Key, i), tmp)) ?
			((static_cast<Common::StringData::EncodingForm::Value>(tmp)
			 == Common::StringData::EncodingForm::UTF8) ?
			Common::StringData::EncodingForm::Unknown :
			static_cast<Common::StringData::EncodingForm::Value>(tmp)) :
			Common::StringData::EncodingForm::UCS2;
		}
		// 配列要素の型を不定にしておく
		// 配列フィールドの時に実際の値は設定される

		m_FieldInfoArray[i].m_ElementType._name = Common::DataType::Undefined;
		m_FieldInfoArray[i].m_ElementType._length = 0;
		m_FieldInfoArray[i].m_ElementType._scale = 0;
		m_FieldInfoArray[i].m_ElementType._encodingForm =
			Common::StringData::EncodingForm::UCS2;

		// フィールド長の検査とデフォルト値の設定

		Tools::ElementNum	elementNum = 0;

		if (iVersion < FileInformation::Vers::Version3)
		{
			if (Common::Data::isFixedSize(fieldType)) {
				// 固定長

				fixed = true;

				Tools::FieldLength requestedLength =
					FileCommon::DataManager::getFixedCommonDataArchiveSize(
						fieldType);

				if (fieldLength == 0)
				{
					// 設定されていない場合はデフォルト値をセット
					fieldLength = requestedLength;
				}
				else if (fieldLength != requestedLength)
				{
					// ユーザーが設定した値が間違っている
					SydErrorMessage
						<< "FIELD[" << i << "] : fieldType = " << fieldType
						<< ", length = " << fieldLength
						<< ". length must be" << requestedLength
						<< ModEndl;

					_SYDNEY_THROW0(Exception::NotSupported);
				}

			} else if (fixed &&
					   (fieldLength == 0 ||
						fieldType == Common::DataType::Array)) {
				
				// 型は可変長だがHINTで固定長領域に書くことが要求されている
				// 以下の場合はこの要求は無視する
				// 1. フィールド長が指定されていない(=0)
				// 2. 配列である

				fixed = false;
			}
		}

		if (fixed) {
			// 固定長
			
			m_FieldInfoArray[i].m_Variable = false;

		} else {
			// 可変長または配列
			; _SYDNEY_ASSERT(!Common::Data::isFixedSize(fieldType));

			m_FieldInfoArray[i].m_Variable = true;
			++m_VariableFieldNum;

			if (fieldType == Common::DataType::Array) {
				// 配列

				// 配列要素のデータ型を取得
				{
					int	fieldTypeDummy;
					if (cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::ElementType::Key,i), fieldTypeDummy)) {
						m_FieldInfoArray[i].m_ElementType._name = static_cast<Common::DataType::Type>(fieldTypeDummy);

					} else {
						// 配列要素のデータ型にデフォルト値はない。
						// 設定されていなければ例外送出

						SydErrorMessage
							<< "FIELD[" << i << "] : must be set ElementType"
							<< ModEndl;

						_SYDNEY_THROW0(Exception::Unexpected);
					}
				}

				// 配列要素データの最大長
				{
					int	elementLengthDummy;
					if (cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::ElementLength::Key,i) ,elementLengthDummy)) {
						m_FieldInfoArray[i].m_ElementType._length = static_cast<Tools::FieldLength>(elementLengthDummy);
					}
				}

				// Scale of element type of array
				m_FieldInfoArray[i].m_ElementType._scale = fieldScale;
				fieldScale = 0; // scale of array data is zero

				// 配列要素の値の符号化方式を取得する
				{
				int tmp;
				if (cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(FileCommon::FileOption::ElementEncodingForm::Key, i), tmp))
					m_FieldInfoArray[i].m_ElementType._encodingForm =
					(static_cast<Common::StringData::EncodingForm::Value>(tmp)
					 == Common::StringData::EncodingForm::UTF8) ?
						Common::StringData::EncodingForm::Unknown :
						static_cast<Common::StringData::EncodingForm::Value>(
							tmp);
				}
				// 要素数はFieldLengthとして指定されている
				elementNum = static_cast<Tools::ElementNum>(fieldLength);
				fieldLength = 0;
			}
		}

		m_FieldInfoArray[i].m_DataType._name = fieldType;
		m_FieldInfoArray[i].m_DataType._length = fieldLength;
		m_FieldInfoArray[i].m_DataType._scale = fieldScale;
		m_FieldInfoArray[i].m_ElementNum = elementNum;

		// 必要になった時点で求める
		m_FieldInfoArray[i].m_FieldSize = 0;
		m_FieldInfoArray[i].m_HeaderSize = 0;
	}//end for()
}

void
MetaData::
setTemporary(const LogicalFile::FileID&	cFileID_) const
{
	bool	paramValue = false;

	if (cFileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key), paramValue)
		== false)
	{
		paramValue = false;
	}

	m_IsTemporary = paramValue;
}

//
void
MetaData::
setReadOnly(const LogicalFile::FileID&	cFileID_) const
{
	// ファイルオプションにReadOnlyが設定されているかどうか

	bool	paramValue = false;

	if (cFileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::ReadOnly::Key), paramValue)
		== false)
	{
		paramValue = false;
	}

	m_ReadOnly = paramValue;
}

void
MetaData::
divideFields() const
{
	m_pDirectFields = new TargetFields(m_FieldNum - m_VariableFieldNum);
	m_pVariableFields = new TargetFields(m_VariableFieldNum);

	TargetFields::divide(*m_pDirectFields, *m_pVariableFields,
						 0, *this);
}

void
MetaData::
setObjectSize() const
{
	// オブジェクトサイズは上位から与えてコントロールできるものではないので
	// 常に計算する

	Os::Memory::Size iObjectSize = 0;
	Tools::FieldNum n = getFieldNumber();

	// null bitmapのためのサイズを得る
	// フィールド数はObjectIDの分を差し引く
	iObjectSize += Tools::getBitmapSize(n - 1);

	// 固定長フィールドのサイズを得る
	for (Tools::FieldNum i = 1; i < n; ++i) {
		if (!isVariable(i)) {
			iObjectSize += getFieldSize(i);
		}
	}
	// 可変長があればそれを指すためのオブジェクトIDのサイズを加える
	if (hasVariable()) {
		iObjectSize += Tools::m_ObjectIDArchiveSize;

	} else {
		// 可変長がない場合でも
		// 削除したオブジェクトをリンクするための
		// オブジェクトIDを書き込むため、
		// 少なくともオブジェクトIDのサイズ以上なければいけない
		if (iObjectSize < Tools::m_ObjectIDArchiveSize) {
			iObjectSize = Tools::m_ObjectIDArchiveSize;
		}
	}
	m_ObjectSize = iObjectSize;
}

void
MetaData::
setDirectPageSize() const
{
	int intValue;
	if (m_FileID.getInteger(_SYDNEY_FILE_PARAMETER_KEY(Record::FileOption::DirectPageSize::Key), intValue) ||
		m_FileID.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), intValue))
	{
		// Kbyte -> byte
		m_DirectPageSize = intValue << 10;
	}
	else {
		// 計算で求める
		calculateDirectPageSize();

		// FileIDにセットする
		// byte -> Kbyte
		intValue = m_DirectPageSize >> 10;
		m_FileID.setInteger(_SYDNEY_FILE_PARAMETER_KEY(Record::FileOption::DirectPageSize::Key), intValue);
	}

	/////////////////////////////////////////////////////////////////////////
	// FileIDに設定されていても計算してもここで再度オブジェクト数を計算する
	/////////////////////////////////////////////////////////////////////////

	PhysicalFile::PageSize iAvailableSize =
		PhysicalFile::File::getPageDataSize(_eDirectFileType, m_DirectPageSize);

	// ページごとのヘッダーにオブジェクト数を使うので使えるサイズから減らす
	iAvailableSize -= sizeof(ModSize);

	m_ObjectPerPage =
		(iAvailableSize * _iBitsPerByte - _iBitsPerByte + 1)
		/ (m_ObjectSize * _iBitsPerByte + 1);
}

void
MetaData::
calculateDirectPageSize() const
{
	///////////////////////////////////////////////////////////////
	// ページあたりのオブジェクト数の最小値に関する設定を得る
	///////////////////////////////////////////////////////////////

	int intValue;
	int iMinimumValue;
	// 最小値より小さかったら無視する
	if (m_FileID.getInteger(_SYDNEY_FILE_PARAMETER_KEY(Record::FileOption::MinimumObjectPerPage::Key), intValue)
		&& intValue >= Tools::Configuration::getMinimumObjectPerPage())
		iMinimumValue = intValue;
	else {
		iMinimumValue = Tools::Configuration::getMinimumObjectPerPage();

		// FileIDにセットする
		m_FileID.setInteger(_SYDNEY_FILE_PARAMETER_KEY(Record::FileOption::MinimumObjectPerPage::Key), iMinimumValue);
	}

	////////////////////////////////////////////////////////////
	// オーバーヘッドを除いた部分でページサイズを計算する
	////////////////////////////////////////////////////////////
	Os::Memory::Size defaultPageSize
		= FileCommon::FileOption::PageSize::getDefault();
	Os::Memory::Size iPageSize = defaultPageSize;

	PhysicalFile::PageSize iAvailableSize =
		PhysicalFile::File::getPageDataSize(_eDirectFileType, iPageSize);

	// ページごとのヘッダーにオブジェクト数を使うので使えるサイズから減らす
	iAvailableSize -= sizeof(ModSize);

	if (iAvailableSize / iMinimumValue < m_ObjectSize) {
		// オブジェクトサイズのiMinimumValue倍を超える
		// デフォルトページサイズの倍数にする
		// ↓計算結果が型の最大値を超えないことを確認している
		; _SYDNEY_ASSERT(Os::Limits<PhysicalFile::PageSize>::IsSpecialized
						 && (m_ObjectSize
							 < (Os::Limits<PhysicalFile::PageSize>::getMax()
								/ iMinimumValue - defaultPageSize)));

		PhysicalFile::PageSize iRequiredSize = m_ObjectSize * iMinimumValue;
		iPageSize = iRequiredSize - (iRequiredSize % defaultPageSize) + defaultPageSize;

		// 新しいページサイズで使えるサイズを得ておく
		iAvailableSize = 
			PhysicalFile::File::getPageDataSize(_eDirectFileType, iPageSize);
	}

	///////////////////////////////////////////
	// ページあたりのオブジェクト数を計算する
	///////////////////////////////////////////

	// 使えるバイト数 >= (x+7)/8 + x*ObjectSize
	// を満たす最大のxがオブジェクト数である
	// --> 8*avail >= x+7+x*size*8
	// --> 8*avail - 7 >= x(size*8+1)

	int iObjectPerPage =
		(iAvailableSize * _iBitsPerByte - _iBitsPerByte + 1)
		/ (m_ObjectSize * _iBitsPerByte + 1);

	; _SYDNEY_ASSERT(iObjectPerPage > 0);

	// 計算の結果最小値を下回ったらdefaultPageSize分増やす
	// ヘッダーの誤差部分なのでこれで超える
	if (iObjectPerPage < iMinimumValue)
		iPageSize += defaultPageSize;

	// ページサイズが確定した
	m_DirectPageSize = iPageSize;
}

void
MetaData::
setVariablePageSize() const
{
	int iVersion = getVersion();

	int intValue;
	if (m_FileID.getInteger(_SYDNEY_FILE_PARAMETER_KEY(Record::FileOption::VariablePageSize::Key), intValue)
		|| (iVersion <= FileInformation::Vers::Version3
			&& m_FileID.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), intValue)))
										// From Version4, VariablePageSize does not refer PageSize::Key
	{
		///////////////////////////////////////////////
		// get the value from FileID
		///////////////////////////////////////////////

		// Kbyte -> byte
		m_VariablePageSize = intValue << 10;
	}
	else {
		///////////////////////////////////////////////
		// calculate proper page size by calculation
		///////////////////////////////////////////////

		// add all the limit length
		Os::Memory::Size iSizeSum = 0;
		Tools::FieldNum n = getFieldNumber();
		for (Tools::FieldNum i = 1; i < n; ++i) {
			if (isVariable(i)) {
				// data length
				int iLength = m_FieldInfoArray[i].m_DataType._length;
				// header length
				int iHeaderLength = getHeaderSize(i);
				if (iLength == 0 || Os::Limits<int>::getMax() - iSizeSum - iLength < iHeaderLength) {
					// unlimited -> use max value
					iSizeSum = Os::Limits<int>::getMax();
					break;
				} else {
					iSizeSum += iLength + iHeaderLength;
				}
			}
		}

		// get available size
		Os::Memory::Size defaultPageSize
			= FileCommon::FileOption::PageSize::getDefault();
		PhysicalFile::PageSize iAvailableSize =
			PhysicalFile::File::getPageDataSize(_eVariableFileType, iSizeSum);
		// make size multiple of defaultPageSize
		PhysicalFile::PageSize iPageSize = iAvailableSize - (iAvailableSize % defaultPageSize) + defaultPageSize;
		m_VariablePageSize = iPageSize;

		// Set the value to FileID
		// byte -> Kbyte
		intValue = m_VariablePageSize >> 10;
		m_FileID.setInteger(_SYDNEY_FILE_PARAMETER_KEY(Record::FileOption::VariablePageSize::Key), intValue);
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
