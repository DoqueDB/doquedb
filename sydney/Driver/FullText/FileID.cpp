// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText/FileID.h"
#include "FullText/FileOption.h"

#include "FileCommon/HintArray.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/OpenOption.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/ObjectPointer.h"
#include "Common/IntegerArrayData.h"

#include "Inverted/OpenOption.h"
#include "Inverted/FileID.h"

#include "Version/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
	//
	//	delayed のヒント
	//
	ModUnicodeString _Delayed("delayed");
	
	//
	//	sectionizedのヒント
	//
	ModUnicodeString _Sectionized("sectionized");

	//
	//	kwicのヒント
	//
	ModUnicodeString _RoughKwic("kwic");

	//
	//	invertedのヒント
	//
	ModUnicodeString _Inverted("inverted");
}


//
//	FUNCTION public
//	FullText::FileID::FileID -- コンストラクタ
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
//	なし
//
FileID::FileID(const LogicalFile::FileID& cFileID_)
	: Inverted::IntermediateFileID(cFileID_),
	  m_uiVectorFieldTotalSize(0), m_bOtherFileFieldInitialized(false),
	  m_bVariableFile(false)
{
}

//
//	FUNCTION public
//	FullText::FileID::~FileID -- デストラクタ
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
//	なし
//
FileID::~FileID()
{
}

//
//	FUNCTION public
//	FullText::FileID::create -- FileIDを作成する
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
	//
	//	ヒントを解釈し、値を設定する
	//
	ModUnicodeString cstrHint;
	getString(_SYDNEY_FILE_PARAMETER_KEY(
				  FileCommon::FileOption::FileHint::Key),
			  cstrHint);
	FileCommon::HintArray cHintArray(cstrHint);

	// 遅延更新かどうか
	// [NOTE] クラスタリングを行うかどうかも設定される
	setDelayed(cstrHint, cHintArray);
	// セクション検索かどうか
	setSectionized(cstrHint, cHintArray);
	// 荒いKWIC取得用のデータを格納するかどうか
	setRoughKwic(cstrHint, cHintArray);

	// マウント
	setMounted(true);

	// ページサイズ
	ModSize pageSize = 4 << 10;	// 4KB
	if (pageSize < FileCommon::FileOption::PageSize::getDefault())
		pageSize = FileCommon::FileOption::PageSize::getDefault();
	pageSize = Version::File::verifyPageSize(pageSize);
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
				   FileCommon::FileOption::PageSize::Key), pageSize >> 10);

	// チェックする
	if (check() == false)
		_SYDNEY_THROW0(Exception::BadArgument);

	// Version
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
				   FileCommon::FileOption::Version::Key), CurrentVersion);
}

//
//	FUNCTION public
//	FullText::FileID::isDelayed -- 遅延更新かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		遅延更新の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isDelayed() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileOption::DelayProc::Key));
}

//
//	FUNCTION public
//	FullText::FileID::isSectionized -- セクション検索かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		セクション検索の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isSectionized() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileOption::Sectionized::Key));
}

//
//	FUNCTION public
//	FullText::FileID::isLanguage -- 言語情報があるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		言語情報がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isLanguage() const
{
	if (getVersion() < OtherVersion)
	{
		// 古いFullTextはキーフィールドの数で判定する
		return (getInteger(_SYDNEY_FILE_PARAMETER_KEY(
			FileCommon::FileOption::KeyFieldNumber::Key)) == 2) ? true : false;
	}
	
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileOption::Language::Key));
}

//
//	FUNCTION public
//	FullText::FileID::isScoreField -- スコア調整用のフィールドがあるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		スコア調整用のフィールドがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isScoreField() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
						  FileOption::ScoreModifier::Key));
}

//
//	FUNCTION public
//	FullText::FileID::isClustering -- クラスタ情報があるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		クラスタ情報がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isClustering() const
{
	bool v;
	if (getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileOption::Clustered::Key),
				   v) == false)
		v = false;
	return v;
}

