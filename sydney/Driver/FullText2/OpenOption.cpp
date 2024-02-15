// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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
#include "SyInclude.h"
#include "SyDynamicCast.h"
#include "FullText2/OpenOption.h"
#include "FullText2/Parameter.h"

#include "FullText2/ExternalScoreCalculator.h"
#include "FullText2/NormalizedOkapiTfIdfScoreCalculator.h"
#include "FullText2/NormalizedOkapiTfScoreCalculator.h"
#include "FullText2/NormalizedTfIdfScoreCalculator.h"
#include "FullText2/OkapiTfIdfScoreCalculator.h"
#include "FullText2/OkapiTfScoreCalculator.h"
#include "FullText2/TfIdfScoreCalculator.h"

#include "FullText2/ASumScoreCombiner.h"
#include "FullText2/MaxScoreCombiner.h"
#include "FullText2/MinScoreCombiner.h"
#include "FullText2/ProdScoreCombiner.h"
#include "FullText2/SumScoreCombiner.h"

#include "FileCommon/OpenOption.h"
#include "FileCommon/DataManager.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Common/BitSet.h"
#include "Common/Thread.h"
#include "Common/UnicodeString.h"
#include "Common/WordData.h"

#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Exception/TooLongConditionalPattern.h"
#include "Exception/InvalidEscape.h"
#include "Exception/WrongParameter.h"

#include "Os/Limits.h"

#include "Utility/CharTrait.h"
#include "Utility/UNA.h"

#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"

#include "ModNLP.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_cQueryMaxLen -- 検索語の最大長
	//
	ParameterInteger _cQueryMaxLen("FullText2_QueryMaxLen",
								   Os::Limits<int>::getMax());

	//
	//	VARIABLE local
	//	_$$::_Combiner_XXX -- 合成器の名称
	//
	ModUnicodeString _Combiner_Sum("Sum");
	ModUnicodeString _Combiner_ASum("ASum");
	ModUnicodeString _Combiner_Prod("Prod");
	ModUnicodeString _Combiner_Max("Max");
	ModUnicodeString _Combiner_Min("Min");

	// combinerをチェックする
	bool _checkCombiner(const ModUnicodeString& combiner_)
	{
		if (combiner_.compare(_Combiner_Sum, ModFalse))
		if (combiner_.compare(_Combiner_Max, ModFalse))
		if (combiner_.compare(_Combiner_ASum, ModFalse))
		if (combiner_.compare(_Combiner_Prod, ModFalse))
		if (combiner_.compare(_Combiner_Min, ModFalse))
			return false;
		
		return true;
	}

	//
	//	VARIABLE local
	//	_$$::_Calculator_XXX -- 計算器の名称
	//
	ModUnicodeString _Calculator_OkapiTfIdf("OkapiTfIdf");
	ModUnicodeString _Calculator_NormalizedOkapiTfIdf("NormalizedOkapiTfIdf");
	ModUnicodeString _Calculator_OkapiTf("OkapiTf");
	ModUnicodeString _Calculator_NormalizedOkapiTf("NormalizedOkapiTf");
	ModUnicodeString _Calculator_TfIdf("TfIdf");
	ModUnicodeString _Calculator_NormalizedTfIdf("NormalizedTfIdf");
	ModUnicodeString _Calculator_External("External");
	
	// calculatorをチェックする
	bool _checkCalculator(const ModUnicodeString& calculator_)
	{
		if (calculator_.compare(_Calculator_NormalizedOkapiTfIdf, ModFalse,
								_Calculator_NormalizedOkapiTfIdf.getLength()))
		if (calculator_.compare(_Calculator_NormalizedOkapiTf, ModFalse,
								_Calculator_NormalizedOkapiTf.getLength()))
		if (calculator_.compare(_Calculator_OkapiTfIdf, ModFalse,
								_Calculator_OkapiTfIdf.getLength()))
		if (calculator_.compare(_Calculator_OkapiTf, ModFalse,
								_Calculator_OkapiTf.getLength()))
		if (calculator_.compare(_Calculator_NormalizedTfIdf, ModFalse,
								_Calculator_NormalizedTfIdf.getLength()))
		if (calculator_.compare(_Calculator_TfIdf, ModFalse,
								_Calculator_TfIdf.getLength()))
		if (calculator_.compare(_Calculator_External, ModFalse,
								_Calculator_External.getLength()))
			return false;
		
		return true;
	}

	//
	//	VARIABLE local
	//	_$$::_ScoreFunction_XXX -- スコア調整器の名称
	//
	ModUnicodeString _ScoreFunction_Sum("Sum");
	ModUnicodeString _ScoreFunction_Multiply("Multiply");
	ModUnicodeString _ScoreFunction_Replace("Replace");

	// score functionをチェックする
	bool _checkScoreFunction(const ModUnicodeString& scoreFunction_)
	{
		if (scoreFunction_.compare(_ScoreFunction_Sum, ModFalse))
		if (scoreFunction_.compare(_ScoreFunction_Multiply, ModFalse))
		if (scoreFunction_.compare(_ScoreFunction_Replace, ModFalse))
			return false;

		return true;
	}

	// デフォルト値
	ModUnicodeString _DefaultCategory = "Helpful";
	double _DefaultScale = 1.0;

	const ModSize _BufferSize = 1024;

	enum _Type
	{
		DIGIT,
		FLOAT,
		FLOAT_E,
		ALPHABET,

		COMMA,
		SHARP,
		COLON,

		END,

		OTHER
	};

	// トークンを得る
	_Type _getElement(const ModUnicodeChar*& p_, ModUnicodeChar* b_)
	{
		_Type t = OTHER;
		
		while (*p_ == ' ' || *p_ == '\t') ++p_;
		
		ModSize n = 0;
		while (*p_ != 0)
		{
			if (*p_ >= '0' && *p_ <= '9') {
				if (t == OTHER)
					t = DIGIT;
				if (++n >= _BufferSize)
					return OTHER;
				*b_++ = *p_++;
			} else if ((*p_ >= 'A' && *p_ <= 'Z') ||
					   (*p_ >= 'a' && *p_ <= 'z')) {
				if ((t == FLOAT || t == DIGIT) && (*p_ == 'E' || *p_ == 'e'))
				{
					t = FLOAT_E;
				}
				else
				{
					t = ALPHABET;
				}
				if (++n >= _BufferSize)
					return OTHER;
				*b_++ = *p_++;
			} else if (*p_ == '.') {
				if (t == DIGIT || t == OTHER)
					t = FLOAT;
				else 
					t = ALPHABET;
				if (++n >= _BufferSize)
					return OTHER;
				*b_++ = *p_++;
			} else {
				// その他は終わり
				break;
			}
		}

		if (t == FLOAT_E) t = FLOAT;

		while (*p_ == ' ' || *p_ == '\t') ++p_;
		
		if (n == 0) {
			if (*p_ == ':') {
				t = COLON;
				++p_;
			} else if (*p_ == '#') {
				t = SHARP;
				++p_;
			} else if (*p_ == ',') {
				t = COMMA;
				++p_;
			} else if (*p_ == 0) {
				t = END;
			}
		}
		
		*b_ = 0;
		return t;
	}

	// 比較クラス
	class _FieldSort
	{
	public:
		ModBoolean operator()(const ModPair<int, ModUnicodeString>& a1,
							  const ModPair<int, ModUnicodeString>& a2)
		{
			return (a1.first < a2.first) ? ModTrue : ModFalse;
		}
	};

	// intをチェックする
	bool _checkInt(const ModVector<int>& fields_, ModUnicodeString& v_)
	{
		ModVector<ModPair<int, ModUnicodeString> > vecValue;
		ModVector<int>::ConstIterator i = fields_.begin();
		ModUnicodeChar buf[_BufferSize];
		const ModUnicodeChar* p = v_;

		while (*p != 0) {
			ModUnicodeChar* b = buf;
			_Type t = _getElement(p, b);
			if (t != DIGIT)
				// 構文エラー
				return false;

			if (i == fields_.end())
				return false;
			vecValue.pushBack(ModPair<int, ModUnicodeString>(*i, b));
			++i;
			
			t = _getElement(p, b);
			if (t == END) {
				// 終了
				break;
			} else if (t == COMMA) {
				;
			} else {
				return false;
			}
		}

		if (vecValue.getSize() == 0 ||
			(vecValue.getSize() != 1 &&
			 vecValue.getSize() != fields_.getSize()))
			return false;

		// ソートする
		ModSort(vecValue.begin(), vecValue.end(), _FieldSort());

		ModUnicodeOstrStream s;
		if (vecValue.getSize() != 1) {
			s << '[';
			ModVector<ModPair<int, ModUnicodeString> >::Iterator
				j = vecValue.begin();
			for (; j != vecValue.end(); ++j)
			{
				if (j != vecValue.begin())
					s << ',';
				s << (*j).second;
			}
			s << ']';
			v_ = s.getString();
		} else {
			v_ =(*vecValue.begin()).second;
		}

		return true;
	}
	
	// floatをチェックする
	bool _checkFloat(const ModVector<int>& fields_, ModUnicodeString& v_)
	{
		ModVector<ModPair<int, ModUnicodeString> > vecValue;
		ModVector<int>::ConstIterator i = fields_.begin();
		ModUnicodeChar buf[_BufferSize];
		const ModUnicodeChar* p = v_;

		while (*p != 0) {
			ModUnicodeChar* b = buf;
			_Type t = _getElement(p, b);
			if (t != DIGIT && t != FLOAT)
				// 構文エラー
				return false;

			if (i == fields_.end())
				return false;
			vecValue.pushBack(ModPair<int, ModUnicodeString>(*i, b));
			++i;
			
			t = _getElement(p, b);
			if (t == END) {
				// 終了
				break;
			} else if (t == COMMA) {
				;
			} else {
				return false;
			}
		}

		if (vecValue.getSize() == 0 ||
			(vecValue.getSize() != 1 &&
			 vecValue.getSize() != fields_.getSize()))
			return false;

		// ソートする
		ModSort(vecValue.begin(), vecValue.end(), _FieldSort());

		ModUnicodeOstrStream s;
		if (vecValue.getSize() != 1) {
			s << '[';
			ModVector<ModPair<int, ModUnicodeString> >::Iterator
				j = vecValue.begin();
			for (; j != vecValue.end(); ++j)
			{
				if (j != vecValue.begin())
					s << ',';
				s << (*j).second;
			}
			s << ']';
			v_ = s.getString();
		} else {
			v_ =(*vecValue.begin()).second;
		}

		return true;
	}


	//
	//	VATIABLE local
	//	_$$::_ScoreCombiner_XXX -- スコア合成方法 (SQL文)
	//
	ModUnicodeString _ScoreCombiner_Concatinate("concatinate");
	ModUnicodeString _ScoreCombiner_Sum("sum");
	ModUnicodeString _ScoreCombiner_Max("max");

	//
	//	VATIABLE local
	//	_$$::_ScoreCombiner_XXX -- スコア合成方法 (tea構文)
	//
	ModUnicodeString _ScoreCombiner_Tea_Single("single");
	ModUnicodeString _ScoreCombiner_Tea_Cat("cat");
	ModUnicodeString _ScoreCombiner_Tea_Sum("or,sum");
	ModUnicodeString _ScoreCombiner_Tea_Max("or,max");
	

	// スコア合成方法をチェックする
	bool _checkScoreCombiner(const ModVector<int>& fields_,
							 const ModUnicodeString& v_,
							 ModUnicodeString& type_,
							 ModUnicodeString& scale_)
	{
		if (fields_.getSize() <= 1)
			return false;
		
		ModUnicodeChar buf[_BufferSize];
		const ModUnicodeChar* p = v_;

		ModUnicodeChar* b = buf;
		_Type t = _getElement(p, b);
		if (t != ALPHABET)
			return false;
		if (_ScoreCombiner_Concatinate.compare(b, ModFalse) == 0)
		{
			// scale あるかも
			if (type_.getLength() && type_.compare(_ScoreCombiner_Tea_Cat) != 0)
				return false;
			type_ = _ScoreCombiner_Tea_Cat;
			if (*p == ':') ++p;
		}
		else if (_ScoreCombiner_Sum.compare(b, ModFalse) == 0)
		{
			// scale あるかも
			if (type_.getLength() && type_.compare(_ScoreCombiner_Tea_Sum) != 0)
				return false;
			type_ = _ScoreCombiner_Tea_Sum;
			if (*p == ':') ++p;
		}
		else if (_ScoreCombiner_Max.compare(b, ModFalse) == 0)
		{
			// scale あるかも
			if (type_.getLength() && type_.compare(_ScoreCombiner_Tea_Max) != 0)
				return false;
			type_ = _ScoreCombiner_Tea_Max;
			if (*p == ':') ++p;
		}
		else
		{
			return false;
		}

		if (*p != 0)
		{
			ModVector<ModPair<int, ModUnicodeString> > vecValue;
			ModVector<int>::ConstIterator i = fields_.begin();
			ModUnicodeOstrStream s;
			
			while (*p != 0) {
				b = buf;
				t = _getElement(p, b);
				switch (t) {
				case DIGIT:
				case FLOAT:
					s << b;
					break;
				case SHARP:
					if (type_ == _ScoreCombiner_Tea_Cat)
						// concatinate では # は利用不可
						return false;
					if (s.isEmpty())
						// いきなり # は不可
						return false;
					s << '#';
					break;
				case COMMA:
					if (i == fields_.end())
						return false;
					vecValue.pushBack(
						ModPair<int, ModUnicodeString>(*i, s.getString()));
					++i;
					s.clear();
					break;
				default:
					return false;
				}
			}
			if (i == fields_.end())
				return false;
			vecValue.pushBack(
				ModPair<int, ModUnicodeString>(*i, s.getString()));
			++i;
			s.clear();

			if (i != fields_.end())
				return false;

			// ソートする
			ModSort(vecValue.begin(), vecValue.end(), _FieldSort());

			s << '[';
			ModVector<ModPair<int, ModUnicodeString> >::Iterator
				j = vecValue.begin();
			for (; j != vecValue.end(); ++j)
			{
				if (j != vecValue.begin())
					s << ',';
				s << (*j).second;
			}
			s << ']';
			
			scale_ = s.getString();
		}
		
		return true;
	}

	//
	//	VARIABLE local
	//	_$$::_ClusteredCombiner_Avg -- クラスターマージ方法
	//
	ModUnicodeString _ClusteredCombiner_Avg("avg");

	
	// clustered combinerをチェックする
	bool _checkClusteredCombiner(const ModUnicodeString& clusteredCombiner_)
	{
		if (clusteredCombiner_.compare(_ClusteredCombiner_Avg, ModFalse))
			return false;
		
		return true;
	}
}

