// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.cpp --
// 
// Copyright (c) 2004, 2005, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "SyInclude.h"
#include "Inverted/OpenOption.h"
#include "Inverted/Parameter.h"
#include "Inverted/UnaAnalyzerManager.h"

#include "FileCommon/OpenOption.h"
#include "FileCommon/DataManager.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "Common/UnicodeString.h"
#include "Common/WordData.h"
#include "Common/Thread.h"

#include "Exception/NotSupported.h"
#include "Exception/TooLongConditionalPattern.h"
#include "Exception/InvalidEscape.h"

#include "Os/Limits.h"

#include "Utility/CharTrait.h"

#include "ModAutoPointer.h"

#include "ModNLP.h"

#include "ModTermElement.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

namespace
{
	//
	//	VARIABLE local
	//	_$$::_cQueryMaxLen -- 検索語の最大長
	//
	ParameterInteger _cQueryMaxLen("Inverted_QueryMaxLen",
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
		if (combiner_.compare(_Combiner_ASum, ModFalse))
		if (combiner_.compare(_Combiner_Prod, ModFalse))
		if (combiner_.compare(_Combiner_Max, ModFalse))
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
	ModUnicodeString _Calculator_ExternalScoreCalculator("ExternalScoreCalculator");
	
	// calculatorをチェックする
	bool _checkCalculator(const ModUnicodeString& calculator_)
	{
		if (calculator_.compare(_Calculator_OkapiTfIdf,
								_Calculator_OkapiTfIdf.getLength()))
		if (calculator_.compare(_Calculator_NormalizedOkapiTfIdf,
								_Calculator_NormalizedOkapiTfIdf.getLength()))
		if (calculator_.compare(_Calculator_OkapiTf,
								_Calculator_OkapiTf.getLength()))
		if (calculator_.compare(_Calculator_NormalizedOkapiTf,
								_Calculator_NormalizedOkapiTf.getLength()))
		if (calculator_.compare(_Calculator_TfIdf,
								_Calculator_TfIdf.getLength()))
		if (calculator_.compare(_Calculator_NormalizedTfIdf,
								_Calculator_NormalizedTfIdf.getLength()))
		if (calculator_.compare(_Calculator_ExternalScoreCalculator,
								_Calculator_ExternalScoreCalculator.getLength()))
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
	OpenOption::ScoreMethod::Value
	_getScoreFunction(const ModUnicodeString& scoreFunction_)
	{
		if (scoreFunction_.compare(_ScoreFunction_Sum, ModFalse) == 0)
			return OpenOption::ScoreMethod::Sum;
		if (scoreFunction_.compare(_ScoreFunction_Multiply, ModFalse) == 0)
			return OpenOption::ScoreMethod::Multiply;
		if (scoreFunction_.compare(_ScoreFunction_Replace, ModFalse) == 0)
			return OpenOption::ScoreMethod::Replace;

		return OpenOption::ScoreMethod::Unknown;
	}

	// デフォルト値
	int _DefaultCategory = Common::WordData::Category::Helpful;
	double _DefaultScale = 1.0;

}

//
//	FUNCTION public
//	Inverted::OpenOption::OpenOption -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::OpenOption& cLogicalOpenOption_
//		論理ファイルのオープンオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
OpenOption::OpenOption(const LogicalFile::OpenOption& cLogicalOpenOption_)
	: m_cOpenOption(const_cast<LogicalFile::OpenOption&>(cLogicalOpenOption_)),
	  m_bEqual(false), m_pAnalyzer(0)
{
}

//
//	FUNCTION public
//	Inverted::OpenOption::~OpenOption -- デストラクタ
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
//	Inverted::OpenOption::parse -- TreeNodeInterfaceを変換する
//
//	NOTES
//
//	ARGUMENTS
//	const Inverted::FileID& cFileID_
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
OpenOption::parse(const LogicalFile::FileID& cFileID_,
				  bool isLanguage_,
				  bool isScoreField_,
				  const LogicalFile::TreeNodeInterface* pCondition_)
{
	using namespace LogicalFile;
	
	// 索引タイプを得る
	m_eIndexType = FileID::getIndexingType(cFileID_);
	// 位置情報を使えるか
	m_bNolocation = FileID::isNolocation(cFileID_);
	// デフォルト言語
	m_cDefaultLanguage = FileID::getDefaultLanguageSetName(cFileID_);
	// 特徴語が抽出されているか
	m_bClustering = FileID::isClustering(cFileID_);
	// リソース番号
	m_uiResourceID = FileID::getResourceID(cFileID_);

	// 言語情報フィールド番号
	m_iLanguageField = -1;
	if (isLanguage_) m_iLanguageField = 1;

	// ROWIDフィールド番号
	m_iRowIDField = 1;
	if (isLanguage_) m_iRowIDField++;
	if (isScoreField_) m_iRowIDField++;
	
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
//	Inverted::OpenOption::getSearchType -- 検索タイプを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverted::OpenOption::Type::Value
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
//	Inverted::OpenOption::getConditionString -- 検索条件文字列を得る
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
ModUnicodeString
OpenOption::getConditionString() const
{
	return m_cOpenOption.getString(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ConditionString));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getFreeTextLanguage -- 自然文言語情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModLanguageSet
//	   	言語情報
//
//	EXCEPTIONS
//
ModLanguageSet
OpenOption::getFreeTextLanguage() const
{
	return m_cOpenOption.getString(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::FreeTextLanguage));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getCalculator -- スコア計算器を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		スコア計算器
//
//	EXCEPTIONS
//
ModUnicodeString
OpenOption::getCalculator() const
{
	return m_cOpenOption.getString(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::Calculator));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getAverageLength -- 平均文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		平均文書長
//
//	EXCEPTIONS
//
int
OpenOption::getAverageLength() const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::AverageLength));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getDocumentFrequency -- 文書頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		文書頻度
//
//	EXCEPTIONS
//
int
OpenOption::getDocumentFrequency() const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::DocumentFrequency));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getExtractor -- Extractorを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		Extractor
//
//	EXCEPTIONS
//
ModUnicodeString
OpenOption::getExtractor() const
{
	return m_cOpenOption.getString(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::Extractor));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getExpandLimit -- 拡張語の上限を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		拡張語の上限