//
//	FUNCTION public
//	FullText::FileID::isRoughKwic -- 荒いKWICのための情報があるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		情報がある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isRoughKwic() const
{
	bool v;
	// 情報は、今のところ正規化前の文字列長
	if (getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
					   FileOption::UnnormalizedCharLength::Key), v) == false)
		v = false;
	return v;
}

//
//	FUNCTION public
//	FullText::FileID::isMounted -- マウントされているか
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
//	FullText::FileID::setMounted -- マウントされているかを設定する
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
//	FullText::FileID::isTemporary -- 一時データベースかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		一時データベースの場合はtrue、それ以外の場合はfalse
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
//	FullText::FileID::isReadOnly -- ReadOnlyかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ReadOnlyの場合はtrue、それ以外の場合はfalse
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
//	FullText::FileID::isArray -- 配列かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		配列の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isArray() const
{
	return (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
		FileCommon::FileOption::FieldType::Key, 0)) == Common::DataType::Array)
		? true : false;
}

//
//	FUNCTION public
//	FullText::FileID::getLockName -- ロック名を得る
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
//	FullText::FileID::getPath -- パス名を得る
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
					  FileCommon::FileOption::Area::Key),
				  m_cPath);
	}
	return m_cPath;
}

//
//	FUNCTION public
//	FullText::FileID::setPath -- パス名を設定する
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
	m_cPath = cPath_;
	setString(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Area::Key), cPath_);
}

//
//	FUNCTION public
//	FullText::FileID::getInverted -- 大転置のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getInverted() const
{
	return setCurrentValue(getInvertedFileID());
}

//
//	FUNCTION public
//	FullText::FileID::getInsert0 -- 挿入用転置0のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getInsert0()
{
	return setCurrentValue(
		getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Ins0));
}

//
//	FUNCTION public
//	FullText::FileID::getInsert1 -- 挿入用転置1のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getInsert1()
{
	return setCurrentValue(
		getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Ins1));
}

//
//	FUNCTION public
//	FullText::FileID::getExpunge0 -- 削除用転置0のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getExpunge0()
{
	return setCurrentValue(
		getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Exp0));
}

//
//	FUNCTION public
//	FullText::FileID::getExpunge1 -- 削除用転置1のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getExpunge1()
{
	return setCurrentValue(
		getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Exp1));
}

//
//	FUNCTION public
//	FullText::FileID::getSection -- セクション情報ファイルのファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getSection()
{
	// 【注意】
	//	古いバージョンの場合にのみ利用される
	
	return setCurrentValue(getSectionInfoFileID());
}

//
//	FUNCTION public
//	FullText::FileID::setInverted -- 大転置のファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setInverted(const LogicalFile::FileID& cFileID_)
{
	getInvertedFileID() = cFileID_;
}

//
//	FUNCTION public
//	FullText::FileID::setInsert0 -- 挿入用転置0のファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setInsert0(const LogicalFile::FileID& cFileID_)
{
	getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Ins0) = cFileID_;
}

//
//	FUNCTION public
//	FullText::FileID::setInsert1 -- 挿入用転置1のファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setInsert1(const LogicalFile::FileID& cFileID_)
{
	getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Ins1) = cFileID_;
}

//
//	FUNCTION public
//	FullText::FileID::setExpunge0 -- 削除用転置0のファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setExpunge0(const LogicalFile::FileID& cFileID_)
{
	getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Exp0) = cFileID_;
}

//
//	FUNCTION public
//	FullText::FileID::setExpunge1 -- 削除用転置1のファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setExpunge1(const LogicalFile::FileID& cFileID_)
{
	getDelayProcFileID(_SYDNEY::Inverted::FileIDNumber::_Exp1) = cFileID_;
}

//
//	FUNCTION public
//	FullText::FileID::setSection -- セクション情報ファイルのファイルIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setSection(const LogicalFile::FileID& cFileID_)
{
	// 【注意】
	//	古いバージョンの場合にのみ利用される
	
	getSectionInfoFileID() = cFileID_;
}

//
//	FUNCTION public
//	FullText::FileID::getProjectionParameter
//		-- プロジェクションパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cProjection_
//		プロジェクションするフィールド
//	LogicalFile::OpenOption& cOpenOption_
//		設定するオープンオプション
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//

