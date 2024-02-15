// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/FileID.h"

#include "FullText2/FieldMask.h"
#include "FullText2/KeyID.h"
#include "FullText2/OpenOption.h"
#include "FullText2/Parameter.h"
#include "FullText2/Tokenizer.h"
#include "FileCommon/DataManager.h"
#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"
#include "FileCommon/HintArray.h"
#include "FileCommon/NodeWrapper.h"
#include "Common/Message.h"
#include "Common/UnicodeString.h"
#include "Exception/NotSupported.h"
#include "Exception/SQLSyntaxError.h"
#include "Version/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	Boolean文字列
	//
	ModUnicodeString _cTrue("true");
	ModUnicodeString _cFalse("false");
	
	//
	//	索引タイプ文字列
	//
	ModUnicodeString _cNgram("Ngram");
	ModUnicodeString _cWord("Word");
	ModUnicodeString _cDual("Dual");

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
	//	バッチ時の最大サイズ
	//
	ParameterInteger _cMaximumBatchSize("Inverted_BatchSizeMax", 60 << 20);

	//
	//	索引タイプ
	//
	ParameterString _cIndexingType("Inverted_FileIndexingType", _cNgram);

	//
	//	トークナイズパラメータ
	//
	ParameterString _cTokenizeParameter("Inverted_TokenizerParameter",
										"NGR:1:1 @UNARSCID:1");
	ParameterString _cDualTokenizeParameter("Inverted_DualTokenizerParameter",
											"DUAL:JAP:ALL:1 @UNARSCID:1");
	ParameterString _cWordTokenizeParameter("Inverted_WordTokenizerParameter",
											"DUAL:JAP @UNARSCID:1");

	//
	//	異表記正規化
	//
	ParameterBoolean _cNormalized("Inverted_Normalized", false);

	//
	//	言語
	//
	ParameterString _cLanguage("Inverted_DefaultLanguageSet",
							   ModUnicodeString());

	//
	//	圧縮器
	//
	ParameterString _cIdCoder("Inverted_IdCoderParameter",
							  "PEG:2");
	ParameterString _cFrequencyCoder("Inverted_FrequencyCoderParameter",
									 "PEG:3");
	ParameterString _cLengthCoder("Inverted_LengthCoderParameter",
								  "PEG:6");
	ParameterString _cLocationCoder("Inverted_LocationCoderParameter",
									"PEG:6");
	ParameterString _cWordIdCoder("Inverted_WordIdCoderParameter",
								  "PEG:1");
	ParameterString _cWordFrequencyCoder("Inverted_WordFrequencyCoderParameter",
										 "PEG:3");
	ParameterString _cWordLengthCoder("Inverted_WordLengthCoderParameter",
									  "PEG:1");
	ParameterString _cWordLocationCoder("Inverted_WordLocationCoderParameter",
										"UNA");
	
	//
	//	抽出パラメータ
	//
	ParameterString _cExtractor("Inverted_Extractor", "@TERMRSCID:0");

	//
	//	最大単語長
	//
	ParameterInteger _cMaxWordLength("Inverted_MaxWordLength", 32);

	// UNAのリソース番号
	ModUnicodeString _cUNA("@UNARSCID:");
	// NORMのリソース番号
	ModUnicodeString _cNORM("@NORMRSCID:");

	namespace _Hint
	{
		// 遅延更新かどうか
		ModUnicodeString _cDelayed("delayed");
		// セクションかどうか
		ModUnicodeString _cSectionized("sectionized");
		// ヒット位置表示を行うかどうか
		ModUnicodeString _cRoughKwic("kwic");
		// 削除フラグを利用するかどうか
		ModUnicodeString _cExpungeFlag("deleteflag");
		// 転置パラメータ
		ModUnicodeString _cInverted("inverted");

		namespace _Delayed
		{
			// 同期 
			ModUnicodeString _cSync("sync");
			// 非同期
			ModUnicodeString _cAsync("async");
			// バキュームあり
			ModUnicodeString _cVacuum("vacuum");
		}
		
		namespace _Inverted
		{
			// トークナイザパラメータ
			ModUnicodeString _cTokenizer("tokenizer");
			// コーダーパラメータ
			ModUnicodeString _cCoder("coder");

			namespace _Coder
			{
				// 文書ID符号器パラメータ
				ModUnicodeString _cId("id");
				// 頻度符号器パラメータ
				ModUnicodeString _cFrequency("frequency");
				// 圧縮長符号器パラメータ
				ModUnicodeString _cLength("length");
				// 位置情報符号器パラメータ
				ModUnicodeString _cLocation("location");
				// 文書ID符号器パラメータ(単語単位検索用)
				ModUnicodeString _cWordId("wordid");
				// 頻度符号器パラメータ(単語単位検索用)
				ModUnicodeString _cWordFrequency("wordfrequency");
				// 圧縮長符号器パラメータ(単語単位検索用)
				ModUnicodeString _cWordLength("wordlength");
				// 位置情報符号器パラメータ(単語単位検索用)
				ModUnicodeString _cWordLocation("wordlocation");
			}
			
			// 索引付けタイプ
			ModUnicodeString _cIndexing("indexing");
			// 異表記正規化を行うかどうか
			ModUnicodeString _cNormalized("normalized");

			namespace _Normalized
			{
				// ステミングを行うかどうか
				ModUnicodeString _cStemming("stemming");
				// スペース除去を行うかどうか
				ModUnicodeString _cDeleteSpace("deletespace");
				// 改行を跨った解析を行うかどうか
				ModUnicodeString _cCarriage("carriage");
			}
			
			// 抽出パラメータ
			ModUnicodeString _cExtractor("extractor");
			// デフォルト言語
			ModUnicodeString _cLanguage("language");
			// ファイル分散数
			ModUnicodeString _cDistribute("distribute");
			// 位置情報を格納しないかどうか
			ModUnicodeString _cNolocation("nolocation");
			// TF値を格納しないかどうか
			ModUnicodeString _cNoTF("notf");
			// クラスタ用の特徴語を抽出するかどうか
			ModUnicodeString _cClustered("clustered");

			namespace _Clustered
			{
				// 抽出する特徴語の数
				ModUnicodeString _cFeature("feature");
			}

			// 最大単語長
			ModUnicodeString _cMaxWordLength("maxwordlength");
		}
	}

}

