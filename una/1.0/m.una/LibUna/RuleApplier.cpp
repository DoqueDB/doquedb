// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	RuleApplier.cpp -- Implement file of RuleApplier class
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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

#include "LibUna/Bitset.h"
#include "LibUna/DicSet.h"
#include "LibUna/RuleHolder.h"
#include "LibUna/Executer.h"
#include "LibUna/Rule.h"
#include "LibUna/RuleApplier.h"
#include "LibUna/RxTools.h"
#include "LibUna/RuleScanner.h"
#include "ModUnicodeRegularExpression.h"

_UNA_USING

#define	OPTION_POS RuleScanner::Position

namespace ApplierLocal {

	//
	// FUNCTION
	//	Local::reflectDiff -- reflect the difference
	//
	// MEMO
	//	X を変更した結果 A と B があった場合、
	//	A と B の変更点だけを X に反映させる
	//
	// ARGUMENTS
	//	const Common::Bitset org_
	//		変更元のビットセット(X)
	//	const Common::Bitset& a_
	//		変更した結果のビットセット(A)
	//	const Common::Bitset& b_
	//		変更した結果のビットセット(B)
	//	Common::Bitset& dst_
	//		結果を格納するビットセット
	//	ModBoolean targetflag_
	//		変更点に設定するビット
	//
	// RETURN
	//
	// EXCEPTION
	//
	void
	reflectDiff(const Bitset& org_,
		const Bitset& a_, const Bitset& b_,
		Bitset& dst_, ModBoolean targetflag_)
	{
		dst_ = org_;

		//  (X^A) & (X^B)
		Bitset atmp(org_);
		atmp ^= a_;

		Bitset btmp(org_);
		btmp ^= b_;
		atmp &= btmp;

		if ( targetflag_ == ModTrue ) {
			//if true, or
				dst_ |= atmp;

		} else {
			// if false, not and
			dst_ &= ~atmp;
		}
	}

	//
	// FUNCTION
	// 	Local::getRegularExpression
	//		-- Common::Data* 型より 
	//		ModUnicodeRegularExpression* を取得する
	//
	// MEMO
	//
	// ARGUMENTS
	//	Common::Data* pin_
	//		ModUnicodeRegularExpression* が格納された Common::Data*
	//
	// RETURN
	//	ModUnicodeRegularExpression*
	//		無効な場合、0 を返す
	//
	Type::RegularExpression*
	getRegularExpression(Data::Object* pin_)
	{
		Type::RegularExpression* ptypeRx = 0;

#ifdef _DEBUG
		       Data::RegularExpression* ptypeData
			= dynamic_cast<Data::RegularExpression*>(pin_);
#else
		       Data::RegularExpression* ptypeData
			= static_cast<Data::RegularExpression*>(pin_);
#endif

		if ( ptypeData )
			ptypeRx = &(ptypeData->getData());

		return ptypeRx;
	}

	// TYEPDEF
	// 	CalcInfo -- スコアルールの情報とスコアのヒット形態素位置のペア
	//
	// MEMO
	typedef ModPair< Type::CalcMaterial, Bitset> CalcInfo;

	// TYPEDEF
	//	CorrectAnswer -- 正解集合
	//
	// MEMO
	typedef	ModMap< ModUnicodeString, Bitset, ModLess<ModUnicodeString> >
				CorrectAnswer;

	// CLASS
	//	CorrectAnswerSet -- 正解集合と形態素数
	//
	// MEMO
	class CorrectAnswerSet
	{
	public:
		CorrectAnswerSet(ModSize len_)
			 : _length(len_)
		{}

		ModBoolean		isRegist(const ModUnicodeString& name_)
		{
			return (( _ans.find(name_) != _ans.end() ) ? ModTrue : ModFalse);
		}

		Bitset&	getBitset(const ModUnicodeString& name_)
		{
			CorrectAnswer::Iterator it = _ans.find(name_);

			if ( it == _ans.end() ) {
				_ans.insert(name_, Bitset(_length));
				it = _ans.find(name_);
			}
			return (*it).second;
		}

		void			getIterator(CorrectAnswer::Iterator& it_,
							CorrectAnswer::Iterator& fin_)
		{
			it_  = _ans.begin();
			fin_ = _ans.end();
		}

	private:
		ModSize			_length;
		CorrectAnswer	_ans;
	};

}