bool
FileID::getProjectionParameter(const Common::IntegerArrayData& cProjection_,
							   LogicalFile::OpenOption& cOpenOption_) const
{
	// 取得するフィールドは固定フィールドのみなので、
	// 固定フィールド用のフィールドマスクを作成
	FieldMask mask(cOpenOption_,isLanguage(),isScoreField(),isSectionized());

	//  2006/6/24 追加仕様
	// getProjectionParameter で平均文字数が要求された場合にはngramかdualのとき
	// のみ true を返し、平均単語数が要求された場合にはwordのときのみ
	// true を返すようにする
	// AverageLength が渡された場合は ngram/dual/word の別に応じて平均文字数か
	// 平均単語数を返す
	
	ModInvertedFileIndexingType type
		= Inverted::FileID::getIndexingType(getInverted());

	for (int i = 0; i < cProjection_.getCount(); ++i)
	{
		int n = cProjection_.getElement(i);

		if (mask.check(n,_SYDNEY::Inverted::FieldType::AverageWordCount))
		{
			if (type != ModInvertedFileWordIndexing)
				return false;
		}
		else if (mask.check(n,_SYDNEY::Inverted::FieldType::AverageCharLength))
		{
			if (!(type == ModInvertedFileNgramIndexing ||
				  type == ModInvertedFileDualIndexing))
				return false;
		}
		else if (mask.check(n,_SYDNEY::Inverted::FieldType::Cluster))
		{
			// [NOTE] TargetFieldIndex::Key を設定するだけでは不十分で、
			//  それ以外の方法で Inverted::SearchCapsule に
			//  クラスタリング検索であることを伝える必要がある。
			//  クラスタIDは、ModInvertedSearchResult に格納されないので、
			//  Inverted::SearchCapsule::execute()等に渡される
			//  ModInvertedSearchResultを見てもクラスタリング検索かどうか
			//  判別できないため。
			cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
								   Inverted::OpenOption::KeyID::Cluster), true);
		}
		else if (mask.check(n,_SYDNEY::Inverted::FieldType::FeatureValue))
		{
			if (isClustering() == false)
				return false;
		}
		
		if (checkProjection(n,mask) == false)
			return false;

		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
			FileCommon::OpenOption::TargetFieldIndex::Key, i),
								cProjection_.getElement(i));
	}

	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::FieldSelect::Key), true);
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::TargetFieldNumber::Key),
							cProjection_.getCount());
	return true;
}

//
//	FUNCTION public
//	FullText::FileID::getUpdateParameter -- 更新パラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cUpdateField_
//		更新するフィールド
//	LogicalFile::OpenOption& cOpenOption_
//		設定するオープンオプション
//
//	RETURN
//	bool
//		実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::getUpdateParameter(const Common::IntegerArrayData& cUpdateField_,
						   LogicalFile::OpenOption& cOpenOption_) const
{
	// 更新するフィールドは非固定フィールドのみなので、
	// 非固定フィールド用のフィールドマスクを作成
	FieldMask mask(isLanguage(),isScoreField());

	for (int i = 0; i < cUpdateField_.getCount(); ++i)
	{
		if(mask.checkValueRangeValidity(cUpdateField_.getElement(i)) == false)
			return false;

		cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
			FileCommon::OpenOption::TargetFieldIndex::Key, i),
								cUpdateField_.getElement(i));
	}
	cOpenOption_.setBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::FieldSelect::Key), true);
	cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::TargetFieldNumber::Key),
							cUpdateField_.getCount());

	return true;
}
//
//	FUNCTION public
//	FullText::FileID::getField -- フィールド番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	FieldMask::FieldType
//
//	RETURN
//	int
//		フィールド番号
//
//	EXCEPTIONS

int FileID::getField(FieldMask::FieldType type) const
{
	// フィールド型を取得するだけなので、非固定フィールド用マスクで十分
	FieldMask mask(isLanguage(),isScoreField());
	return mask.getField(type);
}
//
//	FUNCTION public
//	FullText::FileID::getRowIdField -- ROWIDのフィールド番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ROWIDのフィールド番号
//
//	EXCEPTIONS
//
int
FileID::getRowIdField() const
{
	return getField(_SYDNEY::Inverted::FieldType::Rowid);
}