//
//	FUNCTION public
//	FullText2::OpenOption::OpenOption -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::OpenOption& cLogicalOpenOption_
//		論理ファイルのオープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OpenOption::OpenOption(LogicalFile::OpenOption& cLogicalOpenOption_)
	: m_cOpenOption(cLogicalOpenOption_),
	  m_bEqual(false), m_pAnalyzer(0)
{
}

//
//	FUNCTION public
//	FullText2::OpenOption::~OpenOption -- デストラクタ
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
OpenOption::~OpenOption()
{
	if (m_pAnalyzer) delete m_pAnalyzer;
}

//
//	FUNCTION public
//	FullText2::OpenOption::parse -- TreeNodeInterfaceを変換する
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::FileID& cFileID_
//		転置ファイルID
//	const TreeNodeInterface* pCondition_
//		検索条件ツリー
//
//	RETURN
//	bool
//		転置で実行可能な検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parse(const FileID& cFileID_,
				  const LogicalFile::TreeNodeInterface* pCondition_)
{
	using namespace LogicalFile;
	
	// 索引タイプを得る
	m_eIndexType = cFileID_.getIndexingType();
	// 位置情報を使えるか
	m_bNolocation = cFileID_.isNolocation();
	// デフォルト言語
	m_cDefaultLanguage = cFileID_.getDefaultLanguage();
	// 特徴語が抽出されているか
	m_bClustering = cFileID_.isClustering();
	// リソース番号
	m_uiResourceID = cFileID_.getResourceID();

	// キーの数
	m_iKeyCount = cFileID_.getKeyCount();

	// 言語情報フィールド番号
	m_iLanguageField = -1;
	if (cFileID_.isLanguage()) m_iLanguageField = m_iKeyCount;

	// ROWIDフィールド番号
	m_iRowIDField = m_iKeyCount;
	if (cFileID_.isLanguage()) m_iRowIDField++;
	if (cFileID_.isScoreField()) m_iRowIDField++;
	
	bool bResult = false;

	switch (pCondition_->getType())
	{
	case TreeNodeInterface::Contains:
		// Containsオペランド、Contains演算子('&', '|', '-') の処理
		if (setContains(pCondition_) == true)
			bResult = true;
		break;
	default:
		// LIKE、論理演算子(AND, OR) の処理
		if (setNormal(pCondition_) == true)
			bResult = true;
		break;
	}

	if (bResult == false && m_bEqual == true)
	{
		// 整合性検査時の等号の処理
		if (setEqual(pCondition_) == true)
			bResult = true;
	}

	return bResult;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getSearchType -- 検索タイプを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OpenOption::Type::Value
//		検索タイプ
//
//	EXCEPTIONS
//
OpenOption::Type::Value
OpenOption::getSearchType() const
{
	return static_cast<Type::Value>(m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType)));
}

//
//	FUNCTION public
//	FullText2::OpenOption::getCondition -- 検索条件文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		検索条件文字列
//
//	EXCEPTIONS
//
ModUnicodeString
OpenOption::getCondition() const
{
	return m_cOpenOption.getString(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::Condition));
}

//
//	FUNCTION public
//	FullText2::OpenOption::getTermCount -- 検索語数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		検索条件文字列
//
//	EXCEPTIONS
//
int
OpenOption::getTermCount() const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::TermCount));
}

//
//	FUNCTION public
//	FullText2::OpenOption::getRowID -- RowIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		ROWID。ただしOpenOption中はIntegerなので、INT_MAXまで
//
//	EXCEPTIONS
//
ModUInt32
OpenOption::getRowID() const
{
	return m_cOpenOption.getInteger(_SYDNEY_OPEN_PARAMETER_KEY(KeyID::RowID));
}

//
//	FUNCTION public
//	FullText2::OpenOptin::getSectionValue -- セクションの文字列データを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModVector<ModUnicodeString>
//		セクションの文字列データ
//
//	EXCEPTIONS
//
ModVector<ModUnicodeString>
OpenOption::getSectionValue() const
{
	ModVector<ModUnicodeString> vecValue;
	int size = m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SectionCount));
	for (int i = 0; i < size; ++i)
	{
		vecValue.pushBack(
			m_cOpenOption.getString(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::SectionValue, i)));
	}
	return vecValue;
}

//
//	FUNCTION public
//	FullText2::OpenOptin::getSectionLanguage -- 言語情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModVector<ModLanguageSet>
//		言語情報
//
//	EXCEPTIONS
//
ModVector<ModLanguageSet>
OpenOption::getSectionLanguage() const
{
	ModVector<ModLanguageSet> vecLang;
	int size = m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SectionCount));
	for (int i = 0; i < size; ++i)
	{
		vecLang.pushBack(m_cOpenOption.getString(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::SectionLang, i)));
	}
	return vecLang;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getProjectionFieldCount
//		-- 取得するフィールド数を得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	int
//		取得するフィールド数
//
//	EXCEPTIONS
//
int
OpenOption::getProjectionFieldCount() const
{
	int n = 0;
	if (m_cOpenOption.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::FieldSelect::Key)) == true)
	{
		// プロジェクションパラメータが与えられている

		n = m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::TargetFieldNumber::Key));
	}

	return n;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getProjectionFieldNumber
// 		-- 取得するフィールド番号を得る
//
//	NOTES
//
//	ARGUMETNS
//	int n_
//		要素番号
//
//	RETURN
//	int
//		フィールド番号
//
//	EXCEPTIONS
//
int
OpenOption::getProjectionFieldNumber(int n_) const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
			FileCommon::OpenOption::TargetFieldIndex::Key, n_));

}

//
//	FUNCTION public
//	FullText2::OpenOption::getProjectionFunction
// 		-- 取得するフィールドに適用する関数を得る
//
//	NOTES
//
//	ARGUMETNS
//	int n_
//		要素番号
//
//	RETURN
//	FullText2::OpenOption::Function::Value
//		関数を示す値
//
//	EXCEPTIONS
//
OpenOption::Function::Value
OpenOption::getProjectionFunction(int n_) const
{
	return static_cast<Function::Value>(
		m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				KeyID::Function, n_)));

}

//
//	FUNCTION public
//	FullText2::OpenOption::getProjectionFunctionArgument
// 		-- 取得するフィールドに適用する関数の引数を得る
//
//	NOTES
//
//	ARGUMETNS
//	int n_
//		要素番号
//
//	RETURN
//	ModUnicodeString
//		関数の引数
//
//	EXCEPTIONS
//
ModUnicodeString
OpenOption::getProjectionFunctionArgument(int n_) const
{
	return 	m_cOpenOption.getString(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
										KeyID::FunctionArgument, n_));
}

//
//	FUNCTION public
//	FullText2::OpenOption::getUpdateFieldCount
//		-- 更新対象のフィールド数を得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	int
//		更新対象のフィールド数
//
//	EXCEPTIONS
//
int
OpenOption::getUpdateFieldCount() const
{
	int n = 0;
	if (m_cOpenOption.getBoolean(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::FieldSelect::Key)) == true)
	{
		// 更新パラメータが与えられている

		n = m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::TargetFieldNumber::Key));
	}

	return n;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getUpdateFieldNumber
// 		-- 更新対象のフィールド番号を得る
//
//	NOTES
//
//	ARGUMETNS
//	int n_
//		要素番号
//
//	RETURN
//	int
//		フィールド番号
//
//	EXCEPTIONS
//
int
OpenOption::getUpdateFieldNumber(int n_) const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
			FileCommon::OpenOption::TargetFieldIndex::Key, n_));

}

//
//	FUNCTION public
//	FullText2::OpenOption::getUpdateFieldCount
//		-- 検索対象のフィールド数を得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	int
//		更新対象のフィールド数
//
//	EXCEPTIONS
//
int
OpenOption::getSearchFieldCount() const
{
	int n;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchFieldCount), n) == false)
	{
		n = 0;
	}
	return n;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getSearchFieldNumber
// 		-- 検索対象のフィールド番号を得る
//
//	NOTES
//
//	ARGUMETNS
//	int n_
//		要素番号
//
//	RETURN
//	int
//		フィールド番号
//
//	EXCEPTIONS
//
int
OpenOption::getSearchFieldNumber(int n_) const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::SearchField, n_));
}

