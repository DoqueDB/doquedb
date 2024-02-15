// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorMessage.java -- エラーメッセージ
//
// Copyright (c) 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.exception;

/**
 * メッセージフォーマットのエントリ
 */
final class MessageEntry
{
	/** エラー番号 */
	public int _code;
	/** エラーメッセージフォーマット */
	public String _format;

	/**
	 * メッセージフォーマットのエントリを新たに作成する
	 *
	 * @param code_		エラーコード
	 * @param format_	メッセージフォーマット
	 */
	MessageEntry(int code_, String format_)
	{
		_code = code_;
		_format = format_;
	}
}

/**
 * エラーメッセージを作成するクラス
 *
 */
public final class ErrorMessage
{
	/** エラーメッセージフォーマットのマップ */
	private static java.util.HashMap _formatMap = new java.util.HashMap();

	/**
	 * 初期化
	 */
	static
	{
		if (_formatMap.isEmpty() == true)
		{
			//デフォルトは英語
			MessageEntry[] _table = MessageFormatEnglish._table;

			//ロケールを調べる
			if (java.util.Locale.getDefault().getLanguage()
					.equals(java.util.Locale.JAPANESE.getLanguage()))
			{
				//日本語
				_table = MessageFormatJapanese._table;
			}

			//マップに格納する
			for (int i = 0; i < _table.length; ++i)
			{
				_formatMap.put(new Integer(_table[i]._code),
							   _table[i]._format);
			}
		}
	}

	/**
	 * エラーメッセージを作成する
	 *
	 * @return	作成されたエラーメッセージ
	 */
	public static String makeErrorMessage(int errno_,
										  java.util.Vector arguments_)
	{
		String format = (String)_formatMap.get(new Integer(errno_));
		if (format == null) return "";
		StringBuilder buf = new StringBuilder();
		int	formatLen = format.length();
		int i = 0;
		while (i < formatLen)
		{
			if (format.charAt(i) == '%')
			{
				// %なので次は数字
				i++;
				StringBuilder num = new StringBuilder();
				while (i < formatLen &&
					   format.charAt(i) >= '0' &&
					   format.charAt(i) <= '9')
				{
					num.append(format.charAt(i));
					i++;
				}
				Integer element = Integer.valueOf(num.toString());
				buf.append((String)arguments_.get(element.intValue() - 1));
			}
			else
			{
				buf.append(format.charAt(i));
				i++;
			}
		}
		return buf.toString();
	}
}

//
// Copyright (c) 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

