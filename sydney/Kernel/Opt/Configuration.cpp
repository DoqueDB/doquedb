// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Configuration.cpp -- パラメータ名
// 
// Copyright (c) 2000, 2003, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Opt";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/Configuration.h"

#include "Common/Message.h"

#include "Os/AutoCriticalSection.h"
#include "Os/Limits.h"

#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

namespace
{
	Common::Configuration::ParameterMessage _cParameterOutput("Opt_ParameterOutput");
	Common::Configuration::ParameterIntegerInRange _cOptimizerVersion(
												  "Opt_OptimizerVersion",
												  Configuration::OptimizerVersion::Current,
												  Configuration::OptimizerVersion::Version1,
												  Configuration::OptimizerVersion::ValueNum - 1);
	Common::Configuration::ParameterBoolean _cUseOlderVersion("Opt_UseOlderVersion", false);
	Common::Configuration::ParameterMessage _cTraceOptimization("Plan_TraceOptimizationOutput");
	Common::Configuration::ParameterMessage _cOldTraceOptimization("Opt_ReportOptimization");
	Common::Configuration::ParameterMessage _cTraceExecution("Plan_TraceExecutionOutput");
	Common::Configuration::ParameterString _cTraceLevel("Plan_TraceLevel", "File");
	Common::Configuration::ParameterBoolean _cOverflowNull("Execution_OverflowNull", false);
	Common::Configuration::ParameterBoolean _cNoUnknown("Plan_NoUnknown", false);
	Common::Configuration::ParameterInteger _cUnionMaxNumber("Plan_UnionMaxNumber", 1000);
	Common::Configuration::ParameterInteger _cLikeNormalizedString("Execution_LikeNormalizedString", 0);
	Common::Configuration::ParameterInteger _cTraceMaxStringSize("Plan_TraceMaxStringSize", 100);
	Common::Configuration::ParameterIntegerInRange _cBulkMaxSize(
											 "Plan_BulkMaxSize",
											 2 << 10 << 10, // default(2MB)
											 2 << 10 << 10, // min(2MB)
											 Os::Limits<int>::getMax()); // max(2GB)
	Common::Configuration::ParameterIntegerInRange _cBulkMaxDataLengthInLog(
														"Plan_BulkMaxDataLengthInLog",
														100,
														10,
														1000);
	Common::Configuration::ParameterIntegerInRange _cBulkExternalThreshold(
													   "Plan_BulkExternalThreshold",
													   4 << 10, // default(4KB)
													   0,		  // min(0B)
													   1 << 10 << 10); // max(1MB)
	// bulk input/output buffer size
	Common::Configuration::ParameterIntegerInRange _cBulkBufferSize(
												"Execution_BulkBufferSize",
												4 << 10,		// default(4k)
#ifdef DEBUG
												100,			// min(100B -- debug version)
#else
												4 << 10,		// min(4k)
#endif
												1 << 10 << 10);// max(1M)
	Common::Configuration::ParameterIntegerInRange _cCollectionThreshold(
												"Plan_CollectionThreshold",
												10 << 10 << 10,	// default(10M)
												0,				// min(0)
												Os::Limits<int>::getMax());// max(2G)
	Common::Configuration::ParameterInteger _cJoinMaxCandidates("Plan_JoinMaxCandidates", 5);
	Common::Configuration::ParameterIntegerInRange _cThreadMaxNumber(
												 "Plan_ThreadMaxNumber",
												 1024,			// default(1024)
												 1,				// min(1)
												 Os::Limits<int>::getMax()); // max(2G)

	namespace _TraceLevel
	{
		// specification value obtained from system parameter
		Configuration::TraceLevel::Value getSpecifiedValue();

		Configuration::TraceLevel::Value m_iSpecifiedValue = Configuration::TraceLevel::File;
		bool m_bRead = false;
		Os::CriticalSection m_cLatch;
	}
}

// FUNCTION local
//	$$::_TraceLevel::getSpecifiedValue -- specification value obtained from system parameter
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Configuration::TraceLevel::Value
//
// EXCEPTIONS

