// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnicodeString.java -- ModUnicodeStringに対応するクラス
//
// Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
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
 * ModUnicodeStringに対応するクラス
 *
 */
public class UnicodeString
{
	/**
	 * ストリームから読み込む
	 *
	 * @param input		入力用のストリーム
	 * @return 			ストリームから読み込んだStringクラス
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	public static String readObject(InputStream input)
		throws java.io.IOException
	{
		int len = input.readInt();	// DoqueDBはunsigned intで書いているが
									// Javaにはないのでintで読む
		char buff[] = new char[len];
		for (int i = 0; i < len; ++i)
		{
			buff[i] = input.readChar();
		}
		return new String(buff);
	}

	/**
	 * ストリームに書き出す
	 *
	 * @param output	出力用のストリーム
	 * @param data		出力するデータ
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	public static void writeObject(OutputStream output, String data)
		throws java.io.IOException
	{
		int len = data != null ? data.length() : 0;
		output.writeInt(len);	// DoqueDBはunsigned intで読んでいるが
								// Javaにはないのでintで書く
		for (int i = 0; i < len; ++i)
		{
			output.writeChar(data.charAt(i));
		}
	}

}

//
// Copyright (c) 2004, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

