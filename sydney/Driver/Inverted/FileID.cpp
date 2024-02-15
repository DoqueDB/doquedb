// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/FileID.h"
#include "Inverted/Parameter.h"
#include "Inverted/Hint.h"
#include "Inverted/UnaAnalyzerManager.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/HintArray.h"
#include "Common/Message.h"
#include "Common/UnicodeString.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Version/File.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	索引タイプ文字列
	//
	const ModUnicodeString _cNgram = _TRMEISTER_U_STRING("Ngram");
	const ModUnicodeString _cWord = _TRMEISTER_U_STRING("Word");
	const ModUnicodeString _cDual = _TRMEISTER_U_STRING("Dual");

	//
	//	リーフファイルのページサイズ(kbyte)
	//
	ParameterInteger _cLeafPageSize("Inverted_LeafFilePageSize", 16);

	//
	//	オーバーフローファイルのページサイズ(kbyte)
	//
	ParameterInteger _cOverflowPageSize("Inverted_OverflowFilePageSize", 16);

	//
	//	B木ファイルのページサイズ(kbyte)
	//
	ParameterInteger _cBtreePageSize("Inverted_BtreeFilePageSize", 16);

	//
	//	その他のファイルのページサイズ(kbyte)
	//
	ParameterInteger _cOtherPageSize("Inverted_OtherFilePageSize", 4);

	//
	//	トークナイズパラメータ
	//
	ParameterString _cTokenizeParameter("Inverted_TokenizerParameter", "NGR:1:1 @NORMRSCID:1");

	//
	//	索引タイプ
	//
	ParameterString _cIndexingType("Inverted_FileIndexingType", _cNgram);

	//
	//	異表記正規化
	//
	ParameterBoolean _cNormalized("Inverted_Normalized", false);

	//
	//	圧縮器
	//
	ParameterString _cIdCoder("Inverted_IdCoderParameter", "PEG:2");
	ParameterString _cFrequencyCoder("Inverted_FrequencyCoderParameter", "PEG:3");
	ParameterString _cLengthCoder("Inverted_LengthCoderParameter", "PEG:6");
	ParameterString _cLocationCoder("Inverted_LocationCoderParameter", "PEG:6");
	ParameterString _cWordIdCoder("Inverted_WordIdCoderParameter", "PEG:1");
	ParameterString _cWordFrequencyCoder("Inverted_WordFrequencyCoderParameter", "PEG:3");
	ParameterString _cWordLengthCoder("Inverted_WordLengthCoderParameter", "PEG:1");
	ParameterString _cWordLocationCoder("Inverted_WordLocationCoderParameter", "UNA");
	
	//
	//	抽出パラメータ
	//
	ParameterString _cExtractor("Inverted_Extractor", "@TERMRSCID:0");

	//
	//	転置ファイルのバージョン
	//
	ParameterInteger _cFileVersion("Inverted_FileVersion", 1);

	// UNAのリソース番号
	const ModUnicodeString _cUNA("@UNARSCID:");
	// NORMのリソース番号
	const ModUnicodeString _cNORM("@NORMRSCID:");
}

//
//	FUNCTION public
//	Inverted::FileID::FileID -- コンストラクタ
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
	: LogicalFileID(cLogicalFileID_), m_bLoadLanguage(false)
{
}