//
//	FUNCTION public
//	FullText2::FileID::FileID -- コンストラクタ
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
FileID::FileID()
{
}

//
//	FUNCTION public
//	FullText2::FileID::FileID -- コンストラクタ
//
//	NOTES
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
//	FullText2::FileID::~FileID -- デストラクタ
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
//	FUNCTION public static
//	FullText2::FileID::checkVersion -- バージョンを確認する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::FileID& cLogicalFileID_
//		論理ファイルID
//
//	RETURN
//	bool
//		FullText2のFileIDだったらtrue、それ以外のものだったらfalse
//
//	EXCEPTIONS
//
bool
FileID::checkVersion(const LogicalFile::FileID& cLogicalFileID_)
{
	int iVersion;
	if (cLogicalFileID_.getInteger(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
		iVersion) == false)
		return true;
	return (iVersion >= VersionNum::Version4) ? true : false;
}

//
//	FUNCTION public
//	FullText2::FileID::getVersion -- バージョンを得る
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
FileID::getVersion() const 
{
	int iVersion;
	if (getInteger(
			_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
			iVersion) == false)
	{
		iVersion = VersionNum::Version1;
	}
	return iVersion;
}

//
//	FUNCTION public
//	FullText2::FileID::create -- ファイルIDの内容を作成する
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
	//	ヒントを解釈し値を設定する。ヒントに無いものはシステムパラメータから得る
	//

	ModUnicodeString cstrHint;
	getString(_SYDNEY_FILE_PARAMETER_KEY(
				  FileCommon::FileOption::FileHint::Key),
			  cstrHint);
	FileCommon::HintArray cHintArray(cstrHint);

	// 遅延更新かどうか
	setDelayed(cstrHint, cHintArray);
	// セクション検索かどうか
	setSectionized(cstrHint, cHintArray);
	// 削除フラグかどうか
	setExpungeFlag(cstrHint, cHintArray);

	// 転置部分を設定する
	setInvertedParameter(cstrHint, cHintArray);

	// 荒いKWIC取得用のデータを格納するかどうか
	setRoughKwic(cstrHint, cHintArray);

	// フィールド指定が正しいか確認する
	if (check() == false)
		_SYDNEY_THROW0(Exception::NotSupported);
	
	//
	//	システムパラメータから得る
	//

	ModUInt32 size;
	ModUInt32 defaultSize = FileCommon::FileOption::PageSize::getDefault();

	// LeafFilePageSize
	size = Version::File::verifyPageSize(_cLeafPageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
				   KeyID::LeafPageSize), size >> 10);
	// OverflowFilePageSize
	size = Version::File::verifyPageSize(_cOverflowPageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
				   KeyID::OverflowPageSize), size >> 10);
	// OtherFilePageSize
	size = Version::File::verifyPageSize(_cOtherPageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
				   FileCommon::FileOption::PageSize::Key), size >> 10);
	// BtreeFilePageSize
	size = Version::File::verifyPageSize(_cBtreePageSize.get() << 10);
	if (size < defaultSize) size = defaultSize;
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(
				   KeyID::BtreePageSize), size >> 10);

	//
	//	その他
	//

	// マウント
	setMounted(true);
	// バージョン
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
			   VersionNum::CurrentVersion);
}

//
//	FUNCTION public
//	FullText2::FileID::getLeafPageSize
//		-- リーフページのページサイズを得る
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
//	FullText2::FileID::getOverflowPageSize
//		-- オーバーフローページのページサイズを得る
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
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(
						  KeyID::OverflowPageSize)) << 10;
}

//
//	FUNCTION public
//	FullText2::FileID::getBtreePageSize -- B木ページのページサイズを得る
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
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(
						  KeyID::BtreePageSize)) << 10;
}

//
//	FUNCTION public
//	FullText2::FileID::getPageSize -- その他のページサイズを得る
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
//	FullText2::FileID::getLockName -- ロック名を得る
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
//	FullText2::FileID::isReadOnly -- 読み取り専用か
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
//	FullText2::FileID::isTemporary -- 一時か
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
//	FullText2::FileID::isMounted -- マウントされているか
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
//	FullText2::FileID::setMounted -- マウントされているかを設定する
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
//	FullText2::FileID::getKeyCount -- キー数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		キー数
//
//	EXCEPTIONS
//
int
FileID::getKeyCount() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::KeyCount));
}

//
//	FUNCTION public
//	FullText2::FileID::isKeyArray -- 指定されたキーが配列かどうか
//
//	NOTES
//
//	ARGUMENTS
//	inst key_
//		キーの要素番号
//
//	RETURN
//	bool
//		配列の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isKeyArray(int key_) const
{
	return checkFieldType(key_, DataType::StringArray);
}

//
//	FUNCTION public
//	FullText2::FileID::isNormalized -- 異表記正規化ありか
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsNormalized));
}

//
//	FUNCTION public
//	FullText2::FileID::isStemming -- ステミングありかどうか
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
		if (getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
						   KeyID::IsStemming), b) == true)
			result = b;
	}
	return result;
}

//
//	FUNCTION public
//	FullText2::FileID::isDeleteSpace -- スペースを除去するかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		スペースを除去する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isDeleteSpace() const
{
	bool result = isNormalized();
	if (result == true)
	{
		bool b;
		if (getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
						   KeyID::IsDeleteSpace), b) == true)
			result = b;
	}
	return result;
}