//
//	FUNCTION public
//	FullText2::OpenOption::getSortParameter
//		-- ソートパラメータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OpenOption::SortParamter::Value
//		ソートパラメータ
//
//	EXCEPTIONS
//
OpenOption::SortParameter::Value
OpenOption::getSortParameter() const
{
	SortParameter::Value p = SortParameter::None;

	if (isGetByBitSet() == false)
	{
		int v;
		if (m_cOpenOption.getInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::SortSpec), v)
			== true)
		{
			p = static_cast<SortParameter::Value>(v);
		}
	}
	
	return p;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getLimit -- LIMIT指定を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		LIMIT指定
//
//	EXCEPTIONS
//
ModSize
OpenOption::getLimit() const
{
	if (isGetByBitSet())
	{
		// ビットセットでの取得の場合、LIMITは1に固定
		return 1;
	}
	
	ModSize limit = Os::Limits<ModSize>::getMax();
	int n;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::Limit), n) == true)
	{
		limit = static_cast<ModSize>(n);
	}
	return limit;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getOffset -- OFFSET指定を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		OFFSET指定 (1-base)
//
//	EXCEPTIONS
//
ModSize
OpenOption::getOffset() const
{
	if (isGetByBitSet())
	{
		// ビットセットでの取得の場合、OFFSETは1に固定
		return 1;
	}
	
	ModSize offset = 1;
	int n;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(OpenOption::KeyID::Offset), n) == true)
	{
		offset = static_cast<ModSize>(n);
	}
	return offset;
}

//
//	FUNCTION public
//	FullText2::OpenOption::isEstimate -- 見積もりモードかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		見積もりモードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::isEstimate() const
{
	return m_cOpenOption.getBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::Estimate::Key));
}

//
//	FUNCTION public
//	FullText2::OpenOption::isGetByBitSet -- ビットセットによる取得かどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		ビットセットによる取得の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::isGetByBitSet() const
{
	return m_cOpenOption.getBoolean(
		_SYDNEY_OPEN_PARAMETER_KEY(FileCommon::OpenOption::GetByBitSet::Key));
}

//
//	FUNCTION public
//	FullText2::OpenOption::getBitSetForNarrowing
//		-- 検索結果の絞り込み用のビットセットを取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::BitSet*
//		絞り込み用のビットセット。設定されていない場合は null を返す
//
//	EXCEPTIONS
//
const Common::BitSet*
OpenOption::getBitSetForNarrowing() const
{
	const Common::BitSet* pBitSet = 0;
	
	const Common::Object* p = 0;
	if (m_cOpenOption.getObjectPointer(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::SearchByBitSet::Key), p) == true)
	{
		// ビットセットによる絞り込みがある場合
		 pBitSet = _SYDNEY_DYNAMIC_CAST(const Common::BitSet*, p);
	}
	
	return pBitSet;
}

//
//	FUNCTION public
//	FullText2::OpenOption::getBitSetForRanking
//		-- 絞り込んだ集合でランキング検索する用のビットセットを取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Common::BitSet*
//		絞り込んだ集合でランキング検索する用のビットセット
//		設定されていない場合は null を返す
//
//	EXCEPTIONS
//
const Common::BitSet*
OpenOption::getBitSetForRanking() const
{
	const Common::BitSet* pBitSet = 0;
	
	const Common::Object* p = 0;
	if (m_cOpenOption.getObjectPointer(
			_SYDNEY_OPEN_PARAMETER_KEY(
				FileCommon::OpenOption::RankByBitSet::Key), p) == true)
	{
		// 絞り込んだ集合でランキング検索するためのビットセットがある
		 pBitSet = _SYDNEY_DYNAMIC_CAST(const Common::BitSet*, p);
	}
	
	return pBitSet;
}