//
//	EXCEPTIONS
//
int
OpenOption::getExpandLimit() const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ExpandLimit));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getTermCount -- 検索語数を得る
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
//	Inverted::OpenOption::getRowID -- RowIDを得る
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
//	Inverted::OpenOptin::getSectionByteOffset -- セクションの区切り位置を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModVector<ModSize>
//		セクション区切り位置の配列
//
//	EXCEPTIONS
//
ModVector<ModSize>
OpenOption::getSectionByteOffset() const
{
	ModVector<ModSize> vecOffset;
	int size = m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SectionCount));
	for (int i = 0; i < size; ++i)
	{
		vecOffset.pushBack(m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::SectionSize, i)));
	}
	return vecOffset;
}

//
//	FUNCTION public
//	Inverted::OpenOptin::getLanguageSet -- 言語情報を得る
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
OpenOption::getLanguageSet() const
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
//	Inverted::OpenOption::getWordCount -- word数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		word数
//
//	EXCEPTIONS
//	なし
//
int
OpenOption::getWordCount() const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::WordCount));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getWordPattern -- wordのパターンを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		要素番号
//
//	RETURN
//	ModUnicodeString
//		wordのパターン
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
OpenOption::getWordPattern(int n) const
{
	return m_cOpenOption.getString(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordPattern, n));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getWordCategory -- wordのカテゴリを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		要素番号
//
//	RETURN
//	Common::WordData::Category::Value
//		wordのカテゴリ
//
//	EXCEPTIONS
//	なし
//
Common::WordData::Category::Value
OpenOption::getWordCategory(int n) const
{
	return static_cast<Common::WordData::Category::Value>(
		m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordCategory, n)));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getWordScale -- wordのスケールを得る
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		要素番号
//
//	RETURN
//	double
//		wordのスケール
//
//	EXCEPTIONS
//	なし
//
double
OpenOption::getWordScale(int n) const
{
	return m_cOpenOption.getDouble(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordScale, n));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getWordLanguage -- wordの言語を得る
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		要素番号
//
//	RETURN
//	ModUnicodeString
//		wordの言語
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
OpenOption::getWordLanguage(int n) const
{
	return m_cOpenOption.getString(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordLanguage, n));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getWordMatchMode -- wordの一致条件を得る
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		要素番号
//
//	RETURN
//	int
//		wordの一致条件
//
//	EXCEPTIONS
//	なし
//
int
OpenOption::getWordMatchMode(int n) const
{
	return m_cOpenOption.getInteger(
		_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordMatchMode, n));
}

//
//	FUNCTION public
//	Inverted::OpenOption::getScoreMethod -- スコア調整方法を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverted::OpenOption::ScoreMethod::Value
//		スコア調整方法
//
//	EXCEPTIONS
//
OpenOption::ScoreMethod::Value
OpenOption::getScoreMethod() const
{
	ScoreMethod::Value method = ScoreMethod::Unknown;
	int v;
	if (m_cOpenOption.getInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ScoreFunction), v) == true)
		method = static_cast<ScoreMethod::Value>(v);
	return method;
}

//
//	FUNCTION public
//	Inverted::OpenOption::getClusteredLimit
//		-- クラスタリング時の類似度のしきい値を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	float
//	   	類似度の閾値
//
//	EXCEPTIONS
//
float
OpenOption::getClusteredLimit() const
{
	// defautは、0.8とする
	// ↑？
	// OpenOption.cpp revision 30 の時点ではdefaultは、0.0。
	// setContains参照
	return static_cast<float>(
		m_cOpenOption.getDouble(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ClusteredLimit)));
}