/////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	RuleApplier::RuleApplier
//		-- RuleApplier class constructor
//
// NOTES
//		not using default constructor
//
// ARGUMENTS
//		ModLanguageSet lang_
//
// RETURN
//
// EXCEPTIONS
//
RuleApplier::RuleApplier(DicSet *dicSet_)
: m_dicSet(dicSet_)
{
}

//
// FUNCTION public
//	RuleApplier::~RuleApplier
//		-- RuleApplier class deconstructor
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
RuleApplier::~RuleApplier()
{
}

//
// FUNCTION pubic
// RuleApplier::applies  -- rule apply
//
// NOTES
//	本関数で実行する処理の進捗率は全体の４分の１とする
//
// ARGUMENTS
//	ModVector<Common::Morph>::ConstIterator srcIt_
//		apply start position 
// 	ModVector<Common::Morph>::ConstIterator srcFin_
//		apply end position
//	ModVector<Common::Keyword>& dst_
//		keyword candidate list
//
// RETURN
//
// EXCEPTIONS
//
void
RuleApplier::applies(ModVector<Morph>::ConstIterator srcIt_,
		ModVector<Morph>::ConstIterator srcFin_,
		ModVector<Keyword>& keyword_)
{
	// ModUnicodeRegularExpressionなどを共有しており、MT-safeじゃない
	// そのため、ここでロックする
	
	ModAutoMutex<ModCriticalSection> cAuto(RuleHolder::getLock());
	cAuto.lock();
	
	// get rule
	const ModVector<Rule*>& rules
		= RuleHolder::getRule(m_dicSet );

	// 文字化品詞 ID を作成しておく
	ModVector<ModUnicodeChar> typeString;
	RxTools::getTypeString(srcIt_, srcFin_, typeString);

	// flag preparation
	const int wordCount = srcFin_ - srcIt_;
	if ( wordCount <= 0 ) return;

	ApplierLocal::CorrectAnswerSet answers(wordCount);

	ModVector<ApplierLocal::CalcInfo> scoreInfo;


	for ( int i = 1; i <= 2; i++ ) {

		RuleType pt;
		if ( i == 1 ) pt = POS;
		if ( i == 2 ) pt = WRITE;

		// apply rule in turn
		ModVector<Rule*>::ConstIterator beg = rules.begin();
		ModVector<Rule*>::ConstIterator it  = beg;
		ModVector<Rule*>::ConstIterator fin = rules.end();
		for ( ; it != fin; ++it ) {

			// get wordFlag for morph array
			Bitset &wordFlag = (Bitset&)answers.getBitset(getValueName((*beg)->getOption()));

			// command
			Rule::Command	cmd;
			switch ( cmd = (*it)->getCommand() ) {
			case Rule::PickupFromWhole:
				pickupFromWhole(srcIt_, srcFin_, &(*typeString.begin()),
								(*it)->getOption(),	wordFlag, pt);
				break;
			case Rule::PickupFromBlock:
				pickupFromBlock(srcIt_, &(*typeString.begin()),
								(*it)->getOption(),	wordFlag, pt);
				break;
			case Rule::DeleteFromWhole:
				pickupFromWhole(srcIt_, srcFin_, &(*typeString.begin()),
								(*it)->getOption(),	wordFlag, pt, ModFalse);
				break;
			case Rule::DeleteFromBlock:
				pickupFromBlock(srcIt_, &(*typeString.begin()),
								(*it)->getOption(),	wordFlag, pt, ModFalse);

				break;
			case Rule::DeleteTail:
				{
					deleteTail(srcIt_, &(*typeString.begin()), (*it)->getOption(),
								wordFlag, pt, ModFalse);
					break;
				}
			default:
				ModErrorMessage << "unknown command(" << (*it)->getCommand()
					<< ")" << ModEndl;
				ModThrow(ModModuleStandard,
						 ModCommonErrorBadArgument,
						 ModErrorLevelError);
			}

		} // for ( ; it != fin; ++it ) {

	} // for ( ProcessingTarget i = POS; i < None; i++ ) {

	// make keyword candidate
	{
		KeywordMap keymap;

		ApplierLocal::CorrectAnswer::Iterator it, fin;
		answers.getIterator(it, fin);

		for ( ; it != fin; ++it )
		{
			makeKeyword(srcIt_, srcFin_, (*it).second, scoreInfo, keymap);
		}

		// change from map to vector
		keyword_.reserve(keymap.getSize());
		KeywordMap::Iterator mapit  = keymap.begin();
		KeywordMap::Iterator mapfin = keymap.end();
		for ( ; mapit != mapfin; ++mapit )
			keyword_.pushBack((*mapit).second);
	}
}

