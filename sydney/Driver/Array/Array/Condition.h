// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Condition.h --
// 
// Copyright (c) 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_CONDITION_H
#define __SYDNEY_ARRAY_CONDITION_H

#include "Array/Module.h"
#include "Array/AutoPointer.h"
#include "Array/Data.h"						//Data::Type::Value
#include "Array/Tree.h"						//Tree::Type::Value

#include "Common/Assert.h"
#include "Common/Object.h"
#include "LogicalFile/OpenOption.h"			//OpenOption::DriverNumber::...
#include "LogicalFile/TreeNodeInterface.h"	//TreeNodeInterface::Type

_SYDNEY_BEGIN

namespace Common
{
	class StringData;
}

_SYDNEY_ARRAY_BEGIN

class FileID;

//
//	CLASS
//	Array::Condition -- 条件作成クラス
//
//	NOTES
//
class Condition : public Common::Object
{
public:
	//
	//	STRUCT
	//	Array::Condition::Cond -- 条件をあらわすデータ
	//
	struct Cond
	{
		LogicalFile::TreeNodeInterface::Type m_eMatch;
		// '0' means the value is empty string in the string key type.
		AutoPointer<ModUInt32>	m_pBuffer;
		ModUnicodeChar			m_OptionalChar;

		void clear()
		{
			m_eMatch = LogicalFile::TreeNodeInterface::Undefined;
			m_pBuffer = 0;
			m_OptionalChar = 0;
		}

		const Compare& getKeyCompare(const Tree* pTree_) const
		{
			// NOT support unique search in current implementation.
			return pTree_->getKeyCompare();
		}
	};

	//
	//	STRUCT
	//	Array::Condition::Key -- オープンオプションのキー
	//
	struct Key
	{
		enum Value
		{
			ConditionCount = LogicalFile::OpenOption::DriverNumber::Array,
			Condition,
			Verify,
			RowID,
			FetchFieldNumber
		};
	};
	
	//
	//	CLASS
	//	Array::Condition::ParseValue
	//
	class ParseValue
	{
	public:
		ParseValue() {}
		~ParseValue() {}

		static LogicalFile::TreeNodeInterface::Type
			getStream(const ModUnicodeChar*& p_,
					  ModUnicodeString& cstrValue_,
					  ModUnicodeChar& OptionalChar_)
		{
			using namespace LogicalFile;

			TreeNodeInterface::Type eType;

			//; _SYDNEY_ASSERT(*p_ == '#');
			p_++;
			if (*p_ == 'e')
			{
				// *(p_+1) == 'q'
				eType = TreeNodeInterface::Equals;
			}
			else if (*p_ == 'g')
			{
				if (*(p_+1) == 't')
					eType = TreeNodeInterface::GreaterThan;
				else // *(p_+1) == 'e'
					eType = TreeNodeInterface::GreaterThanEquals;
			}
			else if (*p_ == 'l')
			{
				if (*(p_+1) == 't')
					eType = TreeNodeInterface::LessThan;
				else if (*(p_+1) == 'e')
					eType = TreeNodeInterface::LessThanEquals;
				else // *(p_+1) == 'k'
					eType = TreeNodeInterface::Like;
			}
			else if (*p_ == 'u')
			{
				if (*(p_+1) == 'k')
					eType = TreeNodeInterface::Unknown;
				else // *(p_+1) == 'd'
					eType = TreeNodeInterface::Undefined;
			}
			else
			{
				//; _SYDNEY_ASSERT(false);
			}
			p_ += 2;
			
			if (eType != TreeNodeInterface::Unknown &&
				eType != TreeNodeInterface::Undefined)
			{
				//; _SYDNEY_ASSERT(*p_ == '(');
				getStreamValue(p_, cstrValue_, OptionalChar_);
			}
			else
			{
				//; _SYDNEY_ASSERT(*p_ == '#' || *p_ == ')');
			}

			return eType;
		}