//
//	FUNCTION public
//	Inverted::FileID::~FileID -- デストラクタ
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
//	Inverted::FileID::create -- ファイルIDの内容を作成する
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
FileID::create(bool bBigInverted_)
{
	//
	//	ヒントを解釈し値を設定する。ヒントに無いものはシステムパラメータから得る
	//

	ModUnicodeString cstrHint;
	getString(_SYDNEY_FILE_PARAMETER_KEY(
				  FileCommon::FileOption::FileHint::Key),
			  cstrHint);
	FileCommon::HintArray cHintArray(cstrHint);

	//	TokenizerParameter
	setTokenizeParameter(cstrHint, cHintArray);
	//	IndexingType
	setIndexingType(cstrHint, cHintArray);
	// Normalized
	setNormalized(cstrHint, cHintArray);
	// Coder
	setCoderParameter(cstrHint, cHintArray);
	// Extractor
	setExtractor(cstrHint, cHintArray);
	// Language
	setLanguage(cstrHint, cHintArray);
	// Distribute
	setDistribute(cstrHint, cHintArray, bBigInverted_);
	// No location information
	setNolocation(cstrHint, cHintArray);
	// No TF
	setNoTF(cstrHint, cHintArray);

	// ヒントの整合性をチェック
	if (verifyHint() == false)
	{
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	// clustered
	// cstrHintに"clustered=(feature=??)"が入ってくるので、parseする。
	setClustered(cstrHint, cHintArray);

	//
	//	システムパラメータから得る
	//

	ModUInt32 size;
	ModUInt32 defaultSize = FileCommon::FileOption::PageSize::getDefault();

	// LeafFilePageSize
	size = Version::File::verifyPageSize(_cLeafPageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LeafPageSize), size >> 10);
	// OverflowFilePageSize
	size = Version::File::verifyPageSize(_cOverflowPageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::OverflowPageSize), size >> 10);
	// OtherFilePageSize
	size = Version::File::verifyPageSize(_cOtherPageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key), size >> 10);
	// BtreeFilePageSize
	size = Version::File::verifyPageSize(_cBtreePageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::BtreePageSize), size >> 10);

	//
	//	その他
	//

	// マウント
	setMounted(true);
	// バージョン
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
			   _cFileVersion.get());
}

//
//	FUNCTION public
//	Inverted::FileID::getLeafPageSize -- リーフページのページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		リーフページのページサイズ
//
//	EXCEPTIONS
//
int
FileID::getLeafPageSize() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LeafPageSize)) << 10;
}

//
//	FUNCTION public
//	Inverted::FileID::getOverflowPageSize -- オーバーフローページのページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		オーバーフローページのページサイズ
//
//	EXCEPTIONS
//
int
FileID::getOverflowPageSize() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::OverflowPageSize)) << 10;
}

//
//	FUNCTION public
//	Inverted::FileID::getBtreePageSize -- B木ページのページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		B木ページのページサイズ
//
//	EXCEPTIONS
//
int
FileID::getBtreePageSize() const
{
	int size;
	if (getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::BtreePageSize), size) == false)
		return getPageSize();
	return size << 10;
}

//
//	FUNCTION public
//	Inverted::FileID::getPageSize -- その他のページサイズを得る
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
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::PageSize::Key)) << 10;
}

//
//	FUNCTION public
//	Inverted::FileID::getLockName -- ロック名を得る
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
//	Inverted::FileID::isReadOnly -- 読み取り専用か
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::ReadOnly::Key));
}

//
//	FUNCTION public
//	Inverted::FileID::isTemporary -- 一時か
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Temporary::Key));
}

//
//	FUNCTION public
//	Inverted::FileID::isMounted -- マウントされているか
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key));
}

//
//	FUNCTION public
//	Inverted::FileID::setMounted -- マウントされているかを設定する
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
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Mounted::Key), bFlag_);
}

//
//	FUNCTION public
//	Inverted::FileID::isNormalized -- 異表記正規化ありか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		異表記正規化ありの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isNormalized() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Normalized));
}

//
//	FUNCTION public static
//	Inverted::FileID::isNormalized -- 異表記正規化ありか
//
//	NOTES
//	全文ファイルドライバーから呼び出される。
//	これが呼び出される時点では、転置のFileIDが作成されていないため（と思われる）
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		異表記正規化ありの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isNormalized(const ModUnicodeString& cstrHint_)
{
	FileCommon::HintArray cHintArray(cstrHint_);
	
	ModUnicodeString cstrValue;
	bool bNormalized = false;
	if (readHint(cstrHint_, cHintArray,
				 Hint::Normalized::Key, cstrValue) == false)
	{
		// 正規化パラメータが存在しない場合

		// デフォルトの設定を取得
		bNormalized = _cNormalized.get();
	}
	else
	{
		// 正規化パラメータが存在する場合
		
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bNormalized = true;
		}
		else if (cstrValue.compare(_TRMEISTER_U_STRING("false"), ModFalse) == 0)
		{
			bNormalized = false;
		}
		else
		{
			// 子ヒントがある場合
			
			// [NOTE] サポートしていない子ヒントが与えられたとしても、
			//  正規化ありとみなす。setNormalized に合わせる。
			bNormalized = true;
		}
	}

	return bNormalized;
}

