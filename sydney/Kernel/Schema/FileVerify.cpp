// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileVerify.cpp -- 整合性検査に関連するクラスの定義
// 
// Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/FileVerify.h"
#include "Schema/AccessFile.h"
#include "Schema/Column.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Sequence.h"
#include "Schema/Table.h"
#include "Schema/TreeNode.h"
#include "Schema/Message_FileNotFound.h"
#include "Schema/Message_IndexTupleNotFound.h"
#include "Schema/Message_TupleCountNotMatch.h"
#include "Schema/Message_TupleValueNotMatch.h"
#include "Schema/Message_NotNullIntegrityViolation.h"
#include "Schema/Message_VerifyStarted.h"
#include "Schema/Message_VerifyFinished.h"
#include "Schema/Message_VerifyTupleStarted.h"
#include "Schema/Message_VerifyTupleOnTheWay.h"
#include "Schema/Message_VerifyTupleFinished.h"

#include "Common/Assert.h"
#include "Common/Configuration.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/UnicodeString.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/BadArgument.h"
#include "Exception/MetaDatabaseCorrupted.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileDriverManager.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModHashMap.h"
#include "ModTime.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	// 整合性検査でタプルの比較をするときの
	// ログに出力する件数
	Common::Configuration::ParameterInteger _cVerifyLogStep("Schema_VerifyLogStep", 10000);

	const ModUnicodeString _cstrPath;	// スキーマでProgressに入れるパスは空文字列
	const ModSize _printStringLength = 80; // キー値をメッセージに出すときの最大長
	const ModUnicodeString _printStringTrailer = _TRMEISTER_U_STRING("...");
}

//	FUNCTION public
//	Schema::FileVerify::FileVerify --
//		コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Table& vecTable_
//			整合性検査の対象となっている表
//		const ModVector<Schema::File*>& vecFile_
//			整合性検査の対象となっているファイル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

FileVerify::
FileVerify(Table& cTable_, const ModVector<File*>& vecFile_)
	: m_cTable(cTable_), m_vecFile(vecFile_),
	  m_vecAccess(vecFile_.getSize(), 0),
	  m_mapFieldData()
{
}

//	FUNCTION public
//	Schema::FileVerify::~FileVerify --
//		デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

FileVerify::
~FileVerify()
{
	ModSize n = m_vecAccess.getSize();
	for (ModSize i = 0; i < n; ++i) {
		delete m_vecAccess[i], m_vecAccess[i] = 0;
	}
}

