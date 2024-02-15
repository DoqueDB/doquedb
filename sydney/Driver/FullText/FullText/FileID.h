// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h --
// 
// Copyright (c) 2003, 2004, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_FILEID_H
#define __SYDNEY_FULLTEXT_FILEID_H

#include "FullText/Module.h"
#include "FullText/FieldMask.h"
#include "FullText/FileOption.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"
#include "Lock/Name.h"
#include "FileCommon/HintArray.h"
#include "Os/Path.h"
#include "Common/IntegerArrayData.h"

#include "Inverted/IntermediateFileID.h"
#include "Inverted/FileID.h"


_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//	CLASS
//	FullText::FileID --
//
//	NOTES
//	FullText::FileIDは、ファイルの有無に関係するオプションを扱い、
//	Inverted::FileIDは、索引の作り方に関係するオプションを扱う。
//
//	固定フィールドと非固定フィールド
//	ここでいう「固定」とは、スキーマ定義によらず、常に存在することを意味する。
//	たとえばヒントsectionizedが定義されてなくとも、セクション情報列は存在する。
//	また、定義順が非固定フィールド、固定フィールドの順なので、
//	たとえば、ROWIDの位置は1〜3の値をとり、固定ではないことに注意。
//	
//	以下にフィールド順などの一覧を示す。
//	
//	記号の意味
//	○：格納される、取得できる
//	△：列全体のデータが格納される。
//	◇：各索引のデータが格納される。
//	×：格納されない、取得できない
//	()：スキーマ定義により格納される、取得できる。
//
//						格納	取得	補足
//	---------------------------------------------------------------------
//	0: 文字列			×		×
//
//	(以下は非固定フィールド)
//	1: 言語情報			×		×		スキーマ language column
//	2: スコア調整		(○)	×		スキーマ score column
//
//	(以下は固定フィールド)
//	3: ROWID			◇		○
//	4: スコア			×		○
//	5: セクション情報	(○)	(○)	ヒント sectionized
//	6: ワード			×		○		DF,Scaleを含む
//	7: ワードDF			×		×		単体では取得できない
//	8: ワードScale		×		×		単体では取得できない
//	9: 平均文書長		×		○
//	10:平均単語数		×		○
//	11:TF				(◇)	(○)	invertedヒント notf=false
//	12:登録文書数		△		○
//	13:CLUSTER ID		×		○
//	14:特徴語			(○)	(○)   	invertedヒント clustered
//	15:荒いKWIC開始位置	×		○
//
//	(以下はフィールドとしては未定義)
//	a: 索引語の位置		(◇)	×		invertedヒント nolocation=false
//	b: 正規化後データ長	○		×		単語索引なら単語数
//	c: 総文書長			△		×		単語索引なら総単語数
//	d: 文字列長			(○)	×	   	ヒント kwic
//
//	以下も参照のこと。
//	関数フィールドの種別	: Schema::Field::Function::Value
//	関数フィールドのCommon型: Schema/Field.cpp _$$::_functionTable
//	関数フィールド順の定義	: ???
//	固定フィールドの種別	: Inverted::FieldType::Value
//	固定フィールドのFullText::FileID::DataType型
//							: FullText::FileID::DataType::Value
//	非固定フィールド順の定義: FullText::FileID::checkの実装
//	固定フィールド順の定義	: FullText::FileID::check内のFieldTypeValue
//	DataType型->Common型	: FullText::FileID::checkFieldType内のtypeTalbe
//
class FileID : public Inverted::IntermediateFileID
{
public:
	struct DataType
	{
		enum Value
		{
			// [NOTE] キーを追加する時は末尾に追加する。
			//  その他情報ファイルのキー、Double, UnsigedIntegerArray, Binary
			//  などは、この順番で永続化されるため。FileID::check() を参照。
			
			String,
			StringArray,
			UnsignedInteger,
			UnsignedIntegerArray,
			Language,
			LanguageArray,
			Double,
			Word,
			Integer,
			ObjectID,
			Binary,
			WordArray
		};
	};

	// バージョン
	enum
	{
		Version1 = 0,
		Version2,
		Version3,

		// バージョン数
		ValueNum,
		// 現在のバージョン
		CurrentVersion = ValueNum - 1,

		// その他情報ファイルv2を利用するバージョン
		OtherVersion = Version3
	};

	// コンストラクタ
	FileID(const LogicalFile::FileID& cFileID_);
	// デストラクタ
	virtual ~FileID();

	// ファイルIDを作成する
	void create();

	// 遅延更新かどうか
	bool isDelayed() const;
	// セクション検索かどうか
	bool isSectionized() const;
	// 言語情報があるかどうか
	bool isLanguage() const;
	// スコア調整値があるかどうか
	bool isScoreField() const;
	// クラスタ情報があるかどうか
	bool isClustering() const;
	// 荒いKWIC情報があるかどうか
	bool isRoughKwic() const;
	// その他情報ファイルを利用しているかどうか
	bool isOtherInformationFile() const
	{
		return (isSectionized() || isScoreField() || isClustering() ||
			isRoughKwic());
	}