//
//	FUNCTION public
//	Inverted::FileID::isStemming -- ステミングありかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ステミングありの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isStemming() const
{
	bool result = isNormalized();
	if (result == true)
	{
		bool b;
		if (getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Stemming), b) == true)
			result = b;
	}
	return result;
}

//
//	FUNCTION public
//	Inverted::FileID::isNolocation -- Isn't location information stored?
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		true : Location information is NOT stored.
//
//	EXCEPTIONS
//
bool
FileID::isNolocation() const
{
	return isNolocation(*this);
}
/* static */
bool
FileID::isNolocation(const LogicalFile::FileID& cFileID_)
{
	return cFileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Nolocation));
}

//
//	FUNCTION public
//	Inverted::FileID::isNoTF -- Isn't TF information stored?
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		true : TF information is NOT stored.
//
//	EXCEPTIONS
//
bool
FileID::isNoTF() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::NoTF));
}

//
//	FUNCTION public
//	Inverted::FileID::getFeatureSize --特徴語数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		特徴語数
//
//	EXCEPTIONS
//
int
FileID::getFeatureSize() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Feature));
}

//
//	FUNCTION public
//	Inverted::FileID::getSpaceMode -- スペース処理モードを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedUnaSpaceMode
//		スペース処理モード
//
//	EXCEPTIONS
//
ModInvertedUnaSpaceMode
FileID::getSpaceMode() const
{
	// リソースに従うモード
	// ModNlpAnalyzerのインスタンスは毎回確保されるのでこれでいい
	// 明示的にModInvertedUnaSpaceResetを指定する必要はない
	ModInvertedUnaSpaceMode mode = ModInvertedUnaSpaceAsIs;

	int value;
	if (getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::SpaceMode), value) == true)
			mode = static_cast<ModInvertedUnaSpaceMode>(value);
	
	return mode;
}

//
//	FUNCTION public
//	Inverted::FileID::isCarriage -- 改行を跨って正規化するかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		改行を跨って正規化する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isCarriage() const
{
	bool result = isNormalized();
	if (result == true)
	{
		bool b;
		if (getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Carriage), b) == true)
			result = b;
	}
	return result;
}

//
//	FUNCTION public
//	Inverted::FileID::getPath -- パス名を得る
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
//	Inverted::FileID::setPath -- パス名を設定する
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
	setString(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Area::Key), cPath_);
	m_cPath = cPath_;
}

//
//	FUNCTION public
//	Inverted::FileID::getIndexingType -- 索引タイプを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedFileIndexingType
//		インデックスタイプ
//
//	EXCEPTIONS
//
ModInvertedFileIndexingType
FileID::getIndexingType() const
{
	return getIndexingType(*this);
}

/* static */
ModInvertedFileIndexingType
FileID::getIndexingType(const LogicalFile::FileID& cFileID_)
{
	return static_cast<ModInvertedFileIndexingType>(
		cFileID_.getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IndexingType)));
}

//
//	FUNCTION public static
//	Inverted::FileID::getIndexingType --
//
//	NOTES
//	全文ファイルドライバーから呼び出される。
//	これが呼び出される時点では、転置のFileIDが作成されていないため。
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModInvertedFileIndexingType
FileID::getIndexingType(const ModUnicodeString& cstrHint_)
{
	FileCommon::HintArray cHintArray(cstrHint_);
	return getIndexingType(cstrHint_, cHintArray);
}

//
//	FUNCTION public
//	Inverted::FileID::getTokenizeParameter -- トークナイズパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModCharString&
//		トークナイズパラメータ
//
//	EXCEPTIONS
//
const ModCharString&
FileID::getTokenizeParameter() const
{
	if (m_cTokenizeParameter.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TokenizeParameter), param);
		m_cTokenizeParameter = param.getString();
	}
	return m_cTokenizeParameter;
}