//	FUNCTION public
//	Schema::FileVerify::verify --
//		コンストラクターで与えられたファイルを整合性検査する
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cProgress_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Admin::Verification::Treatment::Value eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		Schema::TupleID::Value* pMaxRowID_
//			RowIDの最大値を設定するための変数
//		int* pMaxIdentity_
//			Identity Columnの値の最大値を設定するための変数
//		int* pMinIdentity_
//			Identity Columnの値の最小値を設定するための変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
verify(Admin::Verification::Progress& cProgress_,
	   Trans::Transaction& cTrans_,
	   Admin::Verification::Treatment::Value eTreatment_,
	   TupleID::Value* pMaxRowID_,
	   int* pMaxIdentity_,
	   int* pMinIdentity_)
{
	bool bCascade = (eTreatment_ & Admin::Verification::Treatment::Cascade);
	bool bCorrect = (eTreatment_ & Admin::Verification::Treatment::Correct);
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);
	bool bValue = (eTreatment_ & Admin::Verification::Treatment::Data);

	// 呼び出し側で検査の経過が良好であることを保証する必要がある
	; _SYDNEY_ASSERT(cProgress_.isGood());

	// RowIDが属するファイルを得る
	; _SYDNEY_ASSERT(m_cTable.getTupleID(cTrans_));

	Field* pTupleIDField = m_cTable.getTupleID(cTrans_)->getField(cTrans_);
	; _SYDNEY_ASSERT(pTupleIDField);

	File* pStartFile = pTupleIDField->getFile(cTrans_);
	; _SYDNEY_ASSERT(pStartFile);

	// Identity Columnのフィールドを得ておく
	Field* pIdentityField = 0;
	if ((pMaxIdentity_ || pMinIdentity_) && m_cTable.getIdentity(cTrans_)) {
		pIdentityField = m_cTable.getIdentity(cTrans_)->getField(cTrans_);
	}

	// RowIDが属するファイルが
	// コンストラクターで与えられたファイルにあるか調べる
	// ついでにAccessFileも用意し、
	// さらにCascadeなら論理ファイルの整合性検査もやる
	ModSize iStartFilePosition = -1;

	verifyLogicalFile(cProgress_, cTrans_, eTreatment_, pStartFile, iStartFilePosition);
	if (!cProgress_.isGood()) {
		return;
	}

	if (bValue == false) {
		// タプルのチェックをスキップする
		return;
	}

	if (iStartFilePosition < 0) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// Scanアクセスのopenをする
	AccessFile* pStartAccess = m_vecAccess[iStartFilePosition];
	pStartAccess->openSearchFile(0);

	// データが尽きるまで繰り返す

	ModSize iLogStep = _cVerifyLogStep.get();
	ModSize iCount = iLogStep;
	ModSize iGetCount = 0;

	const Common::DataArrayData* pArray = pStartAccess->getData();

	while (pArray) {

		if (iLogStep && !iGetCount) {
			// 途中経過を出す
			SydSchemaVerifyMessage
				<< "Verify tuple of " << m_cTable.getName() << " started: "
				<< ModTime::getCurrentTime().getString()
				<< ModEndl;
			_SYDNEY_VERIFY_INFO(cProgress_, "", Message::VerifyTupleStarted(m_cTable.getName()), eTreatment_);
		}

		// 取得したデータを登録する
		Admin::Verification::Progress cTmp(cProgress_.getConnection());
		registerData(cTmp, cTrans_, eTreatment_, *pStartAccess, *pStartFile, *pArray);
		cProgress_ += cTmp;
		if (!cProgress_.isGood() && !bContinue) {
			return;
		}

		if (pMaxRowID_) {
			// タプルIDの値が登録されているはずである
			const Common::Data::Pointer* pRowIDPointer = getFieldData(pTupleIDField);
			; _SYDNEY_ASSERT(pRowIDPointer);
			; _SYDNEY_ASSERT(pRowIDPointer->get()->getType() == Common::DataType::UnsignedInteger);
			Common::UnsignedIntegerData* pTupleID =
				_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, pRowIDPointer->get());
			; _SYDNEY_ASSERT(pTupleID);

			// RowIDの最大値を更新する
			if (*pMaxRowID_ == TupleID::Invalid
				|| *pMaxRowID_ < pTupleID->getValue()) {
				*pMaxRowID_ = pTupleID->getValue();
			}
		}
		if (pIdentityField) {
			// Identity Columnの値が登録されているはずである
			const Common::Data::Pointer* pIdentityPointer = getFieldData(pIdentityField);
			; _SYDNEY_ASSERT(pIdentityPointer);
			; _SYDNEY_ASSERT((*pIdentityPointer).get()->getType() == Common::DataType::Integer);
			Common::IntegerData* pIdentityData =
				_SYDNEY_DYNAMIC_CAST(Common::IntegerData*, pIdentityPointer->get());
			; _SYDNEY_ASSERT(pIdentityData);

			if (pMaxIdentity_) {
				// 最大値を更新する
				if ((*pMaxIdentity_ == Sequence::Signed::Invalid)
					|| (*pMaxIdentity_ < pIdentityData->getValue())) {
					*pMaxIdentity_ = pIdentityData->getValue();
				}
			}
			if (pMinIdentity_) {
				// 最小値を更新する
				if ((*pMinIdentity_ == Sequence::Signed::Invalid)
					|| (*pMinIdentity_ > pIdentityData->getValue())) {
					*pMinIdentity_ = pIdentityData->getValue();
				}
			}
		}

		// タプル単位の整合性検査をする
		verifyTuple(cProgress_, cTrans_, eTreatment_, pArray, iStartFilePosition);
		if (!cProgress_.isGood() && !bContinue) {
			return;
		}

		++iGetCount;
		if (iLogStep && !--iCount) {
			// 途中経過を出す
			SydSchemaVerifyMessage
				<< "Verify tuple of " << m_cTable.getName()
				<< " #" << iGetCount << ": "
				<< ModTime::getCurrentTime().getString() << ModEndl;
			_SYDNEY_VERIFY_INFO(cProgress_, "",
								Message::VerifyTupleOnTheWay(m_cTable.getName(), iGetCount),
								eTreatment_);
			iCount = iLogStep;
		}

		pArray = pStartAccess->getData();
	}

	if (iLogStep && iGetCount) {
		SydSchemaVerifyMessage
			<< "Verify tuple of " << m_cTable.getName() << " finished: "
			<< ModTime::getCurrentTime().getString() << ModEndl;
		_SYDNEY_VERIFY_INFO(cProgress_, "", Message::VerifyTupleFinished(m_cTable.getName()), eTreatment_);
	}

	// すべて終わった時点でタプル数はすべて同じ数でなければならない
	verifyCount(cProgress_, eTreatment_, m_vecTupleCount[iStartFilePosition]);
}