//
//	FUNCTION public
//	FullText2::FileID::isCarriage -- 改行を跨って正規化するかどうか
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
		if (getBoolean(_SYDNEY_FILE_PARAMETER_KEY(
						   KeyID::IsCarriage), b) == true)
			result = b;
	}
	return result;
}

//
//	FUNCTION public
//	FullText2::FileID::isNolocation -- Isn't location information stored?
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsNolocation));
}

//
//	FUNCTION public
//	FullText2::FileID::isNoTF -- Isn't TF information stored?
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsNoTF));
}

//
//	FUNCTION public
//	FullText2::FileID::isExpungeFlag -- 削除フラグを利用するかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		削除フラグを利用する場合は true、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
FileID::isExpungeFlag() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsExpungeFlag));
}

//
//	FUNCTION public
//	FullText2::FileID::isVacuum -- バキュームを利用するかどうか
//
//	NOTES
//	遅延更新(delayed)の場合のみ設定可能
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		バキュームを利用する場合は true、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
FileID::isVacuum() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsVacuum));
}

//
//	FUNCTION public
//	FullText2::FileID::getPath -- パス名を得る
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
//	FullText2::FileID::setPath -- パス名を設定する
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
//	FUNCTION public
//	FullText2::FileID::getIndexingType -- 索引タイプを得る
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
IndexingType::Value
FileID::getIndexingType() const
{
	return static_cast<IndexingType::Value>(
		getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IndexingType)));
}

//
//	FUNCTION public
//	FullText2::FileID::getTokenizeParameter -- トークナイズパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		トークナイズパラメータ
//
//	EXCEPTIONS
//
const ModUnicodeString&
FileID::getTokenizeParameter() const
{
	if (m_cTokenizeParameter.getLength() == 0)
	{
		getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TokenizeParameter),
				  m_cTokenizeParameter);
	}
	return m_cTokenizeParameter;
}

//
//	FUNCTION public
//	FullText2::FileID::getResourceID -- UNAのリソース番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		UNAのリソース番号。見つからなかった場合はデフォルトの 1 を返す。
//
//	EXCEPTIONS
//
ModUInt32
FileID::getResourceID() const
{
	ModUInt32 id = 1;
	
	// トークナイズパラメータを得る
	const ModUnicodeString& param = getTokenizeParameter();

	// @UNARSCID: を検索する
	const ModUnicodeChar* p = param.search(_cUNA);
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

	return id;
}

//
//	FUNCTION public
//	FullText2::FileID::getIdCoder -- 圧縮器を指定するパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		圧縮器を指定するパラメータ
//
//	EXCEPTIONS
//
ModUnicodeString
FileID::getIdCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IdCoder));
}
ModUnicodeString
FileID::getFrequencyCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::FrequencyCoder));
}
ModUnicodeString
FileID::getLengthCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LengthCoder));
}
ModUnicodeString
FileID::getLocationCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LocationCoder));
}

ModUnicodeString
FileID::getWordIdCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordIdCoder));
}
ModUnicodeString
FileID::getWordFrequencyCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordFrequencyCoder));
}
ModUnicodeString
FileID::getWordLengthCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLengthCoder));
}
ModUnicodeString
FileID::getWordLocationCoder() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLocationCoder));
}

//
//	FUNCTION public
//	FullText2::FileID::getExtractor
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
//	FullText2::FileID::getDefaultLanguage -- デフォルト言語を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		言語情報
//
//	EXCEPTIONS
//
ModUnicodeString
FileID::getDefaultLanguage() const
{
	return getString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Language));
}

//
//	FUNCTION public
//	FullText2::FileID::getMaxWordLength -- 最大単語長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		最大単語長
//
//	EXCEPTIONS
//
int
FileID::getMaxWordLength() const
{
	return getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::MaxWordLength));
}

//
//	FUNCTION public
//	FullText2::FileID::isDistribute -- ファイル分散を利用するかどうか
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
FileID::isDistribute() const
{
	int dcount = 0;
	getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Distribute), dcount);
	return (dcount != 0);
}

//
//	FUNCTION public
//	FullText2::FileID::getDistributeCount -- ファイル分散数を得る
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
FileID::getDistributeCount() const
{
	int dcount = 1;
	getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Distribute), dcount);
	return dcount;
}

//
//	FUNCTION public
//	FullText2::FileID::isClustering -- クラスタリングモードか否か
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsClustered));
}

//
//	FUNCTION public
//	FullText2::FileID::getFeatureSize --特徴語数を得る
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
//	FullText2::FileID::isDelayed -- 遅延更新かどうか
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
	return (getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::DelayedMode))
			!= Delayed::None);
}

//
//	FUNCTION public
//	FullText2::FileID::isSyncMerge -- 同期マージが否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		同期マージの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isSyncMerge() const
{
	return (getInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::DelayedMode))
			== Delayed::Sync);
}

//
//	FUNCTION public
//	FullText2::FileID::isSectionized -- セクション検索か否か
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
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsSectionized));
}

//
//	FUNCTION public
//	FullText2::FileID::isLanguage -- 言語情報フィールドがあるか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		言語情報フィールドがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isLanguage() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsLanguage));
}

//
//	FUNCTION public
//	FullText2::FileID::isScoreField -- スコア調整フィールドがあるか否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		スコア調整フィールドがある場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isScoreField() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsScoreField));
}

//
//	FUNCTION public
//	FullText2::FileID::isRoughKwic -- 粗いKWICを取得するかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		粗いKWICを取得する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::isRoughKwic() const
{
	return getBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsRoughKwic));
}

