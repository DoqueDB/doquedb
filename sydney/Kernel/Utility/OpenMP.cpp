// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenMP.cpp
// 
// Copyright (c) 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Utility";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Utility/OpenMP.h"
#include "Utility/Manager.h"

#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Exception/MemoryExhaust.h"
#include "Exception/ModLibraryError.h"
#include "Exception/Unexpected.h"
#include "Exception/UserLevel.h"

#include "Os/AutoCriticalSection.h"

#ifdef _OPENMP
#include <omp.h>
#endif

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{
#ifdef _OPENMP
	//
	// OpenMPで利用するCPU数を設定する
	//
	// 例)
	//	10			スレッド数を 10 にする
	//	CPU - 1		スレッド数を CPUコア数 -1 にする
	//
	Common::Configuration::ParameterString	_cParallel(
		"Utility_ParallelThreadNumber", "");

	//	トークン
	ModUnicodeString _cCPU("CPU");
	ModUnicodeString _cMinus("-");

	//
	//	トークンを切り出す
	//
	ModUnicodeString _getToken(const ModUnicodeChar*& p_)
	{
		ModUnicodeString cToken;
		while (*p_ == ' ' || *p_ == '\t') ++p_;
		while (*p_ != 0)
		{
			if ((*p_ >= 'A' && *p_ <= 'Z') ||
				(*p_ >= 'a' && *p_ <= 'z') ||
				(*p_ >= '0' && *p_ <= '9'))
			{
				cToken.append(*p_);
			}
			else if (*p_ == ' ' || *p_ == '\t')
			{
				++p_;
				break;
			}
			else
			{
				if (cToken.getLength())
					break;
				cToken.append(*p_);
				++p_;
				break;
			}
			++p_;
		}
		return cToken;
	}
#endif
}

//
//	FUNCTION public
//	Utility::Manager::OpenMP::initialize -- 初期化する
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
//
void
Manager::OpenMP::initialize()
{
#ifdef _OPENMP
	// パラメータから利用するCPU数を設定する
	// パラメータが設定されている場合には、環境変数は無視する

	ModUnicodeString cParallel = _cParallel.get();
	if (cParallel.getLength())
	{
		int number = -1;
		
		const ModUnicodeChar* p = cParallel;
		ModUnicodeString t = _getToken(p);
		if (t.compare(_cCPU, ModFalse) == 0)
		{
			// プロセッサーコア数を得る
			number = omp_get_num_procs();

			t = _getToken(p);
			if (t.compare(_cMinus) != 0)
			{
				number = -1;
			}
			else
			{
				t = _getToken(p);
				number -= ModUnicodeCharTrait::toInt(t);
			}
		}
		else
		{
			number = ModUnicodeCharTrait::toInt(t);
		}

		if (number > 0)
		{
			// 最大スレッド数を指定する
			omp_set_num_threads(number);
		}
	}
#endif
}

//
//	FUNCTION public
//	Utility::Manager::OpenMP::terminate -- 後処理する
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
//
void
Manager::OpenMP::terminate()
{
}

//
//	FUNCTION public
//	Utility::OpenMP::OpenMP -- コンストラクタ
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
//
OpenMP::OpenMP()
	: m_bException(false)
{
}

//
//	FUNCTION public
//	Utility::OpenMP::~OpenMP -- デストラクタ
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
//
OpenMP::~OpenMP()
{
}

//
//	FUNCTION public
//	Utility::OpenMP::run
//		-- マルチスレッドで実行するメソッドを実行する
//
//	NOTES
//
//	ARGUMENTS
// 	bool bParallel_
//		マルチスレッドで実行する場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OpenMP::run(bool bParallel_)
{
	m_bException = false;
	
	if (bParallel_)
	{
		// マルチスレッドで実行する
	
#pragma omp parallel
		{
			// このブロックがマルチスレッドで実行される
			//
			// OpenMPの場合、発生した例外は各ブロック内で catch する必要がある
			// そのため、例外を catch するだけの
			// ラッパー関数 runHogeHoge を実装している
		
#pragma omp single
			{
				// このブロックは１つのスレッドでのみ実行される
				// ただし実行されるスレッドは不定である
				// 他のスレッドは single ブロックの実行を待たされる
				
				runPrepare();
			}
			
			// マルチスレッドで実行されるメインのメソッド
			runParallel();
		
#pragma omp barrier
#pragma omp single
			{
				// single ブロックは1つのスレッドでのみ実行されることは
				// 保障されているが、すべてのスレッドが single ブロックの
				// 直前まで実行が終わっているとは限らないので barrier で同期する
				
				runDispose();
			}
		}
	}
	else
	{
		// シングルスレッドで実行する

		runPrepare();
		runParallel();
		runDispose();
	}

	// 例外が発生していたら再送する
	rethrowException();
}

//
//	FUNCTION public
//	Utility::OpenMP::prepare -- 前処理する
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
//
void
OpenMP::prepare()
{
	// 必要ならサブクラスで上書きすること
}

//
//	FUNCTION public
//	Utility::OpenMP::dispose -- 後処理する
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
//
void
OpenMP::dispose()
{
	// 必要ならサブクラスで上書きすること
}

//
//	FUNCTION public
//	Utility::OpenMP::getThreadSize
//		-- 現在のスレッドチームの全スレッド数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
// 	RETURN
// 	int
//		全スレッド数
//
//	EXCEPTIONS
//
int
OpenMP::getThreadSize()
{
#ifdef _OPENMP
	return omp_get_num_threads();
#else
	return 1;
#endif
}