		static void putStream(ModUnicodeOstrStream& cStream_,
							  LogicalFile::TreeNodeInterface::Type eType_,
							  const ModUnicodeString& cstrValue_,
							  ModUnicodeChar OptionalChar_)
		{
			using namespace LogicalFile;

			switch (eType_)
			{
			case TreeNodeInterface::Equals:
				cStream_ << "#eq";
				putStreamValue(cStream_, cstrValue_, OptionalChar_);
				break;
			case TreeNodeInterface::GreaterThan:
				cStream_ << "#gt";
				putStreamValue(cStream_, cstrValue_, OptionalChar_);
				break;
			case TreeNodeInterface::GreaterThanEquals:
				cStream_ << "#ge";
				putStreamValue(cStream_, cstrValue_, OptionalChar_);
				break;
			case TreeNodeInterface::LessThan:
				cStream_ << "#lt";
				putStreamValue(cStream_, cstrValue_, OptionalChar_);
				break;
			case TreeNodeInterface::LessThanEquals:
				cStream_ << "#le";
				putStreamValue(cStream_, cstrValue_, OptionalChar_);
				break;
			case TreeNodeInterface::Like:
				cStream_ << "#lk";
				putStreamLikeValue(cStream_, cstrValue_, OptionalChar_);
				break;
			case TreeNodeInterface::Unknown:
				cStream_ << "#uk";
				break;
			case TreeNodeInterface::Undefined:
				cStream_ << "#ud";
				break;
			}
		}
		
	private:
		static void putStreamValue(ModUnicodeOstrStream& cStream_,
								   const ModUnicodeString& cstrValue_,
								   ModUnicodeChar OptionalChar_)
		{
			cStream_ << '(';
			const ModUnicodeChar* p = cstrValue_;
			while (*p)
			{
				if (*p == ')' || *p == '\\' || *p == ',')
				{
					cStream_ << '\\';
				}
				cStream_ << *p++;
			}
			if (OptionalChar_ != 0)
			{
				cStream_ << ',' << OptionalChar_;
			}
			cStream_ << ')';
		}
		//
		//	NOTES
		//
		//	When the type of the field is char or varchar,
		//	an escape character of SQL-sentence is casted
		//	to the type of unsigned char.
		//	If the escape character is a wide character,
		//	the cast will be failed.
		//	So, when parsing a like condition,
		//	the usOptionalChar_ in the cstrValue_ have to be replaced
		//	to an ascii character.
		//
		//	And it's preferrable that the ascii character is NOT '\'
		//	for avoiding a confusion.
		//	'\' has been used as an escape character for this module.
		//	So, '*' is used as an escape character for SQL-sentence.
		//
		//	This function can NOT be unified with putStreamValue(),
		//	because of indistinguishable from
		//	a white space for a padding character and for a escape character.
		//	(After v16.2, a padding character has been used.)
		//
		static void putStreamLikeValue(ModUnicodeOstrStream& cStream_,
									   const ModUnicodeString& cstrValue_,
									   ModUnicodeChar usOptionalChar_)
		{
			cStream_ << '(';
			const ModUnicodeChar* p = cstrValue_;
			while (*p)
			{
				if (usOptionalChar_ != 0)
				{
					if (*p == usOptionalChar_)
					{
						// Replace an escape character for SQL-sentence.
						// When usOptionalChar_ is '*',
						// NOT need to escape '*' with '*'.
						cStream_ << '*';
						if (*++p == 0)
							break;
					}
					else if (*p == '*')
					{
						// Escape '*' with '*'.
						// Because '*' is NOT an original escape character
						// and it is treated as an escape character after here.
						cStream_ << '*';
					}
				}
				if (*p == ')' || *p == '\\' || *p == ',')
				{
					// The avobe characters are used
					// when parsing a string which is created here.
					// So escape them with '\'.
					cStream_ << '\\';
				}
				cStream_ << *p++;
			}
			if (usOptionalChar_ != 0)
			{
				// Set an new escape character for SQL-sentence.
				cStream_ << ',' << '*';
			}
			cStream_ << ')';
		}
		//
		//	NOTES
		//
		//	usOptionalChar_ has TWO applications.
		//	One is an escape character for LIKE.
		//	In such case, the character is '*'. 
		//	See ParseValue::putStreamLikeValue().
		//	The other is an padding character for other than LIKE.
		//	In such case, the character is ' '.
		//	See Condition::getPaddingChar().
		//	And these characters are NOT used at the same time.
		//
		static void getStreamValue(const ModUnicodeChar*& p_,
								   ModUnicodeString& cstrValue_,
								   ModUnicodeChar& OptionalChar_)
		{
			cstrValue_ = "";
			OptionalChar_ = 0;

			//; _SYDNEY_ASSERT(*p_ == '(');
			p_++;
			while (*p_ != ')' && *p_ != ',')
			{
				if (*p_ == '\\')
				{
					p_++;
				}
				cstrValue_ += *p_;
				p_++;
			}
			if (*p_ == ',')
			{
				p_++;
				OptionalChar_ = *p_;
				p_++;
			}
			//; _SYDNEY_ASSERT(*p_ == ')');
			p_++;
		}
	};

