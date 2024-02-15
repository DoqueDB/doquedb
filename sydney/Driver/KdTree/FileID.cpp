// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "KdTree/FileID.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/HintArray.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/OpenOption.h"

#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Version/File.h"

#include "Exception/NotSupported.h"
#include "Exception/SQLSyntaxError.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
	// ページサイズ(KB単位)
	Common::Configuration::ParameterInteger
	_cPageSize("KdTree_PageSize", 8);

	//
	//	ファイルのパス
	//
	Os::Path _cSmall1("Small1");
	Os::Path _cSmall2("Small2");

	namespace _Hint
	{
		// 最大計算回数
		ModUnicodeString _cMaxCalculateCount("maxcalculatecount");

		// 探索タイプ
		ModUnicodeString _cTraceType("tracetype");
		// 探索タイプの値
		ModUnicodeString _cNormal("normal");
		ModUnicodeString _cRvs("rvs");
		ModUnicodeString _cSerial("serial");
	}
}

//
//	FUNCTION public
//	KdTree::FileID::FileID -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const LogicalFile::FileID& cLogicalFileID_
//		論理ファイルインターフェースのFileID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
FileID::FileID(const LogicalFile::FileID& cLogicalFileID_)
	: LogicalFileID(cLogicalFileID_)
{
}

//
//	FUNCTION public
//	KdTree::FileID::~FileID -- デストラクタ
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
FileID::~FileID()
{
}

//
//	FUNCTION public
//	KdTree::FileID::create -- ファイルIDの内容を作成する
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
FileID::create()
{
	// PageSize
	ModSize pageSize = _cPageSize.get();	// ページサイズ
	if (pageSize < (FileCommon::FileOption::PageSize::getDefault() >> 10))
		pageSize = (FileCommon::FileOption::PageSize::getDefault() >> 10);
	pageSize = Version::File::verifyPageSize(pageSize << 10);
	pageSize >>= 10;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::PageSize::Key), pageSize);

	// 次元数を設定する
	setDimension(pageSize);
	
	//
	//	ヒントを設定する
	//
	//【注意】	検索時にも指定できるが、定義時に指定した値がデフォルトとなる
	//
	int count = -1;
	Node::TraceType::Value type = Node::TraceType::Unknown;
	ModUnicodeString cstrHint;
	getString(_SYDNEY_FILE_PARAMETER_KEY(
				  FileCommon::FileOption::FileHint::Key),
			  cstrHint);
	parseHint(cstrHint, type, count);
	
	// 最大計算回数
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::MaxCalculateCount),
			   count);
	// 探索タイプ
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TraceType),
			   type);
	
	//
	//	その他
	//

	// マウント
	setMounted(true);
	
	// Version
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::Version::Key),
			   CurrentVersion);
}

//
//	FUNCTION public
//	KdTree::FileID::getDimension -- 次元数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		次元数
//
//	EXCEPTIONS
//
int
FileID::getDimension() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Dimension));
}

//
//	FUNCTION public
//	KdTree::FileID::getMaxCalculateCount -- 最大計算回数を得る
//
//	NOTES
//	索引定義時に指定されていなかった場合は -1 を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		最大計算回数
//
//	EXCEPTIONS
//
int
FileID::getMaxCalculateCount() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::MaxCalculateCount));
}

//
//	FUNCTION public
//	KdTree::FileID::getTraceType -- 探索タイプを得る
//
//	NOTES
//	索引定義時に指定されていなかった場合は Node::TraceType::Unknown を返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	KdTree::Node::TraceType::Value
//		探索タイプ
//
//	EXCEPTIONS
//
Node::TraceType::Value
FileID::getTraceType() const
{
	return static_cast<Node::TraceType::Value>(
		getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TraceType)));
}

//
//	FUNCTION public
//	KdTree::FileID::getPageSize -- ページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		その他のページのページサイズ
//
//	EXCEPTIONS
//
int
FileID::getPageSize() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::PageSize::Key)) << 10;
}

//
//	FUNCTION public
//	KdTree::FileID::getLockName -- ロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Lock::FileName&
//		ロック名
//
//	EXCEPTIONS
//
const Lock::FileName&
FileID::getLockName() const
{
	if (m_cLockName.getDatabasePart() == ~static_cast<Lock::Name::Part>(0))
	{
		m_cLockName = FileCommon::IDNumber(*this).getLockName();
	}
	return m_cLockName;
}