//
//	FUNCTION public
//	FullText::FileID::getScoreField -- スコアのフィールド番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		スコアのフィールド番号
//
//	EXCEPTIONS
//
int
FileID::getScoreField() const
{
	return getField(_SYDNEY::Inverted::FieldType::Score);
}

//
//	FUNCTION public
//	FullText::FileID::getWordDfField -- ワードDFのフィールド番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ワードDFのフィールド番号
//
//	EXCEPTIONS
//
int
FileID::getWordDfField() const
{
	return getField(_SYDNEY::Inverted::FieldType::WordDf);
}

//
//	FUNCTION public
//	FullText::FileID::getWordScaleField -- ワードScaleのフィールド番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ワードScaleのフィールド番号
//
//	EXCEPTIONS
//
int
FileID::getWordScaleField() const
{
	return getField(_SYDNEY::Inverted::FieldType::WordScale);
}

//
//	FUNCTION public
//	FulLText::FileID::getVersion -- バージョンを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		バージョン
//
//	EXCEPTIONS
//
int
FileID::getVersion() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(
						  FileCommon::FileOption::Version::Key));
}

//
//	FUNCTION public
//	FullText::FileID::getVectorPageSize -- ベクターファイルのページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		ベクターファイルのページサイズ
//
//	EXCEPTIONS
//
ModSize
FileID::getVectorPageSize() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(
						  FileCommon::FileOption::PageSize::Key)) >> 10;
}

//
//	FUNCTION public
//	FullText::FileID::getVectorElementFieldCount
//		-- ベクターファイルの要素数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		要素数
//
//	EXCEPTIONS
//
ModSize
FileID::getVectorElementFieldCount() const
{
	initializeOtherFileField();
	return m_vecVectorFieldSize.getSize();
}

//
//	FUNCTION public
//	FullText::FileID::getVectorElementTotalSize
//		-- ベクターファイルの1行のサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		ベクターファイルの1行のサイズを得る
//
//	EXCEPTIONS
//
ModSize
FileID::getVectorElementTotalSize() const
{
	initializeOtherFileField();
	return m_uiVectorFieldTotalSize;
}

//
//	FUNCTION public
//	FullText::FileID::getVectorElementSize
//		ベクターファイルの1要素のサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		要素番号
//
//	RETURN
//	ModSize
//		ベクターファイルの1要素のサイズ
//
//	EXCEPTIONS
//
ModSize
FileID::getVectorElementSize(int n_) const
{
	initializeOtherFileField();
	return m_vecVectorFieldSize[n_];
}

//
//	FUNCTION public
//	FullText::FileID::isVariableFile -- 可変長ファイルが利用されているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		利用されている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isVariableFile() const
{
	initializeOtherFileField();
	return m_bVariableFile;
}

//
//	FUNCTION public
//	FullText::FileID::getOtherFileElementType
//		-- その他情報ファイルのデータ型を得る
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		要素番号
//
//	RETURN
//	FullText::FileID::DataType::Value
//		データ型
//
//	EXCEPTIONS
//
FileID::DataType::Value
FileID::getOtherFileElementType(int n_) const
{
	return static_cast<DataType::Value>(
		getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					   FileOption::OtherFieldType::Key, n_)));
}

//
//	FUNCTION private
//	FullText::FileID::readHint -- ヒントを読む
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//	const ModUncodeString& cstrKey_
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
FileID::readHint(ModUnicodeString& cstrHint_,
				 FileCommon::HintArray& cHintArray_,
				 const ModUnicodeString& cstrKey_,
				 ModUnicodeString& cstrValue_)
{
	FileCommon::HintArray::Iterator i = cHintArray_.begin();
	for (; i != cHintArray_.end(); ++i)
	{
		if ((*i)->CompareToKey(cstrHint_,
							   cstrKey_, cstrKey_.getLength()) == true)
		{
			// 見つかった
			if ((*i)->hasValue() == true)
			{
				ModAutoPointer<ModUnicodeString> p = (*i)->getValue(cstrHint_);
				cstrValue_ = *p;
			}
			return true;
		}
	}
	return false;
}

