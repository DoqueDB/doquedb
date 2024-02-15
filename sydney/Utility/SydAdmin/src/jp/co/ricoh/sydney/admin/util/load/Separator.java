// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Separator.java -- 
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util.load;

import java.io.BufferedReader;
import java.io.IOException;
import java.util.ArrayList;

	class Separator
	{
		protected ArrayList<String> _data = null;
		private String _separator;
		protected static String _lineSeparator;
		protected BufferedReader _reader = null;
		protected static boolean _endFlg = false;
		protected static boolean _continueFlg = false;

		Separator(String lineSeparator_, String separator_, BufferedReader reader_)
		{
			_lineSeparator = lineSeparator_;
			this._separator = separator_;
			this._reader = reader_;
			this._data = new ArrayList<String>();
		}
		public  ArrayList<String> getData() throws IOException
		{
			String c = null;
			try {
				String sRecord = "";
				while( (c = _reader.readLine()) != null ) {
					sRecord += c;
					if ( (sRecord = setData(sRecord)) == null )
						break;
				}
				return this._data;
			} finally {
				if (_reader != null && c == null )
					_reader.close();
			}
		}
		static public String getData(String value_)
		{
			String sValue = null;
			int index = value_.indexOf("'");

			switch(index){
				case -1:
					if ( _continueFlg == false ) {// 文字ではない
						_endFlg = true;
						return value_;
					} else {	// 文字列の途中
						sValue += value_;
						sValue += _lineSeparator;
					}
					break;

				default:
					// エスケープを取る
					sValue = deleteEscape(value_);
					if ( !_endFlg ) { // 文字の途中
						// 改行をつける
						sValue += _lineSeparator;
					}
			} // end switch
			return sValue;
		}

		protected static String deleteEscape(String value_)
		{
			char [] cValue = value_.toCharArray();
			StringBuffer buff = new StringBuffer();
			boolean escapeFlg = false;
			for ( int i = 0 ; i < cValue.length ; i++ )
			{
				// 途中の"''"を"'"にする
				if (String.valueOf(cValue[i]).equals("'") && escapeFlg == true ) {
					escapeFlg = false;
					buff.append(cValue[i]);
					continue;
				}
				// 最後に"'"がある
				if ( i == cValue.length - 1 && String.valueOf(cValue[i]).equals("'"))
					_endFlg = true;

				// 途中に"'"がある
				if ( String.valueOf(cValue[i]).equals("'") ) {
					escapeFlg = true;
					if ( _continueFlg )
						_continueFlg = false;
					else
						_continueFlg = true;
				}else
					escapeFlg = false;

				if ( !escapeFlg && !_endFlg)
					buff.append(cValue[i]);
			}
			if (!_continueFlg )
				_endFlg = true;
			return buff.toString();
		}

		protected String[] dataSplit(String value_)
		{
			char [] cValue = value_.toCharArray();
			ArrayList<String> values = new ArrayList<String>();
			StringBuffer value = new StringBuffer();
			for ( int i = 0 ; i < cValue.length ; )
			{
				if ( value == null)
					value = new StringBuffer();
				String separator = "";
				for ( int j = 0 ; j < this._separator.length() ; j++ ) {
					int index = i + j;
					if ( index >= cValue.length )
						break;
					separator += cValue[index];
				}
				if ( separator.equals(this._separator) ) {
					if ( value != null )
						values.add(value.toString());

					value = null;
					i += this._separator.length();
				} else {
					value.append(cValue[i++]);
				}
			}
			if ( value != null )
				values.add(value.toString());
			else
				values.add("");
			return values.toArray(new String[values.size()]);

		}

		protected String setData(String value_) {
			// elementに区切る
			String[] elements = this.dataSplit(value_);
			// リストに登録する
			String sColumn = null;
			for (int i = 0 ; i < elements.length ; i++ ) {
				if ( elements[i] == null ) {
					this._data.add("");
					continue;
				}
				sColumn = getData(elements[i]);
				if ( _endFlg ) {
					this._data.add(sColumn);
					_endFlg = false;
					sColumn = null;
				}
			} // end for

			return sColumn;
		}
	}

//
//	 Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	 All rights reserved.
//