	// コンストラクタ
	Condition(const FileID& cFileID_);
	// デストラクタ
	virtual ~Condition();

	// TreeNodeから検索構文を作成する
	bool getSearchParameter(const LogicalFile::TreeNodeInterface* pCondition_,
							LogicalFile::OpenOption& cOpenOption_);

	// オープンオプションを設定する
	void setOpenOption(const LogicalFile::OpenOption& cOpenOption_,
					   int iNumber_ = 0);
	// fetch用のキーを設定する
	void setFetchKey(const Common::DataArrayData& cKey_);

	// Check whether the key satisfies the upper condition.
	bool isUpperConditionSatisfied(const ModUInt32* pBuffer_);
	
	// Check whether the key satisfies the other conditions.
	bool isOtherConditionMatch(const ModUInt32* pBuffer_);

	// 下限条件を得る
	const Cond& getLowerCondition() { return m_cLowerData; }
	// 上限条件を得る
	const Cond& getUpperCondition() { return m_cUpperData; }

	// 有効な検索条件か
	bool isValid() const { return m_bValid; }

	// Fetchかどうかを設定する
	void setFetchField(int n);
	// Fetchかどうか
	bool isFetch() const { return m_bFetch; }

	Tree::Type::Value getTreeType() const { return m_eTreeType; }

	// Is the condition about NullArray?
	bool isNullArray() const { return m_eTreeType == Tree::Type::NullArray; }

	// Get Compare class.
	static const Compare& getCompare(const Tree* pTree_)
		{ return pTree_->getCompare(); }
	static const Compare& getKeyCompare(const Tree* pTree_)
		{ return pTree_->getKeyCompare(); }
	
private:
	// Set Fetch Condition
	bool setFetchCondition(const LogicalFile::TreeNodeInterface* pCondition_,
						   LogicalFile::OpenOption& cOpenOption_);
	
	// Set the condition of Tree to the OpenOption.
	bool setTreeCondition(const LogicalFile::TreeNodeInterface* pCondition_,
						  LogicalFile::OpenOption& cOpenOption_);
	// Set the Verify condition of Tree to the OpenOption.
	bool setVerifyTreeCondition(
		const LogicalFile::TreeNodeInterface* pCondition_,
		LogicalFile::OpenOption& cOpenOption_);
	
	// Set the condition of Node to the OpenOption.
	bool setNodeCondition(const LogicalFile::TreeNodeInterface* pCondition_,
						  LogicalFile::OpenOption& cOpenOption_,
						  int& iNum_);
	
	// Set the condition of Equal node to the OpenOption.
	bool setEqualsNodeCondition(
		LogicalFile::TreeNodeInterface::Type eMatch_,
		const LogicalFile::TreeNodeInterface* pField_,
		const LogicalFile::TreeNodeInterface* pValue_,
		LogicalFile::OpenOption& cOpenOption_,
		int& iNum_);
	
	// Set the condition of Less Than node to the OpenOption.
	bool setLessThanNodeCondition(
		LogicalFile::TreeNodeInterface::Type eMatch_,
		const LogicalFile::TreeNodeInterface* pField_,
		const LogicalFile::TreeNodeInterface* pValue_,
		LogicalFile::OpenOption& cOpenOption_,
		int& iNum_);
	