//
// FUNCTION pubic
// RuleApplier::makeKeyword -- make keyword
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	ModVector<Common::Morph>::ConstIterator tgtWord_
//		start position
//	ModVector<Common::Morph>::ConstIterator finWord_
//		end position
//	const Common::Bitset& flag_
//		keyword flag
//	ModVector< Local::CalcInfo >& scoreinfo_,
//	MapOfKeyword& keyword_
//		keyword container
//
// EXCEPTIONS
//
void
RuleApplier::makeKeyword(ModVector<Morph>::ConstIterator tgtWord_,
			 ModVector<Morph>::ConstIterator finWord_,
			 const Bitset& flag_,
			 const ModVector< ApplierLocal::CalcInfo >& scoreinfo_,
			 KeywordMap& keyword_)
{
	// フラグが立っているものだけ返却値に追加する
	int iMax = finWord_ - tgtWord_;
	for( int i = 0; i < iMax; ++i ) {

		if ( flag_[i] == ModTrue ) {

			int iStartPos = i;
			while ( i < iMax && flag_[i] == ModTrue )
			{
				++i;
			}

			// スコアルール追加用キーワードコンテナ
			ModVector<Keyword*> newKeyword;

			Executer::getKeywordCombination(tgtWord_ + iStartPos, tgtWord_ + i,
							keyword_, &newKeyword);

		}
	}
}

//
// FUNCTION pubic
//	RuleApplier::pickupFromWhole -- 形態素の列に対して操作をする
//
// NOTES
//
// ARGUMENTS
//	ModVector<Common::Morph>::ConstIterator srcIt_
//		start position
//	ModVector<Common::Morph>::ConstIterator srcFin_
//		end position
//	const ModUnicodeChar* typeString_
//		文字化した品詞列
//	const Common::Rule::CommandOption& option_
//		option list
//	Common::Bitset& wordFlag_
//		候補を保存するビットセット
//	RuleType processing_
//		処理対象（品詞 or 表記）
//	ModBoolean setFlag_
//		set flag
//
// RETURN
//
// EXCEPTIONS
//
void
RuleApplier::pickupFromWhole(ModVector<Morph>::ConstIterator srcIt_,
			ModVector<Morph>::ConstIterator srcFin_,
			const ModUnicodeChar* typeString_,
			const Rule::CommandOption& option_,
			Bitset& wordFlag_,
			RuleType processing_,
			ModBoolean setFlag_)
{
	const unsigned int wordCount = wordFlag_.size();

	using namespace Data;

	// get operator
#ifdef _DEBUG
	const BitsetOperator* arg
		= dynamic_cast<const BitsetOperator*>(option_[OPTION_POS::Operate]);
#else
	const BitsetOperator* arg
		= static_cast<const BitsetOperator*>(option_[OPTION_POS::Operate]);
#endif

	Algorithm::BitsetOperator op = arg->getData();

	// get regular expression
	Type::RegularExpression* ptypeRx
		= ApplierLocal::getRegularExpression(option_[OPTION_POS::POSRule]);
	Type::RegularExpression* ptextRx
		= ApplierLocal::getRegularExpression(option_[OPTION_POS::MarkRule]);
	{
		// check necessary of processing
		if ( (processing_ == POS && !ptypeRx) ||
			 (processing_ == WRITE && !ptextRx) ||
			 (processing_ == WRITE && ptypeRx) ) return;


	}

	ModAutoPointer<Bitset> typeflag;
	ModAutoPointer<Bitset> textflag;
	if ( setFlag_ == ModTrue ) {
		if ( ptypeRx ) typeflag = new Bitset(wordCount);
		if ( ptextRx ) textflag = new Bitset(wordCount);
	} else {
		// フラグを消す作業は元ビット列に対して行うのでコピーコンストラクトを使用する
		if ( ptypeRx ) typeflag = new Bitset(wordFlag_);
		if ( ptextRx ) textflag = new Bitset(wordFlag_);
	}

	// 品詞列での解析
	if ( ptypeRx && processing_ == POS ) {
		// 正規表現クラスが設定されていればマッチング

		// 品詞IDによる正規表現マッチング
		Executer::getWordTypeRx(*ptypeRx, typeString_, 0, wordCount, *typeflag, setFlag_);

	}

	// 表記での解析
	// 品詞マッチの場合、同時に表記マッチも行う
	if ( (ptextRx && processing_ == POS) ||
		 (ptextRx && processing_ == WRITE) ) {

		// 正規表現クラスが設定されていればマッチング

		// 表記によるマッチング
		Executer::getWordTextRxForAll(*ptextRx,
						srcIt_,
						srcIt_ + wordCount,
						srcIt_->getNormPos(),
						0, wordCount,
						*textflag, setFlag_);
	}

	Bitset result(wordFlag_);
	if ( typeflag.get() != 0 && textflag.get() != 0 ) {
		// 品詞と表記の両方が指定されている場合、
		ApplierLocal::reflectDiff(wordFlag_,
					  *typeflag, *textflag,
					  result, setFlag_);

	} else {
		result = typeflag.get() != 0 ? *typeflag : *textflag;
	}

	if ( wordFlag_.count() > 0 ) {
		op(wordFlag_, result);
	} else {
		// それ以外はコピー
		wordFlag_ = result;
	}

}