//
//	FUNCTION public
//	FullText2::FileID::getProjectionParameter
//		-- プロジェクションパラメータを設定する
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		プロジェクション対象
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		プロジェクションできる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::getProjectionParameter(const LogicalFile::TreeNodeInterface* pNode_,
							   LogicalFile::OpenOption& cOpenOption_) const
{
	using namespace LogicalFile;
	
	// プロジェクションの指定はTreeNodeInterfaceで行う
	// 引数の pNode_ は TreeNodeInterface::List であり、
	// 以下のような構造になっている
	//
	//	Field
	//	List -- Operand --> Field など

	//
	//【注意】
	//
	// ここで、検索対象のフィールドとプロジェクションとの関係を確認する
	// 例えば、検索対象のフィールドではないフィールドのスコアは取れないとか...
	//
	// 検索対象のフィールドは、getSearchParameter 実行後に OpenOption に
	// 格納されている
	//
	// OpenOption::getSearchFieldCount	検索対象のフィールド数
	// OpenOption::getSearchFieldNumber	検索対処のフィールド番号(配列)
	//

	ModVector<int> vecSearchField;
	OpenOption cOpenOption(cOpenOption_);
	int n = cOpenOption.getSearchFieldCount();
	for (int i = 0; i < n; ++i)
	{
		// 検索対象のフィールド
		vecSearchField.pushBack(cOpenOption.getSearchFieldNumber(i));
	}
	
	// 取得するフィールドを確認するためのクラスを確保する
	FieldMask mask(*this, cOpenOption_);

	// Listの場合はOperandを取得し、
	// それ以外の場合はpNode_を得るラッパークラス
	FileCommon::ListNodeWrapper node(pNode_);

	// オペランドのサイズ
	int operandSize = node.getSize();

	for (int i = 0; i < operandSize; ++i)
	{
		const TreeNodeInterface* pOperand = node.get(i);

		int iField;
		OpenOption::Function::Value eFunc;
		ModUnicodeString cParam;

		// 同時に取得できるものかどうかを確認する
		if (mask.check(pOperand, vecSearchField,
					   iField, eFunc, cParam) == false)
		{
			return false;
		}

		if (eFunc == OpenOption::Function::WordDf ||
			eFunc == OpenOption::Function::WordScale)
		{
			// ソートしかできない
			return false;
		}

		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i), iField);
		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				OpenOption::KeyID::Function, i), eFunc);
		cOpenOption_.setString(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				OpenOption::KeyID::FunctionArgument, i), cParam);
	}

	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::FieldSelect::Key), true);
	cOpenOption_.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::TargetFieldNumber::Key),
		operandSize);
	
	return true;
}

//
//	FUNCTION public
//	FullText2::FileID::getUpdateParameter
//		-- 更新パラメータを設定する
//
//	NOTES
//
//	ARGUENTS
//	const Common::IntegerArrayData& cUpdateFields_
//		更新対象のフィールドの番号の配列
//	LogicalFile::OpenOpeion& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		更新できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
FileID::getUpdateParameter(const Common::IntegerArrayData& cUpdateFields_,
						   LogicalFile::OpenOption& cOpenOption_) const
{
	// 更新するフィールドを確認するためのクラスを確保する
	FieldMask mask(*this);
	
	for (int i = 0; i < cUpdateFields_.getCount(); ++i)
	{
		int n = cUpdateFields_.getElement(i);

		// 更新対象のフィールドの範囲に収まっているか確認する
		if (mask.checkValueRangeValidity(n) == false)
			return false;

		cOpenOption_.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				FileCommon::OpenOption::TargetFieldIndex::Key, i), n);
	}
	
	cOpenOption_.setBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::FieldSelect::Key), true);
	cOpenOption_.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(
			FileCommon::OpenOption::TargetFieldNumber::Key),
		cUpdateFields_.getCount());

	return true;
}