//
//	FUNCTION private
//	FullText::FileID::setDelayed -- 遅延更新かどうか
//
//	NOTES
//	FileIDに必要な設定を行う
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setDelayed(ModUnicodeString& cstrHint_,
				   FileCommon::HintArray& cHintArray_)
{
	// [YET] 遅延更新だけでなく、クラスタリングについても設定している。
	
	ModUnicodeString cstrValue;
	bool bDelayed = false;

	if (readHint(cstrHint_, cHintArray_, _Delayed, cstrValue) == true)
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bDelayed = true;
		}
		else if (cstrValue.compare(_TRMEISTER_U_STRING("false"), ModFalse) == 0)
		{
			bDelayed = false;
		}
		else
		{
			SydErrorMessage << "Illegal Delayed. " << cstrValue << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileOption::DelayProc::Key),
			   bDelayed);

	// 転置のヒントを得る
	cstrValue = "";
	readHint(cstrHint_, cHintArray_, _Inverted, cstrValue);

	// [NOTE] クラスタリングを行うかどうか、転置のヒントを参照する
	if (Inverted::FileID::isClustered(cstrValue) == true)
	{
		// FileIDに設定する
		setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileOption::Clustered::Key),
				   true);
	}

	// 大転置のFileIDを作成する
	Common::ObjectPointer<LogicalFile::FileID> pFileID
		= new LogicalFile::FileID(*this);
	pFileID->setString(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::FileHint::Key), cstrValue);
	pFileID->setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileOption::DelayProc::Key), bDelayed);	// 互換性のため(使用されない)

	if (bDelayed == true)
	{
		// 遅延更新なので、小転置のFileIDを作成する(大転置のコピー)
		Common::ObjectPointer<LogicalFile::FileID> pInsert0
			= new LogicalFile::FileID(*pFileID);
		Common::ObjectPointer<LogicalFile::FileID> pInsert1
			= new LogicalFile::FileID(*pFileID);
		Common::ObjectPointer<LogicalFile::FileID> pExpunge0
			= new LogicalFile::FileID(*pFileID);
		Common::ObjectPointer<LogicalFile::FileID> pExpunge1
			= new LogicalFile::FileID(*pFileID);
		
		// なぜか大転置に設定する(互換性のためにしょうがない)
		pFileID->setFileID(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::DelayProcFileID::Key,
				_SYDNEY::Inverted::FileIDNumber::_Exp0),
			pExpunge0);
		pFileID->setFileID(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::DelayProcFileID::Key,
				_SYDNEY::Inverted::FileIDNumber::_Ins0),
			pInsert0);
		pFileID->setFileID(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::DelayProcFileID::Key,
				_SYDNEY::Inverted::FileIDNumber::_Exp1),
			pExpunge1);
		pFileID->setFileID(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::DelayProcFileID::Key,
				_SYDNEY::Inverted::FileIDNumber::_Ins1),
			pInsert1);

		// 小転置の組数を設定する(2固定)
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(
			FileOption::InvFilePairNum::Key), 2);
	}

	// FileIDに設定する
	setFileID(_SYDNEY_FILE_PARAMETER_KEY(
		FileOption::InvertedFileID::Key), pFileID);
}

//
//	FUNCTION private
//	FullText::FileID::setSectionized -- セクション検索かどうか
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setSectionized(ModUnicodeString& cstrHint_,
					   FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	bool bSectionized = false;

	if (readHint(cstrHint_, cHintArray_, _Sectionized, cstrValue) == true)
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bSectionized = true;
		}
		else if (cstrValue.compare(_TRMEISTER_U_STRING("false"), ModFalse) == 0)
		{
			bSectionized = false;
		}
		else
		{
			SydErrorMessage << "Illegal Sectionized. " << cstrValue << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileOption::Sectionized::Key), bSectionized);
}

