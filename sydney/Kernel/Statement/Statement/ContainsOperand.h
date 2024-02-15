// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsOperand.h -- CONTAINS 述語関連のクラス定義、関数宣言
// 
// Copyright (c) 2004, 2007, 2008, 2009, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_CONTAINSOPERAND_H
#define __SYDNEY_STATEMENT_CONTAINSOPERAND_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ContainsOperandList;
class ValueExpression;

//	CLASS
//	Statement::ContainsOperand -- CONTAINS 述語を表すクラス
//
//	NOTES

class SYD_STATEMENT_FUNCTION ContainsOperand
	: public Object
{
public:
	typedef Object Super;

	// Containsオペランドの種別
	struct Type {
		enum Value {
			Pattern = 0,
			And,
			Or,
			AndNot,
			FreeText,
			Head,
			Tail,
			ExactWord,
			SimpleWord,
			Weight,
			Within,
			Word,
			WordList,
			String,
			WordHead,
			WordTail,
			Synonym,
			ExpandSynonym,
			ValueNum
		};
	};
	// オペランドの種類を得る
	Type::Value getType() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	// get analyzer
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

	// インスタンス生成
	static Object* getInstance(int iClassID_);

protected:
#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
	// ハッシュコードを計算する
	virtual ModSize getHashCode();
	// 同じ型のオブジェクト同士でless比較する
	virtual bool compare(const Object& cObj_) const;
#endif

	//constructor
	ContainsOperand(Type::Value type);
	// コンストラクタ
	ContainsOperand(Type::Value type, int numElements);
	// コピーコンストラクター
	ContainsOperand(const ContainsOperand& cOther_);

	virtual const char* getName() const = 0;

/////////////////////////////
// Common::Externalizable::
	virtual int getClassID() const;

/////////////////////////////
// ModSerializable::
	virtual void serialize(ModArchive& cArchive_);

private:
	Type::Value m_eType;
};

namespace Contains
{
	// And/Or/AndNot/Withinで実装を共有するためのクラス
	class LogicalOperation : public ContainsOperand
	{
	public:
		typedef ContainsOperand Super;

		// オペランドを得る
		ContainsOperandList* getOperandList() const;
		// オペランドを設定する
		void setOperandList(ContainsOperandList* list);

		// スコア合成方法指定を得る
		ValueExpression* getCombiner() const;
		// スコア合成方法指定を設定する
		void setCombiner(ValueExpression* combiner);

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

	protected:
		LogicalOperation(Type::Value type);
		LogicalOperation(Type::Value type, int numElements,
						 ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner);
		LogicalOperation(Type::Value type, int numElements,
						 ContainsOperandList* list, ValueExpression* combiner);
		LogicalOperation(const LogicalOperation& cOther_);

	private:
		virtual const char* getPrefix() const = 0;
		virtual const char* getDelimiter() const = 0;
		virtual const char* getPostfix() const = 0;
	};

	// Weight/Head/Tail/ExactWord/SimpleWordで実装を共有するためのクラス
	class SpecialPattern : public ContainsOperand
	{
	public:
		typedef ContainsOperand Super;

		// オペランドを得る
		ContainsOperand* getOperand() const;
		// オペランドを設定する
		void setOperand(ContainsOperand* operand);

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		SpecialPattern(Type::Value type);
		SpecialPattern(Type::Value type, int numElements, ContainsOperand* operand);
		SpecialPattern(const SpecialPattern& cOther_);
	};

//////////////////////////////////////////

	// PatternをあらわすContainsOperand
	class Pattern : public ContainsOperand
	{
	public:
		typedef ContainsOperand Super;

		//constructor
		Pattern()
			: Super(Type::Pattern)
		{}
		Pattern(ValueExpression* pattern);
		Pattern(ValueExpression* pattern, ValueExpression* lang);
		Pattern(const Pattern& cOther_);

		// パターンを得る
		ValueExpression* getPattern() const;
		// パターンを設定する
		void setPattern(ValueExpression* pattern);

		// 言語指定を得る
		ValueExpression* getLanguage() const;
		// 言語指定を設定する
		void setLanguage(ValueExpression* language);

		//コピーする
		Object* copy() const;

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	protected:
		const char* getName() const;
	};

	// AndをあらわすContainsOperand
	class And : public LogicalOperation
	{
	public:
		typedef LogicalOperation Super;

		//constructor
		And()
			: Super(Type::And)
		{}
		And(ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner);
		And(const And& cOther_);

	////////////////////////////
	//LogicalOperation::
	//	// オペランドを得る
	//	ContainsOperandList* getOperandList() const;
	//	// オペランドを設定する
	//	void setOperandList(ContainsOperandList* list);
	//
	//	// スコア合成方法指定を得る
	//	ValueExpression* getCombiner() const;
	//	// スコア合成方法指定を設定する
	//	void setCombiner(ValueExpression* combiner);

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		const char* getName() const;

	private:
	////////////////////////////
	//LogicalOperation::
		virtual const char* getPrefix() const;
		virtual const char* getDelimiter() const;
		virtual const char* getPostfix() const;
	};

