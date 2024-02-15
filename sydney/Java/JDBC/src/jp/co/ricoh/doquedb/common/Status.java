// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Status.java -- ステータスをあらわすクラス
//
// Copyright (c) 2002, 2003, 2006, 2023 Ricoh Company, Ltd.
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
 * ステータスをあらわすクラス
 *
 */
public final class Status
	implements Serializable
{
	/** 処理が成功した時にステータス値 */
	public final static int SUCCESS = 0;
	/** 処理が失敗したときのステータス値 */
	public final static int ERROR = 1;
	/** 処理がキャンセルされたときのステータス値 */
	public final static int CANCELED = 2;
	/** 複文で続きのデータがあるときのステータス値 */
	public final static int HAS_MORE_DATA = 3;
	/** 不明なステータス値 */
	public final static int UNDEFINED = -1;

	/** 値 */
	private int _status;

	/**
	 * 新たにステータスを作成する。
	 */
	public Status()
	{
		_status = UNDEFINED;
	}

	/**
	 * 新たにステータスを作成する。
	 *
	 * @param status_	格納する値
	 */
	public Status(int status_)
	{
		_status = status_;
	}

	/**
	 * ステータスを得る
	 *
	 * @return	格納されているステータス
	 */
	public int getStatus()
	{
		return _status;
	}

	/**
	 * ステータスを設定する
	 *
	 * @param status_	格納するステータス
	 */
	public void setStatus(int status_)
	{
		_status = status_;
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
		_status = input_.readInt();
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
		output_.writeInt(_status);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.STATUS;
	}
}

//
// Copyright (c) 2002, 2003, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