//
//	FUNCTION public
//	FullText2::FileID::getSortParameter
//		-- ソートパラメータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		ソートキー
//	LogicalFile::OpenOption& cOpenOption_
//		オープンオプション
//
//	RETURN
//	bool
//		ソートできる場合はtrue、それ以外の場合はfalse
//
bool
FileID::getSortParameter(const LogicalFile::TreeNodeInterface* pNode_,
						 LogicalFile::OpenOption& cOpenOption_) const
{
	//	pNode_ は以下のような構造
	//
	//	TreeNodeInterface::OrderBy - operand --> TreeNodeInterface::SortKey
	//
	//	TreeNodeInterface::SortKey - operand --> TreeNodeInterface::Score
	//								 option --> TreeNodeInterface::SortDirection
	//												0: ASC
	//												1: DESC
	//
	//	TreeNodeInterface::Score - operand --> Field 0
	//                                     --> Field 1
	//

	if (pNode_->getType() != LogicalFile::TreeNodeInterface::OrderBy ||
		pNode_->getOperandSize() != 1)
		// ソートキーは１つしか指定できない
		return false;
	
	const LogicalFile::TreeNodeInterface* p = pNode_->getOperandAt(0);
	if (p->getType() != LogicalFile::TreeNodeInterface::SortKey)
		return false;

	// 昇順 or 降順
	int order = 0;	// デフォルトは昇順
	if (p->getOptionSize() != 0 && p->getOptionSize() != 1)
		return false;
	if (p->getOptionSize() == 1)
		order = FileCommon::DataManager::toInt(p->getOptionAt(0));

	if (p->getOperandSize() != 1)
		return false;
	p = p->getOperandAt(0);

	// 検索対象のフィールドは、getSearchParameter 実行後に OpenOption に
	// 格納されている
	//
	// OpenOption::getSearchFieldCount	検索対象のフィールド数
	// OpenOption::getSearchFieldNumber	検索対処のフィールド番号(配列)

	ModVector<int> vecSearchField;
	OpenOption cOpenOption(cOpenOption_);
	int n = cOpenOption.getSearchFieldCount();
	for (int i = 0; i < n; ++i)
	{
		// 検索対象のフィールド
		vecSearchField.pushBack(cOpenOption.getSearchFieldNumber(i));
	}
	
	// 取得するフィールドを確認するためのクラスを確保する
	FieldMask mask(*this, cOpenOption_);

	int iField;
	OpenOption::Function::Value eFunc;
	ModUnicodeString cParam;

	if (mask.check(p, vecSearchField,
				   iField, eFunc, cParam) == false)
		return false;

	switch (eFunc) {
	case OpenOption::Function::Score:
		{
			// スコア
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
										OpenOption::KeyID::SortSpec),
									(order == 0)
									? OpenOption::SortParameter::ScoreAsc
									: OpenOption::SortParameter::ScoreDesc);
		}
		break;
		
	case OpenOption::Function::WordDf:
		{
			// ワードDF
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
										OpenOption::KeyID::SortSpec),
									(order == 0)
									? OpenOption::SortParameter::WordDfAsc
									: OpenOption::SortParameter::WordDfDesc);
		}
		break;
		
	case OpenOption::Function::WordScale:
		{
			// ワードScale
			cOpenOption_.setInteger(_SYDNEY_OPEN_PARAMETER_KEY(
										OpenOption::KeyID::SortSpec),
									(order == 0)
									? OpenOption::SortParameter::WordScaleAsc
									: OpenOption::SortParameter::WordScaleDesc);
		}
		break;
		
	default:
		// その他のフィールドではソートできない
		return false;
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::FileID::setDelayed -- 遅延更新かどうか
//
//	NOTES
//	FileIDに必要な設定を行う
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
FileID::setDelayed(const ModUnicodeString& cstrHint_,
				   const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	int mode = Delayed::None;
	bool vacuum = true;	// vacuumはdefaultでON

	if (readHint(cstrHint_, cHintArray_, _Hint::_cDelayed, cstrValue) == true)
	{
		if (cstrValue.compare(_cTrue, ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			mode = Delayed::Async;
		}
		else if (cstrValue.compare(_cFalse, ModFalse) == 0)
		{
			mode = Delayed::None;
		}
		else if (cstrValue.compare(_Hint::_Delayed::_cSync, ModFalse) == 0)
		{
			mode = Delayed::Sync;
		}
		else if (cstrValue.compare(_Hint::_Delayed::_cAsync, ModFalse) == 0)
		{
			mode = Delayed::Async;
		}
		else
		{
			// 子ヒントがある場合

			// デフォルトは非同期マージ
			mode = Delayed::Async;

			// 子ヒントを調べる
			FileCommon::HintArray cHintArray(cstrValue);
			ModUnicodeString cstrValue0;

			if (readHint(cstrValue, cHintArray,
						 _Hint::_Delayed::_cSync, cstrValue0) == true)
			{
				mode = Delayed::Sync;
			}
			
			if (readHint(cstrValue, cHintArray,
						 _Hint::_Delayed::_cAsync, cstrValue0) == true)
			{
				mode = Delayed::Async;
			}

			if (readHint(cstrValue, cHintArray,
						 _Hint::_Delayed::_cVacuum, cstrValue0) == true)
			{
				if (cstrValue0.compare(_cFalse, ModFalse) == 0)
					// バキュームは vacuum=false の指定がある場合のみOFFとなる
					vacuum = false;
			}
		}
	}

	// FileIDに設定する
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::DelayedMode),
			   mode);
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsVacuum),
			   vacuum);
}

//
//	FUNCTION private
//	FullText2::FileID::setSectionized -- セクション検索かどうか
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
FileID::setSectionized(const ModUnicodeString& cstrHint_,
					   const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	bool bSectionized = false;

	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_cSectionized, cstrValue) == true)
	{
		if (cstrValue.compare(_cTrue, ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bSectionized = true;
		}
		else if (cstrValue.compare(_cFalse, ModFalse) == 0)
		{
			bSectionized = false;
		}
		else
		{
			SydErrorMessage << "Illegal Sectionized. " << cstrValue << ModEndl;
			_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue);
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
				   KeyID::IsSectionized), bSectionized);
}

//
//	FUNCTION private
//	FullText2::FileID::setRoughKwic
//		-- 粗いKWIC取得用のデータを格納するかどうか
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
FileID::setRoughKwic(const ModUnicodeString& cstrHint_,
					 const FileCommon::HintArray& cHintArray_)
{
	// [NOTE] 粗いKWICは、KWICオプションが指定された索引ではなくても
	//  取得することは可能。
	//  ただし、転置索引に格納される索引語の位置情報が、
	//  元文書における索引語の位置と対応しない場合は、
	//  位置情報を補正するために正規化前文字列長を格納しておく。
	
	ModUnicodeString cstrValue;
	bool result = false;

	// KWICオプションを確認
	if (readHint(cstrHint_, cHintArray_, _Hint::_cRoughKwic, cstrValue) == true)
	{
		if (cstrValue.compare(_cTrue, ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			result = true;
		}
		else if (cstrValue.compare(_cFalse, ModFalse) == 0)
		{
			result = false;
		}
		else
		{
			SydErrorMessage << "Illegal right hand value of RoughKwic. "
							<< cstrValue << ModEndl;
			_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue);
		}
	}

	// FileIDに設定
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
				   KeyID::IsRoughKwic), result);
}