//
// FUNCTION pubic
// RuleApplier::pickupFromBlock -- apply pickupFromBlock 
//
// NOTES
//
// ARGUMENTS
// 	ModVector<Morph>::ConstIterator wordIt_
// 		start position of the morph
//	const ModUnicodeChar* typeString_
//		文字化された品詞ID
//	const Rule::CommandOption& option_
//		option
// 	Bitset& wordFlag_
//		keyword候補を表すbitset
//	RuleType processing_,
//		処理対象識別子（品詞 or 表記）
//	ModBoolean setFlag_
//		set flag
//
// RETURN
//
// EXCEPTIONS
//
void
RuleApplier::pickupFromBlock(ModVector<Morph>::ConstIterator wordIt_,
			const ModUnicodeChar* typeString_,
			const Rule::CommandOption& option_,
			Bitset& wordFlag_,
			RuleType processing_,
			ModBoolean setFlag_)
{
	using namespace Data;

	// get operator
#ifdef _DEBUG
	const BitsetOperator* arg
		= dynamic_cast<const BitsetOperator*>(option_[OPTION_POS::Operate]);
#else
	const BitsetOperator* arg
		= static_cast<const BitsetOperator*>(option_[OPTION_POS::Operate]);
#endif

	Algorithm::Operator<Bitset> op = arg->getData();

	// get regular expression
	Type::RegularExpression* ptypeRx
		= ApplierLocal::getRegularExpression(option_[OPTION_POS::POSRule]);
	Type::RegularExpression* ptextRx
		= ApplierLocal::getRegularExpression(option_[OPTION_POS::MarkRule]);

	{
		// check necessary of processing
		if ( (processing_ == POS && !ptypeRx) ||
			 (processing_ == WRITE && !ptextRx) ||
			 (processing_ == WRITE && ptypeRx) ) return;

	}

	// ビットセットの連続した true の並びを対象に処理する	
	Bitset typeflag(wordFlag_);
	Bitset textflag(wordFlag_);
	unsigned int kwTgt = 0;
	unsigned int kwMax = wordFlag_.size();

	for ( ; kwTgt < kwMax; ++kwTgt ) {

		// 最初の連続位置を取得する

		if ( wordFlag_[kwTgt] == ModTrue ) {

			unsigned int kwStart = kwTgt;
			while ( ++kwTgt < kwMax && wordFlag_[kwTgt] == ModTrue );

			if ( ptypeRx && processing_ == POS ) {
				// 正規表現クラスが設定されていればマッチング

				Executer::getWordTypeRx(*ptypeRx, typeString_,
							kwStart, kwTgt - kwStart, typeflag, setFlag_);

			}

			// 表記列に対する操作
			
			// 品詞マッチ指定時、表記マッチも同時に行う
			if ( (ptextRx != 0 && processing_ == WRITE) ||
				 (ptextRx != 0 && processing_ == POS) ) {

				// 正規表現クラスが設定されていればマッチング

				Executer::getWordTextRx(*ptextRx, wordIt_,
							kwStart, kwTgt - kwStart, textflag, setFlag_);
			}
		}
	}

	Bitset result(wordFlag_);
	if ( ptypeRx != 0 && ptextRx != 0 ) {
		// 品詞と表記の両方が指定されている場合

		ApplierLocal::reflectDiff(wordFlag_, typeflag, textflag,
					  result, setFlag_);

	} else {
		result = (ptypeRx != 0) ? typeflag : textflag;
	}

	op(wordFlag_, result);

}

