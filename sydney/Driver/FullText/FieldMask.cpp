// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldMask.cpp -- 検索フィールドを処理するクラス
// 
// Copyright (c) 1999, 2000, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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
#include "FullText/FieldMask.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING
///////////////////////////////////////////////////
//  FieldMask class
//

//
//	FUNCTION public
//	FullText::FieldMask::FieldMask -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	bool bLang
//		言語情報フィールドの有無
//	bool bScore
//		スコア調整フィールドの有無
//
//	RETURN
//
//	EXCEPTIONS
//
FieldMask::FieldMask(bool bLang,bool bScore)
{
// 非固定フィールドをチェックするためのマスクを生成
	
	// [NOTE] 非固定フィールド用であるが、フィールド型の取得にも使える

	stat = 0;
	bit = 0;
	base = upper = lower = _SYDNEY::Inverted::FieldType::Rowid - 1;

	if(bLang)
		shift();
	if(bScore)
		shift();
}

//
//	FUNCTION public
//	FullText::FieldMask::FieldMask -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::OpenOption& cOpenOption_
//	bool bLang
//		言語情報フィールドの有無
//	bool bScore
//		スコア調整フィールドの有無
//	bool bSection
//		セクション検索かどうか？
//
//	RETURN
//
//	EXCEPTIONS
//
FieldMask::FieldMask(LogicalFile::OpenOption& cOpenOption_,bool bLang,bool bScore,bool bSection)
{
// 固定フィールドをチェックするためのマスクを生成
	
	stat = BASIC_BIT;
	base = 0;
	lower = _SYDNEY::Inverted::FieldType::Rowid;
	upper = _SYDNEY::Inverted::FieldType::Last - 1;
	bit = 0;

	//
	// データ取得方法、Bitsetかどうかを設定
	//
	
	if (cOpenOption_.getBoolean(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::GetByBitSet::Key)) == true)
		stat |= GetByBitSet_BIT;

	//
	// 検索方法、SCANかどうかを設定
	//
	
	// SCANは、索引全体の平均文書長などのデータの取得に索引全体が必要な時に使用
	// SCAN以外、例えばSEARCHは、条件に合致したデータを求める検索に使用
	int tmp;
	stat |= SCAN_BIT;
	if (cOpenOption_.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(
		FileCommon::OpenOption::OpenMode::Key), tmp) == true)
	{
		if (tmp == FileCommon::OpenOption::OpenMode::Search)
			stat ^= SCAN_BIT;
	}

	//
	// グループの設定
	//

	// 全てのフィールド値を同時に取得できるわけではない。
	// 同時に取得できるフィールドを各グループに設定する。

	// セクション情報
	// [NOTE] この時点ではビット形式に変換しない！
	section = _SYDNEY::Inverted::FieldType::Section;
	// ROWID+スコア+TF+ClusterID+特徴語
	// [NOTE] セクション情報は同時に取得できるので、セクション情報を取得する際は
	//  shift(SECTION)で修正する。
	normal = (1 << _SYDNEY::Inverted::FieldType::Rowid) | 
			 (1 << _SYDNEY::Inverted::FieldType::Score) | 
			 (1 << _SYDNEY::Inverted::FieldType::Tf) |
			 (1 << _SYDNEY::Inverted::FieldType::Cluster) |
			 (1 << _SYDNEY::Inverted::FieldType::FeatureValue) |
			 (1 << _SYDNEY::Inverted::FieldType::RoughKwicPosition);
	// ワード
	// [NOTE] WordDF,WordScaleは単体では取得されない。
	word   = (1 << _SYDNEY::Inverted::FieldType::Word);
	// 平均文書長+平均単語数+登録文書数
	length = (1 << _SYDNEY::Inverted::FieldType::AverageLength) | 
			(1 << _SYDNEY::Inverted::FieldType::AverageCharLength) |
			(1 << _SYDNEY::Inverted::FieldType::AverageWordCount) |
			(1 << _SYDNEY::Inverted::FieldType::Count) ;

	//
	// 非固定フィールドに応じて、固定フィールドの位置を移動
	//

	if(bLang)
		shift();
	if(bScore)
		shift();

	// [NOTE] 移動ではない？ normalにsectionを追加している。
	if(bSection)
		shift(SECTION);
}

//
//	FUNCTION public
//	FullText::FieldMask::check -- フィールド番号とフィールド型が一致するか？
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		調べるフィールド番号
//	FieldType type_
//		調べるフィールド型
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
FieldMask::check(int n_,FieldType type_) const
{
	return (n_ == type_ + base);
}

//
//	FUNCTION public
//	FullText::FieldMask::checkValueRangeValidity -- フィールド番号は適切な範囲内か？
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		調べるフィールド番号
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
FieldMask::checkValueRangeValidity(int n_) const
{
	// 指定されたフィールド番号が、コンストラクタ定義された範囲に
	// 収まっていることを確認する。

	// [NOTE] ビットセットで取得するかどうかもチェックしている。
	
	if (n_ < lower || n_ > upper)
		// 収まっていない
		return false;
	
	if (stat & GetByBitSet_BIT)
		// ビットセットで取得する場合は、一つのフィールド値しか取得できない
		// [YET] 最小フィールドであることしか確認していない！
		return (n_ == lower) ? true : false;
	
	return true;
}

//
//	FUNCTION public
//	FullText::FieldMask::checkGroupExclusiveness -- 調査済みのフィールド群と同じグループか？
//
//	NOTES
//
//	ARGUMENTS
//	int n_
//		調べるフィールド番号
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
FieldMask::checkGroupExclusiveness(int n_)
{
	// 指定されたフィールド番号のフィールド型と、
	// 今まで調査済みのフィールド型とが、
	// 同じグループに属していることを確認する。
	
	// [NOTE] SCANで検索するフィールド型かどうかも調べている。
	
	// フィールド番号をビット演算で処理できる内部形式に変換
	int b = (1 << n_);

	if (normal & b)
	{
		if (bit & ~normal || (stat & SCAN_BIT))
			return false;
	}
	else if (word & b)
	{
		if (bit & ~word || (stat & SCAN_BIT))
			return false;
	}
	else if (length & b)
	{
		if (bit & ~length || !(stat & SCAN_BIT))
			return false;
	}
	else
		// 上記のグループに属していないフィールドは取得できない
		return false;

	// 調査済みのフィールドに追加
	bit |= b;
	return true;
}

//
//	FUNCTION private
//	FullText::FieldMask::shift -- 仮想フィールドの位置を移動
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
void
FieldMask::shift()
{
	// 範囲を移動
	upper++;
	base++;
	
	if(stat & BASIC_BIT)
	{
		// 範囲を移動
		lower++;

		// グループを移動
		// [NOTE] sectionはビット形式ではない
		section += 1;
		normal <<= 1;
		word   <<= 1;
		length <<= 1;
	}
}

//
//	FUNCTION private
//	FullText::FieldMask::shift -- 
//
//	NOTES
//
//	ARGUMENTS
//	FieldGroupMaskID groupMaskID
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
void
FieldMask::shift(FieldGroupMaskID groupMaskID)
{
	// [YET] 指定されたグループをnormalに追加？
	
	if(groupMaskID == SECTION)
		// [NOTE] sectionをビット形式に変換して追加する
		normal |= (1 << section);
}

//
// Copyright (c) 1999, 2000, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