//
//	FUNCTION public static
//	FullText2::OpenOption::createScoreCombiner -- スコア合成器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cCombiner_
//		スコア合成器を表す文字列
//
//	REURN
//	FullText2::ScoreCombiner*
//		スコア合成器
//
//	EXCEPTIONS
//
ScoreCombiner*
OpenOption::createScoreCombiner(const ModUnicodeString& cCombiner_)
{
	ScoreCombiner* p = 0;
	
	if (cCombiner_.getLength() == 0 ||
		cCombiner_.compare(_Combiner_Sum, ModFalse) == 0)
	{
		p = new SumScoreCombiner();
	}
	else if (cCombiner_.compare(_Combiner_Max, ModFalse) == 0)
	{
		p = new MaxScoreCombiner();
	}
	else if (cCombiner_.compare(_Combiner_ASum, ModFalse) == 0)
	{
		p = new ASumScoreCombiner();
	}
	else if (cCombiner_.compare(_Combiner_Prod, ModFalse) == 0)
	{
		p = new ProdScoreCombiner();
	}
	else if (cCombiner_.compare(_Combiner_Min, ModFalse) == 0)
	{
		p = new MinScoreCombiner();
	}
	else
	{
		// オープン時にチェックされているので、ここに来ることはないはず
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return p;
}

//
//	FUNCTION public static
//	FullText2::OpenOption::createScoreCalculator -- スコア計算器を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cCombiner_
//		スコア計算器を表す文字列
//
//	REURN
//	FullText2::ScoreCombiner*
//		スコア計算器
//
//	EXCEPTIONS
//
ScoreCalculator*
OpenOption::createScoreCalculator(const ModUnicodeString& cCalculator_)
{
	ScoreCalculator* p = 0;
	
	if (cCalculator_.getLength() == 0 ||
		cCalculator_.compare(
			_Calculator_NormalizedOkapiTfIdf, ModFalse,
			_Calculator_NormalizedOkapiTfIdf.getLength()) == 0)
	{
		// NormalizedOkapiTfIdf
		const ModUnicodeChar* s = cCalculator_;
		if (*s != 0)
		{
			s += _Calculator_NormalizedOkapiTfIdf.getLength();
			if (*s != 0 && *s != ':')
				// エラー
				_SYDNEY_THROW1(Exception::WrongParameter, cCalculator_);
			if (*s == ':') ++s;
		}
		p = new NormalizedOkapiTfIdfScoreCalculator(s);
	}
	else if (cCalculator_.compare(
				 _Calculator_NormalizedOkapiTf, ModFalse,
				 _Calculator_NormalizedOkapiTf.getLength()) == 0)
	{
		// NormalizedOkapiTf
		const ModUnicodeChar* s = cCalculator_;
		s += _Calculator_NormalizedOkapiTf.getLength();
		if (*s != 0 && *s != ':')
			// エラー
			_SYDNEY_THROW1(Exception::WrongParameter, cCalculator_);
		if (*s == ':') ++s;
		p = new NormalizedOkapiTfScoreCalculator(s);
	}
	else if (cCalculator_.compare(
				 _Calculator_OkapiTfIdf, ModFalse,
				 _Calculator_OkapiTfIdf.getLength()) == 0)
	{
		// OkapiTfIdf
		const ModUnicodeChar* s = cCalculator_;
		s += _Calculator_OkapiTfIdf.getLength();
		if (*s != 0 && *s != ':')
			// エラー
			_SYDNEY_THROW1(Exception::WrongParameter, cCalculator_);
		if (*s == ':') ++s;
		p = new OkapiTfIdfScoreCalculator(s);
	}
	else if (cCalculator_.compare(
				 _Calculator_OkapiTf, ModFalse,
				 _Calculator_OkapiTf.getLength()) == 0)
	{
		// OkapiTf
		const ModUnicodeChar* s = cCalculator_;
		s += _Calculator_OkapiTf.getLength();
		if (*s != 0 && *s != ':')
			// エラー
			_SYDNEY_THROW1(Exception::WrongParameter, cCalculator_);
		if (*s == ':') ++s;
		p = new OkapiTfScoreCalculator(s);
	}
	else if (cCalculator_.compare(
				 _Calculator_NormalizedTfIdf, ModFalse,
				 _Calculator_NormalizedTfIdf.getLength()) == 0)
	{
		// NormalizedTfIdf
		const ModUnicodeChar* s = cCalculator_;
		s += _Calculator_NormalizedTfIdf.getLength();
		if (*s != 0 && *s != ':')
			// エラー
			_SYDNEY_THROW1(Exception::WrongParameter, cCalculator_);
		if (*s == ':') ++s;
		p = new NormalizedTfIdfScoreCalculator(s);
	}
	else if (cCalculator_.compare(
				 _Calculator_TfIdf, ModFalse,
				 _Calculator_TfIdf.getLength()) == 0)
	{
		// TfIdf
		const ModUnicodeChar* s = cCalculator_;
		s += _Calculator_TfIdf.getLength();
		if (*s != 0 && *s != ':')
			// エラー
			_SYDNEY_THROW1(Exception::WrongParameter, cCalculator_);
		if (*s == ':') ++s;
		p = new TfIdfScoreCalculator(s);
	}
	else if (cCalculator_.compare(
				 _Calculator_External, ModFalse,
				 _Calculator_External.getLength()) == 0)
	{
		// External
		const ModUnicodeChar* s = cCalculator_;
		s += _Calculator_External.getLength();
		if (*s != 0 && *s != ':')
			// エラー
			_SYDNEY_THROW1(Exception::WrongParameter, cCalculator_);
		if (*s == ':') ++s;
		p = new ExternalScoreCalculator(s);
	}
	else
	{
		// オープン時にチェックされているので、ここに来ることはないはず
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	return p;
}

//
//	FUNCTION public static
//	FullText2::OpenOption::getAdjustMethod
//		-- 文字列表記のスコア調整方法をスコア調整方法に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& method_
//		スコア調整方法の文字列表記
//
//	RETURN
//	FullText2::AdjustMethod::Value
//		スコア調整方法
//
//	EXCEPTIONS
//
AdjustMethod::Value
OpenOption::getAdjustMethod(const ModUnicodeString& method_)
{
	AdjustMethod::Value v = AdjustMethod::Unknown;
	
	if (method_.compare(_ScoreFunction_Sum, ModFalse))
		// 加算
		v = AdjustMethod::Add;
	else if (method_.compare(_ScoreFunction_Multiply, ModFalse))
		// 乗算
		v = AdjustMethod::Multiply;
	else if (method_.compare(_ScoreFunction_Replace, ModFalse))
		// 置き換え
		v = AdjustMethod::Replace;

	return v;
}

//
//	FUNCTION public
//	FullText2::OpenOption::checkScoreCombiner
//		-- 正しいスコア合成器か確認する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cCombiner_
//		確認するスコア合成器
//
//	RETURN
//	bool
//		正しいスコア合成器の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::checkScoreCombiner(const ModUnicodeString& cCombiner_)
{
	return _checkCombiner(cCombiner_);
}

//
//	FUNCTION private
//	FullText2::OpenOption::setContains -- containsの検索条件を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	bool
//		転置ファイルで実行できる条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setContains(const LogicalFile::TreeNodeInterface* pCondition_)
{
	using namespace LogicalFile;
	
	//
	//	Contains -- Operand -- Field
	//		|			|
	//		|			+----- <ContainsOperand>
	//		|
	//		+------ Option --- Expand
	//					|
	//					+----- Extractor
	//					|
	//					+----- Calculator
	//					|
	//					+----- Combiner
	//					|
	//					+----- AverageLength
	//					|
	//					+----- DF
	//					|
	//					+----- ScoreFunction
	//					|
	//					+----- ClusteredLimit
	//					|
	//					+----- ScoreCombiner
	//					|
	//					+----- ClusteredCombiner
	//
	//	<ContainsOperand>には以下のいずれかが来る
	//
	//	Pattern --- Operand -- ConstantValue or Variable
	//		|
	//		+------ Option --- Language
	//
	//	FreeText -- Operand -- ConstantValue or Variable
	//		|
	//		+------ Option --- Language
	//		|
	//		+------ Option --- ScaleParameter	(FullText2 only)
	//		|
	//		+------ Option --- WordLimit		(FullText2 only)
	//
	//	Head ------ Operand -- <ContainsOperand>
	//
	//	Tail ------ Operand -- <ContainsOperand>
	//
	//	ExactWord --- Operand -- Pattern
	//
	//	SimpleWord -- Operand -- Pattern
	//
	//	String ---- Operand -- Pattern
	//
	//	WordHead -- Operand -- Pattern
	//
	//	WordTail -- Operand -- Pattern
	//
	//	Within ---- Operand -- Pattern
	//		|			|
	//		|			+----- Pattern
	//		|			|
	//		|			...
	//		|
	//		+------ Option --- Symmetric (symmetric 1, asymmetric 0)
	//					|
	//					+----- Lower
	//					|
	//					+----- Upper
	//
	//	And ------- Operand -- <ContainsOperand>
	//					|
	//					+----- <ContainsOperand>
	//					|
	//					...
	//
	//	Or -------- Operand -- <ContainsOperand>
	//					|
	//					+----- <ContainsOperand>
	//					|
	//					...
	//		
	//	AndNot----- Operand -- <ContainsOperand>
	//					|
	//					+----- <ContainsOperand>
	//					|
	//					...
	//		
	//	Weight ---- Operand -- <ContainsOperand>
	//		|
	//		+------ Option --- Scale
	//
	//	WordList -- Operand -- Word
	//					|
	//					+----- Word
	//					|
	//					...
	//
	//	Word ------ Operand -- <ContainsOperand>
	//		|
	//		+------ Option --- Category
	//					|
	//					+----- Scale
	//					|
	//					+----- Language
	//
	// Synonym ---- Operand -- <ContainsOperand>
	//
	// ExpandSynonym - Operand -- <ContainsOperand>
	//

	if (pCondition_->getOperandSize() != 2)
		return false;

	const TreeNodeInterface* pField = pCondition_->getOperandAt(0);
	const TreeNodeInterface* pCond = pCondition_->getOperandAt(1);
	
	if (pField->getType() != TreeNodeInterface::Field &&
		pField->getType() != TreeNodeInterface::List)
	{
		// 順番が逆
		const TreeNodeInterface* pTmp = pField;
		pField = pCond;
		pCond = pTmp;
	}

	ModUnicodeOstrStream field;
	int fieldCount = -1;
	ModVector<int> vecOrgField;
	if (pField->getType() == TreeNodeInterface::List)
	{
		// 複数フィールドを対象にした検索である

		ModVector<int> v;
		fieldCount = pField->getOperandSize();
		v.reserve(fieldCount);
		for (int i = 0; i < fieldCount; ++i)
		{
			int f = FileCommon::DataManager::toInt(pField->getOperandAt(i));
			if (f >= m_iKeyCount)
				// キーを超えている
				return false;
			
			v.pushBack(f);
			vecOrgField.pushBack(f);
		}

		// 昇順にソートする
		ModSort(v.begin(), v.end(), ModLess<int>());

		field << '[';
		
		ModVector<int>::Iterator j = v.begin();
		for (int i = 0; i < fieldCount; ++i, ++j)
		{
			if (i != 0) field << ",";
			
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::SearchField, i), (*j));
			field << (*j);
		}

		field << ']';
		
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchFieldCount), fieldCount);
	}
	else
	{
		// 単一フィールドを対象にした検索である
		fieldCount = 1;
		
		int f = FileCommon::DataManager::toInt(pField);
		if (f >= m_iKeyCount)
			// キーを超えている
			return false;
		vecOrgField.pushBack(f);

		// 検索対象のフィールドを設定する
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchFieldCount), fieldCount);
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::SearchField, 0), f);
		field << f;
	}

	ModUnicodeString type;
	ModUnicodeString calculator;
	ModUnicodeString combiner;
	ModUnicodeString averageLength;
	ModUnicodeString documentFrequency;
	int wordLimit = -1;
	ModUnicodeString extractor;
	ModUnicodeString scoreMethod;
	double clusteredLimit = 0.0;
	ModUnicodeString scoreCombiner;
	bool isCat = false;
	ModUnicodeString clusteredCombiner;
	ModUnicodeString scale;
	
	// オプションをチェック
	for (int i = 0; i < static_cast<int>(pCondition_->getOptionSize()); ++i)
	{
		const TreeNodeInterface* pOption = pCondition_->getOptionAt(i);
		switch (pOption->getType())
		{
		case TreeNodeInterface::Calculator:
			calculator = pOption->getValue();
			if (_checkCalculator(calculator) == false)
				_SYDNEY_THROW1(Exception::WrongParameter, calculator);
			break;
		case TreeNodeInterface::Combiner:
			combiner = pOption->getValue();
			if (_checkCombiner(combiner) == false)
				_SYDNEY_THROW1(Exception::WrongParameter, combiner);
			break;
		case TreeNodeInterface::AverageLength:
			averageLength = pOption->getValue();
			if (_checkFloat(vecOrgField, averageLength) == false)
				_SYDNEY_THROW1(Exception::WrongParameter, averageLength);
			break;
		case TreeNodeInterface::Df:
			documentFrequency = pOption->getValue();
			if (_checkInt(vecOrgField, documentFrequency) == false)
				_SYDNEY_THROW1(Exception::WrongParameter, documentFrequency);
			break;
		case TreeNodeInterface::Expand:
			if (parseExpand(pOption, wordLimit) == false)
				return false;
			break;
		case TreeNodeInterface::Extractor:
			extractor = pOption->getValue();
			break;
		case TreeNodeInterface::ScoreFunction:
			scoreMethod = pOption->getValue();
			if (_checkScoreFunction(scoreMethod) == false)
				_SYDNEY_THROW1(Exception::WrongParameter, scoreMethod);
			break;
		case TreeNodeInterface::ClusteredLimit:
			// clusteredLimitには、類似度のしきい値が設定される
			if (m_bClustering == false)
				// 特徴語が抽出されていないと実行できない
				return false;
			clusteredLimit
				= FileCommon::DataManager::toDouble(pOption);	// doubleに変換
			break;
		case TreeNodeInterface::ScoreCombiner:
			scoreCombiner = pOption->getValue();
			if (_checkScoreCombiner(vecOrgField, scoreCombiner, type, scale)
				== false)
				_SYDNEY_THROW1(Exception::WrongParameter, scoreCombiner);
			break;
		case TreeNodeInterface::ClusteredCombiner:
			clusteredCombiner = pOption->getValue();
			if (_checkClusteredCombiner(clusteredCombiner) == false)
				_SYDNEY_THROW1(Exception::WrongParameter, clusteredCombiner);
			break;
		}
	}

	if (type.getLength() == 0)
	{
		// デフォルトを設定する
		
		type = (fieldCount == 1) ?
			_ScoreCombiner_Tea_Single : _ScoreCombiner_Tea_Cat;
	}
	if (fieldCount != 1)
	{
		// field に scale を加える

		field << "," << scale;
	}
	
	// tea構文にする
	//
	// 以下のパターンがある
	// #contains[single,<field number>,
	//			 <average length>,<document frequency>,...](...)
	// #contains[cat,[<field number>,...],[<scale>,...],
	//			 <average length>,<document frequency>,...](...)
	// #contains[(and|or),<score combiner>,[<field number>,...],[<scale>,...],
	//			 [<average length>,...],[<document frequency>,...],...](...)
	//
	// 最後の ... は以下の通り
	//		<calculator>,<combiner>,<expand limit>,
	//		<extractor>,<score function>,<clustered limit>

	ModUnicodeOstrStream cStream;
	cStream << "#contains[" << type << ",";
		// type によって (and|or),<score combiner> の場合がある
	cStream << field.getString() << ",";
		// type によって [<field number>,..],[<scale>,...] の場合がある
	cStream << averageLength << ",";
		// type によって [<average length>,...] の場合がある
	cStream << documentFrequency << ",";
		// type によって [<document frequency>,...] の場合がある
	cStream << calculator << ","
			<< combiner << ",";
	if (wordLimit != -1)
		cStream << wordLimit << ",";
	else
		cStream << ",";
	appendPattern(cStream, extractor);
	cStream	<< ","
			<< scoreMethod << ",";
	if (clusteredLimit != 0.0)
		cStream << clusteredLimit << "](";
	else
		cStream << "](";

	// 検索コストの見積もりに利用する
	int iTermCount = 0;
	
	switch (pCond->getType())
	{
	case TreeNodeInterface::Freetext:
		{
			if (convertFreeText(cStream, pCond, iTermCount) == false)
				return false;
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::FreeText);
		}
		break;
	case TreeNodeInterface::WordList:
		{
			if (convertWordList(cStream, pCond, iTermCount) == false)
				return false;
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::FreeText);
		}
		break;
	default:
		{
			if (convertContains(cStream, pCond, iTermCount) == false)
				return false;
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::Normal);
		}
		break;
	}

	cStream << ")";

	m_cOpenOption.setString(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::Condition),
		cStream.getString());
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::TermCount), iTermCount);
	
	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::setNormal -- 通常の検索条件を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	bool
//		転置ファイルで実行できる条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setNormal(const LogicalFile::TreeNodeInterface* pCondition_)
{
	int iTermCount = 0;

	bool bResult = false;
	ModUnicodeOstrStream cStream;

	// まずはすべての条件をたどって、同じフィールドに対する条件だけか確認する
	int field = checkField(pCondition_);
	if (field == -1)
		// 違うフィールドが含まれているので検索できない
		return false;

	cStream << "#contains[single," << field << ",,,,,,,,](";

	if (convertNormal(cStream, pCondition_, iTermCount) == true)
	{
		cStream << ")";
		
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::Normal);
		m_cOpenOption.setString(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::Condition),
			cStream.getString());
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::TermCount), iTermCount);
		
		// 検索対象のフィールドを設定する
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchFieldCount), 1);
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::SearchField, 0), field);
		
		bResult = true;
	}

	return bResult;
}