//
//	FUNCTION public
//	Inverted::FileID::getResourceID -- UNAのリソース番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	ModUInt32
//		UNAのリソース番号。見つからなかった場合は 0 を返す。
//
//	EXCEPTIONS
//
//static
ModUInt32
FileID::getResourceID(const LogicalFile::FileID& cFileID_)
{
	ModUInt32 id = 0;
	
	// トークナイズパラメータを得る
	ModUnicodeString param;
	if (cFileID_.getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TokenizeParameter),
						   param) == true)
	{
		// @UNARSCID: を検索する
		ModUnicodeChar* p = param.search(_cUNA);
		if (p) p += _cUNA.getLength();
		if (p == 0)
		{
			// 見つからなかったので、@NORMRSCID: を検索する
			p = param.search(_cNORM);
			if (p) p += _cNORM.getLength();
		}

		if (p != 0)
		{
			id = ModUnicodeCharTrait::toInt(p);
		}
	}

	return id;
}

//
//	FUNCTION public
//	Inverted::FileID::getIdCoder -- 圧縮器を指定するパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModCharString&
//		圧縮器を指定するパラメータ
//
//	EXCEPTIONS
//
const ModCharString&
FileID::getIdCoder() const
{
	if (m_cIdCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IdCoder), param);
		m_cIdCoder = param.getString();
	}
	return m_cIdCoder;
}
const ModCharString&
FileID::getFrequencyCoder() const
{
	if (m_cFrequencyCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::FrequencyCoder), param);
		m_cFrequencyCoder = param.getString();
	}
	return m_cFrequencyCoder;
}
const ModCharString&
FileID::getLengthCoder() const
{
	if (m_cLengthCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LengthCoder), param);
		m_cLengthCoder = param.getString();
	}
	return m_cLengthCoder;
}
const ModCharString&
FileID::getLocationCoder() const
{
	if (m_cLocationCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LocationCoder), param);
		m_cLocationCoder = param.getString();
	}
	return m_cLocationCoder;
}

const ModCharString&
FileID::getWordIdCoder() const
{
	if (m_cWordIdCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordIdCoder), param);
		m_cWordIdCoder = param.getString();
	}
	return m_cWordIdCoder;
}
const ModCharString&
FileID::getWordFrequencyCoder() const
{
	if (m_cWordFrequencyCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordFrequencyCoder), param);
		m_cWordFrequencyCoder = param.getString();
	}
	return m_cWordFrequencyCoder;
}
const ModCharString&
FileID::getWordLengthCoder() const
{
	if (m_cWordLengthCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLengthCoder), param);
		m_cWordLengthCoder = param.getString();
	}
	return m_cWordLengthCoder;
}
const ModCharString&
FileID::getWordLocationCoder() const
{
	if (m_cWordLocationCoder.getLength() == 0)
	{
		ModUnicodeString param;
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLocationCoder), param);
		m_cWordLocationCoder = param.getString();
	}
	return m_cWordLocationCoder;
}

//
//	FUNCTION public
//	Inverted::FileID::getExtractor
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		拡張パラメータ
//
//	EXCEPTIONS
//
const ModUnicodeString&
FileID::getExtractor() const
{
	if (m_cExtractor.getLength() == 0)
	{
		if (getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Extractor),
					  m_cExtractor)	== false)
		{
			// パラメータから得る
			m_cExtractor = _cExtractor.get();
		}
	}
	return m_cExtractor;
}

//
//	FUNCTION public
//	Inverted::FileID::getDefaultLanguageSet -- デフォルト言語を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModLanguageSet&
//		言語情報
//
//	EXCEPTIONS
//
const ModLanguageSet&
FileID::getDefaultLanguageSet() const
{
	if (m_bLoadLanguage == false)
	{
		if (getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Language),
					  m_cLanguageSetName)
			== false)
		{
			// パラメータから得る
			m_cLanguageSetName
				= UnaAnalyzerManager::getDefaultLanguageSetName();
		}
		m_cLanguageSet = m_cLanguageSetName;
	}
	return m_cLanguageSet;
}

//
//	FUNCTION public
//	Inverted::FileID::getDefaultLanguageSetName -- デフォルト言語を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		言語情報の文字列表現
//
//	EXCEPTIONS
//
const ModUnicodeString&
FileID::getDefaultLanguageSetName() const
{
	getDefaultLanguageSet();
	return m_cLanguageSetName;
}

/* static */
ModUnicodeString
FileID::getDefaultLanguageSetName(const LogicalFile::FileID& cFileID_)
{
	ModUnicodeString v;
	if (cFileID_.getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Language),
						   v) == false)
		return UnaAnalyzerManager::getDefaultLanguageSetName();
	return v;
}