//	FUNCTION private
//	Schema::FileVerify::verifyLogicalFile --
//		論理ファイルごとの整合性検査をする
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cProgress_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Admin::Verification::Treatment::Value eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		const File* pStartFile_
//			RowIDを格納するファイル
//		ModSize& iStartFilePosition_
//			RowIDを格納するファイルの配列上の位置を返すための変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
verifyLogicalFile(Admin::Verification::Progress& cProgress_,
				  Trans::Transaction& cTrans_,
				  Admin::Verification::Treatment::Value eTreatment_,
				  const File* pStartFile_,
				  ModSize& iStartFilePosition_)
{
	bool bCascade = (eTreatment_ & Admin::Verification::Treatment::Cascade);
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);

	ModSize n = m_vecFile.getSize();
	; _SYDNEY_ASSERT(n);

	m_vecTupleCount.reserve(n);
	for (ModSize i = 0; i < n; ++i) {

		// 中断のポーリング
		Manager::checkCanceled(cTrans_);

		if (m_vecFile[i] == pStartFile_) {
			iStartFilePosition_ = i;
		}
		m_vecAccess[i] = m_vecFile[i]->getAccessFile(cTrans_);

		// ファイル作成遅延のためファイルの存在を調べるチェックは無意味となった
//		// ファイルの存在を調べる
//		if ( !m_vecAccess[i]->getSearchFile().isAccessible(true)) {
//			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, _cstrPath, 
//										Message::FileNotFound());
//
//			// これ以上は続けられない
//			return;
//		}

		if (bCascade) {

			// 途中経過を出す
			SydSchemaVerifyMessage
				<< "Verify cascade " << m_vecFile[i]->getName() << "started." << ModEndl;
			_SYDNEY_VERIFY_INFO(cProgress_, "", Message::VerifyStarted(m_vecFile[i]->getName()), eTreatment_);

			Admin::Verification::Progress cTmp(cProgress_.getConnection());
			cTmp.setSchemaObjectName(cProgress_.getSchemaObjectName());
			m_vecAccess[i]->verify(eTreatment_, cTmp);
			cProgress_ += cTmp;
			if (!bContinue && !cProgress_.isGood()) {
				return;
			}

			// 途中経過を出す
			SydSchemaVerifyMessage
				<< "Verify cascade " << m_vecFile[i]->getName() << "finished." << ModEndl;
			_SYDNEY_VERIFY_INFO(cProgress_, "", Message::VerifyFinished(m_vecFile[i]->getName()), eTreatment_);
		}
		if (cProgress_.isGood()) {
			// タプル数を調べる
			m_vecTupleCount.pushBack(m_vecAccess[i]->getCount());
#ifdef DEBUG
			SydDebugMessage << "getCount[" << m_vecFile[i]->getName() << "]:" << m_vecTupleCount.getBack() << ModEndl;
#endif
		}
	}
}