	// OrをあらわすContainsOperand
	class Or : public LogicalOperation
	{
	public:
		typedef LogicalOperation Super;

		//constructor
		Or()
			: Super(Type::Or)
		{}
		Or(ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner);
		Or(const Or& cOther_);

	////////////////////////////
	//LogicalOperation::
	//	// オペランドを得る
	//	ContainsOperandList* getOperandList() const;
	//	// オペランドを設定する
	//	void setOperandList(ContainsOperandList* list);
	//
	//	// スコア合成方法指定を得る
	//	ValueExpression* getCombiner() const;
	//	// スコア合成方法指定を設定する
	//	void setCombiner(ValueExpression* combiner);

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		const char* getName() const;

	private:
	////////////////////////////
	//LogicalOperation::
		virtual const char* getPrefix() const;
		virtual const char* getDelimiter() const;
		virtual const char* getPostfix() const;
	};

	// AndNotをあらわすContainsOperand
	class AndNot : public LogicalOperation
	{
	public:
		typedef LogicalOperation Super;

		//constructor
		AndNot()
			: Super(Type::AndNot)
		{}
		AndNot(ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner);
		AndNot(const AndNot& cOther_);

	////////////////////////////
	//LogicalOperation::
	//	// オペランドを得る
	//	ContainsOperandList* getOperandList() const;
	//	// オペランドを設定する
	//	void setOperandList(ContainsOperandList* list);
	//
	//	// スコア合成方法指定を得る
	//	ValueExpression* getCombiner() const;
	//	// スコア合成方法指定を設定する
	//	void setCombiner(ValueExpression* combiner);

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		const char* getName() const;

	private:
	////////////////////////////
	//LogicalOperation::
		virtual const char* getPrefix() const;
		virtual const char* getDelimiter() const;
		virtual const char* getPostfix() const;
	};

	// WithinをあらわすContainsOperand
	class Within : public LogicalOperation
	{
	public:
		typedef LogicalOperation Super;

		enum Value {
			Symmetric = 0,
			Asymmetric,
			ValueNum
		};
		
		//constructor
		Within()
			: Super(Type::Within)
		{}
		Within(ContainsOperandList* list, int iSymmetric, ValueExpression* lower, ValueExpression* upper, ValueExpression* combiner);
		Within(const Within& cOther_);

	////////////////////////////
	//LogicalOperation::
	//	// オペランドを得る
	//	ContainsOperandList* getOperandList() const;
	//	// オペランドを設定する
	//	void setOperandList(ContainsOperandList* list);
	//
	//	// スコア合成方法指定を得る
	//	ValueExpression* getCombiner() const;
	//	// スコア合成方法指定を設定する
	//	void setCombiner(ValueExpression* combiner);

		// 対象性を得る
		int getSymmetric() const;
		void setSymmetric(int symmetric);

		// 距離の下限を得る
		ValueExpression* getLowerDist() const;
		// 距離の下限を設定する
		void setLowerDist(ValueExpression* literal);

		// 距離の上限を得る
		ValueExpression* getUpperDist() const;
		// 距離の下限を設定する
		void setUpperDist(ValueExpression* literal);

		//コピーする
		Object* copy() const;

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
		virtual ModSize getHashCode();
		virtual bool compare(const Object& cObj_) const;
#endif
		const char* getName() const;

	/////////////////////////////
	// ModSerializable::
		virtual void serialize(ModArchive& cArchive_);

	private:
	////////////////////////////
	//LogicalOperation::
		virtual const char* getPrefix() const;
		virtual const char* getDelimiter() const;
		virtual const char* getPostfix() const;

		int m_iSymmetric;
	};

	// SynonymをあらわすContainsOperand
	class Synonym : public LogicalOperation
	{
	public:
		typedef LogicalOperation Super;

		//constructor
		Synonym()
			: Super(Type::Synonym)
		{}
		Synonym(ContainsOperandList* list);
		Synonym(const Synonym& cOther_);

	////////////////////////////
	//LogicalOperation::
	//	// オペランドを得る
	//	ContainsOperandList* getOperandList() const;
	//	// オペランドを設定する
	//	void setOperandList(ContainsOperandList* list);
	//
	//	// スコア合成方法指定を得る
	//	ValueExpression* getCombiner() const;
	//	// スコア合成方法指定を設定する
	//	void setCombiner(ValueExpression* combiner);

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		const char* getName() const;

	private:
	////////////////////////////
	//LogicalOperation::
		virtual const char* getPrefix() const;
		virtual const char* getDelimiter() const;
		virtual const char* getPostfix() const;
	};

	// FreeTextをあらわすContainsOperand
	class FreeText : public ContainsOperand
	{
	public:
		typedef ContainsOperand Super;

		//constructor
		FreeText()
			: Super(Type::FreeText)
		{}
		FreeText(ValueExpression* pattern, ValueExpression* lang,
				 ValueExpression* scaleParam, ValueExpression* wordLimit);
		FreeText(const FreeText& cOther_);

		// パターンを得る
		ValueExpression* getPattern() const;
		// パターンを設定する
		void setPattern(ValueExpression* pattern);