//
//	FUNCTION public
//	Inverted::FileID::isDistribution -- ファイル分散を利用するかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ファイル分散を利用する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isDistribution() const
{
	int dcount = 0;
	getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Distribute), dcount);
	return (dcount != 0);
}

//
//	FUNCTION public
//	Inverted::FileID::getDistribute -- ファイル分散数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ファイル分散数
//
//	EXCEPTIONS
//
int
FileID::getDistribute() const
{
	int dcount = 1;
	getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Distribute), dcount);
	return dcount;
}

//
//	FUNCTION public
//	Inverted::FileID::isClustering -- クラスタリングモードか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		クラスタリングモードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isClustering() const
{
	return isClustering(*this);
}

//
//	FUNCTION public
//	Inverted::FileID::isClustering -- クラスタリングモードか否か
//
//	NOTES
//	LogicalFile::FileIDから直接参照する用
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		クラスタリングモードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
/* static */
bool
FileID::isClustering(const LogicalFile::FileID& cFileID_)
{
	bool v;
	if (cFileID_.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Clustered),
							v) == false)
		v = false;
	return v;
}

//
//	FUNCTION public static
//	Inverted::FileID::isClustered -- クラスタリングモードが否か
//
//	NOTES
//	FullTextから転置のヒントを参照するための関数
//
//	ARGUMETNS
//	const ModUnicodeString& cstrHint_
//		転置ファイルのヒント文字列
//
//	RETURN
//	bool
//		クラスタリングモードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isClustered(const ModUnicodeString& cstrHint_)
{
	// 全文ファイルドライバーから呼び出される

	FileCommon::HintArray cHintArray(cstrHint_);
	
	ModUnicodeString cstrValue;
	bool bClustering = false;
	if (readHint(cstrHint_, cHintArray,
				 Hint::Clustered::Key, cstrValue) == true)
		bClustering = true;

	return bClustering;
}

//
//	FUNCTION private
//	Inverted::FileID::setTokenizeParameter
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setTokenizeParameter(const ModUnicodeString& cstrHint_,
							 const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;

	if (readHint(cstrHint_, cHintArray_, Hint::TokenizerParameter::Key,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		cstrValue = _cTokenizeParameter.get();
	}

	if (cstrValue.getLength() == 0)
	{
		SydErrorMessage << "TokenizerParameter is not set." << ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// FileIDに設定する
	setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TokenizeParameter), cstrValue);
}

//
//	FUNCTION private
//	Inverted::FileID::setIndexingType
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setIndexingType(const ModUnicodeString& cstrHint_,
						const FileCommon::HintArray& cHintArray_)
{
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IndexingType),
			   getIndexingType(cstrHint_, cHintArray_));
}