	// Set the condition of Greater Than node to the OpenOption.
	bool setGreaterThanNodeCondition(
		LogicalFile::TreeNodeInterface::Type eMatch_,
		const LogicalFile::TreeNodeInterface* pField_,
		const LogicalFile::TreeNodeInterface* pValue_,
		LogicalFile::OpenOption& cOpenOption_,
		int& iNum_);
	
	// Set the condition of Equals To Null node to the OpenOption.
	bool setEqualsToNullNodeCondition(
		LogicalFile::TreeNodeInterface::Type eMatch_,
		const LogicalFile::TreeNodeInterface* pField_,
		const LogicalFile::TreeNodeInterface* pValue_,
		LogicalFile::OpenOption& cOpenOption_,
		int& iNum_);
	
	// Set the condition of Not Null node to the OpenOption.
	bool setNotNullNodeCondition(
		LogicalFile::TreeNodeInterface::Type eMatch_,
		const LogicalFile::TreeNodeInterface* pField_,
		const LogicalFile::TreeNodeInterface* pValue_,
		LogicalFile::OpenOption& cOpenOption_,
		int& iNum_);
	
	// Set the condition of Between node to the OpenOption.
	bool setBetweenNodeCondition(
		LogicalFile::TreeNodeInterface::Type eMatch_,
		const LogicalFile::TreeNodeInterface* pField_,
		const LogicalFile::TreeNodeInterface* pValue_,
		const LogicalFile::TreeNodeInterface* pValue2_,
		LogicalFile::OpenOption& cOpenOption_,
		int& iNum_);
	
	// Set the condition of Like node to the OpenOption.
	bool setLikeNodeCondition(
		LogicalFile::TreeNodeInterface::Type eMatch_,
		const LogicalFile::TreeNodeInterface* pField_,
		const LogicalFile::TreeNodeInterface* pValue_,
		ModUnicodeChar escape_,
		LogicalFile::OpenOption& cOpenOption_,
		int& iNum_);
	
	// Set the condition of Unknown to the OpenOption.
	void setUnknownNodeCondition(LogicalFile::OpenOption& cOpenOption_);
	
	// 単項演算をチェックする
	bool checkOneTerm(const LogicalFile::TreeNodeInterface* pCondition_,
					  const LogicalFile::TreeNodeInterface*& pField_);
	// 2項演算をチェックする
	bool checkTwoTerm(const LogicalFile::TreeNodeInterface* pCondition_,
					  LogicalFile::TreeNodeInterface::Type& eMatch_,
					  const LogicalFile::TreeNodeInterface*& pField_,
					  const LogicalFile::TreeNodeInterface*& pValue_);
	// 3項演算をチェックする
	bool checkThreeTerm(const LogicalFile::TreeNodeInterface* pCondition_,
						const LogicalFile::TreeNodeInterface*& pField_,
						const LogicalFile::TreeNodeInterface*& pValue_,
						const LogicalFile::TreeNodeInterface*& pValue2_);

	// Get the String value.
	bool getValue(LogicalFile::TreeNodeInterface::Type eMatch_,
				  const LogicalFile::TreeNodeInterface* pValue_,
				  ModUnicodeString& cstrValue_,
				  bool& isNoPad_);

	// Expand the pattern to the lower and the upper.
	bool expandPattern(const ModUnicodeString& cstrValue_,
					   ModUnicodeChar escape_,
					   ModUnicodeString& cstrLower_,
					   ModUnicodeString& cstrUpper_,
					   bool& isPrefixMatch_);

	// Get the escape character.
	ModUnicodeChar getEscapeChar(
		const LogicalFile::TreeNodeInterface*& pCondition_) const;

	// Common::Dataを作成する
	Common::Data::Pointer createCommonData(Data::Type::Value eType_,
										   ModUnicodeString& cstrValue_);
	// Dump the data.
	void dumpCommonData(AutoPointer<ModUInt32>& pBuffer_,
						Data::Type::Value eKeyType_,
						const Common::Data& cData_);