//
//	FUNCTION private
//	FullText2::FileID::setInvertedParameter
//		-- 転置部分のヒントを設定する
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
FileID::setInvertedParameter(const ModUnicodeString& cstrHint_,
							 const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrHint;
	readHint(cstrHint_, cHintArray_,
			 _Hint::_cInverted, cstrHint);

	FileCommon::HintArray cHintArray(cstrHint);

	// IndexingType
	setIndexingType(cstrHint, cHintArray);
	// clustered
	// cstrHintに"clustered=(feature=??)"が入ってくるので、parseする。
	setClustered(cstrHint, cHintArray);
	// TokenizerParameter
	setTokenizeParameter(cstrHint, cHintArray);
	// Normalized
	setNormalized(cstrHint, cHintArray);
	// Coder
	setCoderParameter(cstrHint, cHintArray);
	// Extractor
	setExtractor(cstrHint, cHintArray);
	// Language
	setLanguage(cstrHint, cHintArray);
	// Distribute
	setDistribute(cstrHint, cHintArray);
	// No location information
	setNolocation(cstrHint, cHintArray);
	// No TF
	setNoTF(cstrHint, cHintArray);

	// ヒントの整合性をチェック
	if (verifyHint() == false)
	{
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	// MaxWordLength
	setMaxWordLength(cstrHint, cHintArray);
}

//
//	FUNCTION private
//	FullText2::FileID::setIndexingType
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
	ModUnicodeString cstrValue;

	if (readHint(cstrHint_, cHintArray_, _Hint::_Inverted::_cIndexing,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		cstrValue = _cIndexingType.get();
	}

	if (cstrValue.compare(_cNgram, ModFalse) == 0)
	{
		// Ngram
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IndexingType),
				   IndexingType::Ngram);
	}
	else if (cstrValue.compare(_cWord, ModFalse) == 0)
	{
		// Word
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IndexingType),
				   IndexingType::Word);
	}
	else if (cstrValue.compare(_cDual, ModFalse) == 0)
	{
		// Dual
		setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IndexingType),
				   IndexingType::Dual);
	}
	else
	{
		// エラー
		SydErrorMessage << "Illegal Indexing Type. " << cstrValue << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue);
	}
}

//
//	FUNCTION private
//	FullText2::FileID::setTokenizeParameter
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

	if (readHint(cstrHint_, cHintArray_, _Hint::_Inverted::_cTokenizer,
				 cstrValue) == false)
	{
		if (isClustering() == true)
		{
			// クラスタリングが出来るのはDUALのみ
			cstrValue = _cDualTokenizeParameter.get();
		}

		if (cstrValue.getLength() == 0)
		{
			// 索引タイプごとに違うデフォルト値
			switch (getIndexingType())
			{
			case IndexingType::Dual:
				cstrValue = _cDualTokenizeParameter.get();
				break;
			case IndexingType::Word:
				cstrValue = _cWordTokenizeParameter.get();
				break;
			default:
				cstrValue = _cTokenizeParameter.get();
				break;
			}
		}
	}

	ModSize n = cstrValue.getLength();
	if (n == 0)
	{
		SydErrorMessage << "TokenizerParameter is not set." << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// すべてを大文字にする
	for (ModSize i = 0; i < n; ++i)
		cstrValue[i] = ModUnicodeCharTrait::toUpper(cstrValue[i]);

	// トークナイズパラメータを正規化する
	cstrValue = Tokenizer::check(getIndexingType(), cstrValue);

	// FileIDに設定する
	setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::TokenizeParameter), cstrValue);
}

//
//	FUNCTION private
//	FullText2::FileID::setNormalized
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

	if (readHint(cstrHint_, cHintArray_, _Hint::_Inverted::_cNormalized,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		bNormalized = _cNormalized.get();
	}
	else
	{
		if (cstrValue.compare(_cTrue, ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bNormalized = true;
		}
		else if (cstrValue.compare(_cFalse, ModFalse) == 0)
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
			if (readHint(cstrValue, cHintArray,
						 _Hint::_Inverted::_Normalized::_cStemming,
						 cstrValue0) == true)
			{
				bool bStemming;
				
				if (cstrValue0.compare(_cTrue, ModFalse) == 0
					|| cstrValue0.getLength() == 0)
				{
					bStemming = true;
				}
				else if (cstrValue0.compare(_cFalse, ModFalse) == 0)
				{
					bStemming = false;
				}
				else
				{
					// エラー
					SydErrorMessage
						<< "Illegal Stemming. " << cstrValue0 << ModEndl;
					_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue0);
				}

				// FileIDに設定する
				setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsStemming),
						   bStemming);
			}
			
			// スペース除去
			if (readHint(cstrValue, cHintArray,
						 _Hint::_Inverted::_Normalized::_cDeleteSpace,
						 cstrValue0) == true)
			{
				bool bDeleteSpace;
				
				if (cstrValue0.compare(_cTrue, ModFalse) == 0
					|| cstrValue0.getLength() == 0)
				{
					bDeleteSpace = true;
				}
				else if (cstrValue0.compare(_cFalse, ModFalse) == 0)
				{
					bDeleteSpace = false;
				}
				else
				{
					// エラー
					SydErrorMessage
						<< "Illegal DeleteSpace. " << cstrValue0 << ModEndl;
					_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue0);
				}

				// FileIDに設定する
				setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsDeleteSpace),
						   bDeleteSpace);
			}
			
			// 改行を跨って正規化するかどうか
			if (readHint(cstrValue, cHintArray,
						 _Hint::_Inverted::_Normalized::_cCarriage,
						 cstrValue0) == true)
			{
				bool bCarriage;
				
				if (cstrValue0.compare(_cTrue, ModFalse) == 0
					|| cstrValue0.getLength() == 0)
				{
					bCarriage = true;
				}
				else if (cstrValue0.compare(_cFalse, ModFalse) == 0)
				{
					bCarriage = false;
				}
				else
				{
					// エラー
					SydErrorMessage
						<< "Illegal Carriage. " << cstrValue0 << ModEndl;
					_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue0);
				}

				// FileIDに設定する
				setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsCarriage),
						   bCarriage);
			}
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsNormalized), bNormalized);
}

