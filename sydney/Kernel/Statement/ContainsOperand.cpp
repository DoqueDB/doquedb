// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsOperand.cpp -- ContainsOperand
// 
// Copyright (c) 2004, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Statement";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/ContainsOperand.h"
#include "Statement/ContainsOperandList.h"
#include "Statement/Externalizable.h"
#include "Statement/Utility.h"
#include "Statement/ValueExpression.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/ContainsOperand.h"
#endif
#include "Analysis/Value/ContainsOperand.h"

#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING

namespace
{
namespace _LogicalOperation
{
	namespace _Member
	{
		enum Value
		{
			OperandList = 0,
			Combiner,
			ValueNum
		};
	}
}
namespace _SpecialPattern
{
	namespace _Member
	{
		enum Value
		{
			Operand = 0,
			ValueNum
		};
	}
}
namespace _Pattern
{
	namespace _Member
	{
		enum Value
		{
			Pattern = 0,
			Language,
			ValueNum
		};
	}
}
namespace _And
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _LogicalOperation::_Member::ValueNum
		};
	}
}
namespace _Or
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _LogicalOperation::_Member::ValueNum
		};
	}
}
namespace _AndNot
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _LogicalOperation::_Member::ValueNum
		};
	}
}
namespace _Within
{
	namespace _Member
	{
		enum Value
		{
			LowerDist = _LogicalOperation::_Member::ValueNum,
			UpperDist,
			ValueNum
		};
	}
}
namespace _Synonym
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _LogicalOperation::_Member::ValueNum
		};
	}
}
namespace _FreeText
{
	namespace _Member
	{
		enum Value
		{
			Pattern = 0,
			Language,
			ScaleParameter,
			WordLimit,
			ValueNum
		};
	}
}
namespace _Weight
{
	namespace _Member
	{
		enum Value
		{
			Scale = _SpecialPattern::_Member::ValueNum,
			ValueNum
		};
	}
}
namespace _Head
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _SpecialPattern::_Member::ValueNum
		};
	}
}
namespace _Tail
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _SpecialPattern::_Member::ValueNum
		};
	}
}
namespace _ExactWord
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _SpecialPattern::_Member::ValueNum
		};
	}
}
namespace _SimpleWord
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _SpecialPattern::_Member::ValueNum
		};
	}
}
namespace _Word
{
	namespace _Member
	{
		enum Value
		{
			Pattern = 0,
			Category,
			Scale,
			Language,
			Df,
			ValueNum
		};
	}
}
namespace _WordList
{
	namespace _Member
	{
		enum Value
		{
			OperandList = 0,
			ValueNum
		};
	}
}
namespace _String
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _SpecialPattern::_Member::ValueNum
		};
	}
}
namespace _WordHead
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _SpecialPattern::_Member::ValueNum
		};
	}
}
namespace _WordTail
{
	namespace _Member
	{
		enum Value
		{
			ValueNum = _SpecialPattern::_Member::ValueNum
		};
	}
}
namespace _ExpandSynonym
{
	namespace _Member
	{
		enum Value
		{
			Operand = _SpecialPattern::_Member::ValueNum,
			ValueNum
		};
	}
}
}

_SYDNEY_STATEMENT_USING

// FUNCTION public
//	Statement::ContainsOperand::ContainsOperand -- constructor
//
// NOTES
//
// ARGUMENTS
//	Type::Value type
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//constructor
ContainsOperand::
ContainsOperand(Type::Value type)
	: Object(ObjectType::ContainsOperand),
	  m_eType(type)
{}

// FUNCTION public
//	Statement::ContainsOperand::ContainsOperand -- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//	Type::Value type
//	int numElements
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ContainsOperand::
ContainsOperand(Type::Value type, int numElements)
	: Object(ObjectType::ContainsOperand, numElements),
	  m_eType(type)
{}

// FUNCTION public
//	Statement::ContainsOperand::ContainsOperand -- コピーコンストラクター
//
// NOTES
//
// ARGUMENTS
//	const ContainsOperand& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

ContainsOperand::
ContainsOperand(const ContainsOperand& cOther_)
	: Object(cOther_),
	  m_eType(cOther_.m_eType)
{}

// FUNCTION public
//	Statement::ContainsOperand::getType -- オペランドの種類を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ContainsOperand::Type::Value
//
// EXCEPTIONS

ContainsOperand::Type::Value
ContainsOperand::
getType() const
{
	return m_eType;
}

// FUNCTION public
//	Statement::ContainsOperand::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
ContainsOperand::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;

	stream << getName();

	if (int n = m_vecpElements.getSize()) {
		char delim = '(';
		int i = 0;
		do {
			if (m_vecpElements[i])
				stream << delim << m_vecpElements[i]->toSQLStatement(bForCascade_);
			delim = ' ';
		} while (++i < n);
		stream << ')';
	}

	return stream.getString();
}

// FUNCTION public
//	Statement::ContainsOperand::getAnalyzer2 -- get analyzer
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
ContainsOperand::
getAnalyzer2() const
{
	return Analysis::Value::ContainsOperand::create(this);
}

// FUNCTION public
//	Statement::ContainsOperand::getInstance -- インスタンス生成
//
// NOTES
//
// ARGUMENTS
//	int iClassID_
//	
// RETURN
//	Object*
//
// EXCEPTIONS