//	FUNCTION private
//	Schema::FileVerify::verifyTuple --
//		タプルごとの整合性検査をする
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cProgress_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Admin::Verification::Treatment::Value eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		const Common::DataArrayData* pTuple_
//			レコードファイルから読んだ1タプル
//		ModSize iStartFilePosition_
//			RowIDを格納するファイルの配列上の位置
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
verifyTuple(Admin::Verification::Progress& cProgress_,
			Trans::Transaction& cTrans_,
			Admin::Verification::Treatment::Value eTreatment_,
			const Common::DataArrayData* pTuple_,
			ModSize iStartFilePosition_)
{
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);

	ModSize n = m_vecFile.getSize();

	// 処理したファイルのIDを覚えるIDSet
	IDSet cRemainedFile(m_vecFile);
	// スタートファイルは除く
	cRemainedFile.erase(m_vecFile[iStartFilePosition_]->getID());

	bool bErrorContinued = false; // エラーが起きたがContinueされている状態か

	// 処理していないファイルがある限り繰り返す
	while (cRemainedFile.any()) {
		bool bProcessedAny = false;

		// すべてのファイルについて
		// if (未処理)
		//   if (タプルを特定するデータがある)
		//		fetch
		//		for each (Source-Destinationのあるフィールド)
		//			if (データがある)
		//				比較する
		//			else
		//				自身とSource-Destinationのフィールドについてデータを登録する

		for (ModSize i = 0; i < n; ++i) {
			if (i == iStartFilePosition_) continue;

			// 中断のポーリング
			Manager::checkCanceled(cTrans_);

			File* pFile = m_vecFile[i];
			if (cRemainedFile.contains(pFile->getID())) {
				// 特定するデータがあるか調べる

				AccessFile* pAccess = m_vecAccess[i];
				; _SYDNEY_ASSERT(pAccess);

				Common::DataArrayData cKey;
				const Common::Data::Pointer* pValue = 0;
				bool bMayNotExist = false;
				bool bAllNull = false;
				if (!isKeyAllFound(cTrans_, *pAccess, *pFile, cKey, pValue, bMayNotExist, bAllNull)) {
					// まだ取得できない
					continue;
				}

				if (bMayNotExist && !pFile->hasAllTuples()) {
					// タプルが登録されていないはずである
					// -> 値の登録されていないフィールドはNULLを入れる
					fillNullData(cTrans_, *pAccess, *pFile);

					// タプル数に1カウントする
					++m_vecTupleCount[i];

				} else if (pFile->isAbleToVerifyTuple()) {
					// その他の場合はキーが取得できたのでopenして値を取得する
					//	verify tupleができるファイル(全文以外)にだけ以下を行う
					pAccess->openSearchFile(&cKey, pValue);

					const Common::DataArrayData*
						pGetData = pAccess->getData(pValue ? pValue->get() : 0);

					if (pGetData) {
						// 得られた場合
						// -> 取得したデータを登録する
						verifyData(cProgress_, cTrans_, eTreatment_, *pAccess, *pFile, *pGetData);
						if (!cProgress_.isGood() && !bContinue) {
							return;
						}

					} else {
						// 得られない場合
						// 長いキー値をそのまま出すとメモリーを破壊してしまうので
						// 先頭の80文字だけを切り出す
						const ModVector<Common::DataArrayData::Pointer>& vecValue = cKey.getValue();
						ModUnicodeOstrStream cStream;
						cStream << '{';
						if (ModSize n = vecValue.getSize()) {
							const ModSize limit = _printStringLength * (2*n/(n+1)); // nがどれだけ多くても2倍以上にはならない
							ModVector<Common::DataArrayData::Pointer>::ConstIterator iterator = vecValue.begin();
							const ModVector<Common::DataArrayData::Pointer>::ConstIterator& end = vecValue.end();
							do {
								if (iterator != vecValue.begin()) cStream << ',';
								if ((*iterator).get()) {
									ModUnicodeString key;
									ModUnicodeString keyTemp;
									key = (*iterator)->getString();
									key.copy(keyTemp, 0, limit);
									cStream << keyTemp;
									if (key.getLength() > limit)
										// 文字列にあまりがあったらそのことを示す文字列を後ろにつける
										cStream << _printStringTrailer;

								} else {
									cStream << "(0)";
								}
							} while (++iterator != end);
						}
						cStream << '}';
						if (pValue)
							cStream << '[' << (*pValue)->getString() << ']';

						_SYDNEY_VERIFY_INCONSISTENT(cProgress_, _cstrPath,
													Message::IndexTupleNotFound(pFile->getName(),
																				cStream.getString()));
						if (!bContinue)
							return;

						// このタプルではこれ以降処理できないファイルがあってもよしとする
						bErrorContinued = true;
					}
				}

				// 処理したことを記録する
				cRemainedFile.erase(pFile->getID());
				bProcessedAny = true;
			}
		}
		if (!bProcessedAny) {
			// ひとつも新たに処理しなかった
			if (bErrorContinued) break;
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}
	}
	// データマップをクリアする
	clearMap();
}