//
//	FUNCTION private
//	Inverted::OpenOption::setContains -- containsの検索条件を作成する
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
	//					+----- AverageLength
	//					|
	//					+----- DF
	//					|
	//					+----- ScoreFunction
	//					|
	//					+----- ClusteredLimit
	//					|
	//					+----- KwicSize
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
	//					|
	//					+----- Combiner
	//
	//	And ------- Operand -- <ContainsOperand>
	//		|			|
	//		|			+----- <ContainsOperand>
	//		|			|
	//		|			...
	//		|
	//		|
	//		+------- Option -- Combiner
	//
	//	Or -------- Operand -- <ContainsOperand>
	//		|			|
	//		|			+----- <ContainsOperand>
	//		|			|
	//		|			...
	//		|
	//		+------- Option -- Combiner
	//
	//	AndNot----- Operand -- <ContainsOperand>
	//		|			|
	//		|			+----- <ContainsOperand>
	//		|			|
	//		|			...
	//		|
	//		+------- Option -- Combiner
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
	
	// ここで、たとえば
	//   select rowid,cluster(F) from T where F contains clustered limit 0.8
	// のSQL文において、 contains以下のparseを行う？
	double limit = 0.0;

	if (pField->getType() != TreeNodeInterface::Field)
	{
		// 順番が逆
		const TreeNodeInterface* pTmp = pField;
		pField = pCond;
		pCond = pTmp;
	}

	// フィールドチェック
	if (FileCommon::DataManager::toInt(pField) != 0)
		return false;

	int averageLength = 0;
	int documentFrequency = 0;
	int wordLimit = 0;
	ModUnicodeString extractor;
	ScoreMethod::Value scoreMethod = ScoreMethod::Unknown;
	int iKwicSize = 0;
	
	// オプションをチェック
	for (int i = 0; i < static_cast<int>(pCondition_->getOptionSize()); ++i)
	{
		const TreeNodeInterface* pOption = pCondition_->getOptionAt(i);
		switch (pOption->getType())
		{
		case TreeNodeInterface::Calculator:
			m_cCalc = pOption->getValue();
			if (_checkCalculator(m_cCalc) == false)
				return false;
			break;
		case TreeNodeInterface::Combiner:
			m_cComb = pOption->getValue();
			if (_checkCombiner(m_cComb) == false)
				return false;
			break;
		case TreeNodeInterface::AverageLength:
			averageLength = FileCommon::DataManager::toInt(pOption);
			break;
		case TreeNodeInterface::Df:
			documentFrequency
				= FileCommon::DataManager::toInt(pOption);
			break;
		case TreeNodeInterface::Expand:
			if (parseExpand(pOption, wordLimit) == false)
				return false;
			break;
		case TreeNodeInterface::Extractor:
			extractor = pOption->getValue();
			break;
		case TreeNodeInterface::ScoreFunction:
			scoreMethod =_getScoreFunction(pOption->getValue());
			if (scoreMethod == ScoreMethod::Unknown)
				return false;
			break;
		case TreeNodeInterface::ClusteredLimit:
			// limitには、類似度のしきい値が設定される
			if (m_bClustering == false)
				// 特徴語が抽出されていないと実行できない
				return false;
			limit = FileCommon::DataManager::toDouble(pOption);	// doubleに変換
			break;
		case TreeNodeInterface::KwicSize:
			// [NOTE] 荒いKWICに関する情報がなくても荒いKWICを返すことはできる
			iKwicSize = FileCommon::DataManager::toInt(pOption);
			if (iKwicSize <= 0)
				return false;
			
			break;
		}
	}

	switch (pCond->getType())
	{
	case TreeNodeInterface::Freetext:
		{
			ModUnicodeString cFreeText;
			ModUnicodeString cLanguage;
			if (parseFreeText(pCond, cFreeText, cLanguage) == false)
				return false;
			
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::FreeText);
			m_cOpenOption.setString(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ConditionString),
				cFreeText);
			m_cOpenOption.setString(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::FreeTextLanguage),
				cLanguage);
		}
		break;
	case TreeNodeInterface::WordList:
		{
			int size = static_cast<int>(pCond->getOperandSize());
			if (size < 1) return false;
			int prohibitive = 0;
			bool bEssential = false;
			for (int j = 0; j < size; ++j)
			{
				const TreeNodeInterface* pWord = pCond->getOperandAt(j);
				if (pWord->getType() != TreeNodeInterface::Word)
					return false;
				ModUnicodeString pattern;
				int category;
				double scale;
				ModUnicodeString language;
				int matchMode;
				if (setWord(
						pWord, pattern, category, scale, language, matchMode)
					== false)
					return false;

				m_cOpenOption.setString(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordPattern, j),
					pattern);
				m_cOpenOption.setInteger(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordCategory, j),
					category);
				m_cOpenOption.setDouble(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordScale, j),
					scale);
				m_cOpenOption.setString(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordLanguage, j),
					language);
				m_cOpenOption.setInteger(
					_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(KeyID::WordMatchMode, j),
					matchMode);

				switch (category)
				{
				case Common::WordData::Category::Essential:
				case Common::WordData::Category::EssentialRelated:
					bEssential = true;
					break;
				case Common::WordData::Category::Important:
				case Common::WordData::Category::ImportantRelated:
				case Common::WordData::Category::Helpful:
				case Common::WordData::Category::HelpfulRelated:
					// ignored
					break;
				case Common::WordData::Category::Prohibitive:
				case Common::WordData::Category::ProhibitiveRelated:
					prohibitive++;
					break;
				default:
					; _TRMEISTER_ASSERT(false);
					break;
				}
			}
			if (size == prohibitive) return false;
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::WordList);
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::WordCount), size);
			m_cOpenOption.setBoolean(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::EssentialWordList),
				bEssential);
		}
		break;
	default:
		{
			ModUnicodeString cstrTea;
			int iTermCount = 0;
			if (convertContains(pCond, cstrTea, iTermCount) == false)
				return false;
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::Normal);
			m_cOpenOption.setString(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ConditionString), cstrTea);
			m_cOpenOption.setInteger(
				_SYDNEY_OPEN_PARAMETER_KEY(KeyID::TermCount), iTermCount);
		}
		break;
	}

	if (m_cCalc.getLength())
		m_cOpenOption.setString(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::Calculator), m_cCalc);
	if (averageLength != 0)
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::AverageLength), averageLength);
	if (documentFrequency != 0)
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::DocumentFrequency),
			documentFrequency);
	if (wordLimit != 0)
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ExpandLimit),	wordLimit);
	if (extractor.getLength())
		m_cOpenOption.setString(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::Extractor), extractor);
	if (scoreMethod != ScoreMethod::Unknown)
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ScoreFunction), scoreMethod);
	m_cOpenOption.setDouble(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ClusteredLimit), limit);
	if (iKwicSize > 0)
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::KwicSize), iKwicSize);

	return true;
}

