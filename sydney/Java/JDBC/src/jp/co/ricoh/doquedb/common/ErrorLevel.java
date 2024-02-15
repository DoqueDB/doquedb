// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorLevel.java -- エラーレベルをあらわすクラス
//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
 * エラーレベルをあらわすクラス
 *
 */
public final class ErrorLevel
	implements Serializable
{
	/** ユーザレベル */
	public final static int USER = 1;
	/** システムレベル */
	public final static int SYSTEM = 2;
	/** 不明なエラーレベル値 */
	public final static int UNDEFINED = -1;

	/** 値 */
	private int _level;

	/**
	 * 新たにエラーレベルを作成する。
	 */
	public ErrorLevel()
	{
		_level = UNDEFINED;
	}

	/**
	 * 新たにエラーレベルを作成する。
	 *
	 * @param level_	格納する値
	 */
	public ErrorLevel(int level_)
	{
		_level = level_;
	}

	/**
	 * エラーレベルを得る
	 *
	 * @return	格納されているエラーレベル
	 */
	public int getLevel()
	{
		return _level;
	}

	/**
	 * エラーレベルを設定する
	 *
	 * @param level_	格納するエラーレベル
	 */
	public void setLevel(int level_)
	{
		_level = level_;
	}

	/**
	 * ユーザレベルかどうか
	 *
	 * @return	ユーザレベルの場合はtrue、それ以外の場合はfalse
	 */
	public boolean isUserLevel()
	{
		return (_level == USER);
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
		_level = input_.readInt();
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
		output_.writeInt(_level);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.ERROR_LEVEL;
	}
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