//
//	FUNCTION private static
//	Inverted::FileID::getIndexingType --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModInvertedFileIndexingType
FileID::getIndexingType(const ModUnicodeString& cstrHint_,
						const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	if (readHint(cstrHint_, cHintArray_, Hint::FileIndexingType::Key,
				 cstrValue) == false)
	{
		// Get a default value.
		cstrValue = _cIndexingType.get();
	}
	
	ModInvertedFileIndexingType eType = ModInvertedFileUndefinedIndexingType;
	
	if (cstrValue.compare(_cNgram, ModFalse) == 0)
	{
		eType = ModInvertedFileNgramIndexing;
	}
	else if (cstrValue.compare(_cWord, ModFalse) == 0)
	{
		eType = ModInvertedFileWordIndexing;
	}
	else if (cstrValue.compare(_cDual, ModFalse) == 0)
	{
		eType = ModInvertedFileDualIndexing;
	}
	else
	{
		SydErrorMessage << "Illegal Indexing Type. " << cstrValue << ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return eType;
}

//
//	FUNCTION private
//	Inverted::FileID::setNormalized
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setNormalized(const ModUnicodeString& cstrHint_,
					  const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	bool bNormalized;

	if (readHint(cstrHint_, cHintArray_, Hint::Normalized::Key,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		bNormalized = _cNormalized.get();
	}
	else
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bNormalized = true;
		}
		else if (cstrValue.compare(_TRMEISTER_U_STRING("false"), ModFalse) == 0)
		{
			bNormalized = false;
		}
		else
		{
			// 子ヒントがある場合

			// [NOTE] サポートしていない子ヒントが与えられたとしても、
			//  正規化ありとみなす。
			bNormalized = true;
			
			// 子ヒントを調べる
			FileCommon::HintArray cHintArray(cstrValue);
			ModUnicodeString cstrValue0;

			// ステミング
			if (readHint(cstrValue, cHintArray, Hint::Stemming::Key, cstrValue0)
				== true)
			{
				bool bStemming;
				
				if (cstrValue0.compare(
						_TRMEISTER_U_STRING("true"), ModFalse) == 0
					|| cstrValue0.getLength() == 0)
				{
					bStemming = true;
				}
				else if (cstrValue0.compare(
					_TRMEISTER_U_STRING("false"), ModFalse) == 0)
				{
					bStemming = false;
				}
				else
				{
					// エラー
					SydErrorMessage
						<< "Illegal Stemming. " << cstrValue0 << ModEndl;
					_SYDNEY_THROW0(Exception::BadArgument);
				}

				// FileIDに設定する
				setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Stemming),
						   bStemming);
			}

			// スペース除去
			if (readHint(cstrValue, cHintArray,
						 Hint::DeleteSpace::Key, cstrValue0) == true)
			{
				ModInvertedUnaSpaceMode mode;
				
				if (cstrValue0.compare(
						_TRMEISTER_U_STRING("true"), ModFalse) == 0
					|| cstrValue0.getLength() == 0)
				{
					mode = ModInvertedUnaSpaceDelete;
				}
				else if (cstrValue0.compare(
					_TRMEISTER_U_STRING("false"), ModFalse) == 0)
				{
					//	【注意】
					//	以前はModInvertedUnaSpaceNormalizeを指定していたが、
					//	これはスペースを除去しないだけではなく、
					//	正規化もしないのでModInvertedUnaSpaceAsIsに変更。
					//	スペースを除去するかどうかはUNAのリソースまかせになる。
					//	でも、このヒントはdeleteSpaceなのにこれでいいのか？
					//	この修正により、Sydneyではスペースを除去するUNAリソース
					//	は使用できないことになる。
					
					mode = ModInvertedUnaSpaceAsIs;
				}
				else
				{
					// エラー
					SydErrorMessage
						<< "Illegal DeleteSpace. " << cstrValue0 << ModEndl;
					_SYDNEY_THROW0(Exception::BadArgument);
				}

				// FileIDに設定する
				setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::SpaceMode),
						   mode);
			}

			// 改行を跨って正規化するかどうか
			if (readHint(cstrValue, cHintArray, Hint::Carriage::Key, cstrValue0)
				== true)
			{
				bool bCarriage;
				
				if (cstrValue0.compare(
						_TRMEISTER_U_STRING("true"), ModFalse) == 0
					|| cstrValue0.getLength() == 0)
				{
					bCarriage = true;
				}
				else if (cstrValue0.compare(
					_TRMEISTER_U_STRING("false"), ModFalse) == 0)
				{
					bCarriage = false;
				}
				else
				{
					// エラー
					SydErrorMessage
						<< "Illegal Carriage. " << cstrValue0 << ModEndl;
					_SYDNEY_THROW0(Exception::BadArgument);
				}

				// FileIDに設定する
				setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Carriage),
						   bCarriage);
			}

		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Normalized), bNormalized);
}

