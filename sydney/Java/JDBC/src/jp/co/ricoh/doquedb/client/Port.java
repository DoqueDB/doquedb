// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Port.java -- DoqueDBとのコネクション
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.client;

import jp.co.ricoh.doquedb.common.ClassID;
import jp.co.ricoh.doquedb.common.ErrorLevel;
import jp.co.ricoh.doquedb.common.ExceptionData;
import jp.co.ricoh.doquedb.common.IntegerData;
import jp.co.ricoh.doquedb.common.Serializable;
import jp.co.ricoh.doquedb.common.Status;
import jp.co.ricoh.doquedb.common.StringData;

/**
 * サーバとのコネクション。
 *
 */
final class Port extends Object
{
	/** コネクション */
	private jp.co.ricoh.doquedb.port.Connection _connection;
	/** ワーカID */
	private int _workerID;
	/** エラー時に再利用可能かどうか */
	private boolean _reuse;

	/**
	 * サーバとのコネクションを新しく作成する。
	 *
	 * @param hostName_		ホスト名
	 * @param portNumber_	ポート番号
	 * @param protocolVersion_	プロトコルバージョン(暗号化対応)
	 * @param slaveID_		スレーブID
	 */
	protected Port(String hostName_, int portNumber_, int protocolVersion_, int slaveID_)
	{
		super(Object.PORT);

		// マスターIDは3
		_connection = new jp.co.ricoh.doquedb.port.RemoteClientConnection(
												hostName_, portNumber_,
												protocolVersion_, slaveID_);
		_reuse = false;
	}

	/**
	 * マスターIDを得る。
	 *
	 * @return	マスターID
	 */
	protected int getMasterID()
	{
		return _connection.getMasterID();
	}

	/**
	 * 認証方式を得る。
	 *
	 * @return	認証方式
	 */
	protected int getAuthorization()
	{
		return _connection.getAuthorization();
	}

	/**
	 * スレーブIDを得る。
	 *
	 * @return	スレーブID
	 */
	protected int getSlaveID()
	{
		return _connection.getSlaveID();
	}

	/**
	 * オープンする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	protected void open()
		throws java.io.IOException
	{
		_connection.open();
	}

	/**
	 * クローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public void close()
		throws java.io.IOException
	{
		_connection.close();
	}

	/**
	 * サーバと同期を取る
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	protected void sync()
		throws java.io.IOException, ClassNotFoundException
	{
		_connection.sync();
	}

	/**
	 * オブジェクトを読み込む。
	 *
	 * @return	読み込んだオブジェクト
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected Serializable readObject()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		Serializable object = _connection.readObject();
		if (object instanceof ErrorLevel)
		{
			ErrorLevel level = (ErrorLevel)object;
			_reuse = level.isUserLevel();

			// ErrorLevelの次は必ず例外
			object = _connection.readObject();
			ExceptionData e = (ExceptionData)object;
			//例外を投げる
			jp.co.ricoh.doquedb.exception.ThrowClassInstance.throwException(e);
		}
		else if (object instanceof ExceptionData)
		{
			ExceptionData e = (ExceptionData)object;
			//例外を投げる
			jp.co.ricoh.doquedb.exception.ThrowClassInstance.throwException(e);
		}
		return object;
	}

	/**
	 * オブジェクトを読み込む。
	 *
	 * @param	data	データを格納するSerializableクラス
	 * @return	読み込んだオブジェクト
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected Serializable readObject(Serializable data)
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		Serializable object = _connection.readObject(data);
		if (object instanceof ErrorLevel)
		{
			ErrorLevel level = (ErrorLevel)object;
			_reuse = level.isUserLevel();

			// ErrorLevelの次は必ず例外
			object = _connection.readObject(data);
			ExceptionData e = (ExceptionData)object;
			//例外を投げる
			jp.co.ricoh.doquedb.exception.ThrowClassInstance.throwException(e);
		}
		else if (object instanceof ExceptionData)
		{
			ExceptionData e = (ExceptionData)object;
			//例外を投げる
			jp.co.ricoh.doquedb.exception.ThrowClassInstance.throwException(e);
		}
		return object;
	}

	/**
	 * IntegerDataを読み込む。
	 *
	 * @return	読み込んだIntegerData
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected IntegerData readIntegerData()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		Serializable object = readObject();
		if ((object instanceof IntegerData) == false)
		{
			//ここにくるのはサーバとの同期がとれていないこと
			throw new jp.co.ricoh.doquedb.exception.Unexpected();
		}
		return (IntegerData)object;
	}

	/**
	 * StringDataを読み込む。
	 *
	 * @return	読み込んだStringData
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected StringData readStringData()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		Serializable object = readObject();
		if ((object instanceof StringData) == false)
		{
			//ここにくるのはサーバとの同期がとれていないこと
			throw new jp.co.ricoh.doquedb.exception.Unexpected();
		}
		return (StringData)object;
	}

	/**
	 * Statusを読み込む。
	 *
	 * @return	読み込んだStatusのタイプ
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @throws	java.sql.SQLException
	 *			データベースエラーが発生した
	 */
	protected int readStatus()
		throws java.io.IOException, ClassNotFoundException,
			   java.sql.SQLException
	{
		Serializable object = readObject();
		if ((object instanceof Status) == false)
		{
			//ここにくるのはサーバとの同期がとれていないこと
			throw new jp.co.ricoh.doquedb.exception.Unexpected();
		}
		return ((Status)object).getStatus();
	}

	/**
	 * オブジェクトを書き出す。
	 *
	 * @param object_	書き出すオブジェクト
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	protected void writeObject(Serializable object_)
		throws java.io.IOException
	{
		_connection.writeObject(object_);
	}

	/**
	 * 出力をフラッシュする。
	 *
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	protected void flush()
		throws java.io.IOException
	{
		_connection.flush();
	}

	/**
	 * ワーカIDを得る
	 *
	 * @return	ワーカID
	 */
	protected int getWorkerID()
	{
		return _workerID;
	}

	/**
	 * ワーカIDを設定する
	 *
	 * @param workerID_	ワーカID
	 */
	protected void setWorkerID(int workerID_)
	{
		_workerID = workerID_;
	}

	/**
	 * 例外が発生した場合に、このポートが再利用可能かどうかを得る
	 *
	 * @return	再利用可能な場合はtrue、それ以外の場合はfalse
	 */
	protected boolean isReuse()
	{
		return _reuse;
	}

	/**
	 * 再利用のための初期化を行う
	 */
	protected void reset()
	{
		_reuse = false;
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
