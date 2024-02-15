// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.java -- DoqueDBとのコネクションの基底クラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.port;

import jp.co.ricoh.doquedb.common.*;

/**
 * DoqueDBとのコネクションの基底クラス。
 * すべてのコネクションクラスはこのクラスを継承する必要がある。
 *
 */
public abstract class Connection
{
	/** コネクションタイプ */
	private int _connectionType;
	/** マスターID */
	private int _masterID;
	/** スレーブID */
	private int _slaveID;

	/** 入力ストリーム */
	protected InputStream _inputStream;
	/** 出力ストリーム */
	protected OutputStream _outputStream;

	/**
	 * DoqueDBとのコネクションクラスを新しく作成する。
	 * このクラスは抽象クラスなので、直接このインスタンスが
	 * 確保されることはない。
	 *
	 * @param connectionType_	{@link ConnectionType コネクションタイプ}
	 * @param masterID_			マスターID
	 * @param slaveID_			スレーブID
	 */
	public Connection(int connectionType_, int masterID_, int slaveID_)
	{
		_connectionType = connectionType_;
		_masterID = masterID_;
		_slaveID = slaveID_;
		_inputStream = null;
		_outputStream = null;
	}

	/**
	 * コネクションのタイプを得る。
	 *
	 * @return	{@link ConnectionType コネクションタイプ}
	 */
	public int getType()
	{
		return _connectionType;
	}

	/**
	 * マスターIDを得る。
	 *
	 * @return	マスターID
	 */
	public int getMasterID()
	{
		return _masterID;
	}

	/**
	 * プロトコルバージョン以外も含めたマスターIDを得る
	 *
	 * @return	FullマスターID
	 */
	public int getFullMasterID()
	{
		return _masterID;
	}

	/**
	 * 認証方式を得る。
	 *
	 * @return	認証方式
	 */
	public int getAuthorization()
	{
		return _masterID & AuthorizeMode.MaskMode;
	}

	/**
	 * スレーブIDを得る。
	 *
	 * @return	スレーブID
	 */
	public int getSlaveID()
	{
		return _slaveID;
	}

	/**
	 * FullマスターIDを設定する。(暗号化対応：旧setMasterID)
	 *
	 * @param masterID_	マスターID
	 */
	public void setFullMasterID(int masterID_)
	{
		_masterID = masterID_;
	}

	/**
	 * スレーブIDを設定する。
	 *
	 * @param slaveID_	スレーブID
	 */
	public void setSlaveID(int slaveID_)
	{
		_slaveID = slaveID_;
	}

	/**
	 * コネクションをオープンする。
	 * 純粋仮想関数なので、継承クラスで実装する必要がある。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public abstract void open()
		throws java.io.IOException;

	/**
	 * コネクションをクローズする。
	 * 純粋仮想関数なので、継承クラスで実装する必要がある。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public abstract void close()
		throws java.io.IOException;

	/**
	 * 通信相手との同期を取る。
	 * 純粋仮想関数なので、継承クラスで実装する必要がある。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	public abstract void sync()
		throws java.io.IOException, ClassNotFoundException;

	/**
	 * オブジェクトを読み込む。
	 * DoqueDBとの互換性を維持し、オブジェクトを読み込む。
	 *
	 * @return	読み込んだオブジェクト
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	public Serializable readObject()
		throws java.io.IOException, ClassNotFoundException
	{
		return _inputStream.readObject();
	}

	/**
	 * オブジェクトを読み込む。
	 * DoqueDBとの互換性を維持し、オブジェクトを読み込む。
	 *
	 * @param	data	データを格納するSerializableクラス
	 * @return	読み込んだオブジェクト
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 */
	public Serializable readObject(Serializable data)
		throws java.io.IOException, ClassNotFoundException
	{
		return _inputStream.readObject(data);
	}

	/**
	 * オブジェクトを書き出す。
	 * DoqueDBとの互換性を維持し、オブジェクトを書き出す。
	 *
	 * @param object_	書き出すオブジェクト
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	public void writeObject(Serializable object_)
		throws java.io.IOException
	{
		_outputStream.writeObject(object_);
	}

	/**
	 * 出力をフラッシュする。
	 *
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	public void flush()
		throws java.io.IOException
	{
		_outputStream.flush();
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