//
//	FUNCTION private
//	FullText2::FileID::setCoderParameter
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

	if (readHint(cstrHint_, cHintArray_, _Hint::_Inverted::_cCoder,
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
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cId,
					 cstrValue0) == false)
		{
			cstrValue0 = _cIdCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IdCoder),
				  cstrValue0);

		// FrequencyCoder
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cFrequency,
					 cstrValue0) == false)
		{
			cstrValue0 = _cFrequencyCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::FrequencyCoder),
				  cstrValue0);

		// LengthCoder
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cLength,
					 cstrValue0) == false)
		{
			cstrValue0 = _cLengthCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LengthCoder),
				  cstrValue0);

		// LocationCoder
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cLocation,
					 cstrValue0) == false)
		{
			cstrValue0 = _cLocationCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::LocationCoder),
				  cstrValue0);

		// WordIdCoder
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cWordId,
					 cstrValue0) == false)
		{
			cstrValue0 = _cWordIdCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordIdCoder),
				  cstrValue0);

		// WordFrequencyCoder
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cWordFrequency,
					 cstrValue0) == false)
		{
			cstrValue0 = _cWordFrequencyCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordFrequencyCoder),
				  cstrValue0);

		// WordLengthCoder
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cWordLength,
					 cstrValue0) == false)
		{
			cstrValue0 = _cWordLengthCoder.get();
		}
		setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::WordLengthCoder),
				  cstrValue0);

		// WordLocationCoder
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Coder::_cWordLocation,
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
//	FullText2::FileID::setExtractor
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

	if (readHint(cstrHint_, cHintArray_, _Hint::_Inverted::_cExtractor,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		cstrValue = _cExtractor.get();
	}

	if (cstrValue.getLength() == 0)
	{
		SydErrorMessage << "Extractor is not set." << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// FileIDに設定する
	setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Extractor), cstrValue);
}

//
//	FUNCTION private
//	FullText2::FileID::setLanguage
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

	if (readHint(cstrHint_, cHintArray_, _Hint::_Inverted::_cLanguage,
				 cstrValue) == false)
	{
		// 無いのでパラメータ
		cstrValue = _cLanguage.get();
	}

	// FileIDに設定する
	setString(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Language), cstrValue);
}

//
//	FUNCTION private
//	FullText2::FileID::setDistribute
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
FileID::setDistribute(const ModUnicodeString& cstrHint_,
					  const FileCommon::HintArray& cHintArray_)
{
	int dcount = 0;

	ModUnicodeString cstrValue;

	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_Inverted::_cDistribute, cstrValue) == true)
	{
		dcount = ModUnicodeCharTrait::toInt(cstrValue);
		if (dcount < 0 || dcount > 100)
		{
			// 上限は100
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}

	// FileIDに設定する
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Distribute), dcount);
}

//
//	FUNCTION private
//	FullText2::FileID::setNoloacation -- 
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
	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_Inverted::_cNolocation, cstrValue) == true)
	{
		if (cstrValue.compare(_cTrue, ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			// The hint is 'nolocation=true' or 'nolocaiton'.
			bNolocation = true;
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsNolocation), bNolocation);
}

//
//	FUNCTION private
//	FullText2::FileID::setNoTF -- 
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
	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_Inverted::_cNoTF, cstrValue) == true)
	{
		if (cstrValue.compare(_cTrue, ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			// The hint is 'notf=true' or 'notf'.
			bNoTF = true;
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsNoTF), bNoTF);
}

//
//	FUNCTION private
//	FullText2::FileID::setClustered
//	NOTES
//	検索結果のクラスタリングを支持するためのパラメータの解析
//	構文は、
//		cluster=(feature=特徴数)
//  (example)
//　　　create fulltext index I1_1 on T1(C1)
//			hint 'inverted=(normalized=true), clustered=(feature=10)'
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
	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_Inverted::_cClustered, cstrValue) == true)
	{
		// 子ヒント(feature数)を調べる
		FileCommon::HintArray cHintArray(cstrValue);
		ModUnicodeString cstrValue0;
		bClustered = true;
		if (readHint(cstrValue, cHintArray,
					 _Hint::_Inverted::_Clustered::_cFeature,
					 cstrValue0) == true)
		{
			// cstrValue0にfeature数が入る
			if (cstrValue0.getLength() > 0)
			{
				// feature数を設定する
				// cstrValue0にfeature数が入るので、
				// cstrValue0をintegerに変換する
				
				int feature = ModUnicodeCharTrait::toInt(cstrValue0);
				if (feature <= 0)
				{
					// feature数は0より大きい必要がある
					// 例外発生
					// エラー
					SydErrorMessage
						<< "Illegal Feature Numbers " << cstrValue0 << ModEndl;
					_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue0);
				}
				setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Feature),
						   feature);		
			}
			else
			{
				// エラー
				SydErrorMessage
				<< "Feature Numbers Missing" << cstrValue0 << ModEndl;
				_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue0);
			}
		}
		else
		{
			// featureの指定がないので、default feature数(10)を使用して、
			// clusteringをする
			setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::Feature), 10);
		}
	}
	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsClustered), bClustered);
}

//
//	FUNCTION private
//	FullText2::FileID::setMaxWordLength -- 最大単語長を設定する
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
FileID::setMaxWordLength(const ModUnicodeString& cstrHint_,
						 const FileCommon::HintArray& cHintArray_)
{
	
	ModUnicodeString cstrValue;
	int max = 0;

	// MaxWordLength
	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_Inverted::_cMaxWordLength, cstrValue) == true)
	{
		// 見つかった
		max = ModUnicodeCharTrait::toInt(cstrValue);
	}
	else
	{
		// 見つからないのでパラメータ
		max = _cMaxWordLength.get();
	}

	// FileIDに設定する
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::MaxWordLength), max);
}