Configuration::TraceLevel::Value
_TraceLevel::getSpecifiedValue()
{
	if (!m_bRead) {
		Os::AutoCriticalSection l(m_cLatch);
		if (!m_bRead) {
			const ModUnicodeString& cstrValue = _cTraceLevel.get();
			if (cstrValue.getLength() > 0) {
				if (ModUnicodeCharTrait::isDigit(cstrValue[0])) {
					// treate the value as integer value
					m_iSpecifiedValue = ModUnicodeCharTrait::toInt(cstrValue);
				} else {
					// parse
					m_iSpecifiedValue = 0;
					const ModUnicodeChar* pTop = cstrValue;
					const ModUnicodeChar* pTail = cstrValue.getTail();
					while (pTop < pTail) {
						const ModUnicodeChar* p = ModUnicodeCharTrait::find(pTop, ModUnicodeChar(':'),
																			static_cast<ModSize>(pTail - pTop));
						if (p == 0) p = pTail;
						switch (ModSize iLength = static_cast<ModSize>(p - pTop)) {
						default:
						case 0:
						case 1:
						case 2:
							{
								break;
							}
						case 3:
							{
								if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Log"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Log;
								}
								break;
							}
						case 4:
							{
								if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("File"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::File;
								} else if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Cost"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Cost;
								} else if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Lock"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Lock;
								} else if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Time"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Time;
								}
								break;
							}
						case 5:
							{
								break;
							}
						case 6:
							{
								if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Normal"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Normal;
								} else if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Thread"),
																		ModFalse /* case insensitive */,
																		iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Thread;
								}
								break;
							}
						case 7:
							{
								if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Process"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Process;
								} else if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("RowInfo"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::RowInfo;
								}
								break;
							}
						case 8:
							{
								if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Contents"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Contents;
								} else if (ModUnicodeCharTrait::compare(pTop, ModUnicodeString("Variable"),
																 ModFalse /* case insensitive */,
																 iLength) == 0) {
									m_iSpecifiedValue |= Configuration::TraceLevel::Variable;
								}
								break;
							}
						}
						// go to next token
						pTop = p + 1;
					}
				}
			}
			m_bRead = true;
		}
	}
	return m_iSpecifiedValue;
}

// FUNCTION public
//	Opt::Configuration::getParameterOutput -- Opt::Optimizer::optimize に渡されたパラメータを出力する先
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterMessage&
//
// EXCEPTIONS

Common::Configuration::ParameterMessage&
Configuration::
getParameterOutput()
{
	return _cParameterOutput;
}

// FUNCTION public
//	Opt::Configuration::getOptimizerVersion -- 使用するオプティマイザーバージョン
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterInteger&
//
// EXCEPTIONS

Common::Configuration::ParameterIntegerInRange&
Configuration::
getOptimizerVersion()
{
	return _cOptimizerVersion;
}

// FUNCTION public
//	Opt::Configuration::getUseOlderVersion --
//			現バージョンでNotSupportedのときに古いオプティマイザーバージョンを使用する
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterBoolean&
//
// EXCEPTIONS

Common::Configuration::ParameterBoolean&
Configuration::
getUseOlderVersion()
{
	return _cUseOlderVersion;
}

// FUNCTION public
//	Opt::Configuration::getTraceOptimization -- Optimizerのトレースメッセージ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterMessage&
//
// EXCEPTIONS

Common::Configuration::ParameterMessage&
Configuration::
getTraceOptimization()
{
	return _cTraceOptimization.isOutput() ? _cTraceOptimization : _cOldTraceOptimization;
}

// FUNCTION public
//	Opt::Configuration::getTraceExecution -- Executorのトレースメッセージ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterMessage&
//
// EXCEPTIONS

Common::Configuration::ParameterMessage&
Configuration::
getTraceExecution()
{
	return _cTraceExecution;
}

// FUNCTION public
//	Opt::Configuration::getOverflowNull -- 数値計算のオーバーフローで例外の代わりにNULLにする
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterBoolean&
//
// EXCEPTIONS

Common::Configuration::ParameterBoolean&
Configuration::
getOverflowNull()
{
	return _cOverflowNull;
}