//
//	FUNCTION private
//	FullText::FileID::setRoughKwic
//		-- 荒いKWIC取得用のデータを格納するかどうか
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setRoughKwic(ModUnicodeString& cstrHint_,
					 FileCommon::HintArray& cHintArray_)
{
	// [NOTE] 荒いKWICは、KWICオプションが指定された索引ではなくても
	//  取得することは可能。
	//  ただし、転置索引に格納される索引語の位置情報が、
	//  元文書における索引語の位置と対応しない場合は、
	//  位置情報を補正するために正規化前文字列長を格納しておく。
	
	ModUnicodeString cstrValue;
	bool result = false;

	// KWICオプションを確認
	if (readHint(cstrHint_, cHintArray_, _RoughKwic, cstrValue) == true)
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			result = true;
		}
		else if (cstrValue.compare(_TRMEISTER_U_STRING("false"), ModFalse) == 0)
		{
			result = false;
		}
		else
		{
			SydErrorMessage << "Illegal right hand value of RoughKwic." << cstrValue << ModEndl;
			_SYDNEY_THROW0(Exception::BadArgument);
		}
	}

	if (result == true)
	{
		// 転置のヒントを得る
		cstrValue.clear();
		readHint(cstrHint_, cHintArray_, _Inverted, cstrValue);
		
		if (Inverted::FileID::isNormalized(cstrValue) == false &&
			Inverted::FileID::getIndexingType(cstrValue) != 
			ModInvertedFileWordIndexing)
		{
			// 正規化なし、かつ、単語索引以外の場合
			result = false;

			// [NOTE] 正規化ありは正規化後の文字列における位置、
			//  単語索引は何番目の単語か、が索引に格納される。
		}
	}

	// FileIDに設定
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileOption::UnnormalizedCharLength::Key), result);
}

//
//	FUNCTION private
//	FullText::FileID::getInvertedFileID -- 大転置のFileIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getInvertedFileID() const
{
	Common::ObjectPointer<LogicalFile::FileID> pFileID
		= getFileID(_SYDNEY_FILE_PARAMETER_KEY(
			FileOption::InvertedFileID::Key));
	return *pFileID;
}

//
//	FUNCTION private
//	FullText::FileID::getDelayProcFileID -- 小転置のファイルIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	int iIndex_
//		取得する小転置の配列上の位置
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID&
FileID::getDelayProcFileID(int iIndex_)
{
	Common::ObjectPointer<LogicalFile::FileID> pFileID
		= getInvertedFileID().getFileID(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
			FileOption::DelayProcFileID::Key, iIndex_));
	return *pFileID;
}

//
//	FUNCTION public
//	FullText::FileID::getSectionInfoFileID -- セクション情報のFileIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	LogicalFile::FileID&
//		ファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID &
FileID::getSectionInfoFileID()
{
	// 【注意】
	//	古いバージョンの場合にのみ利用される
	
	Common::ObjectPointer<LogicalFile::FileID> pFileID
		= getFileID(_SYDNEY_FILE_PARAMETER_KEY(
			FileOption::SectionInfoFileID::Key));
	return *pFileID;
}

//
//	FUNCTION public
//	FullText::FileID::setCurrentValue -- 子FileIDに親の現在値を設定する
//
//	NOTES
//	スキーマ等が設定してくれるFileID中の値を、子のFileIDにも反映する。
//
//	ARGUMENTS
//	LogicalFile::FileID& cFileID_
//		設定するファイルID
//
//	RETURN
//	LogicalFile::FileID&
//		値が設定された引数のファイルID
//
//	EXCEPTIONS
//
LogicalFile::FileID &
FileID::setCurrentValue(LogicalFile::FileID& cFileID_) const
{
	// マウントされているかどうか
	cFileID_.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::Mounted::Key), isMounted());
	// ReadOnlyかどうか
	cFileID_.setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::ReadOnly::Key), isReadOnly());
	// データベースID
	cFileID_.setInteger(_SYDNEY_FILE_PARAMETER_KEY(
		FileCommon::FileOption::DatabaseID::Key),
						getInteger(_SYDNEY_FILE_PARAMETER_KEY(
							FileCommon::FileOption::DatabaseID::Key)));

	return cFileID_;
}