//
//	FUNCTION private
//	Inverted::FileID::setCoderParameter
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setCoderParameter(const ModUnicodeString& cstrHint_,
						  const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;

	if (readHint(cstrHint_, cHintArray_, Hint::CoderParameter::Key,
				 cstrValue) == false)
	{
		// ないのでシステムパラメータ
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IdCoder),
				  _cIdCoder.get());
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::FrequencyCoder),
				  _cFrequencyCoder.get());
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LengthCoder),
				  _cLengthCoder.get());
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LocationCoder),
				  _cLocationCoder.get());
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordIdCoder),
				  _cWordIdCoder.get());
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordFrequencyCoder),
				  _cWordFrequencyCoder.get());
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLengthCoder),
				  _cWordLengthCoder.get());
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLocationCoder),
				  _cWordLocationCoder.get());
	}
	else
	{
		// ヒントにある -> 子ヒントを調べる

		FileCommon::HintArray cHintArray(cstrValue);
		ModUnicodeString cstrValue0;

		// IdCoder
		if (readHint(cstrValue, cHintArray, Hint::IdCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cIdCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IdCoder),
				  cstrValue0);

		// FrequencyCoder
		if (readHint(cstrValue, cHintArray, Hint::FrequencyCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cFrequencyCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::FrequencyCoder),
				  cstrValue0);

		// LengthCoder
		if (readHint(cstrValue, cHintArray, Hint::LengthCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cLengthCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LengthCoder),
				  cstrValue0);

		// LocationCoder
		if (readHint(cstrValue, cHintArray, Hint::LocationCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cLocationCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LocationCoder),
				  cstrValue0);

		// WordIdCoder
		if (readHint(cstrValue, cHintArray, Hint::WordIdCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cWordIdCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordIdCoder),
				  cstrValue0);

		// WordFrequencyCoder
		if (readHint(cstrValue, cHintArray,
					 Hint::WordFrequencyCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cWordFrequencyCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordFrequencyCoder),
				  cstrValue0);

		// WordLengthCoder
		if (readHint(cstrValue, cHintArray, Hint::WordLengthCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cWordLengthCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLengthCoder),
				  cstrValue0);

		// WordLocationCoder
		if (readHint(cstrValue, cHintArray,
					 Hint::WordLocationCoderParameter::Key,
					 cstrValue0) == false)
		{
			cstrValue0 = _cWordLocationCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLocationCoder),
				  cstrValue0);
	}
}

//
//	FUNCTION private
//	Inverted::FileID::setExtractor
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setExtractor(const ModUnicodeString& cstrHint_,
					 const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;

	if (readHint(cstrHint_, cHintArray_, Hint::Extractor::Key,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		cstrValue = _cExtractor.get();
	}

	if (cstrValue.getLength() == 0)
	{
		SydErrorMessage << "Extractor is not set." << ModEndl;
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// FileIDに設定する
	setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Extractor), cstrValue);
}

//
//	FUNCTION private
//	Inverted::FileID::setLanguage
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setLanguage(const ModUnicodeString& cstrHint_,
					const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;

	if (readHint(cstrHint_, cHintArray_, Hint::Language::Key,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		cstrValue = UnaAnalyzerManager::getDefaultLanguageSetName();
	}

	// FileIDに設定する
	setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Language), cstrValue);
}

//
//	FUNCTION private
//	Inverted::FileID::setDistribute
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrHint_
//		ヒント文字列
//	const FileCommon::HintArray& cHintArray_
//		ヒント配列
//	bool bBigInverted_
//		大転置かどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setDistribute(const ModUnicodeString& cstrHint_,
					  const FileCommon::HintArray& cHintArray_,
					  bool bBigInverted_)
{
	int dcount = 0;

	if (bBigInverted_)
	{
		ModUnicodeString cstrValue;

		if (readHint(cstrHint_, cHintArray_, Hint::Distribute::Key, cstrValue)
			== true)
		{
			dcount = ModUnicodeCharTrait::toInt(cstrValue);
			if (dcount < 0 || dcount > 100)
			{
				// 上限は100
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		}
	}

	// FileIDに設定する
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Distribute), dcount);
}

//
//	FUNCTION private
//	Inverted::FileID::setNoloacation -- 
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setNolocation(const ModUnicodeString& cstrHint_,
					  const FileCommon::HintArray& cHintArray_)
{
	// Default value is false; Location information is stored.
	bool bNolocation = false;

	ModUnicodeString cstrValue;

	// Nolocation
	if (readHint(cstrHint_, cHintArray_, Hint::Nolocation::Key, cstrValue)
		== true)
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			// The hint is 'nolocation=true' or 'nolocaiton'.
			bNolocation = true;
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Nolocation), bNolocation);
}

