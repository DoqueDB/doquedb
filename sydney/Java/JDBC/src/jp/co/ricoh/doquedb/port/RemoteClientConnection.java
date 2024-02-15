// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RemoteClientConnection.java -- DoqueDBとのリモートクライアントコネクション
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2011, 2016, 2023 Ricoh Company, Ltd.
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
 * DoqueDBとのリモートクライアントコネクション。
 * ソケットでDoqueDBとの通信を行う
 *
 */
public class RemoteClientConnection extends ClientConnection
{
	/** ホスト名 */
	private String _hostName;
	/** ポート番号 */
	private int _portNumber;

	/** ソケット */
	private java.net.Socket _socket;

	/** 接続に成功したIPアドレスのマップ */
	private static java.util.HashMap<Entry, java.net.InetAddress> _ipMap = null;

	static {
		_ipMap = new java.util.HashMap<Entry, java.net.InetAddress>();
	}

	/**
	 * DoqueDBとのコネクションクラスを新しく作成する。
	 *
	 * @param hostName_		ホスト名
	 * @param portNumber_	ポート番号
	 * @param masterID_		マスターID
	 * @param slaveID_		スレーブID
	 */
	public RemoteClientConnection(String hostName_, int portNumber_,
								  int masterID_, int slaveID_)
	{
		super(ConnectionType.REMOTE, masterID_, slaveID_);
		_hostName = hostName_;
		_portNumber = portNumber_;
	}

	/**
	 * コネクションをオープンする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public void open()
		throws java.io.IOException
	{
		//ソケットを得る
		_socket = getSocket(_hostName, _portNumber);

		int masterID;
		int slaveID;

		try
		{
			//ストリームを得る
			java.io.BufferedInputStream istream
				= new java.io.BufferedInputStream(_socket.getInputStream(),
												  4096);
			java.io.BufferedOutputStream ostream
				= new java.io.BufferedOutputStream(_socket.getOutputStream(),
												   4096);
			_inputStream = new InputStream(istream);
			_outputStream = new OutputStream(ostream);

			//マスターIDとスレーブIDを送る
			_outputStream.writeInt(getFullMasterID());// 暗号化対応
			_outputStream.writeInt(getSlaveID());
			_outputStream.flush();

			//マスターIDを得る
			masterID = _inputStream.readInt();
			//スレーブIDを得る
			slaveID = _inputStream.readInt();
			//チェックする
			if (ConnectionSlaveID.isNormal(slaveID) == false)
			{
				throw new java.io.IOException("connect trmeister failed.");
			}
		}
		catch (java.io.IOException e)
		{
			close();
			throw e;
		}

		//マスターIDを設定する
		setFullMasterID(masterID);// 暗号化対応
		//スレーブIDを設定する
		setSlaveID(slaveID);
	}

	/**
	 * コネクションをクローズする。
	 *
	 * @throws	java.io.IOException
	 *			通信関係のエラー
	 */
	public void close()
		throws java.io.IOException
	{
		//ストリームを閉じる
		if (_inputStream != null) _inputStream.close();
		if (_outputStream != null) _outputStream.close();

		//ソケットを閉じる
		if (_socket != null) _socket.close();
	}

	/** ホスト名とポート番号のペア */
	public static class Entry {

		public String hostName;
		public int portNumber;

		public Entry() {}
		public Entry(String hostName, int portNumber) {
			this.hostName = hostName;
			this.portNumber = portNumber;
		}

		public int hashCode()
			{ return (hostName.hashCode() << 4 | portNumber); }
	};

	/**
	 * ホスト名とポート番号からソケットを得る
	 */
	private synchronized
	static java.net.Socket getSocket(String hostName, int portNumber)
		throws java.io.IOException
	{
		Entry key = new Entry(hostName, portNumber);

		java.net.InetAddress ip = _ipMap.get(key);
		java.net.Socket socket = null;

		if (ip != null) {
			// すでに成功しているので、同じアドレスで接続してみる
			try {
				socket = new java.net.Socket(ip, portNumber);
				// 接続できたのでそれを返す
				return socket;
			} catch (java.io.IOException e) {
				// 接続できなかったので、初めから
				_ipMap.remove(key);
				ip = null;
			}
		}

		// マップにエントリがないので、接続できるIPアドレスを確認する
		java.io.IOException tmp = null;
		java.net.InetAddress[] ips
			= java.net.InetAddress.getAllByName(hostName);
		for (int i = 0; i < ips.length; ++i) {
			ip = ips[i];
			try {
				socket = new java.net.Socket(ip, portNumber);

				// 接続できたので、マップに格納して終了
				_ipMap.put(new Entry(hostName, portNumber), ip);
				return socket;

			} catch (java.io.IOException e) {
				// すべてダメだった場合のために例外を保存
				tmp = e;
			}
		}

		throw tmp;
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2011, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
