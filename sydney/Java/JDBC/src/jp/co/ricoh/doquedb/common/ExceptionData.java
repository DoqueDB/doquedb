// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ExceptionData.java -- 例外データをあらわすクラス
//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.exception.ErrorMessage;

/**
 * 例外データをあらわすクラス
 *
 */
public final class ExceptionData
	implements Serializable
{
	/** エラー番号 */
	private int _errno;
	/** 引数 */
	private java.util.Vector _arguments;
	/** モジュール名 */
	private String _moduleName;
	/** ファイル名 */
	private String _fileName;
	/** 行番号 */
	private int _lineNumber;

	/**
	 * 新たに例外データを作成する。
	 */
	public ExceptionData()
	{
		_arguments = new java.util.Vector();
		_errno = 0;
	}

	/**
	 * 新たに例外データを作成する。
	 *
	 * @param errno_	エラー番号
	 */
	public ExceptionData(int errno_)
	{
		_arguments = new java.util.Vector();
		_errno = errno_;
	}

	/**
	 * エラー番号を得る
	 *
	 * @return	格納されているエラー番号
	 */
	public int getErrorNumber()
	{
		return _errno;
	}

	/**
	 * エラーメッセージを得る
	 *
	 * @return	格納されているエラーメッセージ
	 */
	public String getErrorMessage()
	{
		return ErrorMessage.makeErrorMessage(_errno, _arguments);
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
		_arguments.clear();
		//エラー番号
		_errno = input_.readInt();
		//引数の数
		int len = input_.readInt();
		//引数
		for (int i = 0; i < len; ++i)
		{
			int size = input_.readInt();
			char[] buff = new char[size];
			for (int j = 0; j < size; ++j)
			{
				buff[j] = input_.readChar();
			}
			_arguments.add(new String(buff));
		}
		//モジュール名
		int size = input_.readInt();
		char[] buff = new char[size];
		for (int j = 0; j < size; ++j)
		{
			buff[j] = input_.readChar();
		}
		_moduleName = new String(buff);
		//ファイル名
		size = input_.readInt();
		buff = new char[size];
		for (int j = 0; j < size; ++j)
		{
			buff[j] = input_.readChar();
		}
		_fileName = new String(buff);
		//行番号
		_lineNumber = input_.readInt();
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
		throw new java.io.IOException();
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.EXCEPTION_DATA;
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
