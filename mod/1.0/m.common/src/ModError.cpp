// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
//	ModError.cpp --- エラー処理関連のメソッドの定義
// 
// Copyright (c) 1997, 1999, 2003, 2023 Ricoh Company, Ltd.
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


#include "ModError.h"
#include "ModThread.h"
#include "ModParameter.h"

//
// VARIABLE
// ModErrorHandle::outputLevel -- 例外送出時、メッセージ出力するエラーレベルの深刻度
//
// NOTES
//	例外送出時のエラーメッセージは、エラーレベルに応じて出力するかどうかが
//	決まる。通常はModErrorLevelFatalの場合にのみ出力される。
//	しかし、障害発生時のデバッグのためにより詳細な
//	バックトレースが必要な場合に備えて、レベルが設定できる。指定レベルより
//	深刻なレベルの例外が送出されればエラーメッセージとして出力される。
//	この値はパラメータ"ExceptionOutputLevel"に以下の値で設定できる。
//
//	-2: ModErrorLevelFatalのみ
//	-1: ModErrorLevelError + 上記
//	 0: ModErrorLevelWarning + 上記
//	 1: ModErrorLevelRetry + 上記
//	 2: ModErrorLevelOk + 上記
//
ModErrorLevel ModErrorHandle::outputLevel = ModErrorLevelFatal;

//
// FUNCTION public
// ModErrorHandle::getException -- セットする例外オブジェクトを得る
//
// NOTES
//	実行スレッドで利用すべき例外オブジェクトへの参照を返す。
//	実際には、スレッドオブジェクトModThreadが持つ例外オブジェクトを得るため、
//	ModThread::getExceptionを呼び出すだけであるが、
//	エラー処理のためだけにModThread.hをインクルードするのは心苦しい。
//	インクルードせずに済ませるため、呼び出すだけのものをModErrorクラスに
//	用意し、関数本体はinlineとせずに実体ファイルに置く。
//
// ARGUMENTS
//	なし
//
// RETURN
//	例外オブジェクトへの参照
//
// EXCEPTIONS
//	なし
//

// static
ModException*
ModErrorHandle::getException()
{
	return ModThisThread::getException();
}

// 
//
// FUNCTION public
// ModErrorHandle::initialize -- 初期化で例外時に出力すべきエラーレベルの設定を行う
//
// NOTES
//	パラメータ"ExceptionOutputLevel"で設定されている値をチェックし、
//	例外時に出力すべきエラーレベルメッセージの深刻度の設定を行う。
//	設定値より深刻なものだけがエラーメッセージとして出力される。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし(ひょっとしたらModParameter::ModParameter関係の例外がありえるかも。)
//

// static
void
ModErrorHandle::initialize()
{
	ModParameter parameter(ModFalse);			// 初期化中なので領域を
												// 確保することはないようにする
	int	level;
	if (parameter.getInteger(level, "ExceptionOutputLevel") == ModTrue)
		ModErrorHandle::outputLevel = (ModErrorLevel) level;
}

#ifdef DEBUG
//
// FUNCTION public
// ModErrorHandle::assertCheck -- アサートが実行された時、止めるための関数
//
// NOTES
// 何もしないが、assertが実行された時にブレークポイントを設定できる。
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//

// static
void
ModErrorHandle::assertCheck()
{ }
#endif

//
// FUNCTION 
// ModErrorHandle::reset -- 実行スレッドのエラー状態を正常に戻す
//
// NOTES
//	実行スレッドのエラー状態をリセットする。
//	エラー状態は、スレッドごとに用意される例外オブジェクトに設定されている。
//	これをエラー状態を示している状態から、正常状態にリセットする。
//	エラー状態では、
//	メモリ獲得に失敗すると非常用メモリを使って動作するモードとなっているので
//	それを元に戻す働きがある。
//	ついでにエラー番号も未定義値にリセットし、メッセージ領域もクリアする。
//
//	例外オブジェクトに対して直接実行すると、それがコピーされたオブジェクト
//	であった場合に、スレッドから参照できない。そこでスレッドごとの
//	例外オブジェクトに確実に設定するために本メソッドを用意した。
//	catch節でModThrowしていない部分では必ず呼び出すこと。
//
//	(注、もし例外のキャッチでコピーコンストラクタが呼び出されなくなったら
//	直接ModExceptionに設定し、チェックできる。)
//
//	************* 問題点(known bug) *************
//	エラー処理中に別のエラーがおき、そのエラーがresetされている場合は
//  その時点でエラー状態が正常状態にリセットされてしまうのが問題。
//	このような状態でかつ、メモリがとれなくなり、ネゴもうまくいかない
//	状態はめったにおきないと思われる。解決方法が思い付かないので保留とする。
//	
//	エラー状態をフラグでなくカウントするようにして、Throwに対して必ず
//	catch節があるとは限らないこと、catch節でのModThrowがエラー発生との見分けが
//	つかないこと、などによりうまくいかない。
//	
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//