	// マウントされているか
	bool isMounted() const;
	// マウントされているかどうかを設定する
	void setMounted(bool flag_);

	// 一時データベースかどうか
	bool isTemporary() const;
	// ReadOnlyかどうか
	bool isReadOnly() const;

	// 配列かどうか
	bool isArray() const;

	// ロック名を得る
	const Lock::FileName& getLockName() const;
	// パス名を得る
	const Os::Path& getPath() const;
	// パス名を設定する
	void setPath(const Os::Path& cPath_);

	// 大転置のファイルIDを得る
	LogicalFile::FileID& getInverted() const;
	// 挿入用転置のファイルIDを得る
	LogicalFile::FileID& getInsert0();
	LogicalFile::FileID& getInsert1();
	// 削除用転置のファイルIDを得る
	LogicalFile::FileID& getExpunge0();
	LogicalFile::FileID& getExpunge1();
	// セクション情報ファイルのファイルIDを得る
	LogicalFile::FileID& getSection();

	// 大転置のファイルIDを設定する
	void setInverted(const LogicalFile::FileID& cFileID_);
	// 挿入用転置のファイルIDを設定する
	void setInsert0(const LogicalFile::FileID& cFileID_);
	void setInsert1(const LogicalFile::FileID& cFileID_);
	// 削除用小転置のファイルIDを設定する
	void setExpunge0(const LogicalFile::FileID& cFileID_);
	void setExpunge1(const LogicalFile::FileID& cFileID_);
	// セクション情報ファイルのファイルIDを設定する
	void setSection(const LogicalFile::FileID& cFileID_);

	// プロジェクションパラメータを設定する
	bool getProjectionParameter(const Common::IntegerArrayData& cProjection_,
								LogicalFile::OpenOption& cOpenOption_) const;
	// 更新パラメータを設定する
	bool getUpdateParameter(const Common::IntegerArrayData& cUpdateFields_,
							LogicalFile::OpenOption& cOpenOption_) const;

	// ROWIDのフィールド番号を得る
	int getRowIdField() const;
	// スコアのフィールド番号を得る
	int getScoreField() const;
	// ワードDFのフィールド番号を得る
	int getWordDfField() const;
	// ワードScaleのフィールド番号を得る
	int getWordScaleField() const;

	// バージョンを得る
	int getVersion() const;

	// ベクターファイルのページサイズを得る
	ModSize getVectorPageSize() const;
	// 可変長ファイルのページサイズを得る
	ModSize getVariablePageSize() const { return getVectorPageSize(); }

	// ベクターファイルの要素数を得る
	ModSize getVectorElementFieldCount() const;
	// ベクターファイルの1行のサイズを得る
	ModSize getVectorElementTotalSize() const;
	// ベクターファイルの1要素のサイズを得る
	ModSize getVectorElementSize(int n) const;

	// 可変長ファイルが利用されているかどうか
	bool isVariableFile() const;

	// その他情報ファイルのデータ型を得る
	DataType::Value getOtherFileElementType(int n) const;

private:
	// ヒントを解釈し、格納されている文字列を得る
	bool readHint(ModUnicodeString& cstrHint_,
				  FileCommon::HintArray& cHintArray_,
				  const ModUnicodeString& cstrKey_,
				  ModUnicodeString& cstrValue_);

	// 遅延更新かどうか
	void setDelayed(ModUnicodeString& cstrHint_,
					FileCommon::HintArray& cHintArray_);
	// セクション検索かどうかを設定する
	void setSectionized(ModUnicodeString& cstrHint_,
						FileCommon::HintArray& cHintArray_);
	// 荒いKWIC情報があるかどうかを設定する
	void setRoughKwic(ModUnicodeString& cstrHint_,
					  FileCommon::HintArray& cHintArray_);

	// 大転置のFileIDを得る
	LogicalFile::FileID& getInvertedFileID() const;
	// 小転置のファイルIDを得る
	LogicalFile::FileID& getDelayProcFileID(int iIndex_);
	// セクション情報ファイルのFileIDを得る
	LogicalFile::FileID& getSectionInfoFileID();
	// 子FileIDに親の現在値を設定する
	LogicalFile::FileID& setCurrentValue(LogicalFile::FileID& cFileID_) const;

	// 正しく設定されているかチェックする
	bool check();
	// フィールドの型をチェックする
	bool checkFieldType(int n, DataType::Value eType_);
	// プロジェクションできるか
	bool checkProjection(int n_,FieldMask &mask ) const;

	// その他情報ファイルのフィールド情報を初期化する
	void initializeOtherFileField() const;

	int getField(FieldMask::FieldType type) const;
	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
	// ベクターファイルのフィールドサイズ
	mutable ModVector<ModSize> m_vecVectorFieldSize;
	// ベクターファイルの合計フィールドサイズ
	mutable ModSize m_uiVectorFieldTotalSize;
	// その他情報ファイルのフィールドサイズを初期化したかどうか
	mutable bool m_bOtherFileFieldInitialized;
	// 可変長ファイルを利用しているかどうか
	mutable bool m_bVariableFile;
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_FILEID_H

//
//	Copyright (c) 2003, 2004, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