//	FUNCTION private
//	Schema::FileVerify::verifyCount --
//		タプル数の整合性検査をする
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cProgress_
//			検査結果を格納する変数
//		Admin::Verification::Treatment::Value eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//		ModInt64 iExpectedCount_
//			この数値でないファイルは不整合
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
verifyCount(Admin::Verification::Progress& cProgress_,
			Admin::Verification::Treatment::Value eTreatment_,
			ModInt64 iExpectedCount_)
{
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);

	ModSize n = m_vecFile.getSize();

	for (ModSize i = 0; i < n; ++i) {
		if (m_vecTupleCount[i] != iExpectedCount_) {
			// タプル数が異なる
			// ★注意★
			// NULLのキーなら格納しないタイプのファイルでも
			// verifyTupleの処理の中でNULLを調べて
			// このタプル数をカウントアップしているので
			// すべてのファイルで一致していなければならない

			_SYDNEY_VERIFY_INCONSISTENT(cProgress_, _cstrPath,
										Message::TupleCountNotMatch(m_vecFile[i]->getName(),
																	m_vecTupleCount[i],
																	iExpectedCount_));
			if (!bContinue)
				break;
		}
	}
}

//	FUNCTION private
//	Schema::FileVerify::verifyData --
//		ファイルから取得したデータの整合性検査をする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::AccessFile& cAccess_
//		Schema::File& cFile_
//		const Common::DataArrayData& cData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
verifyData(Admin::Verification::Progress& cProgress_,
		   Trans::Transaction& cTrans_,
		   Admin::Verification::Treatment::Value eTreatment_,
		   AccessFile& cAccess_,
		   File& cFile_,
		   const Common::DataArrayData& cData_,
		   bool bNoCheck_)
{
	bool bContinue = (eTreatment_ & Admin::Verification::Treatment::Continue);

	// Projectionの設定からフィールドを得て登録する

	const ModVector<Field::Position>& cProjection = cAccess_.getProjection();
	int n = cProjection.getSize();
	for (int i = 0; i < n; ++i) {
		Field::Position iPos = cProjection[i];
		Field* pField = cFile_.getFieldByPosition(iPos, cTrans_);

		if (Column* pColumn = pField->getColumn(cTrans_)) {
			// 列の制約を検査する
			Admin::Verification::Progress cTmp(cProgress_.getConnection());
			cTmp.setSchemaObjectName(cProgress_.getSchemaObjectName());
			verifyIntegrity(cTmp, cTrans_, eTreatment_,
							cFile_, *pField, *pColumn,
							*(cData_.getElement(i)));
			cProgress_ += cTmp;
			if (!cProgress_.isGood() && !bContinue) {
				return;
			}
		}

		if (bNoCheck_) {
			setFieldData(cTrans_, pField, cData_.getElement(i));

		} else {
			const Common::Data::Pointer* pFieldData = getFieldData(pField);
			if (!pFieldData) {
				setFieldData(cTrans_, pField, cData_.getElement(i));

			} else {
				// すでに登録されている場合は値が等しいかを調べる
				if (!pFieldData->get()->equals(cData_.getElement(i).get())) {

					_SYDNEY_VERIFY_INCONSISTENT(cProgress_, _cstrPath,
												Message::TupleValueNotMatch(cFile_.getName(),
																			pField->setName(cTrans_)));
					if (!bContinue) return;
				}
			}
		}
	}
}