//static
Object*
ContainsOperand::
getInstance(int iClassID_)
{
	switch (iClassID_ - Statement::Externalizable::ClassID::ContainsOperand) {
	case Type::Pattern:		return new Contains::Pattern;
	case Type::And:			return new Contains::And;
	case Type::Or:			return new Contains::Or;
	case Type::AndNot:		return new Contains::AndNot;
	case Type::FreeText:	return new Contains::FreeText;
	case Type::Head:		return new Contains::Head;
	case Type::Tail:		return new Contains::Tail;
	case Type::ExactWord:	return new Contains::ExactWord;
	case Type::SimpleWord:	return new Contains::SimpleWord;
	case Type::Weight:		return new Contains::Weight;
	case Type::Within:		return new Contains::Within;
	case Type::Word:		return new Contains::Word;
	case Type::WordList:	return new Contains::WordList;
	case Type::String:		return new Contains::String;
	case Type::WordHead:	return new Contains::WordHead;
	case Type::WordTail:	return new Contains::WordTail;
	case Type::Synonym:		return new Contains::Synonym;
	case Type::ExpandSynonym:return new Contains::ExpandSynonym;
	default:				return 0;
	}
}

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない

// FUNCTION public
//	Statement::ContainsOperand::getHashCode -- ハッシュコードを計算する
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
ContainsOperand::
getHashCode()
{
	ModSize value = Super::getHashCode();
	value <<= 4;
	value += static_cast<ModSize>(m_eType);
	return value;
}

// FUNCTION public
//	Statement::ContainsOperand::compare -- 同じ型のオブジェクト同士でless比較する
//
// NOTES
//
// ARGUMENTS
//	const Object& cObj_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ContainsOperand::
compare(const Object& cObj_) const
{
	return Super::compare(cObj_)
		|| (m_eType < _SYDNEY_DYNAMIC_CAST(const ContainsOperand&, cObj_).m_eType);
}
#endif

// FUNCTION public
//	Statement::ContainsOperand::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
ContainsOperand::
getClassID() const
{
	return Statement::Externalizable::ClassID::ContainsOperand + m_eType;
}

// FUNCTION public
//	Statement::ContainsOperand::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ContainsOperand::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::EnumValue(cArchive_, m_eType);
}

//////////////////
// LogicalOperation
//////////////////

// FUNCTION public
//	Statement::Contains::LogicalOperation::LogicalOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Type::Value type
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::LogicalOperation::
LogicalOperation(Type::Value type)
	: Super(type)
{}

// FUNCTION public
//	Statement::Contains::LogicalOperation::LogicalOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Type::Value type
//	int numElements
//	ContainsOperand* operand0
//	ContainsOperand* operand1
//	ValueExpression* combiner
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::LogicalOperation::
LogicalOperation(Type::Value type, int numElements,
				 ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner)
	: Super(type, numElements)
{
	setCombiner(combiner);

	ModAutoPointer<ContainsOperandList> list = new ContainsOperandList;
	if (operand0->getType() == getType()) {
		LogicalOperation* p = _SYDNEY_DYNAMIC_CAST(LogicalOperation*, operand0);
		; _SYDNEY_ASSERT(p);
		// combinerが指定されていなければ合成できる
		if (p->getCombiner() == 0)
		{
			ContainsOperandList* l = p->getOperandList();
			list->merge(*l); // lの要素は空になるので2重解放はない
			delete p;
		}
		else
			list->append(operand0);
	} else
		list->append(operand0);

	if (operand1->getType() == getType()) {
		LogicalOperation* p = _SYDNEY_DYNAMIC_CAST(LogicalOperation*, operand1);
		; _SYDNEY_ASSERT(p);
		// combinerが指定されていなければ合成できる
		if (p->getCombiner() == 0)
		{
			ContainsOperandList* l = p->getOperandList();
			list->merge(*l); // lの要素は空になるので2重開放はない
			delete p;
		}
		else
			list->append(operand1);
	} else
		list->append(operand1);

	setOperandList(list.release());
}

// FUNCTION public
//	Statement::Contains::LogicalOperation::LogicalOperation -- 
//
// NOTES
//
// ARGUMENTS
//	Type::Value type
//	int numElements
//	ContainsOperandList* list
//	ValueExpression* combiner
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::LogicalOperation::
LogicalOperation(Type::Value type, int numElements,
				 ContainsOperandList* list, ValueExpression* combiner)
	: Super(type, numElements)
{
	setOperandList(list);
	setCombiner(combiner);
}