	// Alternate two terms.
	void alternateTerm(const LogicalFile::TreeNodeInterface*& pFirst_,
					   const LogicalFile::TreeNodeInterface*& pSecond_,
					   LogicalFile::TreeNodeInterface::Type& eMatch_);

	// Set equal condition to the stream.
	void setEqualsStream(ModUnicodeOstrStream& cStream_,
						 LogicalFile::TreeNodeInterface::Type eMatch_,
						 ModUnicodeString& cstrValue_,
						 bool bNoPadField_,
						 bool bNoPadKey_);
	
	// Set Inequal condition to the stream.
	void setInequalityStream(ModUnicodeOstrStream& cStream_,
							 LogicalFile::TreeNodeInterface::Type eLower_,
							 ModUnicodeString& cstrLower_,
							 bool bLowerNoPadField_,
							 bool bLowerNoPadKey_,
							 LogicalFile::TreeNodeInterface::Type eUpper_,
							 ModUnicodeString& cstrUpper_,
							 bool bUpperNoPadField_,
							 bool bUpperNoPadKey_);

	// Set prefix match condition to the stream.
	void setPrefixMatchStream(ModUnicodeOstrStream& cStream_,
							  const ModUnicodeString& cstrLower_,
							  const ModUnicodeString& cstrUpper_,
							  bool bNoPadField_,
							  bool bNoPadKey_);

	// Set like condition to the stream.
	void setLikeStream(ModUnicodeOstrStream& cStream_,
					   const ModUnicodeString& cstrLower_,
					   const ModUnicodeString& cstrUpper_,
					   const ModUnicodeString& cstrLike_,
					   ModUnicodeChar escape_,
					   bool bNoPadField_,
					   bool bNoPadKey_);
	
	// Set Scan condition to the stream.
	void setScanStream(ModUnicodeOstrStream& cStream_,
					   Tree::Type::Value eTreeType_);

	// Set Unknown condition to the stream.
	void setUnknownStream(ModUnicodeOstrStream& cStream_);

#ifdef BETWEEN_SYMMETRIC
	// Sort the values.
	void sortValueOrder(const LogicalFile::TreeNodeInterface*& pValue_,
						const LogicalFile::TreeNodeInterface*& pValue2_);
#endif
	
	// Check whether the sort order of the field is NO PAD.
	bool checkNoPadSortOrder(Data::Type::Value eType_,
							 bool bFixedField_, bool bNoPadKey_) const;

	// Set the condition.
	void setCondition(LogicalFile::OpenOption& cOpenOption_,
					  int iNum_,
					  ModUnicodeOstrStream& cStream_);

	// Set the cond.
	bool setCond(Cond& cCond_,
				 const ModUnicodeChar*& p);

	// Get the padding char.
	ModUnicodeChar getPaddingChar(bool bNoPad_) const;
	
	// Get the key type.
	Data::Type::Value getKeyType(Data::Type::Value eType_,
								 ModUnicodeChar optionalChar_) const;

	// Get the position of the trailling SOH. [0-base]
	int getPositionOfTraillingSOH(
		const ModUnicodeString& cstrString_) const;
	// Get the position of the first smaller character than PaddingChar.[0-base]
	int getPositionOfFirstSmallCharacter(
		const ModUnicodeString& cstrString_) const;
	
	// ファイルID
	const FileID& m_cFileID;
	// キータイプ
	Data::Type::Value	m_eKeyType;
	// 異表記正規化するかどうか
	bool m_isNormalized;

	// 下限条件
	Cond m_cLowerData;
	// 上限条件
	Cond m_cUpperData;
	// Other condition
	ModVector<Cond> m_vecOtherData;

	// 有効な検索条件かどうか
	bool m_bValid;

	// Whether the condition is fetch
	bool m_bFetch;
	// This fetch condition is used at first time
	bool m_bFirstFetch;
	// The field is NO PAD for fetch condition.
	bool m_bNoPadFieldForFetch;
	// The key is NO PAD for fetch condition.
	bool m_bNoPadKeyForFetch;

	// The type of searched tree
	Tree::Type::Value m_eTreeType;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_CONDITION_H

//
//	Copyright (c) 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