//
//	FUNCTION private
//	FullText2::OpenOption::setEqual -- 整合性検査のためのequal条件を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//
//	RETURN
//	bool
//		転置ファイルで実行できる条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setEqual(const LogicalFile::TreeNodeInterface* pCondition_)
{
	// 複合索引の場合には、修正が必要
	// 複数のフィールドがあり、かつ配列である可能性もあるので、
	// 現在のOpenOptionへのしまい方では対応不可
	
	using namespace LogicalFile;

	//
	//	整合性検査のためのequalの検索は以下のようになっている
	//

	// 言語指定ありの場合)
	//
	// 単一文書の場合
	// AND ---- Equals -------- Field(0)
	//	 |			|
	//	 |			+---------- ConstantValue/Variable("文字列1")
	//	 |
	//	 +----- Equals -------- Field(1)
	//	 |			|
	//	 |			+---------- ConstantValue/Variable("ja+en") ← 言語指定
	//	 |
	//	 +----- Equals -------- Field(2)
	//				|
	//				+---------- ConstantValue/Variable("1") ← ROWID=1の場合
	// 配列の場合
	// AND ---- Equals -------- Field(0)
	//	 |			|
	//	 |			+---------- List -------- ConstantValue/Variable("文字列1")
	//	 |						  |
	//	 |						  +---------- ConstantValue/Variable("文字列2")
	//	 |
	//	 +----- Equals -------- Field(1)
	//	 |			|
	//	 |			+---------- List -------- ConstantValue/Variable("ja+en")
	//	 |						  |
	//	 |						  +---------- ConstantValue/Variable("ja+en")
	//	 |
	//	 +----- Equals -------- Field(2)
	//				|
	//				+---------- ConstantValue/Variable("1") ← ROWID=1の場合

	// 言語指定なしの場合)
	//
	// 単一文書の場合
	// AND ---- Equals -------- Field(0)
	//	 |			|
	//	 |			+---------- ConstantValue/Variable("文字列1")
	//	 |
	//	 +----- Equals -------- Field(1)
	//				|
	//				+---------- ConstantValue/Variable("1") ← ROWID=1の場合
	// 配列の場合
	// AND ---- Equals -------- Field(0)
	//	 |			|
	//	 |			+---------- List -------- ConstantValue/Variable("文字列1")
	//	 |						  |
	//	 |						  +---------- ConstantValue/Variable("文字列2")
	//	 |
	//	 +---- Equals -------- Field(1)
	//			   |
	//			   +---------- ConstantValue/Variable("1") ← ROWID=1の場合

	int iRowID = 0;
	ModVector<ModUnicodeString> vecStrValue;
	ModVector<ModLanguageSet> vecLanguageSet;

	if (pCondition_->getType() != TreeNodeInterface::And
		|| pCondition_->getOperandSize() !=
				static_cast<ModSize>(m_iRowIDField + 1))
		// 上記の入力パターンを参照
		return false;

	for (int i = 0; i < static_cast<int>(pCondition_->getOperandSize()); ++i)
	{
		const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);

		if (pNode->getType() != TreeNodeInterface::Equals
			|| pNode->getOperandSize() != 2)
			// 上記の入力パターンを参照
			return false;
		
		const TreeNodeInterface* pField = pNode->getOperandAt(0);
		const TreeNodeInterface* pValue = pNode->getOperandAt(1);

		if (pField->getType() != TreeNodeInterface::Field
			&& pValue->getType() == TreeNodeInterface::Field)
		{
			// FieldがValueの位置に設定されているので、入れ替える。
			const TreeNodeInterface* p = pField;
			pField = pValue;
			pValue = p;
		}

		if (FileCommon::DataManager::toInt(pField) == 0)
		{
			// 文字列データ
			if (setStringData(pValue, vecStrValue) == false)
				return false;
		}
		else if (FileCommon::DataManager::toInt(pField) == m_iRowIDField)
		{
			// タプルID
			if (setTupleID(pValue, iRowID) == false)
				return false;
		}
		else if (FileCommon::DataManager::toInt(pField) == m_iLanguageField)
		{
			// 言語情報
			if (setLanguageData(pValue, vecLanguageSet) == false)
				return false;
		}
		else
		{
			return false;
		}
	}

	// 設定する
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::Equal);
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::RowID), iRowID);

	for (int i = 0; i < static_cast<int>(vecStrValue.getSize()); ++i)
	{
		// 文字列を設定
		m_cOpenOption.setString(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
			KeyID::SectionValue, i), vecStrValue[i]);

		// 言語情報を設定
		if (vecLanguageSet.getSize() == 0)
			m_cOpenOption.setString(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				KeyID::SectionLang, i), m_cDefaultLanguage);
		else
			m_cOpenOption.setString(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
				KeyID::SectionLang, i), vecLanguageSet[i].getName());
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertFreeText
//		-- TreeNodeInterfaceから自然文情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		変換結果を格納するストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	int& iTermCount_
//		検索語数
//
//	RETURN
//	bool
//		転置で実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertFreeText(ModUnicodeOstrStream& cStream_,
							const LogicalFile::TreeNodeInterface* pCondition_,
							int& iTermCount_)
{
	using namespace LogicalFile;

	//
	//	FreeText -- Operand -- Variable or ConstantValue
	//		|
	//		+------ Option --- Language
	//		|
	//		+------ Option --- ScaleParameter
	//		|
	//		+------ Option --- WordLimit
	//

	//	#freetext[matchMode,language,scaleParameter,wordLimit](...)

	if (pCondition_->getOperandSize() != 1
		|| pCondition_->getOptionSize() > 3)
		return false;
	
	cStream_ << "#freetext[";
	
	ModUnicodeString matchMode;
	ModUnicodeString lang;
	ModUnicodeString scaleParameter;
	ModUnicodeString wordLimit;
	
	switch (m_eIndexType)
	{
	case IndexingType::Dual:
		matchMode = 'm';
		break;
	case IndexingType::Word:
		matchMode = 'e';
		break;
	case IndexingType::Ngram:
		matchMode = 'n';
		break;
	}

	if (pCondition_->getOptionSize())
	{
		for (int i = 0; i < static_cast<int>(pCondition_->getOptionSize()); ++i)
		{
			const TreeNodeInterface* pOption = pCondition_->getOptionAt(i);
			if (pOption->getType() == TreeNodeInterface::Language)
			{
				lang = ModLanguageSet(pOption->getValue()).getName();
			}
			else if (pOption->getType() == TreeNodeInterface::ScaleParameter)
			{
				scaleParameter = pOption->getValue();
			}
			else if (pOption->getType() == TreeNodeInterface::WordLimit)
			{
				wordLimit = pOption->getValue();
			}
			else
			{
				// エラー
				_TRMEISTER_THROW0(Exception::NotSupported);
			}
		}
	}
		
	cStream_ << matchMode << ","
			 << lang << ","
			 << scaleParameter << ","
			 << wordLimit;

	const TreeNodeInterface* pNode = pCondition_->getOperandAt(0);

	// コピーが発生するが、インタフェース的にそうなっているのでしょうがない
	ModUnicodeString v = pNode->getValue();
	
	cStream_ << "](";
	appendPattern(cStream_, v);
	cStream_ << ")";

	// iTermCount_ は検索コストの見積もりに利用するだけなので、ざっくりでいい
	// 概念検索の場合には、文字列長 / 20 とする
	
	iTermCount_ += ((v.getLength() + 19) / 20);

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertWordList
//		-- TreeNodeInterfaceから単語リストを得る
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		変換結果を格納するストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	int& iTermCount_
//		検索語数
//
//	RETURN
//	bool
//		転置で実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertWordList(ModUnicodeOstrStream& cStream_,
							const LogicalFile::TreeNodeInterface* pCondition_,
							int& iTermCount_)
{
	using namespace LogicalFile;

	//
	//	WordList -- Operand -- Word
	//

	//	#wordlist[10](...)

	if (pCondition_->getOperandSize() == 0)
		return false;

	int n = static_cast<int>(pCondition_->getOperandSize());
	cStream_ << "#wordlist[" << n << "](";

	for (int i = 0; i < n; ++i)
	{
		const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);

		if (convertWord(cStream_, pNode, iTermCount_) == false)
			return false;

	}
	
	cStream_ << ")";

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::parseExpand
//		-- TreeNodeInterfaceから拡張情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	int& iExpandLimit_
//		拡張語上限
//
//	RETURN
//	bool
//		転置で実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseExpand(const LogicalFile::TreeNodeInterface* pCondition_,
						int& iExpandLimit_)
{
	using namespace LogicalFile;

	//
	//	Expand --- Operand --- ???
	//		|
	//		+------ Option --- SortKey
	//					|
	//					+----- Limit
	//

	for (int i = 0; i < static_cast<int>(pCondition_->getOptionSize()); ++i)
	{
		const TreeNodeInterface* pOption = pCondition_->getOptionAt(i);
		if (pOption->getType() == TreeNodeInterface::Limit)
		{
			iExpandLimit_ = FileCommon::DataManager::toInt(pOption);
		}
		// [NOTE] SortKeyは無視
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertNormal
//		-- 1つのTeeNodeInerfaceをTea構文に変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		変換結果を書き込むストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	int& iTermCount_
//		検索語数(検索コストの見積もりに利用する)
//
//	RETURN
//	bool
//		転置で実行可能な検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertNormal(ModUnicodeOstrStream& cStream_,
						  const LogicalFile::TreeNodeInterface* pCondition_,
						  int& iTermCount_)
{
	using namespace LogicalFile;

	bool bResult = false;

	switch (pCondition_->getType())
	{
	case TreeNodeInterface::And:
		{
			// A and B and not C and not D を
			// #and-not(#and-not(#and(A,B),C),D) にする
			// A and not B なら
			// #and-not(A,B) にする
			
			bool allNot = true;
			int countNot = 0;
			int i;
			int count = static_cast<int>(pCondition_->getOperandSize());
			// not の数を数える
			for (i = 0; i < count; ++i)
			{
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (pNode->getType() != TreeNodeInterface::Not)
					allNot = false;
				else
					++countNot;
			}

			if (allNot == true) return false;	// 全部notである

			for (i = 0; i < countNot; ++i)
			{
				cStream_ << "#and-not(";
			}

			// and-not以外の処理
			if ((count - countNot) > 1)
				cStream_ << "#and(";
			for (i = 0; i < count; ++i)
			{
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (pNode->getType() != TreeNodeInterface::Not)
				{
					if (i != 0)
						cStream_ << ",";
					if (convertNormal(cStream_, pNode, iTermCount_) == false)
						return false;
					allNot = false;
				}
			}
			if ((count - countNot) > 1)
				cStream_ << ")";

			// and-notの処理
			for (i = 0; i < count; ++i)
			{
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (pNode->getType() == TreeNodeInterface::Not)
				{
					cStream_ << ",";
					pNode = pNode->getOperandAt(0);
					if (convertNormal(cStream_, pNode, iTermCount_) == false)
						return false;
					cStream_ << ")";
				}
			}
		}
		bResult = true;
		break;
	case TreeNodeInterface::Or:
		{
			cStream_ << "#or(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			int i;
			for (i = 0; i < count; ++i)
			{
				if (i != 0)
					cStream_ << ",";
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertNormal(cStream_, pNode, iTermCount_) == false)
					return false;
			}
			cStream_ << ")";
		}
		bResult = true;
		break;
	case TreeNodeInterface::Like:
		{
			if (convertLike(cStream_, pCondition_, iTermCount_) == false)
				return false;
		}
		bResult = true;
		break;
	case TreeNodeInterface::Equals:
		{
			if (convertEqual(cStream_, pCondition_, iTermCount_) == false)
			{
				// フラグを設定する
				m_bEqual = true;
				return false;
			}
		}
		bResult = true;
		break;
	case TreeNodeInterface::Not:
	default:
		;
	}

	return bResult;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertLike -- likeを変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		変換結果を書き込むストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		条件
//	int& iTermCount_
//		検索語数
//
//	RETURN
//	bool
//		転置で実行可能な検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertLike(ModUnicodeOstrStream& cStream_,
						const LogicalFile::TreeNodeInterface* pCondition_,
						int& iTermCount_)
{
	using namespace LogicalFile;

	//
	//	Like -- Operand -- Field
	//    |			|
	//    |			+----- Variable or ConstantValue
	//    |
	//    +---- Option --- Variable or ConstantValue
	//				|		(Escape -> Common::StringData)
	//            	|
	//				+----- Language
	//						(Language -> Common::LanguageData)
	//

	if (m_bNolocation == true)
	{
		// 位置情報が格納されていない場合、LIKEは検索できない。
		
		// 前提として、索引や列のヒント句等による定義により、
		// 実行結果が変わるのは問題なく、
		// 索引を使うか使わないかで、実行結果が変わるのは問題だ、
		// ということがある。
		// 後者は、オプティマイザが判断し、利用者が指定できないためである。
		
		// LIKEは、索引を使っても使わなくても実行できるため、
		// 索引の使用によらず、実行結果は同一である必要がある。
		
		// 位置情報が格納されていなくても検索できる条件は、
		// 索引語長と同じ長さの完全一致検索のみである。
		// 全文索引を使った検索では、そのような検索はまれだと思われる。
		// したがって、全てのLIKE検索ができないこととする。
		
		return false;
	}
	
	if (pCondition_->getOperandSize() != 2)
		return false;

	const TreeNodeInterface* pField = pCondition_->getOperandAt(0);
	const TreeNodeInterface* pValue = pCondition_->getOperandAt(1);
	if (pField->getType() != TreeNodeInterface::Field)
	{
		// 順番が逆
		const TreeNodeInterface* pTmp = pField;
		pField = pValue;
		pValue = pTmp;
	}

	// フィールド番号
	int f = FileCommon::DataManager::toInt(pField);
	if (f >= m_iKeyCount)
		// キーを超えている
		return false;

	// 検索文字列をtea構文の変換する

	ModUnicodeString cstrEscape;
	ModUnicodeString cstrLanguage;
	if (pCondition_->getOptionSize())
	{
		bool bLang = false;
		for (int i = 0; i < static_cast<int>(pCondition_->getOptionSize()); ++i)
		{
			const TreeNodeInterface* pOption = pCondition_->getOptionAt(i);
			if (pOption->getType() == TreeNodeInterface::Language)
			{
				cstrLanguage = pOption->getValue();
				bLang = true;
			}
			else
			{
				cstrEscape = pOption->getValue();
				if (cstrEscape.getLength() != 1)
					_SYDNEY_THROW0(Exception::InvalidEscape);
			}
		}
		if (bLang == false)
			cstrLanguage = m_cDefaultLanguage;
	}

	if (convertValue(cStream_,
					 pValue->getValue(), cstrEscape, cstrLanguage,
					 iTermCount_) == false)
		return false;

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertEqual -- equalを変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOsrStream& cStream_
//		変換結果を書き込むストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		条件
//	int& iTermCount_
//		検索語数
//
//	RETURN
//	bool
//		転置で実行可能な検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertEqual(ModUnicodeOstrStream& cStream_,
						 const LogicalFile::TreeNodeInterface* pCondition_,
						 int& iTermCount_)
{
	return false;
}