// FUNCTION public
//	Statement::Contains::LogicalOperation::LogicalOperation -- 
//
// NOTES
//
// ARGUMENTS
//	const LogicalOperation& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::LogicalOperation::
LogicalOperation(const LogicalOperation& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::LogicalOperation::getOperandList -- オペランドを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ContainsOperandList*
//
// EXCEPTIONS

ContainsOperandList*
Contains::LogicalOperation::
getOperandList() const
{
	return _SYDNEY_DYNAMIC_CAST(ContainsOperandList*, getElement(_LogicalOperation::_Member::OperandList, ObjectType::ContainsOperandList));
}

// FUNCTION public
//	Statement::Contains::LogicalOperation::setOperandList -- オペランドを設定する
//
// NOTES
//
// ARGUMENTS
//	ContainsOperandList* list
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::LogicalOperation::
setOperandList(ContainsOperandList* list)
{
	setElement(_LogicalOperation::_Member::OperandList, list);
}

// FUNCTION public
//	Statement::Contains::LogicalOperation::getCombiner -- スコアの合成方法指定を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::LogicalOperation::
getCombiner() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_LogicalOperation::_Member::Combiner, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::LogicalOperation::setCombiner -- スコアの合成方法指定を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* combiner
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::LogicalOperation::
setCombiner(ValueExpression* combiner)
{
	setElement(_LogicalOperation::_Member::Combiner, combiner);
}

// FUNCTION public
//	Statement::Contains::LogicalOperation::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Contains::LogicalOperation::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << getPrefix();
	ContainsOperandList* pOperandList = getOperandList();
	int n = pOperandList->getCount();
	for (int i = 0; i < n; ++i) {
		if (i > 0) stream << getDelimiter();
		stream << pOperandList->getAt(i)->toSQLStatement(bForCascade_);
	}
	ValueExpression* pCombiner = getCombiner();
	if (pCombiner) {
		stream << " combiner " << pCombiner->toSQLStatement(bForCascade_);
	}
	stream << getPostfix();
	return stream.getString();
}

//////////////////
// SpecialPattern
//////////////////

// FUNCTION public
//	Statement::Contains::SpecialPattern::SpecialPattern -- 
//
// NOTES
//
// ARGUMENTS
//	Type::Value type
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::SpecialPattern::
SpecialPattern(Type::Value type)
	: Super(type)
{}

// FUNCTION public
//	Statement::Contains::SpecialPattern::SpecialPattern -- 
//
// NOTES
//
// ARGUMENTS
//	Type::Value type
//	int numElements
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::SpecialPattern::
SpecialPattern(Type::Value type, int numElements, ContainsOperand* operand)
	: Super(type, numElements)
{
	setOperand(operand);
}