//
//	FUNCTION private
//	FullText::FileID::check -- 設定をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		正しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::check()
{
	// フィールド型をチェック
	
	// [NOTE] 以下の処理も実行
	//  * 非固定フィールドの有無(スキーマ情報のチェック)
	//  * ヒント情報のチェック
	//  * その他情報ファイルのフィールド型と数

	// フィールド番号
	int n = 0;
	// その他情報ファイルのフィールド数
	int k = 0;
	// 先頭フィールドは配列？
	bool field0;
	
	//////////////////////////////////////
	// 以下は非固定部分のfieldの型検査
	//////////////////////////////////////

	//
	// 先頭フィールドは常に文字列
	//
	; _TRMEISTER_ASSERT(checkFieldType(n, DataType::String) ||
						checkFieldType(n, DataType::StringArray));
	
	//
	// 次のフィールドは言語情報かもしれない。
	//
	
	if (field0 = checkFieldType(n++, DataType::StringArray))
	{
		// 配列
		if (checkFieldType(n, DataType::Language))
			// 言語指定も配列じゃないとだめ
			return false;
		if (checkFieldType(n, DataType::LanguageArray))
		{
			// 言語指定あり
			setBoolean(
				_SYDNEY_FILE_PARAMETER_KEY(FileOption::Language::Key),
				true);
			n++;
		}
	}
	else
	{
		// 配列ではない
		if (checkFieldType(n, DataType::LanguageArray))
			// 言語指定が配列じゃだめ
			return false;
		if (checkFieldType(n, DataType::Language))
		{
			// 言語指定あり
			setBoolean(
				_SYDNEY_FILE_PARAMETER_KEY(FileOption::Language::Key),
				true);
			n++;
		}
	}

	//
	// 次のフィールドはスコア調整かもしれない。
	//

	if (checkFieldType(n, DataType::Double))
	{
		// スコア調整用カラムあり
		setBoolean(
			_SYDNEY_FILE_PARAMETER_KEY(FileOption::ScoreModifier::Key),
			true);
		n++;

		// その他情報ファイルのフィールドの型を設定する
		setInteger(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::OtherFieldType::Key, k++),
			DataType::Double);
	}

	//////////////////////////////////////
	// 以下は固定部分のfieldの型検査
	//////////////////////////////////////

	//
	// まず、個別の確認とその他情報ファイルの設定
	//

	if (isSectionized())
	{
		// セクションの場合は、配列でなければならない
		if(field0 == false)
			return false;
		
		// その他情報ファイルのフィールドの型を設定する
		setInteger(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::OtherFieldType::Key, k++),
			DataType::UnsignedIntegerArray);
	}

	if (isClustering())
	{
		// その他情報ファイルのフィールドの型を設定する
		setInteger(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::OtherFieldType::Key, k++),
			DataType::Binary);
	}

	if (isRoughKwic())
	{
		setInteger(
			_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
				FileOption::OtherFieldType::Key, k++),
			DataType::UnsignedInteger);
	}

	// その他情報ファイルのフィールド数を設定する
	setInteger(
		_SYDNEY_FILE_PARAMETER_KEY(FileOption::OtherFieldNum::Key),
		k);

	//
	// 次に、全体の確認。
	//

	// 固定フィールドの定義
	static DataType::Value FieldTypeValue[] =
	{
		DataType::UnsignedInteger,			/*Rowid*/
		DataType::Double,					/*Score*/
		DataType::UnsignedIntegerArray,		/*Section*/
		DataType::Word,						/*Word*/
		DataType::Integer,					/*WordDf*/
		DataType::Double,					/*WordScale*/
		DataType::Double,					/*AverageLength*/
		DataType::Double,					/*AverageCharLength*/
		DataType::Double,					/*AverageWordCount*/
		DataType::UnsignedIntegerArray,		/*Tf*/
		DataType::UnsignedInteger,			/*Count*/
		DataType::Integer,					/*ClusterId*/
		DataType::WordArray,				/*FeatureValue*/
		DataType::Integer					/*RoughKwicPosition*/
	};
	
	for(int i = 0 ; i < sizeof(FieldTypeValue)/sizeof(FieldTypeValue[0]); i++)
	{
		// 各フィールドのデータ型を確認する。
		if (checkFieldType(n++, FieldTypeValue[i]) == false)
			return false;
	}

	return true;
}
//
//	FUNCTION private
//	FullText::FileID::checkFieldType -- フィールドの型をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		フィールド番号
//	FullText::FileID::DataType::Value
//		データ型
//
//	RETURN
//	bool
//		引数と同じ型の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//