// static
void
ModErrorHandle::reset()
{
	// スレッドをエラー状態でなくする

	ModException* e = ModThisThread::getException();
	if (e) e->setErrorStatus(ModFalse);

	// スレッドがエラー状態でなくなったので、
	// 中断要求、終了要求の受付を可能にする

	(void) ModThisThread::setInterruptable(ModTrue);
}

//
// FUNCTION 
// ModErrorHandle::setError -- 実行スレッドのエラー状態を設定する
//
// NOTES
//	実行スレッドのエラー状態を設定する。
//	ModThrowから使われるModException::setErrorから呼び出されるので、普通は
//	明示的に呼び出す必要はない。
//	エラー状態は、スレッドごとに用意される例外オブジェクトに設定されなければ
//	ならない。これをエラー状態に設定する。
//	エラー状態では、
//	メモリ獲得に失敗すると非常用メモリを使って動作するモードとなる。
//
//	例外オブジェクトに対して直接実行すると、それがコピーされたオブジェクト
//	であった場合に、スレッドから参照できない。そこでスレッドごとの
//	例外オブジェクトに確実に設定するために本メソッドを用意した。
//
//	(注、もし例外のキャッチでコピーコンストラクタが呼び出されなくなったら
//	直接ModExceptionに設定し、チェックできる。)
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS
//	なし
//

// static
void
ModErrorHandle::setError()
{
	ModException*	exception = ModThisThread::getException();
	if (exception && exception->isError() == ModFalse) {

		// エラー状態でないスレッドをエラー状態にするので、
		// 中断要求、終了要求の受付を不可にする

		(void) ModThisThread::setInterruptable(ModFalse);

		// スレッドをエラー状態にする

		exception->setErrorStatus(ModTrue);
	}
}

//
// FUNCTION public
// ModErrorHandle::isError -- 実行スレッドがエラー状態かどうかをチェックする
//
// NOTES
//	実行スレッドにエラー状態が設定されているかどうかをチェックする。
//	エラー状態は、スレッドごとに用意される例外オブジェクトに設定されている。
//	
//	ModExceptionとスレッド、ModErrorとの関係についてはModErrorHandle::resetのコメント
//	参照のこと。
//
// ARGUMENTS
//	なし
//
// RETURN
//	エラー状態の場合はModTrue、正常状態の場合はModFalseを返す。
//
// EXCEPTIONS
//	なし
//

// static
ModBoolean
ModErrorHandle::isError()
{
	ModException* e = ModThisThread::getException();
	return (e && e->isError()) ? ModTrue : ModFalse;
}

//
// FUNCTION public
// ModErrorHandle::shouldOutputMessage -- 例外発生のメッセージ出力をすべきかどうかを返す
//
// NOTES
//	例外送出時のエラーメッセージを出力すべきかどうかを、対象例外の
//	エラーレベルに応じて判断し、返す。
//
// ARGUMENTS
//	ModErrorLevel	level
//		対象となる例外のエラーレベル
//
// RETURN
//	対象となる例外のメッセージ出力をすべきかどうかを返す
//
// EXCEPTIONS
//	なし
//

// static
ModBoolean
ModErrorHandle::shouldOutputMessage(ModErrorLevel level)
{
	return ((int) level <= (int) outputLevel) ? ModTrue : ModFalse;
}

//
// Copyright (c) 1997, 1999, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