//
//	FUNCTION public
//	FullText2::OpenOption::convertValue -- 検索語を変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOsrStream& cStream_
//		変換結果を書き込むストリーム
//	const ModUnicodeString& cstrValue_
//		検索語
//	const ModUnicodeString& cstrEscape_
//		エスケープ文字
//	const ModUnicodeString& cstrLanguage_,
//		言語指定
//	int& iTermCount_
//		検索語数
//
//	RETURN
//	bool
//		転置で実行可能な検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertValue(ModUnicodeOstrStream& cStream_,
						 const ModUnicodeString& cstrValue_,
						 const ModUnicodeString& cstrEscape_,
						 const ModUnicodeString& cstrLanguage_,
						 int& iTermCount_)
{
	using namespace Common;

	// 言語情報チェック
	ModUnicodeString cLang;
	if (cstrLanguage_.getLength() != static_cast<ModSize>(0))
	{
		try
		{
			ModLanguageSet cLangSet(cstrLanguage_);
			cLang = cLangSet.getName();
		}
		catch (ModException&)
		{
			Common::Thread::resetErrorCondition();
			return false;
		}
	}

	// マッチモード
	ModUnicodeChar matchMode;
	switch (m_eIndexType)
	{
	case IndexingType::Dual:
		matchMode = 'm';
		break;
	case IndexingType::Word:
		matchMode = 'e';
		break;
	case IndexingType::Ngram:
		matchMode = 'n';
		break;
	}

	// 検索語数を更新する
	iTermCount_++;

	ModUnicodeChar escape = 0;
	if (cstrEscape_.getLength())
		escape = cstrEscape_[0];

	ModVector<Term> vecTerm;

	// '%'で文字列を分割する
	TermType type = separate(cstrValue_, escape, vecTerm);
	if (type == Error) return false;

	// 文字列検索
	if (vecTerm.getSize() > 1)
	{
		// '%'が含まれていた

		// 複数の文字列から構成されるので、
		// windowで出現順を指定する(距離は無制限)
		cStream_ << "#window[" << 0 << ","
				 << Os::Limits<int>::getMax() << "](";
	}

	//
	// 分割された各文字列を変換する
	//
	for (ModVector<Term>::Iterator i = vecTerm.begin(); i != vecTerm.end(); ++i)
	{
		if (i != vecTerm.begin())
			cStream_ << ",";

		ModUnicodeOstrStream cNewStream;
		// f は、部分文字列の前方にある'_'の数 - 1。
		// e は、部分文字列の後方にある'_'の数。
		// fを-1しているのは、
		// そのまま前方、後方一致で何文字目から一致するかに使うため
		int f = 1;
		int e = 0;
		//
		// 前処理
		// f, e が変更される可能性があるので、事前に処理する。
		//
		if ((*i).m_bRegrex == true)
		{
			// '_'が含まれている場合
			if (convertDistance(cNewStream,	(*i).m_pszValue, (*i).m_uiLength,
								escape,	cLang, f, e) == false)
				return false;
		}

		//
		// まず、後方一致と前方一致
		//
		if ((*i).m_bBack == false)
		{
			// 後方一致
			cStream_ << "#end[" << e << "](";
		}
		if ((*i).m_bFront == false)
		{
			// 前方一致
			cStream_ << "#location[" << f << "](";
		}

		//
		// 検索語を設定
		//
		if ((*i).m_bRegrex == true)
		{
			// '_'が含まれている場合

			// 前処理で解析済みデータを設定
			cStream_ << cNewStream.getString();
		}
		else
		{
			// '_'がない場合
			const ModUnicodeChar* p = (*i).m_pszValue;
			cStream_ << "#term[" << matchMode << ",," << cLang << "](";
			unsigned int l = 0;
			for (; l < (*i).m_uiLength; p++, l++)
			{
				if (*p == escape)
				{
					p++;
					l++;
				}
				if (isEscapeChar(*p))
					// エスケープ
					cStream_ << "\\";
				if (*p) cStream_ << *p;
			}
			cStream_ << ")";
		}

		//
		// 前方一致、後方一致の順で閉じる
		//
		if ((*i).m_bFront == false)
		{
			// 前方一致
			cStream_ << ")";
		}
		if ((*i).m_bBack == false)
		{
			// 後方一致
			cStream_ << ")";
		}
	}

	if (vecTerm.getSize() > 1)
	{
		cStream_ << ")";
	}
	
	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertDistance
//		-- '_'が含まれる文字列を#windowに変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		出力ストリーム
//	const ModUnicodeChar* pValue_
//		文字列
//	ModSize uiLength_
//		文字列長
//	ModUnicodeChar escape_
//		エスケープ文字
//	const ModUnicodeString& cLang_
//		言語情報
//	int& f_
//		前方に存在する'-'の数
//	int& e_
//		後方に存在する'-'の数
//
//	RETURN
//	bool
//		転置ファイルで実行できるものの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertDistance(ModUnicodeOstrStream& cStream_,
							const ModUnicodeChar* pValue_,
							ModSize uiLength_,
							ModUnicodeChar escape_,
							const ModUnicodeString& cLang_,
							int& f_, int& e_)
{
	using namespace Common;

	if (m_eIndexType == IndexingType::Word)
		// Word索引では'_'は使用できない
		return false;

	const ModUnicodeChar* p = pValue_;

	// 前後の'_'は無視する -> 本当は無視しちゃだめだけどしょうがない
	while (*p == '_')
	{
		f_++;
		p++;
	}

	if ((p - pValue_) >= static_cast<int>(uiLength_))
		return false;	// '_'しかない

	Entry* pEntry = new Entry;
	ModAutoPointer<Entry> pHead = pEntry;

	const ModUnicodeChar* s = p;
	int position = 0;
	bool bPrev = false;

	ModUnicodeChar matchMode;
	if (m_eIndexType == IndexingType::Dual)
		matchMode = 'm';
	else
		matchMode = 'n';

	//
	// まず'_'で分割する
	//
	while ((p - pValue_) < static_cast<int>(uiLength_))
	{
		if (*p == UnicodeChar::usLowLine)
		{
			if (bPrev == false)
			{
				// 直前の文字は'_'以外
				
				Entry* pTmp = new Entry;
				pEntry->m_pNext = pTmp;
				pEntry = pTmp;
				position = 1;
				bPrev = true;
			}
			else
			{
				// 直前の文字は'_'

				// positionは、隣の部分文字列と離れている文字数を示す
				position++;
			}
		}
		else
		{
			if (*p == escape_)
				p++;

			// 離れている文字数と、文字を設定する
			if (pEntry->m_uiPosition == 0)
				pEntry->m_uiPosition = position;
			pEntry->m_cstrValue += *p;

			// 解析状況を更新する
			bPrev = false;
			position = 0;
		}

		p++;
	}
	// '_'で終わっていた場合、e_を進める
	e_ += position;

	//
	// tea構文に変換する 
	//

	// 先頭要素は、距離の制約がないので、別処理。
	pEntry = pHead;
	// まず先頭要素の特殊文字をエスケープする
	{
		ModUnicodeOstrStream cTmp;
		const ModUnicodeChar* p = pEntry->m_cstrValue;
		cTmp << "#term[" << matchMode << ",," << cLang_ << "](";
		while (*p != 0)
		{
			if (isEscapeChar(*p) == true)
				cTmp << '\\';
			cTmp << *p++;
		}
		cTmp << ")";
		pEntry->m_cstrValue = cTmp.getString();
	}

	// 各エントリを処理する
	// 次のエントリを得る
	Entry* pNext = pEntry->m_pNext;
	while (pNext && pNext->m_cstrValue.getLength() != 0)
	{
		// 前のエントリとの距離の制約を設定
		ModUnicodeOstrStream cTmp;
		cTmp << "#window[" << pNext->m_uiPosition + 1
			<< "," << pNext->m_uiPosition + 1 << "](";
		cTmp << pEntry->m_cstrValue;
		cTmp << ",";

		// 例えば、'A_B_C'は以下の様な入れ子になる。
		// #window[*,*](
		//		#window[*,*](#term[*,,*](A),#term[*,,*](B)),
		//		#term[*,,*](C))
		cTmp << "#term[" << matchMode << ",," << cLang_ << "](";
		const ModUnicodeChar* p = pNext->m_cstrValue;
		while (*p != 0)
		{
			if (isEscapeChar(*p) == true)
				cTmp << '\\';
			cTmp << *p++;
		}
		cTmp << "))";

		// 次へ
		pNext->m_cstrValue = cTmp.getString();
		pEntry = pNext;
		pNext = pEntry->m_pNext;
	}
	cStream_ << pEntry->m_cstrValue;

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::separate -- '%'で文字を分割する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeChar* pszValue_
//		分割する文字列
//	ModUnicodeChar cEscape_
//		'%'をエスケープする文字
//	ModVector<Term>& vecTerm_
//		分割結果
//
//	RETURN
//	FullText2::OpenOption::TermType
//		 分割結果に応じた結果
//
//	EXCEPTIONS
//
OpenOption::TermType
OpenOption::separate(const ModUnicodeChar* pszValue_,
					 ModUnicodeChar cEscape_,
					 ModVector<Term>& vecTerm_)
{
	using namespace Common;

	// 現状、TermTypeは分割できたかどうかを
	// Errorかどうかで判断するだけなので、
	// Undefinedも分割できたことを示す。

	const ModUnicodeChar* p = pszValue_;
	const ModUnicodeChar* s = p;
	TermType eType = Undefined;
	bool bRegex = false;
	bool bFront = false;

	while (*p != 0)
	{
		if (*p == cEscape_)
		{
			p++;
			if (*p != 0) p++;
			continue;
		}
		
		switch (*p)
		{
		case UnicodeChar::usPercent:
			if (s != p)
			{
				if ((p - s) > _cQueryMaxLen.get())
				{
					// 検索語の最大長を超えた
					_SYDNEY_THROW0(Exception::TooLongConditionalPattern);
				}

				// 分割文字列を格納する
				
				// Termに設定されるFrontやEndのbool値は、
				// 前方や後方に'%'があるかどうか、
				// つまり、任意の文字列が入りうるかどうかを示す。
				
				vecTerm_.pushBack(Term(s, static_cast<ModSize>(p - s),
									   bRegex, bFront, true));

				// 次の部分文字列の初期値を設定する
				bRegex = false;
				bFront = true;
				eType = String;
			}
			else
			{
				// 分割文字列の先頭が'%'、つまり分割文字列は'%'だけなので、
				// 格納せず次へ

				// 次の部分文字列の初期値を設定する
				bFront = true;
			}

			// 部分文字列の先頭を更新する
			s = ++p;
			break;

		case UnicodeChar::usLowLine:
			// separate内では'_'は一般の文字と同様に処理する
			// ただし、'_'が含まれていることを示すフラグは立てる
			bRegex = true;
			eType = String;
		default:
			p++;
		}
	}
	
	//
	// 残りの部分文字列を格納する
	//
	if (s != p && !(p - s == 1 && *s == cEscape_))
	{
		// 残りの部分文字列が、エスケープ文字だけだった場合は無視する
		
		if ((p - s) > _cQueryMaxLen.get())
		{
			// 検索語の最大長を超えた
			_SYDNEY_THROW0(Exception::TooLongConditionalPattern);
		}
		vecTerm_.pushBack(Term(s, static_cast<ModSize>(p - s),
							   bRegex, bFront, false));
	}

	return (vecTerm_.getSize() ? eType : Error);
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertContains
//		-- Contains Operandをtea構文に変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOsrStream& cStream_
//		変換結果を書き込むストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		変換する条件
//	int& iTermCount_
//		タームの数
//
//	RETURN
//	bool
//		転置索引で検索できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertContains(ModUnicodeOstrStream& cStream_,
							const LogicalFile::TreeNodeInterface* pCondition_,
							int& iTermCount_)
{
	using namespace LogicalFile;

	bool bResult = false;

	switch (pCondition_->getType())
	{
	case TreeNodeInterface::Within:
		{
			if (m_bNolocation == false)
			{
				// 位置情報が格納されていない場合、
				// この指定を無視してもよいが、
				// そもそもNolocationを指定した時に、
				// この条件は指定されないと思われるので、
				// 仕様の簡便さとテストの手間から、
				// サポートしないことにする。
				
				int upper = -1;
				int lower = 0;
				ModUnicodeChar order = 'u';

				// オプションを設定する
				for (int i = 0;
					 i < static_cast<int>(pCondition_->getOptionSize()); ++i)
				{
					const TreeNodeInterface* pOption =
						pCondition_->getOptionAt(i);
					switch (pOption->getType())
					{
					case TreeNodeInterface::Upper:
						upper = FileCommon::DataManager::toInt(pOption);
						break;
					case TreeNodeInterface::Lower:
						lower = FileCommon::DataManager::toInt(pOption);
						break;
					case TreeNodeInterface::Symmetric:
						if (FileCommon::DataManager::toInt(pOption) == 1)
							order = 'o';
						break;
					default:
						return false;
					}
				}

				if (upper == -1)
				{
					// upperがないとエラー
					return false;
				}

				cStream_ << "#window[" << lower << "," << upper << "," << order
						 << "](";
			
				int count = static_cast<int>(pCondition_->getOperandSize());
				for (int i = 0; i < count; ++i)
				{
					if (i != 0)
						cStream_ << ",";
					const TreeNodeInterface* pNode =
						pCondition_->getOperandAt(i);
					if (convertContains(cStream_, pNode, iTermCount_) == false)
						return false;
				}

				cStream_ << ")";
			
				bResult = true;
			}
		}
		break;
	case TreeNodeInterface::And:
		{
			cStream_ << "#and";
			if (pCondition_->getOptionSize())
			{
				if (pCondition_->getOptionSize() != 1) return false;
				const TreeNodeInterface* pOption = pCondition_->getOptionAt(0);
				ModUnicodeString combiner = pOption->getValue();
				if (_checkCombiner(combiner) == false) return false;
				cStream_ << "[" << combiner << "]";
			}
			cStream_ << "(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < count; ++i)
			{
				if (i != 0)
					cStream_ << ",";
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(cStream_, pNode, iTermCount_) == false)
					return false;
			}
			cStream_ << ")";
			bResult = true;
		}
		break;
	case TreeNodeInterface::Or:
		{
			cStream_ << "#or";
			if (pCondition_->getOptionSize())
			{
				if (pCondition_->getOptionSize() != 1) return false;
				const TreeNodeInterface* pOption = pCondition_->getOptionAt(0);
				ModUnicodeString combiner = pOption->getValue();
				if (_checkCombiner(combiner) == false) return false;
				cStream_ << "[" << combiner << "]";
			}
			cStream_ << "(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < count; ++i)
			{
				if (i != 0)
					cStream_ << ",";
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(cStream_, pNode, iTermCount_) == false)
					return false;
			}
			cStream_ << ")";
			bResult = true;
		}
		break;
	case TreeNodeInterface::AndNot:
		{
			// #and-not(#and-not(A,B),C) の様な入れ子にする
			
			ModUnicodeString combiner;
			if (pCondition_->getOptionSize())
			{
				if (pCondition_->getOptionSize() != 1) return false;
				const TreeNodeInterface* pOption = pCondition_->getOptionAt(0);
				combiner = pOption->getValue();
				if (_checkCombiner(combiner) == false) return false;
			}
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < (count - 1); ++i)
			{
				cStream_ << "#and-not(";
			}
			for (int i = 0; i < count; ++i)
			{
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(cStream_, pNode, iTermCount_) == false)
					return false;
				if (i == 0)
				{
					// 例えば、CONTAINS A-B-C の A の条件
					cStream_ << ",";
				}
				else
				{
					// 例えば、CONTAINS A-B-C の B,C の条件
					cStream_ << ")";
				}
			}
			bResult = true;
		}
		break;
	case TreeNodeInterface::Pattern:
		{
			ModUnicodeChar m;
			switch (m_eIndexType)
			{
			case IndexingType::Dual:
				m = 'm';
				break;
			case IndexingType::Word:
				m = 'e';
				break;
			case IndexingType::Ngram:
				m = 'n';
				break;
			}
			if (convertPattern(cStream_, pCondition_, m) == true)
			{
				iTermCount_++;
				bResult = true;
			}
		}
		break;
	case TreeNodeInterface::Head:
		{
			if (pCondition_->getOperandSize() == 1
				&& m_bNolocation == false)
			{
				// 位置情報が格納されていない場合、
				// この指定を無視しても良いが、
				// Withinと同様にサポートしないことにする。

				cStream_ << "#location[1](";
				if (convertContains(cStream_,
									pCondition_->getOperandAt(0),
									iTermCount_) == true)
				
				{
					bResult = true;
				}
				cStream_ << ")";
			}
		}
		break;
	case TreeNodeInterface::Tail:
		{
			if (pCondition_->getOperandSize() == 1
				&& m_bNolocation == false)
			{
				// 位置情報が格納されていない場合、
				// この指定を無視しても良いが、
				// Withinと同様にサポートしないことにする。
				
				cStream_ << "#end[0](";
				if (convertContains(cStream_,
									pCondition_->getOperandAt(0),
									iTermCount_) == true)
				{
					bResult = true;
				}
				cStream_ << ")";
			}
		}
		break;
	case TreeNodeInterface::ExactWord:
		{
			if (pCondition_->getOperandSize() == 1
				&& m_bNolocation == false)
			{
				// 位置情報が格納されていない場合、
				// この指定を無視してWordListとして扱っても良いが、
				// Withinと同様にサポートしないことにする。
				
				ModUnicodeChar m = 'e';
				if (convertPattern(cStream_, pCondition_->getOperandAt(0), m)
					== true)
				{
					iTermCount_++;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::SimpleWord:
		{
			if (pCondition_->getOperandSize() == 1)
			{
				ModUnicodeChar m = 's';
				if (convertPattern(cStream_, pCondition_->getOperandAt(0), m)
					== true)
				{
					iTermCount_++;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::String:
		{
			if (pCondition_->getOperandSize() == 1)
			{
				ModUnicodeChar m = 'n';
				if (convertPattern(cStream_, pCondition_->getOperandAt(0), m)
					== true)
				{
					iTermCount_++;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::WordHead:
		{
			if (pCondition_->getOperandSize() == 1
				&& m_bNolocation == false)
			{
				// 位置情報が格納されていない場合、
				// この指定を無視してWordListとして扱っても良いが、
				// Withinと同様にサポートしないことにする。
				
				ModUnicodeChar m = 'h';
				if (convertPattern(cStream_, pCondition_->getOperandAt(0), m)
					== true)
				{
					iTermCount_++;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::WordTail:
		{
			if (pCondition_->getOperandSize() == 1)
			{
				ModUnicodeChar m = 't';
				if (convertPattern(cStream_, pCondition_->getOperandAt(0), m)
					== true)
				{
					iTermCount_++;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::Weight:
		{
			if (pCondition_->getOperandSize() == 1
				&& pCondition_->getOptionSize() == 1)
			{
				
				cStream_ << "#scale["
						<< pCondition_->getOptionAt(0)->getValue()
						<< "](";
				if (convertContains(cStream_, pCondition_->getOperandAt(0),
									iTermCount_) == true)
				{
					bResult = true;
				}
				cStream_ << ")";
			}
		}
		break;
	case TreeNodeInterface::Synonym:
		{
			cStream_ << "#syn(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < count; ++i)
			{
				if (i != 0)
					cStream_ << ",";
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(cStream_, pNode, iTermCount_) == false)
					return false;
			}
			cStream_ << ")";
			bResult = true;
		}
		break;
	case TreeNodeInterface::ExpandSynonym:
		{
			// 展開語を取得する
			ModVector<ModUnicodeString> exp;
			if (getExpandSynonym(pCondition_->getOperandAt(0), exp) == false)
				return false;
			if (exp.getSize() > 1)
			{
				// 展開されたので、#syn に変換する
				
				cStream_ << "#syn(";
				ModVector<ModUnicodeString>::Iterator i = exp.begin();
				for (; i != exp.end(); ++i)
				{
					// 展開語の設定する
					m_cExpandWord = *i;

					// tea構文に変換する
					if (i != exp.begin())
						cStream_ << ",";
					const TreeNodeInterface* pNode
						= pCondition_->getOperandAt(0);
					if (convertContains(cStream_, pNode, iTermCount_) == false)
						return false;
				}
				m_cExpandWord.clear();
				cStream_ << ")";
			}
			else
			{
				// 展開されなかったのでそのまま

				if (convertContains(cStream_, pCondition_->getOperandAt(0),
									iTermCount_) == false)
					return false;
			}
			bResult = true;
		}
		break;
	}

	return bResult;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertPattern -- patternをtea構文に変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		変換結果を書き込むストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeChar match_
//	   マッチモード
//
//	RETURN
//	bool
//		転置索引で実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertPattern(ModUnicodeOstrStream& cStream_,
						   const LogicalFile::TreeNodeInterface* pCondition_,
						   ModUnicodeChar match_)
{
	using namespace LogicalFile;
	
	//	Pattern --- Operand -- ConstantValue or Variable
	//		|
	//		+------ Option --- Language
	//

	if (pCondition_->getOperandSize() != 1)
		return false;

	// matchモードを決定する
	switch (m_eIndexType)
	{
	case IndexingType::Dual:
		if (match_ == 0)
			match_ = 'm';
		break;
	case IndexingType::Word:
		if (match_ == 0)
			match_ = 'e';
		else if (match_ != 'e' && match_ != 'h')
			return false;
		break;
	case IndexingType::Ngram:
		if (match_ == 0)
			match_ = 'n';
		else if (match_ != 'n')
			return false;
		break;
	}

	// optionを設定する
	ModUnicodeString lang = m_cDefaultLanguage;

	for (int i = 0; i < static_cast<int>(pCondition_->getOptionSize()); ++i)
	{
		const TreeNodeInterface* pOption = pCondition_->getOptionAt(i);
		switch (pOption->getType())
		{
		case TreeNodeInterface::Language:
			lang = pOption->getValue();
			break;
		default:
			return false;
		}
	}

	//
	// tea構文に変換
	//

	cStream_ << "#term[" << match_ << ",," << lang << "](";
	
	// パターンを取得
	ModUnicodeString value = pCondition_->getOperandAt(0)->getValue();
	if (m_cExpandWord.getLength() != 0)
		value = m_cExpandWord;
	if (value.getLength() == 0)
		return false;

	appendPattern(cStream_, value);
	
	cStream_ << ")";

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::convertWord -- wordの要素を設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		変換結果を格納するストリーム
//	const LogicalFile::TreeNodeInterface* pCondition_
//		wordノード
//	int& iTermCount_
//		検索語数
//
//	RETURN
//	bool
//		転置索引で実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertWord(ModUnicodeOstrStream& cStream_,
						const LogicalFile::TreeNodeInterface* pCondition_,
						int& iTermCount_)
{
	using namespace LogicalFile;
	
	//	Word ------ Operand -- ConstantValue or Variable
	//		|
	//		+------ Option --- Category
	//					|
	//					+----- Scale
	//					|
	//					+----- Language

	ModUnicodeString category = _DefaultCategory;
	double scale = _DefaultScale;
	ModUnicodeString language;
	int df = 0;

	if (pCondition_->getOperandSize() != 1)
		return false;

	ModUnicodeString matchMode;
	const TreeNodeInterface* m = pCondition_->getOperandAt(0);

	switch (m_eIndexType)
	{
	case IndexingType::Dual:
		matchMode = 'm';
		break;
	case IndexingType::Word:
		matchMode = 'e';
		break;
	case IndexingType::Ngram:
		matchMode = 'n';
		break;
	}

	if (m->getType() != TreeNodeInterface::Pattern)
	{
		// 単純な文字列以外の場合
		
		if (m->getOperandSize() != 1)
			return false;
		
		switch (m->getType())
		{
		case TreeNodeInterface::ExactWord:
			matchMode = "e";
			break;
		case TreeNodeInterface::SimpleWord:
			matchMode = "s";
			break;
		case TreeNodeInterface::String:
			matchMode = "n";
			break;
		case TreeNodeInterface::WordHead:
			matchMode = "h";
			break;
		case TreeNodeInterface::WordTail:
			matchMode = "t";
			break;
		default:
			return false;
		}
		
		m = m->getOperandAt(0);
	}

	if (m->getType() != TreeNodeInterface::Pattern ||
		m->getOperandSize() != 1 ||
		m->getOptionSize() != 0)
		return false;

	m = m->getOperandAt(0);

	for (int i = 0; i < static_cast<int>(pCondition_->getOptionSize()); ++i)
	{
		const TreeNodeInterface* p = pCondition_->getOptionAt(i);
		switch (p->getType())
		{
		case TreeNodeInterface::Category:
			{
				Common::WordData::Category::Value c
					= Common::WordData::toCategory(p->getValue());
				if (c == Common::WordData::Category::Undefined)
					return false;
				category = p->getValue();
			}
			break;
		case TreeNodeInterface::Scale:
			scale = FileCommon::DataManager::toDouble(p);
			break;
		case TreeNodeInterface::Language:
			{
				language = ModLanguageSet(p->getValue()).getName();
			}
			break;
		case TreeNodeInterface::Df:
			df = FileCommon::DataManager::toInt(p);
			break;
		}
	}

	cStream_ << "#word["
			 << matchMode << ","
			 << language << ","
			 << category << ","
			 << scale << ","
			 << df << "](";
	appendPattern(cStream_, m->getValue());
	cStream_ << ")";

	++iTermCount_;

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::isEscapeChar
//		-- tea構文でエスケープしなければならない文字か？
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeChar c
//		調べる文字
//
//	RETURN
//	bool
//		エスケープしなければならない場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::isEscapeChar(ModUnicodeChar c)
{
	using namespace Common;

	switch (c)
	{
	case UnicodeChar::usSharp:			// '#'
	case UnicodeChar::usLparent:		// '('
	case UnicodeChar::usRparent:		// ')'
	case UnicodeChar::usComma:			// ','
	case UnicodeChar::usLbracket:		// '['
	case UnicodeChar::usBackSlash:		// '\'
	case UnicodeChar::usRbracket:		// ']'
		return true;
	default:
		;
	}
	return false;
}

//
//	FUNCTION private
//	FullText2::OpenOption::appendPattern
//		-- エスケープしながら文字列を追加する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		出力先のストリーム
//	const ModUnicodeString& cstrPattern_
//		出力する文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OpenOption::appendPattern(ModUnicodeOstrStream& cStream_,
						  const ModUnicodeString& cstrPattern_)
{
	const ModUnicodeChar* p = cstrPattern_;
	while (*p != 0)
	{
		if (isEscapeChar(*p))
			cStream_ << "\\";
		cStream_ << *p;
		p++;
	}
}

//
//	FUNCTION private
//	FullText2::OpenOption::setTupleID
//		-- 整合性検査時のタプルIDのデータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pValue_
//		値のノード
//	int iRowID
//		タプルID
//
//	RETURN
//	bool
//		転置ファイルで実行できるノードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setTupleID(const LogicalFile::TreeNodeInterface* pValue_,
					   int& iRowID_)
{
	iRowID_ = FileCommon::DataManager::toInt(pValue_);
	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::setStringData -- 整合性検査時の文字列データを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pValue_
//		値のノード
//	ModVector<ModUnicodeString> vecStrValue_
//		文字列データ
//
//	RETURN
//	bool
//		転置ファイルで実行できるノードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setStringData(const LogicalFile::TreeNodeInterface* pValue_,
						  ModVector<ModUnicodeString>& vecStrValue_)
{
	using namespace LogicalFile;

	if (pValue_->getType() == TreeNodeInterface::List)
	{
		// 配列である
		int size = pValue_->getOperandSize();
		for (int i = 0; i < size; ++i)
		{
			vecStrValue_.pushBack(pValue_->getOperandAt(i)->getValue());
		}
	}
	else
	{
		// 配列じゃない
		vecStrValue_.pushBack(pValue_->getValue());
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::setLanguageData -- 言語データを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pValue_
//		値のノード
//	ModVector<ModLanguageSet>& vecLanguageSet_
//		言語情報
//
//	RETURN
//	bool
//		転置ファイルで実行できるノードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setLanguageData(const LogicalFile::TreeNodeInterface* pValue_,
							ModVector<ModLanguageSet>& vecLanguageSet_)
{
	using namespace LogicalFile;

	if (pValue_->getType() == TreeNodeInterface::List)
	{
		// 配列である
		int size = pValue_->getOperandSize();
		for (int i = 0; i < size; ++i)
		{
			ModUnicodeString s = pValue_->getOperandAt(i)->getValue();
			if (s.getLength() != 0)
				vecLanguageSet_.pushBack(ModLanguageSet(s));
			else
				vecLanguageSet_.pushBack(ModLanguageSet(m_cDefaultLanguage));
		}
	}
	else
	{
		// 配列じゃない
		ModUnicodeString s = pValue_->getValue();
		if (s.getLength() != 0)
			vecLanguageSet_.pushBack(ModLanguageSet(s));
	}

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::getExpangSynonym -- 同期語を展開する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		EXPAND_SYNONYM のオペランド
//
//	ModVector<ModUnicodeString>& vecExp_
//		展開された単語
//
//	RETURN
//	bool
//		転置で実行可能な検索条件の場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::getExpandSynonym(const LogicalFile::TreeNodeInterface* pNode_,
							 ModVector<ModUnicodeString>& vecExp_)
{
	if (m_uiResourceID == 0)

		// UNAが指定されていないので例外
		
		_TRMEISTER_THROW0(Exception::NotSupported);
	
	if (m_pAnalyzer == 0)
	{
		// UNAのAnalyzerを準備する

		m_pAnalyzer = Utility::Una::Manager::getModNlpAnalyzer(m_uiResourceID);

		// パラメータを設定する

		ModMap<ModUnicodeString, ModUnicodeString, ModLess<ModUnicodeString> >
			param;

		m_pAnalyzer->prepare(param);
	}

	// 引数のノードを辿り、検索語と言語を得る

	ModUnicodeString pattern;
	ModLanguageSet lang = m_cDefaultLanguage;

	const LogicalFile::TreeNodeInterface* p = pNode_;
	while (p != 0)
	{
		if (p->getType() == LogicalFile::TreeNodeInterface::Pattern)
		{
			//	Pattern --- Operand -- ConstantValue or Variable
			//		|
			//		+------ Option --- Language
			//

			// オプションから言語を得る

			for (int i = 0; i < static_cast<int>(p->getOptionSize()); ++i)
			{
				const LogicalFile::TreeNodeInterface* pOption
					= p->getOptionAt(i);
				switch (pOption->getType())
				{
				case LogicalFile::TreeNodeInterface::Language:
					lang = pOption->getValue();
					break;
				default:
					return false;
				}
			}

			pattern = p->getOperandAt(0)->getValue();

			break;
		}

		// EXPAND_SYNONYM のオペランドには、
		// 複数のオペランドを持つノードはない
		
		p = p->getOperandAt(0);
		
	}

	// 同義語展開する
	
	Utility::CharTrait::expandSynonym(m_pAnalyzer,
									  pattern,
									  lang,
									  vecExp_);

	return true;
}

//
//	FUNCTION private
//	FullText2::OpenOption::checkField
//		-- 同じフィールドに対する条件だけか確認する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pNode_
//		確認する検索ノード
//
//	RETURN
//	int
//		すべて同じフィールドだった場合はフィールド番号、それ以外の場合は -1
//
//	EXCEPTIONS
//
int
OpenOption::checkField(const LogicalFile::TreeNodeInterface* pNode_)
{
	using namespace LogicalFile;

	ModVector<const LogicalFile::TreeNodeInterface*> stack;
	int field = -1;

	stack.pushBack(pNode_);
	int n = 1;

	for (int i = 0; i < n; ++i)
	{
		const TreeNodeInterface* p = stack[i];
		if (p->getType() == TreeNodeInterface::Field)
		{
			int f = FileCommon::DataManager::toInt(p);
			if (field != -1 && f != field)
				return -1;
			field = f;
		}
		else if (p->getType() == TreeNodeInterface::Contains)
		{
			// contains は先頭のノードにないとだめ
			return -1;
		}
		else if (p->getOperandSize())
		{
			int size = static_cast<int>(p->getOperandSize());
			for (int j = 0; j < size; ++j, ++n)
				stack.pushBack(p->getOperandAt(j));
		}
	}

	return field;
}

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