//
// FUNCTION pubic
// RuleApplier::getValueName
// 	-- 対象となるBitset名を取得する
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
const ModUnicodeString&
RuleApplier::getValueName(const Rule::CommandOption& options_)
{
#ifdef _DEBUG
	const Data::UnicodeString* name = dynamic_cast<const Data::UnicodeString*>(options_[OPTION_POS::ValueName]);
#else
	const Data::UnicodeString* name = static_cast<const Data::UnicodeString*>(options_[OPTION_POS::ValueName]);
#endif

	return name->getData();
}

//
// FUNCTION pubic
// RuleApplier::setNPflag
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void
RuleApplier::setNPflag(ModVector<Morph>::ConstIterator srcIt_,
			ModVector<Morph>::ConstIterator srcFin_,
			Bitset& wordFlag_)
{
	const unsigned int wordCount = wordFlag_.size();

	// set the wordFlag for NP
	for (unsigned int i = 0; srcFin_ - srcIt_ > 0; srcIt_++)
	{
		// if the wordFlag is not the NP final flag, it is the NP
		if( srcIt_->getType() != 0)
		{
			wordFlag_[i] = ModTrue;
		}
		i++;
	}
}


// FUNCTION pubic
// RuleApplier::deleteTail
//	delete the word at the end of NP
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
void
RuleApplier::deleteTail(ModVector<Morph>::ConstIterator wordIt_,
			const ModUnicodeChar* typeString_,
			const Rule::CommandOption& option_,
			Bitset& wordFlag_,
			RuleType processing_,
			ModBoolean setFlag_)
{
	using namespace Data;

	// get operator
#ifdef _DEBUG
	const BitsetOperator* arg
		= dynamic_cast<const BitsetOperator*>(option_[OPTION_POS::Operate]);
#else
	const BitsetOperator* arg
		= static_cast<const BitsetOperator*>(option_[OPTION_POS::Operate]);
#endif

	Algorithm::Operator<Bitset> op = arg->getData();

	// get regular expression
	Type::RegularExpression* ptypeRx
		= ApplierLocal::getRegularExpression(option_[OPTION_POS::POSRule]);
	Type::RegularExpression* ptextRx
		= ApplierLocal::getRegularExpression(option_[OPTION_POS::MarkRule]);

	{
		// check necessary of processing
		if ( (processing_ == POS && !ptypeRx) ||
			 (processing_ == WRITE && !ptextRx) ||
			 (processing_ == WRITE && ptypeRx) ) return;
	}

	Bitset typeflag(wordFlag_);
	Bitset textflag(wordFlag_);
	//unsigned int kwTgt = 0;
	unsigned int kwTgt = 1;
	unsigned int kwMax = wordFlag_.size();


	for ( ; kwTgt < kwMax; ++kwTgt )
	{
		// get the final word of NP according to its WordFlag (finalNP flag == 0)
		if ( wordFlag_[kwTgt] == ModFalse && wordFlag_[kwTgt - 1] == ModTrue )
		{
			unsigned int kwStart = kwTgt - 1;

			if ( ptypeRx && processing_ == POS )
			{
				// 正規表現クラスが設定されていればマッチング

				Executer::getWordTypeRx(*ptypeRx, typeString_, kwStart, kwTgt - kwStart, typeflag, setFlag_);

			}

			// 表記列に対する操作
			
			// 品詞マッチ指定時、表記マッチも同時に行う
			if ( (ptextRx != 0 && processing_ == WRITE) ||
				 (ptextRx != 0 && processing_ == POS) )
			{
				// 正規表現クラスが設定されていればマッチング

				Executer::getWordTextRx(*ptextRx, wordIt_, kwStart, kwTgt - kwStart, textflag, setFlag_);
			}
		}
	}


	Bitset result(wordFlag_);
	if ( ptypeRx != 0 && ptextRx != 0 ) {
		// 品詞と表記の両方が指定されている場合

		ApplierLocal::reflectDiff(wordFlag_, typeflag, textflag, result, setFlag_);

	} else {
		result = (ptypeRx != 0) ? typeflag : textflag;
	}

	op(wordFlag_, result);
}

//
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