//
//	FUNCTION private
//	Inverted::OpenOption::setNormal -- 通常の検索条件を作成する
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
	ModUnicodeString cstrTea;
	int iTermCount = 0;

	bool bResult = false;
	
	if (convertNormal(pCondition_, cstrTea, iTermCount) == true)
	{
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SearchType), Type::Normal);
		m_cOpenOption.setString(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ConditionString), cstrTea);
		m_cOpenOption.setInteger(
			_SYDNEY_OPEN_PARAMETER_KEY(KeyID::TermCount), iTermCount);
		bResult = true;
	}

	return bResult;
}

//
//	FUNCTION private
//	Inverted::OpenOption::setEqual -- 整合性検査のためのequal条件を作成する
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
	ModUnicodeString cstrValue;
	ModVector<ModSize> vecSectionOffset;
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
			// 配列の場合、一つの文字列と、配列と同数のセクション区切りに変換
			if (setStringData(pValue, cstrValue, vecSectionOffset) == false)
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
	m_cOpenOption.setString(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::ConditionString), cstrValue);
	m_cOpenOption.setInteger(
		_SYDNEY_OPEN_PARAMETER_KEY(KeyID::SectionCount),
		vecSectionOffset.getSize());
	for (int i = 0; i < static_cast<int>(vecSectionOffset.getSize()); ++i)
	{
		m_cOpenOption.setInteger(_SYDNEY_OPEN_PARAMETER_FORMAT_KEY(
			KeyID::SectionSize, i), vecSectionOffset[i]);
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
//	Inverted::OpenOption::parseFreeText
//		-- TreeNodeInterfaceから自然文情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeString& cFreeText_
//		自然文
//	ModUnicodeString& cLanguage_
//		言語
//
//	RETURN
//	bool
//		転置で実行可能な場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::parseFreeText(const LogicalFile::TreeNodeInterface* pCondition_,
						  ModUnicodeString& cFreeText_,
						  ModUnicodeString& cLanguage_)
{
	using namespace LogicalFile;

	//
	//	FreeText -- Operand -- Variable or ConstantValue
	//		|
	//		+------ Option --- Language
	//

	//
	// FreeTextは文字列をTea構文に変換しない
	//
	
	if (pCondition_->getOperandSize() != 1
		|| pCondition_->getOptionSize() > 1)
		return false;
	
	const TreeNodeInterface* pNode = pCondition_->getOperandAt(0);
	cFreeText_ = pNode->getValue();

	if (pCondition_->getOptionSize())
	{
		const TreeNodeInterface* pOption = pCondition_->getOptionAt(0);
		if (pOption->getType() != TreeNodeInterface::Language)
			return false;
		cLanguage_ = ModLanguageSet(pOption->getValue()).getName();
	}
	else
	{
		cLanguage_ = m_cDefaultLanguage;
	}

	return true;
}