bool
FileID::checkFieldType(int n_, DataType::Value eType_)
{
	//
	// FullText::FileID::DataType と Common::DataType との対応表
	//
	struct typeTable
	{
		DataType::Value dv;		// data value
		int ct;					// common type
		bool array;				// simple or array
	};
	static	typeTable tbl[]	=
	{
		{DataType::Integer, Common::DataType::Integer,false},
		{DataType::UnsignedInteger,Common::DataType::UnsignedInteger,false},
		{DataType::String,Common::DataType::String,false},
		{DataType::Language,Common::DataType::Language,false},
		{DataType::Double,Common::DataType::Double,false},
		{DataType::Word,Common::DataType::Word,false},
		{DataType::UnsignedIntegerArray,Common::DataType::UnsignedInteger,true},
		{DataType::StringArray,Common::DataType::String,true},
		{DataType::LanguageArray,Common::DataType::Language,true},
		{DataType::WordArray,Common::DataType::Word,true}
	};

	//
	// チェックする
	//
	for(int i = 0; i < sizeof(tbl)/sizeof(*tbl) ; i++)
	{
		if(tbl[i].dv == eType_)
		{
			// eType_に対応するCommon::DataTypeが見つかった。
			
			//
			// 呼び出し側で設定されたCommon::DataTypeと一致するか？
			//
			if(tbl[i].array)
			{		// check type
				if (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldType::Key, n_))
					== Common::DataType::Array)
				{	// check element type
					return (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						FileCommon::FileOption::ElementType::Key, n_))
						== tbl[i].ct);
				}
			}
			else
			{		// check type
				return (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldType::Key, n_))
					== tbl[i].ct);
			}
		}
	}
	// eType_に対応するデータがなかった
	return false;
}

//
//	FUNCTION private
//	FullText::FileID::checkProjection
//		-- 与えられたフィールドが取得できるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		調べるフィールド番号
//	unsigned int& bit_
//		既にチェック済みのフィールド番号
//	bool isBitset_
//		ビットセットで取得するかどうか
//	bool isScan_
//		スキャンモードかどうか
//
//	RETURN
//	bool
//		取得できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::checkProjection(int n_,FieldMask &mask ) const
{
	if(mask.checkGroupExclusiveness(n_) == false)
		return false;

	return mask.checkValueRangeValidity(n_);
}

//
//	FUNCTION private
//	FullText::FileID::initalizeOtherFileField
//		-- その他情報ファイルの情報を初期化する
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
FileID::initializeOtherFileField() const
{
	if (m_bOtherFileFieldInitialized == false)
	{
		m_vecVectorFieldSize.clear();
		m_uiVectorFieldTotalSize = 0;
		
		int count = getInteger(
			_SYDNEY_FILE_PARAMETER_KEY(FileOption::OtherFieldNum::Key));
		
		m_vecVectorFieldSize.reserve(count);
		
		for (int i = 0; i < count; ++i)
		{
			ModSize s = 0;
			switch (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
								   FileOption::OtherFieldType::Key, i)))
			{
			case DataType::Double:
				m_uiVectorFieldTotalSize += sizeof(double);
				m_vecVectorFieldSize.pushBack(sizeof(double));
				break;
			case DataType::UnsignedInteger:
				m_uiVectorFieldTotalSize += sizeof(unsigned int);
				m_vecVectorFieldSize.pushBack(sizeof(unsigned int));
				break;
			case DataType::UnsignedIntegerArray:
			case DataType::Binary:
				m_uiVectorFieldTotalSize += 6;
				m_vecVectorFieldSize.pushBack(6);
				m_bVariableFile = true;
				break;
			}
		}
		m_bOtherFileFieldInitialized = true;
	}
}

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