//
//	FUNCTION private
//	FullText2::FileID::setExpungeFlag -- 削除フラグかどうかを設定する
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
FileID::setExpungeFlag(const ModUnicodeString& cstrHint_,
					   const FileCommon::HintArray& cHintArray_)
{
	ModUnicodeString cstrValue;
	bool bExpungeFlag = false;

	if (readHint(cstrHint_, cHintArray_,
				 _Hint::_cExpungeFlag, cstrValue) == true)
	{
		if (cstrValue.compare(_cTrue, ModFalse) == 0
			|| cstrValue.getLength() == 0)
		{
			bExpungeFlag = true;
		}
		else if (cstrValue.compare(_cFalse, ModFalse) == 0)
		{
			bExpungeFlag = false;
		}
		else
		{
			SydErrorMessage << "Illegal DeleteFlag. " << cstrValue << ModEndl;
			_SYDNEY_THROW1(Exception::SQLSyntaxError, cstrValue);
		}
	}

	// FileIDに設定する
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(
				   KeyID::IsExpungeFlag), bExpungeFlag);
}

//
//	FUNCTION private
//	FullText2::FileID::readHint -- ヒントを読む
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

/* static */
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
//	FullText2::FileID::verifyHint -- ヒントの整合性をチェックする
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
	if ((isNolocation() == true && getIndexingType() == IndexingType::Dual) ||
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
//	FUNCTION private
//	FullText2::FileID::check -- 設定をチェックする
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
	
	// フィールド番号
	int n = 0;
	
	//////////////////////////////////////
	// 以下は非固定部分のfieldの型検査
	//////////////////////////////////////

	//
	// 先頭フィールドは常に文字列
	//
	
	// キーの数
	int k = 0;
	while (checkFieldType(n, DataType::String) ||
		   checkFieldType(n, DataType::StringArray))
	{
		++n;
		++k;
	}

	if (k > 31)
	{
		// FieldMask::checkでキー値をビットで表現している箇所があるのと、
		// -1 を特別な数としているので、キーの最大数は 31 個となる

		return false;
	}

	//
	// 次のフィールドは言語情報かもしれない
	//

	bool lang = false;
	if (k != 1)
	{
		// 複合索引の場合には、言語列は配列ではない
		
		if (checkFieldType(n, DataType::LanguageArray))
			return false;
		if (checkFieldType(n, DataType::Language))
		{
			lang = true;
			++n;
		}
	}
	else
	{
		// 複合索引ではない場合には、キーに応じて配列か否かが決まる
		
		if (checkFieldType(n, DataType::Language))
		{
			if (checkFieldType(0, DataType::String) == false)
				// キーは配列であってはならない
				return false;
			lang = true;
			++n;
		}
		else if (checkFieldType(n, DataType::LanguageArray))
		{
			if (checkFieldType(0, DataType::StringArray) == false)
				// キーは配列である必要がある
				return false;
			lang = true;
			++n;
		}
	}

	// キーの数を設定する
	setInteger(_SYDNEY_FILE_PARAMETER_KEY(KeyID::KeyCount), k);
	// 言語指定があるかどうか
	setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsLanguage), lang);
	
	//
	// 次のフィールドはスコア調整かもしれない。
	//

	if (checkFieldType(n, DataType::Double))
	{
		// スコア調整用カラムあり
		setBoolean(_SYDNEY_FILE_PARAMETER_KEY(KeyID::IsScoreField),
				   true);
		++n;
	}

	if (isSectionized())
	{
		// セクションの場合は、キー数が1でかつ配列でなければならない
		if (k != 1 || checkFieldType(0, DataType::StringArray) == false)
			return false;
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::FileID::checkFieldType -- フィールドの型をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		フィールド番号
//	FullText2::FileID::DataType::Value
//		データ型
//
//	RETURN
//	bool
//		引数と同じ型の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//

bool
FileID::checkFieldType(int n_, DataType::Value eType_) const
{
	//
	// FullText2::FileID::DataType と Common::DataType との対応表
	//
	struct typeTable
	{
		DataType::Value dv;		// data value
		int ct;					// common type
		bool array;				// simple or array
	};
	static	typeTable tbl[]	=
	{
	{DataType::Integer,			Common::DataType::Integer,		false},
	{DataType::UnsignedInteger,	Common::DataType::UnsignedInteger, false},
	{DataType::String,			Common::DataType::String,		false},
	{DataType::Language,		Common::DataType::Language,		false},
	{DataType::Double,			Common::DataType::Double,		false},
	{DataType::Word,			Common::DataType::Word,			false},
	{DataType::UnsignedIntegerArray, Common::DataType::UnsignedInteger, true},
	{DataType::StringArray,		Common::DataType::String,		true},
	{DataType::LanguageArray,	Common::DataType::Language,		true},
	{DataType::WordArray,		Common::DataType::Word,			true}
	};

	//
	// チェックする
	//

	bool result = false;
	
	for (int i = 0; i < sizeof(tbl)/sizeof(*tbl) ; ++i)
	{
		if (tbl[i].dv == eType_)
		{
			// eType_に対応するCommon::DataTypeが見つかった。
			
			//
			// 呼び出し側で設定されたCommon::DataTypeと一致するか？
			//
			if (tbl[i].array)
			{
				// check type
				if (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldType::Key, n_))
					== Common::DataType::Array)
				{
					// check element type
					if (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
						FileCommon::FileOption::ElementType::Key, n_))
						== tbl[i].ct)
						result = true;
				}
			}
			else
			{
				// check type
				if (getInteger(_SYDNEY_FILE_PARAMETER_FORMAT_KEY(
					FileCommon::FileOption::FieldType::Key, n_))
					== tbl[i].ct)
					result = true;
			}

			break;
		}
	}

	return result;
}

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