//
//	FUNCTION private
//	Inverted::OpenOption::parseExpand
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

	iExpandLimit_ = 0;

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
//	Inverted::OpenOption::convertNormal
//		-- 1つのTeeNodeInerfaceをTea構文に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeString& cstrTea_
//		変換結果
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
OpenOption::convertNormal(const LogicalFile::TreeNodeInterface* pCondition_,
						  ModUnicodeString& cstrTea_,
						  int& iTermCount_)
{
	using namespace LogicalFile;

	// FullTextFile::isAbleToSearch によると、
	// AND, OR, LIKE が渡される。
	
	// ただし、setEqual によると、= もverifyで渡される。
	
	// NOT やそれ以外は渡されない。
	// もし渡されたとしてもfalseを返す。
	// また、(NOT (c LIKE 'ABC')) AND (NOT (c LIKE 'XYZ')) のように、
	// ANDの全ての被演算子がNOTだった場合もfalseを返す。

	bool bResult = false;

	ModUnicodeOstrStream cStream;

	switch (pCondition_->getType())
	{
	case TreeNodeInterface::And:
		{
			cStream << "#and(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			int i;
			bool allNot = true;

			// and-not以外の処理
			for (i = 0; i < count; ++i)
			{
				ModUnicodeString cstrTea;
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (pNode->getType() != TreeNodeInterface::Not)
				{
					if (convertNormal(pNode, cstrTea, iTermCount_) == false)
						return false;
					if (allNot == false)
						cStream << ",";
					cStream << cstrTea;
					allNot = false;
				}
			}
			if (allNot == true) return false;	// 全部notである
			cStream << ")";

			// and-notの処理
			for (i = 0; i < count; ++i)
			{
				ModUnicodeString cstrTea;
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (pNode->getType() == TreeNodeInterface::Not)
				{
					pNode = pNode->getOperandAt(0);
					if (convertNormal(pNode, cstrTea, iTermCount_) == false)
						return false;
					ModUnicodeString tmp = cStream.getString();
					cStream.clear();
					cStream << "#and-not(" << tmp << "," << cstrTea << ")";
				}
			}
		}
		bResult = true;
		break;
	case TreeNodeInterface::Or:
		{
			cStream << "#or(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			int i;
			for (i = 0; i < count; ++i)
			{
				ModUnicodeString cstrTea;
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertNormal(pNode, cstrTea, iTermCount_) == false)
					return false;
				if (i != 0)
					cStream << ",";
				cStream << cstrTea;
			}
			cStream << ")";
		}
		bResult = true;
		break;
	case TreeNodeInterface::Like:
		{
			ModUnicodeString cstrTea;
			if (convertLike(pCondition_, cstrTea, iTermCount_) == false)
				return false;
			cStream << cstrTea;
		}
		bResult = true;
		break;
	case TreeNodeInterface::Equals:
		{
			ModUnicodeString cstrTea;
			if (convertEqual(pCondition_, cstrTea, iTermCount_) == false)
			{
				// フラグを設定する
				m_bEqual = true;
				return false;
			}
			cStream << cstrTea;
		}
		bResult = true;
		break;
	case TreeNodeInterface::Not:
	default:
		;
	}

	if (bResult == true)
		cstrTea_ = cStream.getString();

	return bResult;
}

//
//	FUNCTION private
//	Inverted::OpenOption::convertLike -- likeを変換する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		条件
//	ModUnicodeString& cstrTea_
//		変換結果
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
OpenOption::convertLike(const LogicalFile::TreeNodeInterface* pCondition_,
						ModUnicodeString& cstrTea_,
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

	// フィールドチェック
	if (FileCommon::DataManager::toInt(pField) != 0)
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

	return convertValue(pValue->getValue(), cstrEscape, cstrLanguage,
						cstrTea_, iTermCount_);
}

//
//	FUNCTION private
//	Inverted::OpenOption::convertEqual -- equalを変換する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		条件
//	ModUnicodeString& cstrTea_
//		変換結果
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
OpenOption::convertEqual(const LogicalFile::TreeNodeInterface* pCondition_,
						 ModUnicodeString& cstrTea_,
						 int& iTermCount_)
{
	return false;
}

//
//	FUNCTION public
//	Inverted::OpenOption::convertValue -- 検索語を変換する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrValue_
//		検索語
//	const ModUnicodeString& cstrEscape_
//		エスケープ文字
//	const ModUnicodeString& cstrLanguage_,
//		言語指定
//	ModUnicodeString& cstrTea_
//		変換結果
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
OpenOption::convertValue(const ModUnicodeString& cstrValue_,
						 const ModUnicodeString& cstrEscape_,
						 const ModUnicodeString& cstrLanguage_,
						 ModUnicodeString& cstrTea_,
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
	case ModInvertedFileDualIndexing:
		matchMode = 'm';
		break;
	case ModInvertedFileWordIndexing:
		matchMode = 'e';
		break;
	case ModInvertedFileNgramIndexing:
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

	ModUnicodeOstrStream cStream;

	// 文字列検索
	if (vecTerm.getSize() > 1)
	{
		// '%'が含まれていた

		// 複数の文字列から構成されるので、
		// windowで出現順を指定する(距離は無制限)
		cStream << "#window[" << 0 << ","
				<< Os::Limits<int>::getMax() << "](";
	}

	//
	// 分割された各文字列を変換する
	//
	for (ModVector<Term>::Iterator i = vecTerm.begin(); i != vecTerm.end(); ++i)
	{
		if (i != vecTerm.begin())
			cStream << ",";

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
			cStream << "#end[" << e << "](";
		}
		if ((*i).m_bFront == false)
		{
			// 前方一致
			cStream << "#location[" << f << "](";
		}

		//
		// 検索語を設定
		//
		if ((*i).m_bRegrex == true)
		{
			// '_'が含まれている場合

			// 前処理で解析済みデータを設定
			cStream << cNewStream.getString();
		}
		else
		{
			// '_'がない場合
			const ModUnicodeChar* p = (*i).m_pszValue;
			cStream << "#term[" << matchMode << ",," << cLang << "](";
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
					cStream << "\\";
				if (*p) cStream << *p;
			}
			cStream << ")";
		}

		//
		// 前方一致、後方一致の順で閉じる
		//
		if ((*i).m_bFront == false)
		{
			// 前方一致
			cStream << ")";
		}
		if ((*i).m_bBack == false)
		{
			// 後方一致
			cStream << ")";
		}
	}

	if (vecTerm.getSize() > 1)
	{
		cStream << ")";
	}
	
	cstrTea_ = cStream.getString();

	return true;
}