// FUNCTION public
//	Statement::Contains::SpecialPattern::SpecialPattern -- 
//
// NOTES
//
// ARGUMENTS
//	const SpecialPattern& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::SpecialPattern::
SpecialPattern(const SpecialPattern& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::SpecialPattern::getOperand -- パターンを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ContainsOperand*
//
// EXCEPTIONS

ContainsOperand*
Contains::SpecialPattern::
getOperand() const
{
	return _SYDNEY_DYNAMIC_CAST(ContainsOperand*, getElement(_SpecialPattern::_Member::Operand, ObjectType::ContainsOperand));
}

// FUNCTION public
//	Statement::Contains::SpecialPattern::setOperand -- パターンを設定する
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::SpecialPattern::
setOperand(ContainsOperand* operand)
{
	setElement(_SpecialPattern::_Member::Operand, operand);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::ContainsOperand_SpecialPattern _analyzer_special;
}

// FUNCTION public
//	Statement::Contains::SpecialPattern::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::SpecialPattern::
getAnalyzer() const
{
	return &_analyzer_special;
}
#endif

//////////////////
// Pattern
//////////////////

// FUNCTION public
//	Statement::Contains::Pattern::Pattern -- 
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pattern
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Pattern::
Pattern(ValueExpression* pattern)
	: Super(Type::Pattern, _Pattern::_Member::ValueNum)
{
	setPattern(pattern);
}

// FUNCTION public
//	Statement::Contains::Pattern::Pattern -- 
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pattern
//	ValueExpression* lang
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Pattern::
Pattern(ValueExpression* pattern, ValueExpression* lang)
	: Super(Type::Pattern, _Pattern::_Member::ValueNum)
{
	setPattern(pattern);
	setLanguage(lang);
}

// FUNCTION public
//	Statement::Contains::Pattern::Pattern -- 
//
// NOTES
//
// ARGUMENTS
//	const Pattern& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Pattern::
Pattern(const Pattern& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::Pattern::getPattern -- パターンを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Pattern::
getPattern() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_Pattern::_Member::Pattern, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Pattern::setPattern -- パターンを設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pattern
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Pattern::
setPattern(ValueExpression* pattern)
{
	setElement(_Pattern::_Member::Pattern, pattern);
}

// FUNCTION public
//	Statement::Contains::Pattern::getLanguage -- 言語指定を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Pattern::
getLanguage() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_Pattern::_Member::Language, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Pattern::setLanguage -- 言語指定を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* lang
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Pattern::
setLanguage(ValueExpression* lang)
{
	setElement(_Pattern::_Member::Language, lang);
}

// FUNCTION public
//	Statement::Contains::Pattern::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Contains::Pattern::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ValueExpression* pLanguage = getLanguage();
	if (pLanguage) {
		ModUnicodeOstrStream stream;
		stream << getPattern()->toSQLStatement(bForCascade_);
		stream << " language " << pLanguage->toSQLStatement(bForCascade_);
		return stream.getString();
	} else {
		return getPattern()->toSQLStatement(bForCascade_);
	}
}

// FUNCTION public
//	Statement::Contains::Pattern::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Pattern::
copy() const
{
	return new Pattern(*this);
}

namespace
{
	const char* const _name_pattern = "pattern";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_Pattern _analyzer_pattern;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::Pattern::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::Pattern::
getAnalyzer() const
{
	return &_analyzer_pattern;
}
#endif

// FUNCTION public
//	Statement::Contains::Pattern::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Pattern::
getName() const
{
	return _name_pattern;
}

//////////////////
// And
//////////////////

// FUNCTION public
//	Statement::Contains::And::And -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand0
//	ContainsOperand* operand1
//	ValueExpression* combiner
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::And::
And(ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner)
	: Super(Type::And, _And::_Member::ValueNum, operand0, operand1, combiner)
{}

// FUNCTION public
//	Statement::Contains::And::And -- 
//
// NOTES
//
// ARGUMENTS
//	const And& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::And::
And(const And& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::And::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::And::
copy() const
{
	return new And(*this);
}

namespace
{
	const char* const _name_and = "and";
	const char* const _prefix_and = "(";
	const char* const _delimiter_and = " & ";
	const char* const _postfix_and = ")";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_And _analyzer_and;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::And::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::And::
getAnalyzer() const
{
	return &_analyzer_and;
}
#endif

// FUNCTION public
//	Statement::Contains::And::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::And::
getName() const
{
	return _name_and;
}

// FUNCTION private
//	Statement::Contains::And::getPrefix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::And::
getPrefix() const
{
	return _prefix_and;
}

// FUNCTION private
//	Statement::Contains::And::getDelimiter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::And::
getDelimiter() const
{
	return _delimiter_and;
}

// FUNCTION private
//	Statement::Contains::And::getPostfix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::And::
getPostfix() const
{
	return _postfix_and;
}

//////////////////
// Or
//////////////////

// FUNCTION public
//	Statement::Contains::Or::Or -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand0
//	ContainsOperand* operand1
//	ValueExpression* combiner
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Or::
Or(ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner)
	: Super(Type::Or, _Or::_Member::ValueNum, operand0, operand1, combiner)
{}

// FUNCTION public
//	Statement::Contains::Or::Or -- 
//
// NOTES
//
// ARGUMENTS
//	const Or& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Or::
Or(const Or& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::Or::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Or::
copy() const
{
	return new Or(*this);
}

namespace
{
	const char* const _name_or = "or";
	const char* const _prefix_or = "(";
	const char* const _delimiter_or = " | ";
	const char* const _postfix_or = ")";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_Or _analyzer_or;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::Or::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::Or::
getAnalyzer() const
{
	return &_analyzer_or;
}
#endif

// FUNCTION public
//	Statement::Contains::Or::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Or::
getName() const
{
	return _name_or;
}

// FUNCTION private
//	Statement::Contains::Or::getPrefix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Or::
getPrefix() const
{
	return _prefix_or;
}

// FUNCTION private
//	Statement::Contains::Or::getDelimiter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Or::
getDelimiter() const
{
	return _delimiter_or;
}

// FUNCTION private
//	Statement::Contains::Or::getPostfix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Or::
getPostfix() const
{
	return _postfix_or;
}

//////////////////
// AndNot
//////////////////

// FUNCTION public
//	Statement::Contains::AndNot::AndNot -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand0
//	ContainsOperand* operand1
//	ValueExpression* combiner
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::AndNot::
AndNot(ContainsOperand* operand0, ContainsOperand* operand1, ValueExpression* combiner)
	: Super(Type::AndNot, _AndNot::_Member::ValueNum, operand0, operand1, combiner)
{}

// FUNCTION public
//	Statement::Contains::AndNot::AndNot -- 
//
// NOTES
//
// ARGUMENTS
//	const AndNot& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::AndNot::
AndNot(const AndNot& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::AndNot::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::AndNot::
copy() const
{
	return new AndNot(*this);
}

namespace
{
	const char* const _name_andNot = "andNot";
	const char* const _prefix_andNot = "(";
	const char* const _delimiter_andNot = " - ";
	const char* const _postfix_andNot = ")";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_AndNot _analyzer_andNot;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::AndNot::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::AndNot::
getAnalyzer() const
{
	return &_analyzer_andNot;
}
#endif

// FUNCTION public
//	Statement::Contains::AndNot::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::AndNot::
getName() const
{
	return _name_andNot;
}

// FUNCTION private
//	Statement::Contains::AndNot::getPrefix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::AndNot::
getPrefix() const
{
	return _prefix_andNot;
}

// FUNCTION private
//	Statement::Contains::AndNot::getDelimiter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::AndNot::
getDelimiter() const
{
	return _delimiter_andNot;
}

// FUNCTION private
//	Statement::Contains::AndNot::getPostfix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::AndNot::
getPostfix() const
{
	return _postfix_andNot;
}

//////////////////
// Within
//////////////////

// FUNCTION public
//	Statement::Contains::Within::Within -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperandList* list
//	int symmetric
//	ValueExpression* lower
//	ValueExpression* upper
//	ValueExpression* combiner
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Within::
Within(ContainsOperandList* list, int symmetric, ValueExpression* lower, ValueExpression* upper, ValueExpression* combiner)
	: Super(Type::Within, _Within::_Member::ValueNum, list, combiner)
{
	setSymmetric(symmetric);
	setLowerDist(lower);
	setUpperDist(upper);
}

// FUNCTION public
//	Statement::Contains::Within::Within -- 
//
// NOTES
//
// ARGUMENTS
//	const Within& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Within::
Within(const Within& cOther_)
	: Super(cOther_),
	  m_iSymmetric(cOther_.m_iSymmetric)
{}

// FUNCTION public
//	Statement::Contains::Within::getSymmetric -- 対象性を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Contains::Within::getSymmetric() const
{
	return m_iSymmetric;
}

// FUNCTION public
//	Statement::Contains::Within::setSymmetric -- 対象性を設定する
//
// NOTES
//
// ARGUMENTS
//	int iSymmetric
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Within::setSymmetric(int iSymmetric)
{
	m_iSymmetric = iSymmetric;
}

// FUNCTION public
//	Statement::Contains::Within::getLowerDist -- 距離の下限を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Within::getLowerDist() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*,
								getElement(_Within::_Member::LowerDist,
										   ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Within::setLowerDist -- 距離の下限を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* lower
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Within::setLowerDist(ValueExpression* lower)
{
	setElement(_Within::_Member::LowerDist, lower);
}

// FUNCTION public
//	Statement::Contains::Within::getUpperDist -- 距離の上限を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Within::getUpperDist() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*,
								getElement(_Within::_Member::UpperDist,
										   ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Within::setUpperDist -- 距離の上限を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* upper
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Within::setUpperDist(ValueExpression* upper)
{
	setElement(_Within::_Member::UpperDist, upper);
}

// FUNCTION public
//	Statement::Contains::Within::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Within::
copy() const
{
	return new Within(*this);
}

// FUNCTION public
//	Statement::Contains::Within::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Contains::Within::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << getPrefix();
	ContainsOperandList* pOperandList = getOperandList();
	int n = pOperandList->getCount();
	for (int i = 0; i < n; ++i) {
		if (i > 0) stream << getDelimiter();
		stream << pOperandList->getAt(i)->toSQLStatement(bForCascade_);
	}
	if (getSymmetric() == Asymmetric) {
		stream << " asymmetric";
	}
	ValueExpression* pLower = getLowerDist();
	if (pLower) {
		stream << " lower " << pLower->toSQLStatement(bForCascade_);
	}
	stream << " upper " << getUpperDist()->toSQLStatement(bForCascade_);
	stream << getPostfix();
	return stream.getString();
}

namespace
{
	const char* const _name_within = "within";
	const char* const _name_within_asym = "within_asym";
	const char* const _prefix_within = "within(";
	const char* const _delimiter_within = " ";
	const char* const _postfix_within = ")";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_Within _analyzer_within;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::Within::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::Within::
getAnalyzer() const
{
	return &_analyzer_within;
}
#endif

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
// FUNCTION public
//	Statement::Contains::Within::getHashCode -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
Contains::Within::
getHashCode()
{
	ModSize value = Super::getHashCode();
	value <<= 4;
	value += m_iSymmetric;
	return value;
}

// FUNCTION public
//	Statement::Contains::Within::compare -- 
//
// NOTES
//
// ARGUMENTS
//	const Object& cObj_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Contains::Within::
compare(const Object& cObj_) const
{
	return Super::compare(cObj_)
		|| getType() < _SYDNEY_DYNAMIC_CAST(const ContainsOperand&, cObj_).getType()
		|| (getType() == _SYDNEY_DYNAMIC_CAST(const ContainsOperand&, cObj_).getType()
			&& m_iSymmetric == _SYDNEY_DYNAMIC_CAST(const Contains::Within&, cObj_).m_iSymmetric);
}
#endif

// FUNCTION public
//	Statement::Contains::Within::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Within::
getName() const
{
	return m_iSymmetric == Symmetric ? _name_within : _name_within_asym;
}

// FUNCTION private
//	Statement::Contains::Within::getPrefix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Within::
getPrefix() const
{
	return _prefix_within;
}

// FUNCTION private
//	Statement::Contains::Within::getDelimiter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Within::
getDelimiter() const
{
	return _delimiter_within;
}

// FUNCTION private
//	Statement::Contains::Within::getPostfix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Within::
getPostfix() const
{
	return _postfix_within;
}

// FUNCTION public
//	Statement::Contains::Within::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Contains::Within::
serialize(ModArchive& cArchive_)
{
	Super::serialize(cArchive_);
	cArchive_(m_iSymmetric);
}

//////////////////
// Synonym
//////////////////

// FUNCTION public
//	Statement::Contains::Synonym::Synonym -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperandList* list
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Synonym::
Synonym(ContainsOperandList* list)
	: Super(Type::Synonym, _Synonym::_Member::ValueNum, list, 0)
{
}

// FUNCTION public
//	Statement::Contains::Synonym::Synonym -- 
//
// NOTES
//
// ARGUMENTS
//	const Synonym& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Synonym::
Synonym(const Synonym& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::Synonym::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Synonym::
copy() const
{
	return new Synonym(*this);
}

namespace
{
	const char* const _name_synonym = "synonym";
	const char* const _prefix_synonym = "synonym(";
	const char* const _delimiter_synonym = " ";
	const char* const _postfix_synonym = ")";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_Synonym _analyzer_synonym;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::Synonym::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::Synonym::
getAnalyzer() const
{
	return &_analyzer_synonym;
}
#endif

// FUNCTION public
//	Statement::Contains::Synonym::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Synonym::
getName() const
{
	return _name_synonym;
}

// FUNCTION private
//	Statement::Contains::Synonym::getPrefix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Synonym::
getPrefix() const
{
	return _prefix_synonym;
}

// FUNCTION private
//	Statement::Contains::Synonym::getDelimiter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Synonym::
getDelimiter() const
{
	return _delimiter_synonym;
}

// FUNCTION private
//	Statement::Contains::Synonym::getPostfix -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
Contains::Synonym::
getPostfix() const
{
	return _postfix_synonym;
}

//////////////////
// FreeText
//////////////////

// FUNCTION public
//	Statement::Contains::FreeText::FreeText -- 
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pattern
//	ValueExpression* lang
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::FreeText::
FreeText(ValueExpression* pattern, ValueExpression* lang,
		 ValueExpression* scaleParam, ValueExpression* wordLimit)
	: Super(Type::FreeText, _FreeText::_Member::ValueNum)
{
	setPattern(pattern);
	setLanguage(lang);
	setScaleParameter(scaleParam);
	setWordLimit(wordLimit);
}

// FUNCTION public
//	Statement::Contains::FreeText::FreeText -- 
//
// NOTES
//
// ARGUMENTS
//	const FreeText& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::FreeText::
FreeText(const FreeText& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::FreeText::getPattern -- パターンを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::FreeText::
getPattern() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_FreeText::_Member::Pattern, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::FreeText::setPattern -- パターンを設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pattern
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::FreeText::
setPattern(ValueExpression* pattern)
{
	setElement(_FreeText::_Member::Pattern, pattern);
}

// FUNCTION public
//	Statement::Contains::FreeText::getLanguage -- 言語指定を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::FreeText::
getLanguage() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_FreeText::_Member::Language, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::FreeText::setLanguage -- 言語指定を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* lang
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::FreeText::
setLanguage(ValueExpression* lang)
{
	setElement(_FreeText::_Member::Language, lang);
}

// FUNCTION public
//	Statement::Contains::FreeText::getScaleParameter -- スケールパラメータを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::FreeText::
getScaleParameter() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_FreeText::_Member::ScaleParameter,
				   ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::FreeText::setScaleParameter
//		-- スケールパラメータを設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* scaleParam
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::FreeText::
setScaleParameter(ValueExpression* scaleParam)
{
	setElement(_FreeText::_Member::ScaleParameter, scaleParam);
}

// FUNCTION public
//	Statement::Contains::FreeText::getWordLimit -- 特徴語抽出上限を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::FreeText::
getWordLimit() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_FreeText::_Member::WordLimit,
				   ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::FreeText::setWordLimit
//		-- 特徴語抽出上限を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* wordLimit
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::FreeText::
setWordLimit(ValueExpression* wordLimit)
{
	setElement(_FreeText::_Member::WordLimit, wordLimit);
}

// FUNCTION public
//	Statement::Contains::FreeText::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
Contains::FreeText::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;

	stream << getName() << '(';
	stream << getPattern()->toSQLStatement(bForCascade_);
	
	ValueExpression* p = getLanguage();
	if (p) {
		stream << " language " << p->toSQLStatement(bForCascade_);
	}

	p = getScaleParameter();
	if (p) {
		stream << " scale parameter " << p->toSQLStatement(bForCascade_);
	}

	p = getWordLimit();
	if (p) {
		stream << " word limit " << p->toSQLStatement(bForCascade_);
	}
	stream << ')';

	return stream.getString();
}

// FUNCTION public
//	Statement::Contains::FreeText::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::FreeText::
copy() const
{
	return new FreeText(*this);
}

namespace
{
	const char* const _name_freeText = "freetext";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_FreeText _analyzer_freeText;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::FreeText::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::FreeText::
getAnalyzer() const
{
	return &_analyzer_freeText;
}
#endif

// FUNCTION public
//	Statement::Contains::FreeText::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::FreeText::
getName() const
{
	return _name_freeText;
}

//////////////////
// Weight
//////////////////

// FUNCTION public
//	Statement::Contains::Weight::Weight -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	ValueExpression* scale
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Weight::
Weight(ContainsOperand* operand, ValueExpression* scale)
	: Super(Type::Weight, _Weight::_Member::ValueNum, operand)
{
	setScale(scale);
}

// FUNCTION public
//	Statement::Contains::Weight::Weight -- 
//
// NOTES
//
// ARGUMENTS
//	const Weight& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Weight::
Weight(const Weight& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::Weight::getScale -- スケールを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Weight::
getScale() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_Weight::_Member::Scale, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Weight::setScale -- パターンを設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* scale
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Weight::
setScale(ValueExpression* scale)
{
	setElement(_Weight::_Member::Scale, scale);
}

// FUNCTION public
//	Statement::Contains::Weight::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Weight::
copy() const
{
	return new Weight(*this);
}

namespace
{
	const char* const _name_weight = "weight";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_Weight _analyzer_weight;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::Weight::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::Weight::
getAnalyzer() const
{
	return &_analyzer_weight;
}
#endif

// FUNCTION public
//	Statement::Contains::Weight::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

ModUnicodeString
Contains::Weight::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	cStream << getName();
	cStream << '(';
	cStream << getOperand()->toSQLStatement(bForCascade_);
	cStream << " scale ";
	cStream << getScale()->toSQLStatement(bForCascade_);
	cStream << ')';
	return cStream.getString();
}


// FUNCTION public
//	Statement::Contains::Weight::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Weight::
getName() const
{
	return _name_weight;
}

//////////////////
// Head
//////////////////

// FUNCTION public
//	Statement::Contains::Head::Head -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Head::
Head(ContainsOperand* operand)
	: Super(Type::Head, _Head::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::Head::Head -- 
//
// NOTES
//
// ARGUMENTS
//	const Head& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Head::
Head(const Head& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::Head::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Head::
copy() const
{
	return new Head(*this);
}

namespace
{
	const char* const _name_head = "head";
}

// FUNCTION public
//	Statement::Contains::Head::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Head::
getName() const
{
	return _name_head;
}

//////////////////
// Tail
//////////////////

// FUNCTION public
//	Statement::Contains::Tail::Tail -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Tail::
Tail(ContainsOperand* operand)
	: Super(Type::Tail, _Tail::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::Tail::Tail -- 
//
// NOTES
//
// ARGUMENTS
//	const Tail& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Tail::
Tail(const Tail& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::Tail::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Tail::
copy() const
{
	return new Tail(*this);
}

namespace
{
	const char* const _name_tail = "tail";
}

// FUNCTION public
//	Statement::Contains::Tail::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Tail::
getName() const
{
	return _name_tail;
}

//////////////////
// ExactWord
//////////////////

// FUNCTION public
//	Statement::Contains::ExactWord::ExactWord -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::ExactWord::
ExactWord(ContainsOperand* operand)
	: Super(Type::ExactWord, _ExactWord::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::ExactWord::ExactWord -- 
//
// NOTES
//
// ARGUMENTS
//	const ExactWord& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::ExactWord::
ExactWord(const ExactWord& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::ExactWord::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::ExactWord::
copy() const
{
	return new ExactWord(*this);
}

namespace
{
	const char* const _name_exactWord = "exactWord";
}

// FUNCTION public
//	Statement::Contains::ExactWord::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::ExactWord::
getName() const
{
	return _name_exactWord;
}

//////////////////
// SimpleWord
//////////////////

// FUNCTION public
//	Statement::Contains::SimpleWord::SimpleWord -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::SimpleWord::
SimpleWord(ContainsOperand* operand)
	: Super(Type::SimpleWord, _SimpleWord::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::SimpleWord::SimpleWord -- 
//
// NOTES
//
// ARGUMENTS
//	const SimpleWord& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::SimpleWord::
SimpleWord(const SimpleWord& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::SimpleWord::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::SimpleWord::
copy() const
{
	return new SimpleWord(*this);
}

namespace
{
	const char* const _name_simpleWord = "simpleWord";
}

// FUNCTION public
//	Statement::Contains::SimpleWord::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::SimpleWord::
getName() const
{
	return _name_simpleWord;
}

//////////////////
// Word
//////////////////

// FUNCTION public
//	Statement::Contains::Word::Word -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* pattern
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Word::
Word(ContainsOperand* pattern)
	: Super(Type::Word, _Word::_Member::ValueNum)
{
	setPattern(pattern);
}

// FUNCTION public
//	Statement::Contains::Word::Word -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* pattern
//	ValueExpression* category
//	ValueExpression* scale
//	ValueExpression* lang
//	ValueExpression* df
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Word::
Word(ContainsOperand* pattern, ValueExpression* category, ValueExpression* scale, ValueExpression* lang, ValueExpression* df)
	: Super(Type::Word, _Word::_Member::ValueNum)
{
	setPattern(pattern);
	setCategory(category);
	setScale(scale);
	setLanguage(lang);
	setDf(df);
}

// FUNCTION public
//	Statement::Contains::Word::Word -- 
//
// NOTES
//
// ARGUMENTS
//	const Word& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::Word::
Word(const Word& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::Word::getPattern -- パターンを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ContainsOperand*
//
// EXCEPTIONS

ContainsOperand*
Contains::Word::
getPattern() const
{
	return _SYDNEY_DYNAMIC_CAST(ContainsOperand*, getElement(_Word::_Member::Pattern, ObjectType::ContainsOperand));
}

// FUNCTION public
//	Statement::Contains::Word::setPattern -- パターンを設定する
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* pattern
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Word::
setPattern(ContainsOperand* pattern)
{
	setElement(_Word::_Member::Pattern, pattern);
}

// FUNCTION public
//	Statement::Contains::Word::getCategory -- カテゴリーを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Word::
getCategory() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_Word::_Member::Category, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Word::setCategory -- カテゴリーを設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* category
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Word::
setCategory(ValueExpression* category)
{
	setElement(_Word::_Member::Category, category);
}

// FUNCTION public
//	Statement::Contains::Word::getScale -- スケールを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Word::
getScale() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_Word::_Member::Scale, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Word::setScale -- スケールを設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* scale
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Word::
setScale(ValueExpression* scale)
{
	setElement(_Word::_Member::Scale, scale);
}

// FUNCTION public
//	Statement::Contains::Word::getLanguage -- 言語指定を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Word::
getLanguage() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_Word::_Member::Language, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Word::setLanguage -- 言語指定を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* language
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Word::
setLanguage(ValueExpression* language)
{
	setElement(_Word::_Member::Language, language);
}

// FUNCTION public
//	Statement::Contains::Word::getDf -- Dfを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
Contains::Word::
getDf() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(_Word::_Member::Df, ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::Contains::Word::setDf -- Dfを設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* df
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::Word::
setDf(ValueExpression* df)
{
	setElement(_Word::_Member::Df, df);
}

// FUNCTION public
//	Statement::Contains::Word::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::Word::
copy() const
{
	return new Word(*this);
}

namespace
{
	const char* const _name_word = "";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_Word _analyzer_word;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::Word::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::Word::
getAnalyzer() const
{
	return &_analyzer_word;
}
#endif

// FUNCTION public
//	Statement::Contains::Word::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::Word::
getName() const
{
	return _name_word;
}

//////////////////
// WordList
//////////////////

// FUNCTION public
//	Statement::Contains::WordList::WordList -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperandList* list
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::WordList::
WordList(ContainsOperandList* list)
	: Super(Type::WordList, _WordList::_Member::ValueNum)
{
	setOperandList(list);
}

// FUNCTION public
//	Statement::Contains::WordList::WordList -- 
//
// NOTES
//
// ARGUMENTS
//	const WordList& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::WordList::
WordList(const WordList& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::WordList::getOperandList -- オペランドを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ContainsOperandList*
//
// EXCEPTIONS

ContainsOperandList*
Contains::WordList::
getOperandList() const
{
	return _SYDNEY_DYNAMIC_CAST(ContainsOperandList*, getElement(_WordList::_Member::OperandList, ObjectType::ContainsOperandList));
}

// FUNCTION public
//	Statement::Contains::WordList::setOperandList -- オペランドを設定する
//
// NOTES
//
// ARGUMENTS
//	ContainsOperandList* list
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Contains::WordList::
setOperandList(ContainsOperandList* list)
{
	setElement(_WordList::_Member::OperandList, list);
}

// FUNCTION public
//	Statement::Contains::WordList::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::WordList::
copy() const
{
	return new WordList(*this);
}

namespace
{
	const char* const _name_wordList = "wordList";
#ifdef USE_OLDER_VERSION
	Analysis::ContainsOperand_WordList _analyzer_wordList;
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION public
//	Statement::Contains::WordList::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
Contains::WordList::
getAnalyzer() const
{
	return &_analyzer_wordList;
}
#endif

// FUNCTION public
//	Statement::Contains::WordList::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::WordList::
getName() const
{
	return _name_wordList;
}

//////////////////
// String
//////////////////

// FUNCTION public
//	Statement::Contains::String::String -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::String::
String(ContainsOperand* operand)
	: Super(Type::String, _String::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::String::String -- 
//
// NOTES
//
// ARGUMENTS
//	const String& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::String::
String(const String& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::String::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::String::
copy() const
{
	return new String(*this);
}

namespace
{
	const char* const _name_string = "string";
}

// FUNCTION public
//	Statement::Contains::String::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::String::
getName() const
{
	return _name_string;
}

//////////////////
// WordHead
//////////////////

// FUNCTION public
//	Statement::Contains::WordHead::WordHead -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::WordHead::
WordHead(ContainsOperand* operand)
	: Super(Type::WordHead, _WordHead::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::WordHead::WordHead -- 
//
// NOTES
//
// ARGUMENTS
//	const WordHead& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::WordHead::
WordHead(const WordHead& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::WordHead::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::WordHead::
copy() const
{
	return new WordHead(*this);
}

namespace
{
	const char* const _name_wordHead = "wordHead";
}

// FUNCTION public
//	Statement::Contains::WordHead::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::WordHead::
getName() const
{
	return _name_wordHead;
}

//////////////////
// WordTail
//////////////////

// FUNCTION public
//	Statement::Contains::WordTail::WordTail -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::WordTail::
WordTail(ContainsOperand* operand)
	: Super(Type::WordTail, _WordTail::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::WordTail::WordTail -- 
//
// NOTES
//
// ARGUMENTS
//	const WordTail& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::WordTail::
WordTail(const WordTail& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::WordTail::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::WordTail::
copy() const
{
	return new WordTail(*this);
}

namespace
{
	const char* const _name_wordTail = "wordTail";
}

// FUNCTION public
//	Statement::Contains::WordTail::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::WordTail::
getName() const
{
	return _name_wordTail;
}

//////////////////
// ExpandSynonym
//////////////////

// FUNCTION public
//	Statement::Contains::ExpandSynonym::ExpandSynonym -- 
//
// NOTES
//
// ARGUMENTS
//	ContainsOperand* operand
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::ExpandSynonym::
ExpandSynonym(ContainsOperand* operand)
	: Super(Type::ExpandSynonym, _ExpandSynonym::_Member::ValueNum, operand)
{}

// FUNCTION public
//	Statement::Contains::ExpandSynonym::ExpandSynonym -- 
//
// NOTES
//
// ARGUMENTS
//	const ExpandSynonym& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Contains::ExpandSynonym::
ExpandSynonym(const ExpandSynonym& cOther_)
	: Super(cOther_)
{}

// FUNCTION public
//	Statement::Contains::ExpandSynonym::copy -- コピーする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object*
//
// EXCEPTIONS

Object*
Contains::ExpandSynonym::
copy() const
{
	return new ExpandSynonym(*this);
}

namespace
{
	const char* const _name_expandSynonym = "expandSynonym";
}

// FUNCTION public
//	Statement::Contains::ExpandSynonym::getName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

const char*
Contains::ExpandSynonym::
getName() const
{
	return _name_expandSynonym;
}

//
//	Copyright (c) 2004, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