//	FUNCTION private
//	Schema::FileVerify::verifyIntegrity --
//		データが列についている制約に合致しているかを検査する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::AccessFile& cAccess_
//		Schema::File& cFile_
//		Schema::Field& cField_
//		Schema::Column& cColumn_
//		Common::Data& cData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
verifyIntegrity(Admin::Verification::Progress& cProgress_,
				Trans::Transaction& cTrans_,
				Admin::Verification::Treatment::Value eTreatment_,
				const File& cFile_,
				Field& cField_,
				const Column& cColumn_,
				const Common::Data& cData_)
{
	// NotNullのときはNullデータならIntegrity違反にする
	if (!cColumn_.isNullable()
		&& cData_.isNull()) {

		_SYDNEY_VERIFY_INCONSISTENT(cProgress_, _cstrPath,
									Message::NotNullIntegrityViolation(cFile_.getName(),
																	   cField_.setName(cTrans_)));
	}
}

//	FUNCTION private
//	Schema::FileVerify::isKeyAllFound --
//		タプルを特定するキーがすべて得られたか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::AccessFile& cAccess_
//		Schema::File& cFile_
//		Common::DataArrayData& cKey_
//		Common::Data::Pointer*& pValue_
//		bool& bMayNotExist_
//		bool& bAllNull_
//
//	RETURN
//		true ... すべて得られた
//		false... 得られなかったフィールドがある
//
//	EXCEPTIONS

bool
FileVerify::
isKeyAllFound(Trans::Transaction& cTrans_,
			  AccessFile& cAccess_, File& cFile_,
			  Common::DataArrayData& cKey_,
			  const Common::Data::Pointer*& pValue_,
			  bool& bMayNotExist_, bool& bAllNull_)
{
	bool bResult = true;				// データが入っていないフィールドがあればfalse
	bool bFirstKeyIsNull = false;		// 第一キーまたはOIDがNULLならばtrue
	bool bAllNull = true;				// 少なくとも1つのキーがNULLでなければfalse

	const ModVector<Field*>& vecKeyField = cAccess_.getKeyFields();
	ModSize n = vecKeyField.getSize();
	cKey_.reserve(n);

	for (ModSize i = 0; i < n; ++i) {
		if (const Common::Data::Pointer*
			pDataPointer = getFieldData(vecKeyField[i])) {

			cKey_.pushBack(*pDataPointer);

			// ひとつでもNullでないものがあれば読み込んだデータを使う
			if (!pDataPointer->get()->isNull())
				bAllNull = false;

			if (i == 0) {
				// 対象のファイルがこのキーを受け付けるか
				if (pDataPointer->get()->isNull()
					|| !cFile_.isKeyImportable(*pDataPointer)) {
					bFirstKeyIsNull = true;
				}
			}

		} else {
			bResult = false;
			break;
		}
	}
	if (cAccess_.isValueNeeded()) {
		// バリューが必要なので調査対象にも追加する
		_SYDNEY_ASSERT(cAccess_.getValueField());
		if (const Common::Data::Pointer*
			pDataPointer = getFieldData(cAccess_.getValueField())) {
			pValue_ = pDataPointer;

		} else {
			bResult = false;
		}
	}
	bMayNotExist_ = bFirstKeyIsNull;
	bAllNull_ = bAllNull;
	return bResult;
}