//
//	FUNCTION private
//	Inverted::OpenOption::convertDistance
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

	if (m_eIndexType == ModInvertedFileWordIndexing)
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
	if (m_eIndexType == ModInvertedFileDualIndexing)
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
		// #window[*,*](#window[*,*](#term[*,,*](A), #term[*,,*](B)), #term[*,,*](C))
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
//	Inverted::OpenOption::separate -- '%'で文字を分割する
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
//	Inverted::OpenOption::TermType
//		 分割結果に応じた結果
//
//	EXCEPTIONS
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
//	Inverted::OpenOption::convertContains -- Contains Operandをtea構文に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		変換する条件
//	ModUnicodeString& cstrTea_
//		変換されたtea構文
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
OpenOption::convertContains(const LogicalFile::TreeNodeInterface* pCondition_,
							ModUnicodeString& cstrTea_,
							int& iTermCount_)
{
	using namespace LogicalFile;

	bool bResult = false;

	ModUnicodeOstrStream cStream;

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

				cStream << "#window[" << lower << "," << upper << "," << order
						<< "](";
			
				int count = static_cast<int>(pCondition_->getOperandSize());
				for (int i = 0; i < count; ++i)
				{
					ModUnicodeString cstrTea;
					const TreeNodeInterface* pNode =
						pCondition_->getOperandAt(i);
					if (convertContains(pNode, cstrTea, iTermCount_) == false)
						return false;
					if (i != 0)
						cStream << ",";
					cStream << cstrTea;
				}

				cStream << ")";
			
				bResult = true;
			}
		}
		break;
	case TreeNodeInterface::And:
		{
			cStream << "#and";
			if (pCondition_->getOptionSize())
			{
				if (pCondition_->getOptionSize() != 1) return false;
				const TreeNodeInterface* pOption = pCondition_->getOptionAt(0);
				ModUnicodeString combiner = pOption->getValue();
				if (_checkCombiner(combiner) == false) return false;
				cStream << "[" << combiner << "]";
			}
			else if (m_cComb.getLength())
			{
				cStream << "[" << m_cComb << "]";
			}
			cStream << "(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < count; ++i)
			{
				ModUnicodeString cstrTea;
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(pNode, cstrTea, iTermCount_) == false)
					return false;
				if (i != 0)
					cStream << ",";
				cStream << cstrTea;
			}
			cStream << ")";
			bResult = true;
		}
		break;
	case TreeNodeInterface::Or:
		{
			cStream << "#or";
			if (pCondition_->getOptionSize())
			{
				if (pCondition_->getOptionSize() != 1) return false;
				const TreeNodeInterface* pOption = pCondition_->getOptionAt(0);
				ModUnicodeString combiner = pOption->getValue();
				if (_checkCombiner(combiner) == false) return false;
				cStream << "[" << combiner << "]";
			}
			else if (m_cComb.getLength())
			{
				cStream << "[" << m_cComb << "]";
			}
			cStream << "(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < count; ++i)
			{
				ModUnicodeString cstrTea;
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(pNode, cstrTea, iTermCount_) == false)
					return false;
				if (i != 0)
					cStream << ",";
				cStream << cstrTea;
			}
			cStream << ")";
			bResult = true;
		}
		break;
	case TreeNodeInterface::AndNot:
		{
			ModUnicodeString combiner;
			if (pCondition_->getOptionSize())
			{
				if (pCondition_->getOptionSize() != 1) return false;
				const TreeNodeInterface* pOption = pCondition_->getOptionAt(0);
				combiner = pOption->getValue();
				if (_checkCombiner(combiner) == false) return false;
			}
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < count; ++i)
			{
				ModUnicodeString cstrTea;
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(pNode, cstrTea, iTermCount_) == false)
					return false;
				if (i == 0)
				{
					// 例えば、CONTAINS(A-B-C) の A の条件
					cStream << cstrTea;
					continue;
				}
				// #and-not(#and-not(A, B), C) の様な入れ子にする
				ModUnicodeString tmp = cStream.getString();
				cStream.clear();
				cStream << "#and-not";
				if (combiner.getLength())
					cStream << "[" << combiner << "]";
				cStream << "(" << tmp << "," << cstrTea << ")";
			}
			bResult = true;
		}
		break;
	case TreeNodeInterface::Pattern:
		{
			ModUnicodeString cstrTea;
			ModUnicodeChar m = 0;
			if (convertPattern(m, pCondition_, cstrTea) == true)
			{
				iTermCount_++;
				cStream << cstrTea;
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

				ModUnicodeString cstrTea;
				if (convertContains(pCondition_->getOperandAt(0),
									cstrTea, iTermCount_) == true)
				
				{
					cStream << "#location[1](" << cstrTea << ")";
					bResult = true;
				}
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
				
				ModUnicodeString cstrTea;
				if (convertContains(pCondition_->getOperandAt(0),
									cstrTea, iTermCount_) == true)
				{
					cStream << "#end[0](" << cstrTea << ")";
					bResult = true;
				}
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
				
				ModUnicodeString cstrTea;
				ModUnicodeChar m = 'e';
				if (convertPattern(m, pCondition_->getOperandAt(0),
								   cstrTea) == true)
				{
					iTermCount_++;
					cStream << cstrTea;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::SimpleWord:
		{
			if (pCondition_->getOperandSize() == 1)
			{
				ModUnicodeString cstrTea;
				ModUnicodeChar m = 's';
				if (convertPattern(m, pCondition_->getOperandAt(0),
								   cstrTea) == true)
				{
					iTermCount_++;
					cStream << cstrTea;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::String:
		{
			if (pCondition_->getOperandSize() == 1)
			{
				ModUnicodeString cstrTea;
				ModUnicodeChar m = 'n';
				if (convertPattern(m, pCondition_->getOperandAt(0),
								   cstrTea) == true)
				{
					iTermCount_++;
					cStream << cstrTea;
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
				
				ModUnicodeString cstrTea;
				ModUnicodeChar m = 'h';
				if (convertPattern(m, pCondition_->getOperandAt(0),
								   cstrTea) == true)
				{
					iTermCount_++;
					cStream << cstrTea;
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::WordTail:
		{
			if (pCondition_->getOperandSize() == 1)
			{
				ModUnicodeString cstrTea;
				ModUnicodeChar m = 't';
				if (convertPattern(m, pCondition_->getOperandAt(0),
								   cstrTea) == true)
				{
					iTermCount_++;
					cStream << cstrTea;
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
				
				ModUnicodeString cstrTea;
				if (convertContains(pCondition_->getOperandAt(0),
									cstrTea, iTermCount_) == true)
				{
					cStream << "#scale["
							<< pCondition_->getOptionAt(0)->getValue()
							<< "](" << cstrTea << ")";
					bResult = true;
				}
			}
		}
		break;
	case TreeNodeInterface::Synonym:
		{
			cStream << "#syn(";
			int count = static_cast<int>(pCondition_->getOperandSize());
			for (int i = 0; i < count; ++i)
			{
				ModUnicodeString cstrTea;
				const TreeNodeInterface* pNode = pCondition_->getOperandAt(i);
				if (convertContains(pNode, cstrTea, iTermCount_) == false)
					return false;
				if (i != 0)
					cStream << ",";
				cStream << cstrTea;
			}
			cStream << ")";
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
				
				cStream << "#syn(";
				ModVector<ModUnicodeString>::Iterator i = exp.begin();
				for (; i != exp.end(); ++i)
				{
					// 展開語の設定する
					m_cExpandWord = *i;

					// tea構文に変換する
					ModUnicodeString cstrTea;
					const TreeNodeInterface* pNode
						= pCondition_->getOperandAt(0);
					if (convertContains(pNode, cstrTea, iTermCount_) == false)
						return false;
					if (i != exp.begin())
						cStream << ",";
					cStream << cstrTea;
				}
				m_cExpandWord.clear();
				cStream << ")";
			}
			else
			{
				// 展開されなかったのでそのまま

				ModUnicodeString cstrTea;
				if (convertContains(pCondition_->getOperandAt(0),
									cstrTea, iTermCount_) == false)
					return false;
				cStream << cstrTea;
			}
			bResult = true;
		}
		break;
	}

	if (bResult == true)
		cstrTea_ = cStream.getString();

	return bResult;
}

//
//	FUNCTION private
//	Inverted::OpenOption::convertPattern -- patternをtea構文に変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeChar match_
//	   マッチモード
//	const LogicalFile::TreeNodeInterface* pCondition_
//		検索条件
//	ModUnicodeString& cstrTea_
//		変換したtea構文
//
//	RETURN
//	bool
//		転置索引で実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::convertPattern(ModUnicodeChar match_,
						   const LogicalFile::TreeNodeInterface* pCondition_,
						   ModUnicodeString& cstrTea_)
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
	case ModInvertedFileDualIndexing:
		if (match_ == 0)
			match_ = 'm';
		break;
	case ModInvertedFileWordIndexing:
		if (match_ == 0)
			match_ = 'e';
		else if (match_ != 'e' && match_ != 'h')
			return false;
		break;
	case ModInvertedFileNgramIndexing:
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

	ModUnicodeOstrStream cStream;

	cStream << "#term[" << match_ << ",," << lang << "](";
	
	// パターンを取得
	ModUnicodeString value = pCondition_->getOperandAt(0)->getValue();
	if (m_cExpandWord.getLength() != 0)
		value = m_cExpandWord;
	if (value.getLength() == 0)
		return false;
	
	const ModUnicodeChar* p = value;
	while (*p != 0)
	{
		if (isEscapeChar(*p))
			cStream << "\\";
		cStream << *p;
		p++;
	}
	cStream << ")";

	cstrTea_ = cStream.getString();

	return true;
}

//
//	FUNCTION private
//	Inverted::OpenOption::setWord -- wordの要素を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pCondition_
//		wordノード
//	ModUnicodeString& pattern_
//		検索語
//	int& category_
//		カテゴリー
//	double& scale_
//		スケール
//	ModUnicodeString& language_
//		言語
//
//	RETURN
//	bool
//		転置索引で実行できる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setWord(const LogicalFile::TreeNodeInterface* pCondition_,
					ModUnicodeString& pattern_,
					int& category_,
					double& scale_,
					ModUnicodeString& language_,
					int& matchMode_)
{
	using namespace LogicalFile;
	
	//	Word ------ Operand -- ConstantValue or Variable
	//		|
	//		+------ Option --- Category
	//					|
	//					+----- Scale
	//					|
	//					+----- Language

	//
	// Wordは文字列をTea構文に変換しない
	//

	category_ = _DefaultCategory;
	scale_ = _DefaultScale;
	language_ = m_cDefaultLanguage;

	if (pCondition_->getOperandSize() != 1)
		return false;

	matchMode_ = ModTermElement::voidMatch;
	const TreeNodeInterface* m = pCondition_->getOperandAt(0);

	if (m->getType() != TreeNodeInterface::Pattern)
	{
		// 単純な文字列以外の場合
		
		if (m->getOperandSize() != 1)
			return false;
		
		switch (m->getType())
		{
		case TreeNodeInterface::ExactWord:
			matchMode_ = ModTermElement::exactMatch;
			break;
		case TreeNodeInterface::SimpleWord:
			matchMode_ = ModTermElement::simpleMatch;
			break;
		case TreeNodeInterface::String:
			matchMode_ = ModTermElement::stringMatch;
			break;
		case TreeNodeInterface::WordHead:
			matchMode_ = ModTermElement::headMatch;
			break;
		case TreeNodeInterface::WordTail:
			matchMode_ = ModTermElement::tailMatch;
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

	pattern_ = m->getOperandAt(0)->getValue();
	
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
				category_ = c;
			}
			break;
		case TreeNodeInterface::Scale:
			scale_ = FileCommon::DataManager::toDouble(p);
			break;
		case TreeNodeInterface::Language:
			{
				language_ = ModLanguageSet(p->getValue()).getName();
			}
			break;
		}
	}

	return true;
}

//
//	FUNCTION private
//	Inverted::OpenOption::isEscapeChar
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
//	Inverted::OpenOption::setTupleID -- 整合性検査時のタプルIDのデータを設定する
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
//	Inverted::OpenOption::setStringData -- 整合性検査時の文字列データを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::TreeNodeInterface* pValue_
//		値のノード
//	ModUnicodeString& cValue_
//		文字列データ
//	ModVector<ModSize>& vecSectionOffset_
//		セクション区切り
//
//	RETURN
//	bool
//		転置ファイルで実行できるノードの場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenOption::setStringData(const LogicalFile::TreeNodeInterface* pValue_,
						  ModUnicodeString& cValue_,
						  ModVector<ModSize>& vecSectionOffset_)
{
	using namespace LogicalFile;

	if (pValue_->getType() == TreeNodeInterface::List)
	{
		// 配列である
		int size = pValue_->getOperandSize();
		for (int i = 0; i < size; ++i)
		{
			// 各文字列を連結する
			cValue_ += pValue_->getOperandAt(i)->getValue();
			// セクション区切りは個々に挿入する
			vecSectionOffset_.pushBack(
				cValue_.getLength() * sizeof(ModUnicodeChar));
		}
	}
	else
	{
		// 配列じゃない
		cValue_ = pValue_->getValue();
		vecSectionOffset_.pushBack(
			cValue_.getLength() * sizeof(ModUnicodeChar));
	}

	return true;
}

//
//	FUNCTION private
//	Inverted::OpenOption::setLanguageData -- 言語データを設定する
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
//	Inverted::OpenOption::getExpangSynonym -- 同期語を展開する
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

		m_pAnalyzer = UnaAnalyzerManager::get(m_uiResourceID);

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
//	Copyright (c) 2004, 2005, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