//
//	FUNCTION public
//	KdTree::FileID::isReadOnly -- 読み取り専用か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		読み取り専用ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isReadOnly() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::ReadOnly::Key));
}

//
//	FUNCTION public
//	KdTree::FileID::isTemporary -- 一時か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		一時ならtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isTemporary() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::Temporary::Key));
}

//
//	FUNCTION public
//	KdTree::FileID::isMounted -- マウントされているか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		マウントされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isMounted() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::Mounted::Key));
}

//
//	FUNCTION public
//	KdTree::FileID::setMounted -- マウントされているかを設定する
//
//	NOTES
//
//	ARGUMENTS
//	bool bFlag_
//		マウントされている場合はtrue、それ以外の場合はfalseを指定
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setMounted(bool bFlag_)
{
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::Mounted::Key), bFlag_);
}

//
//	FUNCTION public
//	KdTree::FileID::getPath -- パス名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Os::Path&
//		パス名
//
//	EXCEPTIONS
//
const Os::Path&
FileID::getPath() const
{
	if (m_cPath.getLength() == 0)
	{
		getString(_SYDNEY_FILE_PARAMETER_KEY(
					  FileCommon::FileOption::Area::Key), m_cPath);
	}
	return m_cPath;
}

//
//	FUNCTION public
//	KdTree::FileID::setPath -- パス名を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cPath_
//		パス名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setPath(const Os::Path& cPath_)
{
	setString(_SYDNEY_FILE_PARAMETER_KEY(
						FileCommon::FileOption::Area::Key), cPath_);
	m_cPath = cPath_;
}

//
//	FUNCTION public static
//	KdTree::FileID::getSmallPath -- 差分ファイルのサブパスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Os::Path&
//	   	差分ファイルのサブパス
//
//	EXCEPTIONS
//
const Os::Path&
FileID::getSmallPath1()
{
	return _cSmall1;
}
const Os::Path&
FileID::getSmallPath2()
{
	return _cSmall2;
}

//
//	FUNCTION public
//	KdTree::FileID::getUpdateParameter -- 更新パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cUpdateField_
//		更新対象フィールド
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETRUN
//	bool
//		パラメータが正しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::getUpdateParameter(const Common::IntegerArrayData& cUpdateField_,
						   LogicalFile::OpenOption& cOpenOption_) const
{
	// キーしか更新できない
	
	if (cUpdateField_.getCount() != 1 ||
		cUpdateField_.getElement(0) != 0)
		return false;
	
	// オープンモード
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
								FileCommon::OpenOption::OpenMode::Key),
							FileCommon::OpenOption::OpenMode::Update);

	return true;
}

//
//	FUNCTION public static
//	KdTree::FileID::parseHint -- ヒント文字列をパースする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHint_
//		ヒント文字列
//	KdTree::Node::TraceType::Value& eTraceType_
//		探索タイプ (指定されていない場合は Node::TraceType::Unknown)
//	int& iMaxCount_
//		最大計算回数 (指定されていない場合は -1)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::parseHint(const ModUnicodeString& cstrHint_,
				  Node::TraceType::Value& eTraceType_,
				  int& iMaxCount_)
{
	// ヒントをパースする
	FileCommon::HintArray cHintArray(cstrHint_);

	// 探索タイプ
	eTraceType_ = getTraceType(cstrHint_, cHintArray);
	// 最大計算回数
	iMaxCount_ = getMaxCalculateCount(cstrHint_, cHintArray);
}

//
//	FUNCTION public static
//	KdTree::FileID::castTraceType -- 探索タイプ文字列を列挙型に変更する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& type
//		探索タイプ文字列
//
//	RETURN
//	KdTree::Node::TraceType::Value
//		列挙型 (該当するものがなかった場合は Node::TraceType::Unknown を返す)
//
//	EXCEPTIONS
//
Node::TraceType::Value
FileID::castTraceType(const ModUnicodeString& type)
{
	Node::TraceType::Value t = Node::TraceType::Unknown;
	
	if (type.compare(_Hint::_cNormal, ModFalse) == 0)
		t = Node::TraceType::Normal;
	else if (type.compare(_Hint::_cRvs, ModFalse) == 0)
		t = Node::TraceType::RicohVisualSearch;
	else if (type.compare(_Hint::_cSerial, ModFalse) == 0)
		t = Node::TraceType::Serial;

	return t;
}