// FUNCTION public
//	Opt::Configuration::getNoUnknown -- PREDICATEの計算でUNKNOWNをFALSEとして扱う
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterBoolean&
//
// EXCEPTIONS

Common::Configuration::ParameterBoolean&
Configuration::
getNoUnknown()
{
	return _cNoUnknown;
}

// FUNCTION public
//	Opt::Configuration::getUnionMaxNumber -- UNION展開の最大数
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterInteger&
//
// EXCEPTIONS

Common::Configuration::ParameterInteger&
Configuration::
getUnionMaxNumber()
{
	return _cUnionMaxNumber;
}

// FUNCTION public
//	Opt::Configuration::getLikeNormalizedString -- LIKEの計算で正規化パラメーターを指定する
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterInteger&
//
// EXCEPTIONS

Common::Configuration::ParameterInteger&
Configuration::
getLikeNormalizedString()
{
	return _cLikeNormalizedString;
}

// FUNCTION public
//	Opt::Configuration::getTraceMaxStringSize -- トレースで出力する文字列の最大長
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterInteger&
//
// EXCEPTIONS

Common::Configuration::ParameterInteger&
Configuration::
getTraceMaxStringSize()
{
	return _cTraceMaxStringSize;
}

// FUNCTION public
//	Opt::Configuration::getBulkMaxSize -- Bulkで読み込むデータの最大サイズ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterIntegerInRange&
//
// EXCEPTIONS

Common::Configuration::ParameterIntegerInRange&
Configuration::
getBulkMaxSize()
{
	return _cBulkMaxSize;
}

// FUNCTION public
//	Opt::Configuration::getBulkMaxDataLengthInLog -- Bulkでログに書き込むデータの最大サイズ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterIntegerInRange&
//
// EXCEPTIONS

Common::Configuration::ParameterIntegerInRange&
Configuration::
getBulkMaxDataLengthInLog()
{
	return _cBulkMaxDataLengthInLog;
}

// FUNCTION public
//	Opt::Configuration::getBulkExternalThreshold -- Bulkで外部ファイルに出力するデータの閾値
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterIntegerInRange&
//
// EXCEPTIONS

Common::Configuration::ParameterIntegerInRange&
Configuration::
getBulkExternalThreshold()
{
	return _cBulkExternalThreshold;
}

// FUNCTION public
//	Opt::Configuration::getBulkBufferSize -- Bulkで使用するバッファサイズ
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterIntegerInRange&
//
// EXCEPTIONS

Common::Configuration::ParameterIntegerInRange&
Configuration::
getBulkBufferSize()
{
	return _cBulkBufferSize;
}

// FUNCTION public
//	Opt::Configuration::getCollectionThreshold -- Collectionからの取得をファイルコストで計算する閾値
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterIntegerInRange&
//
// EXCEPTIONS

Common::Configuration::ParameterIntegerInRange&
Configuration::
getCollectionThreshold()
{
	return _cCollectionThreshold;
}

// FUNCTION public
//	Opt::Configuration::getJoinMaxCandidates -- Joinの候補最大数
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterInteger&
//
// EXCEPTIONS

Common::Configuration::ParameterInteger&
Configuration::
getJoinMaxCandidates()
{
	return _cJoinMaxCandidates;
}

// FUNCTION public
//	Opt::Configuration::getThreadMaxNumber -- Program中に作成するThread数の上限
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::Configuration::ParameterIntegerInRange&
//
// EXCEPTIONS

Common::Configuration::ParameterIntegerInRange&
Configuration::
getThreadMaxNumber()
{
	return _cThreadMaxNumber;
}

// FUNCTION public
//	Opt::Configuration::TraceLevel::isOn -- TraceLevelのビットが立っているかを検査する関数
//
// NOTES
//
// ARGUMENTS
//	Value uLevel_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Configuration::TraceLevel::
isOn(Value uLevel_)
{
	return (_TraceLevel::getSpecifiedValue() & uLevel_) == uLevel_;
}

//
// Copyright (c) 2000, 2003, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