//	FUNCTION private
//	Schema::FileVerify::registerData --
//		ファイルから取得したデータを登録する
//
//	NOTES
//		登録ずみのデータを調べないことがverifyDataとの唯一の違い
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::AccessFile& cAccess_
//		Schema::File& cFile_
//		const Common::DataArrayData& cData_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
registerData(Admin::Verification::Progress& cProgress_,
			 Trans::Transaction& cTrans_,
			 Admin::Verification::Treatment::Value eTreatment_,
			 AccessFile& cAccess_,
			 File& cFile_,
			 const Common::DataArrayData& cData_)
{
	verifyData(cProgress_, cTrans_, eTreatment_, cAccess_, cFile_, cData_, true /* NoCheck */);
}

//	FUNCTION private
//	Schema::FileVerify::fillNullData --
//		未登録のフィールドにNullDataを登録する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::AccessFile& cAccess_
//		Schema::File& cFile_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
fillNullData(Trans::Transaction& cTrans_,
			 AccessFile& cAccess_, File& cFile_)
{
	// Projectionの設定からフィールドを得て登録する

	const ModVector<Field::Position>& cProjection = cAccess_.getProjection();
	int n = cProjection.getSize();
	for (int i = 0; i < n; ++i) {
		Field::Position iPos = cProjection[i];
		Field* pField = cFile_.getFieldByPosition(iPos, cTrans_);
		if (!getFieldData(pField)) {
			setFieldData(cTrans_, pField,
						 Common::Data::Pointer(Common::NullData::getInstance()));
		}
	}
}

//	FUNCTION private
//	Schema::FileVerify::clearMap --
//		フィールドに対応するデータの登録をクリアする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileVerify::
clearMap()
{
	m_mapFieldData.clear();
}

//	FUNCTION private
//	Schema::FileVerify::getFieldData --
//		フィールドに対応するデータが登録されていれば得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field* pField_
//			データを得るフィールド
//
//	RETURN
//		0以外	登録されているデータ
//		0		データは登録されていない
//
//	EXCEPTIONS

const Common::Data::Pointer*
FileVerify::
getFieldData(Field* pField_) const
{
	FieldDataMap::ConstIterator iterator = m_mapFieldData.find(pField_->getID());
	if (iterator != m_mapFieldData.end()) {
		return &(FieldDataMap::getValue(iterator));
	}
	return 0;
}

//	FUNCTION private
//	Schema::FileVerify::setFieldData --
//		フィールドに対応するデータを登録する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Field* pField_
//			データを登録するフィールド
//		const Common::Data::Pointer& pData_
//			登録するデータ
//		bool bCheckSource_ = true
//			Sourceにさかのぼって調べるかを示すフラグ
//
//	RETURN
//		0以外	登録されているデータ
//		0		データは登録されていない
//
//	EXCEPTIONS

void
FileVerify::
setFieldData(Trans::Transaction& cTrans_, Field* pField_,
			 const Common::Data::Pointer& pData_,
			 bool bCheckSource_)
{
	// 登録されているかどうかのチェックはしない
	// Source-Destination関係のすべてのフィールドについて登録する
	// -> Sourceがあるならさかのぼって呼ぶ

	if (bCheckSource_) {
		if (Field* pSource = pField_->getSource(cTrans_)) {
			setFieldData(cTrans_, pSource, pData_);
			// 自身の登録はSourceから戻って行われたはずなので
			// ここで終了してよい
			return;
		}
	}
	// 自身のデータを登録する
	m_mapFieldData.insert(pField_->getID(), pData_, ModTrue);
	// destinationのデータを登録する
	const ModVector<Field*>& vecDestination = pField_->getDestination(cTrans_);
	ModSize n = vecDestination.getSize();
	for (ModSize i = 0; i < n; ++i) {
		// Destinationに再帰呼び出しするときはもうSourceはチェックしない
		setFieldData(cTrans_, vecDestination[i], pData_, false);
	}
}

//
// Copyright (c) 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