//
//	FUNCTION private
//	Inverted::FileID::setNoTF -- 
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
//	なし
//
//	EXCEPTIONS
//
void
FileID::setNoTF(const ModUnicodeString& cstrHint_,
				const FileCommon::HintArray& cHintArray_)
{
	// Default value is false; TF is stored.
	bool bNoTF = false;

	ModUnicodeString cstrValue;

	// NoTF
	if (readHint(cstrHint_, cHintArray_, Hint::NoTF::Key, cstrValue)
		== true)
	{
		if (cstrValue.compare(_TRMEISTER_U_STRING("true"), ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			// The hint is 'notf=true' or 'notf'.
			bNoTF = true;
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::NoTF), bNoTF);
}

//
//	FUNCTION private
//	Inverted::FileID::setClustered
//	NOTES
//	検索結果のクラスタリングを支持するためのパラメータの解析
//	構文は、
//		cluster=(feature=特徴数)
//  (example)
//　　　create fulltext index I1_1 on T1(C1) hint 'inverted=(normalized=true),clustered=(feature=10)'
//	ARGUMENTS
//	const ModUnicodeString& cstrHint_
//		ヒント文字列
//	const FileCommon::HintArray& cHintArray_
//		ヒント配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FileID::setClustered(const ModUnicodeString& cstrHint_,
					 const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	bool bClustered = false;;
	if (readHint(cstrHint_, cHintArray_, Hint::Clustered::Key,
				 cstrValue) == true)
	{
		// 子ヒント(feature数)を調べる
		FileCommon::HintArray cHintArray(cstrValue);
		ModUnicodeString cstrValue0;
		bClustered = true;
		if (readHint(cstrValue, cHintArray, Hint::Feature::Key, cstrValue0)
			== true)
		{
			// cstrValue0にfeature数が入る
			if (cstrValue0.getLength() > 0)
			{
				// feature数を設定する
				// cstrValue0にfeature数が入るので、
				// cstrValue0をintegerに変換する
				
				int feature = ModUnicodeCharTrait::toInt(cstrValue0);
				if(feature <= 0)
				{
					// feature数は0より大きい必要がある
					// 例外発生
					// エラー
					SydErrorMessage
						<< "Illegal Feature Numbers " << cstrValue0 << ModEndl;
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Feature),
						   feature);		
			}
			else
			{
				// エラー
				SydErrorMessage
				<< "Feature Numbers Missing" << cstrValue0 << ModEndl;
				_SYDNEY_THROW0(Exception::BadArgument);
			}
		}
		else
		{
			// featureの指定がないので、default feature数(10)を使用して、
			// clusteringをする
			setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Feature),10);
		}
	}
	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Clustered), bClustered);
}

//
//	FUNCTION private
//	Inverted::FileID::readHint -- ヒントを読む
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeString& cstrHint_
//		ヒント文字列
//	FileCommon::HintArray& cHintArray_
//		ヒント配列
//	const char* const pszKey_
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

/* static */
bool
FileID::readHint(const ModUnicodeString& cstrHint_,
				 const FileCommon::HintArray& cHintArray_,
				 const char* const pszKey_, 
				 ModUnicodeString& cstrValue_)
{
	ModUnicodeString cstrKey(pszKey_);

	FileCommon::HintArray::ConstIterator i = cHintArray_.begin();
	for (; i != cHintArray_.end(); ++i)
	{
		if ((*i)->CompareToKey(cstrHint_, cstrKey, cstrKey.getLength()) == true)
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
//	Inverted::FileID::verifyHint -- ヒントの整合性をチェックする
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//		ヒントをサポートしていればtrue
//
//	EXCEPTIONS
//
bool
FileID::verifyHint() const
{
	if ((isNolocation() == true &&
		 getIndexingType() == ModInvertedFileDualIndexing)
		||
		(isNolocation() == false && isNoTF() == true))
	{
		// 位置情報リストを格納しない時は、N-gramかWord索引だけ。
		// 位置情報リストがなければ、DUAL索引は意味がないため。
		// ----
		// Nolocation & N-gram	Supported
		// Nolocation & Word	Supported
		// Nolocation & DUAL	Not Supported
		
		// 位置情報リストを格納する時は、頻度も格納する。
		// 位置情報リストだけ必要で頻度が不要なパターンは少ないと思われるため。
		// ----
		// Nolocation & NoTF	Supported
		// Nolocation			Supported
		// NoTF					Not Supported

		return false;
	}
	return true;
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