		// 言語指定を得る
		ValueExpression* getLanguage() const;
		// 言語指定を設定する
		void setLanguage(ValueExpression* language);

		// スケールパラメータを得る
		ValueExpression* getScaleParameter() const;
		// スケールパラメータを設定する
		void setScaleParameter(ValueExpression* scaleParam);

		// 特徴語抽出上限を得る
		ValueExpression* getWordLimit() const;
		// 特徴語抽出上限を設定する
		void setWordLimit(ValueExpression* wordLimit);

		// SQL文で値を得る
		ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		const char* getName() const;
	};

	// WeightをあらわすContainsOperand
	class Weight : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		Weight()
			: Super(Type::Weight)
		{}
		Weight(ContainsOperand* operand, ValueExpression* scale);
		Weight(const Weight& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
		
		// スケールを得る
		ValueExpression* getScale() const;
		// 言語指定を設定する
		void setScale(ValueExpression* scale);

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

		// SQL文で値を得る
		virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;


	protected:
		const char* getName() const;
	};

	// HeadをあらわすContainsOperand
	class Head : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		Head()
			: Super(Type::Head)
		{}
		Head(ContainsOperand* operand);
		Head(const Head& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

	// TailをあらわすContainsOperand
	class Tail : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		Tail()
			: Super(Type::Tail)
		{}
		Tail(ContainsOperand* operand);
		Tail(const Tail& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

	// ExactWordをあらわすContainsOperand
	class ExactWord : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		ExactWord()
			: Super(Type::ExactWord)
		{}
		ExactWord(ContainsOperand* operand);
		ExactWord(const ExactWord& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

	// SimpleWordをあらわすContainsOperand
	class SimpleWord : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		SimpleWord()
			: Super(Type::SimpleWord)
		{}
		SimpleWord(ContainsOperand* operand);
		SimpleWord(const SimpleWord& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

	// WordをあらわすContainsOperand
	class Word : public ContainsOperand
	{
	public:
		typedef ContainsOperand Super;

		//constructor
		Word()
			: Super(Type::Word)
		{}
		Word(ContainsOperand* pattern);
		Word(ContainsOperand* pattern, ValueExpression* category, ValueExpression* scale, ValueExpression* lang, ValueExpression* df);
		Word(const Word& cOther_);

		// パターンを得る
		ContainsOperand* getPattern() const;
		// パターンを設定する
		void setPattern(ContainsOperand* pattern);

		// カテゴリーを得る
		ValueExpression* getCategory() const;
		// パターンを設定する
		void setCategory(ValueExpression* category);

		// スケールを得る
		ValueExpression* getScale() const;
		// スケールを設定する
		void setScale(ValueExpression* scale);

		// 言語指定を得る
		ValueExpression* getLanguage() const;
		// 言語指定を設定する
		void setLanguage(ValueExpression* language);

		// DFを得る
		ValueExpression* getDf() const;
		// DFを設定する
		void setDf(ValueExpression* df);

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		const char* getName() const;
	};

	// WordListをあらわすContainsOperand
	class WordList : public ContainsOperand
	{
	public:
		typedef ContainsOperand Super;

		//constructor
		WordList()
			: Super(Type::WordList)
		{}
		WordList(ContainsOperandList* list);
		WordList(const WordList& cOther_);

		// オペランドを得る
		ContainsOperandList* getOperandList() const;
		// オペランドを設定する
		void setOperandList(ContainsOperandList* list);

		//コピーする
		Object* copy() const;

#ifdef USE_OLDER_VERSION
		// Analyzerを得る
		virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

	protected:
		const char* getName() const;
	};

	// StringをあらわすContainsOperand
	class String : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		String()
			: Super(Type::String)
		{}
		String(ContainsOperand* operand);
		String(const String& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

	// WordHeadをあらわすContainsOperand
	class WordHead : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		WordHead()
			: Super(Type::WordHead)
		{}
		WordHead(ContainsOperand* operand);
		WordHead(const WordHead& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

	// WordTailをあらわすContainsOperand
	class WordTail : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		WordTail()
			: Super(Type::WordTail)
		{}
		WordTail(ContainsOperand* operand);
		WordTail(const WordTail& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

	// ExpandSynonymをあらわすContainsOperand
	class ExpandSynonym : public SpecialPattern
	{
	public:
		typedef SpecialPattern Super;

		//constructor
		ExpandSynonym()
			: Super(Type::ExpandSynonym)
		{}
		ExpandSynonym(ContainsOperand* operand);
		ExpandSynonym(const ExpandSynonym& cOther_);

	/////////////////////
	// SpecialPattern::
	//	// オペランドを得る
	//	ContainsOperand* getOperand() const;
	//	// オペランドを設定する
	//	void setOperand(ContainsOperand* operand);
	//	// Analyzerを得る
	//	virtual const Analysis::Analyzer* getAnalyzer() const;

		//コピーする
		Object* copy() const;

	protected:
		const char* getName() const;
	};

} // namespace Contains

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_CONTAINSOPERAND_H

//
// Copyright (c) 2004, 2007, 2008, 2009, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