//
//	FUNCTION public static
//	KdTree::FileID::readHint -- ヒントを読む
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//	const ModUnicodeString& cstrKey_
//		キー
//	ModUnicodeString& cstrValue_
//		ヒントの値
//
//	RETURN
//	bool
//		ヒントに存在した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//

bool
FileID::readHint(const ModUnicodeString& cstrHint_,
				 const FileCommon::HintArray& cHintArray_,
				 const ModUnicodeString& cstrKey_,
				 ModUnicodeString& cstrValue_)
{
	FileCommon::HintArray::ConstIterator i = cHintArray_.begin();
	for (; i != cHintArray_.end(); ++i)
	{
		if ((*i)->CompareToKey(cstrHint_,
							   cstrKey_, cstrKey_.getLength()) == true)
		{
			// 見つかった
			ModAutoPointer<ModUnicodeString> p = (*i)->getValue(cstrHint_);
			cstrValue_ = *p;

			return true;
		}
	}
	return false;
}

//
//	FUNCTION private
//	KdTree::FileID::setDimension -- 次元数を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiPageSize_
// 		ページサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setDimension(ModSize uiPageSize_)
{
	// キーの数は1つか？
	int keynum = getInteger(_SYDNEY_FILE_PARAMETER_KEY(
								FileCommon::FileOption::KeyFieldNumber::Key));
	if (keynum != 1)
		_SYDNEY_THROW0(Exception::NotSupported);

	// キーのデータ型は配列か？
	Common::DataType::Type eType
		= static_cast<Common::DataType::Type>(
			getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						   FileCommon::FileOption::FieldType::Key, 0)));
	if (eType != Common::DataType::Array)
		_SYDNEY_THROW0(Exception::NotSupported);

	// 配列要素のデータ型は double か?
	eType = static_cast<Common::DataType::Type>(
		getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					   FileCommon::FileOption::ElementType::Key, 0)));
	if (eType != Common::DataType::Double)
		_SYDNEY_THROW0(Exception::NotSupported);

	// 配列の要素数を得る
	int dimension
		= getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						 FileCommon::FileOption::FieldLength::Key, 0));

	// 今は、最大要素数 = 次元数なので、次元数に登録する
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Dimension), dimension);
}

//
//	FUNCTION private static
//	KdTree::FileID::setMaxCalculateCount -- 最大計算回数
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHint_
//		ヒント文字列
//	const FileCommon::HintArray& cHintArray_
//		ヒント配列
//
//	RETURN
//	int
//		最大計算回数 (設定されていない場合は -1)
//
//	EXCEPTIONS
//
int
FileID::getMaxCalculateCount(const ModUnicodeString& cstrHint_,
							 const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	int count = -1;	// 指定されていない場合は-1を設定する

	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_cMaxCalculateCount, cstrValue) == true)
	{
		count = ModUnicodeCharTrait::toInt(cstrValue);
		if (count < 0)
		{
			SydErrorMessage << "Illegal Hint: "
							<< _Hint::_cMaxCalculateCount << "="
							<< cstrValue << ModEndl;
			_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue);
		}
	}

	return count;
}

//
//	FUNCTION private static
//	KdTree::FileID::getTraceType -- 探索タイプを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHint_
//		ヒント文字列
//	const FileCommon::HintArray& cHintArray_
//		ヒント配列
//
//	RETURN
//	KdTree::Node::TraceType::Value
//		探索タイプ (指定されていない場合は Node::TraceType::Unknown)
//
//	EXCEPTIONS
//
Node::TraceType::Value
FileID::getTraceType(const ModUnicodeString& cstrHint_,
					 const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	// 指定されていない場合は unknown を設定する
	Node::TraceType::Value type = Node::TraceType::Unknown;

	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_cTraceType, cstrValue) == true)
	{
		type = castTraceType(cstrValue);
		if (type == Node::TraceType::Unknown)
		{
			SydErrorMessage << "Illegal Hint: "
							<< _Hint::_cTraceType << "="
							<< cstrValue << ModEndl;
			_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue);
		}
	}

	return type;
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
