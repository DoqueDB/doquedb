// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Request.java -- リクエストをあらわすクラス
//
// Copyright (c) 2002, 2003, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.common;

/**
 * リクエストをあらわすクラス
 *
 */
public final class Request
	implements Serializable
{
	/** コネクション開始 */
	public final static int BEGIN_CONNECTION = 1;
	/** コネクション終了 */
	public final static int END_CONNECTION = 2;
	/** セッション開始 */
	public final static int BEGIN_SESSION = 3;
	/** セッション終了 */
	public final static int END_SESSION = 4;
	/** ワーカ開始 */
	public final static int BEGIN_WORKER = 5;
	/** ワーカ中断 */
	public final static int CANCEL_WORKER = 6;

	/** サーバ終了 */
	public final static int SHUTDOWN = 7;

	/** SQL文の実行 */
	public final static int EXECUTE_STATEMENT = 8;
	/** SQL文のコンパイル */
	public final static int PREPARE_STATEMENT = 9;
	/** コンパイル結果の実行 */
	public final static int EXECUTE_PREPARE_STATEMENT = 10;
	/** コンパイル結果の削除 */
	public final static int ERASE_PREPARE_STATEMENT = 11;

	/** コネクションを再利用する */
	public final static int REUSE_CONNECTION = 12;
	/** コネクションを再利用しない */
	public final static int NO_REUSE_CONNECTION = 13;

	/** 利用可能性チェック */
	public final static int CHECK_AVAILABILITY = 14;

	/** SQL文のコンパイル */
	public final static int PREPARE_STATEMENT2 = 15;
	/** コンパイル結果の削除 */
	public final static int ERASE_PREPARE_STATEMENT2 = 16;

	/** セッション開始(ユーザーつき) */
	public final static int BEGIN_SESSION2 = 17;
	/** セッション終了(ユーザーつき;未使用) */
	public final static int END_SESSION2 = 18;

	/** ユーザーの追加 */
	public final static int CREATE_USER = 19;
	/** ユーザーの削除 */
	public final static int DROP_USER = 20;
	/** パスワードの変更(自分自身) */
	public final static int CHANGE_OWN_PASSWORD = 21;
	/** パスワードの変更(他人) */
	public final static int CHANGE_PASSWORD = 22;
	/** サーバの終了 */
	public final static int SHUTDOWN2 = 23;

	/** 同期を取る */
	public final static int SYNC = 101;

	/** ProductVersionを問い合わせる */
	public final static int QUERY_PRODUCT_VERSION = 201;

	/** 不明なリクエスト */
	public final static int UNDEFINED = -1;

	/** 利用可能性のチェック対象：サーバー */
	public final static int AVAILABILITY_TARGET_SERVER = 0;

	/** 利用可能性のチェック対象：データベース */
	public final static int AVAILABILITY_TARGET_DATABASE = 1;

	/** 値 */
	private int _request;

	/**
	 * 新たにリクエストを作成する。
	 */
	public Request()
	{
		_request = UNDEFINED;
	}

	/**
	 * 新たにリクエストを作成する。
	 *
	 * @param request_	格納する値
	 */
	public Request(int request_)
	{
		_request = request_;
	}

	/**
	 * リクエストを得る
	 *
	 * @return	格納されているリクエスト
	 */
	public int getRequest()
	{
		return _request;
	}

	/**
	 * リクエストを設定する
	 *
	 * @param request_	格納するリクエスト
	 */
	public void setRequest(int request_)
	{
		_request = request_;
	}

	/**
	 * ストリームから読み込む
	 *
	 * @param input_	入力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream input_)
		throws java.io.IOException
	{
		_request = input_.readInt();
	}

	/**
	 * ストリームに書き出す
	 *
	 * @param output_	出力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see	Serializable#writeObject(OutputStream) writeObject
	 */
	public void writeObject(OutputStream output_)
		throws java.io.IOException
	{
		output_.writeInt(_request);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.REQUEST;
	}
}

//
// Copyright (c) 2002, 2003, 2006, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