//
//	FUNCTION public
//	Utility::OpenMP::getThreadNum
//		-- 自スレッドのスレッド番号を得る
//		   スレッド番号は 0 から getThreadSize() - 1 までの整数値である
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		自スレッドのスレッド番号
//
//	EXCEPTIONS
//
int
OpenMP::getThreadNum()
{
#ifdef _OPENMP
	return omp_get_thread_num();
#else
	return 0;
#endif
}

//
//	FUNCTION private
//	Utility::OpenMP::runPrepare
//		-- 準備関数を実行する
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
//
void
OpenMP::runPrepare()
{
	try
	{
		// 実行する
		prepare();
	}
	catch (Exception::UserLevel& e)
	{
		SydInfoMessage << e << ModEndl;
			
		setException(e);
	}
	catch (Exception::Object& e)
	{
		SydErrorMessage << e << ModEndl;
			
		setException(e);
	}
	catch (ModException& e)
	{
		SydErrorMessage
			<< Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)
			<< ModEndl;

		setException(
			Exception::ModLibraryError(moduleName, srcFile, __LINE__, e));
	}
#ifndef NO_CATCH_ALL
	catch (std::bad_alloc& e)
	{
		SydErrorMessage << "std::bad_alloc occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;

		setException(
			Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
	}
	catch (std::exception& e)
	{
		SydErrorMessage << "std::exception occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;

		setException(
			Exception::Unexpected(moduleName, srcFile, __LINE__));
	}
	catch (...)
	{
		SydErrorMessage << "Unexpected Exception" << ModEndl;
			
		setException(Exception::Unexpected(moduleName, srcFile, __LINE__));
	}
#endif
}

//
//	FUNCTION private
//	Utility::OpenMP::runParallel
//		-- 実行関数を実行する
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
//
void
OpenMP::runParallel()
{
	try
	{
		// 実行する
		parallel();
	}
	catch (Exception::UserLevel& e)
	{
		SydInfoMessage << e << ModEndl;
			
		setException(e);
	}
	catch (Exception::Object& e)
	{
		SydErrorMessage << e << ModEndl;
			
		setException(e);
	}
	catch (ModException& e)
	{
		SydErrorMessage
			<< Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)
			<< ModEndl;

		setException(
			Exception::ModLibraryError(moduleName, srcFile, __LINE__, e));
	}
#ifndef NO_CATCH_ALL
	catch (std::bad_alloc& e)
	{
		SydErrorMessage << "std::bad_alloc occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;

		setException(
			Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
	}
	catch (std::exception& e)
	{
		SydErrorMessage << "std::exception occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;

		setException(
			Exception::Unexpected(moduleName, srcFile, __LINE__));
	}
	catch (...)
	{
		SydErrorMessage << "Unexpected Exception" << ModEndl;
			
		setException(Exception::Unexpected(moduleName, srcFile, __LINE__));
	}
#endif
}
		
//
//	FUNCTION private
//	Utility::OpenMP::runDispose
//		-- 後処理関数を実行する
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
//
void
OpenMP::runDispose()
{
	try
	{
		// 実行する
		dispose();
	}
	catch (Exception::UserLevel& e)
	{
		SydInfoMessage << e << ModEndl;
			
		setException(e);
	}
	catch (Exception::Object& e)
	{
		SydErrorMessage << e << ModEndl;
			
		setException(e);
	}
	catch (ModException& e)
	{
		SydErrorMessage
			<< Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)
			<< ModEndl;

		setException(
			Exception::ModLibraryError(moduleName, srcFile, __LINE__, e));
	}
#ifndef NO_CATCH_ALL
	catch (std::bad_alloc& e)
	{
		SydErrorMessage << "std::bad_alloc occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;

		setException(
			Exception::MemoryExhaust(moduleName, srcFile, __LINE__));
	}
	catch (std::exception& e)
	{
		SydErrorMessage << "std::exception occurred. "
						<< (e.what() ? e.what() : "") << ModEndl;

		setException(
			Exception::Unexpected(moduleName, srcFile, __LINE__));
	}
	catch (...)
	{
		SydErrorMessage << "Unexpected Exception" << ModEndl;
			
		setException(Exception::Unexpected(moduleName, srcFile, __LINE__));
	}
#endif
}

//
//	FUNCTION protected
//	Utility::OpenMP::isException -- 例外が発生しているかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		例外が発生している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
OpenMP::isException()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	return m_bException;
}
		
//
//	FUNCTION protected
//	Utility::OpenMP::setException -- 例外を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Exception::Object& e
//		設定する例外クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OpenMP::setException(const Exception::Object& e)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	
	m_cException = e;
	m_bException = true;
	m_uiMessageThreadID = Common::Message::getThreadID();
}

//
//	FUNCTION private
//	Utility::OpenMP::rethrowException -- 例外が発生していたら再送する
//
//	NOTES
//
//	ARGUEMTNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OpenMP::rethrowException()
{
	// ここはシングルスレッドで実行される
	
	if (m_bException)
	{
		SydMessage << "Exception occurred in ThreadID: "
				   << m_uiMessageThreadID << ModEndl;
		Exception::throwClassInstance(m_cException);
	}
}

//
//	Copyright (c) 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
