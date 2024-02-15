// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DecimalData.java -- decimal型をあらわすクラス
//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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
 * decimal型をあらわすクラス
 *
 */
public final class DecimalData extends ScalarData
	implements Serializable
{
	/** 値 */
	private java.math.BigDecimal _value;

	/** 型の属性として設定されているPrecisionとScale */
	int _precision;
	int _scale;

	/** UnitあたりのDigit数 */
	private final static int _iDigitPerUnit = 9;
	/** Unit分のDigitにするマスク */
	private final static int _iDigitMask = 100000000;

	/**
	 * 新たにdecimal型のデータを作成する。
	 */
	public DecimalData()
	{
		super(DataType.DECIMAL);
		_precision = _iDigitPerUnit;
		_scale = 0;
		_value = new java.math.BigDecimal(0);
	}

	/**
	 * 新たにdecimal型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public DecimalData(DecimalData value_)
	{
		super(DataType.DECIMAL);
		_precision = value_._precision;
		_scale = value_._scale;
		_value = value_.getValue().plus(); // copy
	}

	/**
	 * 新たにdecimal型のデータを作成する。
	 *
	 * @param value_	格納する値
	 */
	public DecimalData(java.math.BigDecimal value_)
	{
		super(DataType.DECIMAL);
		setValue(value_);
	}

	/**
	 * 値を得る
	 *
	 * @return	格納されている値
	 */
	public java.math.BigDecimal getValue()
	{
		return _value;
	}

	/**
	 * 値を設定する
	 *
	 * @param value_	格納する値
	 */
	public void setValue(java.math.BigDecimal value_)
	{
		_precision = value_.precision();
		_scale = value_.scale();

		// precision might be less than scale in Java (ex. 0.00123)
		if (_precision < _scale) {
			_precision = _scale;
		}

		_value = value_;
	}

	/**
	 * ストリームから読み込む
	 *
	 * @param input_	入力用のストリーム
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream input_)
		throws java.io.IOException
	{
		/**
		 * following part is converted from (c++)DecimalData::getString
		 */

		// precision
		_precision = input_.readInt();
		// scale
		_scale = input_.readInt();

		// integer part length
		int iIntegerPartLength = input_.readInt();
		// fraction part length
		int iFractionPartLength = input_.readInt();

		byte bNegative = input_.readByte();

		int iVectorSize = input_.readInt();
		int[] vecDigit = new int[iVectorSize];
		for (int i = 0; i < iVectorSize; ++i)
		{
			vecDigit[i] = input_.readInt();
		}

		String cDecimalString = "";

		if (bNegative != 0)
		{
			cDecimalString += '-';
		}

		if (iIntegerPartLength > 0) // the integer part is not zero
		{
			int i = 0;
			int last = (iIntegerPartLength + _iDigitPerUnit - 1) / _iDigitPerUnit;

			//first part
			int iUnitValue = vecDigit[i];
			int nDigit = 0;
			if (iUnitValue > 0) {
				cDecimalString += String.valueOf(iUnitValue);
			}

			// rest part
			for (++i; i < last; ++i)
			{
				iUnitValue = vecDigit[i];
				cDecimalString += String.format("%09d", iUnitValue);
			}
		}
		else
		{
			cDecimalString += '0';
		}

		if (iFractionPartLength > 0)
		{
			cDecimalString += '.';
			String cFractionString = new String();

			int i = (iIntegerPartLength + _iDigitPerUnit - 1) / _iDigitPerUnit;;
			for(int l = iFractionPartLength; l > 0; l -= _iDigitPerUnit)
			{
				int iUnitValue = vecDigit[i++];
				cFractionString += String.format("%09d", iUnitValue);
			}
			if (cFractionString.length() > iFractionPartLength) {
				cDecimalString += cFractionString.substring(0, iFractionPartLength);
			} else {
				cDecimalString += cFractionString;
			}
		}
		_value = new java.math.BigDecimal(cDecimalString);
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
		/**
		 * following part is converted from (c++)DecimalData::castFromString
		 */

		// negative flag
		byte bIsNegative = (byte)(_value.signum() < 0 ? 1 : 0);
		// Following parts are devired from string representation
		String cDecimalString = _value.abs().toPlainString();

		// integer/fraction part length obtained from data type
		int iIntegerPartLength = _precision - _scale;
		int iFractionPartLength = _scale;

		// integer/fraction part length obtained from value
		int iIntegerLength = 0;
		int iFractionLength = 0;
		int iSize = 0;
		int[] vecDigit;

		int iPeriodPosition = cDecimalString.indexOf('.');
		if (iPeriodPosition < 0)
		{
			iPeriodPosition = cDecimalString.length();
		}
		else
		{
			// if period is there, set fraction length
			iFractionLength = cDecimalString.length() - iPeriodPosition - 1;
		}
		iIntegerLength = iPeriodPosition;
		if (_value.compareTo(java.math.BigDecimal.ZERO) == 0)
		{
			// use part length obtained from data type
			iIntegerLength = iIntegerPartLength;
			iFractionLength = iFractionPartLength;
			iSize = ((iIntegerLength + _iDigitPerUnit - 1) / _iDigitPerUnit)
				+ ((iFractionLength + _iDigitPerUnit - 1) / _iDigitPerUnit);
			vecDigit = new int[iSize];
			for (int i = 0; i < iSize; ++i) {
				vecDigit[i] = 0;
			}
		}
		else
		{
			boolean bIntegerPartIsZero = false;

			String cIntegerString = cDecimalString.substring(0, iIntegerLength);
			String cFractionString = null;
			if (iFractionLength > 0)
			{
				cFractionString = cDecimalString.substring(iIntegerLength + 1, cDecimalString.length());
			}

			StringBuilder cWorkIntegerString = new StringBuilder();
			StringBuilder cWorkFractionString = null;
			// add heading zeros if integer part length has more room.
			for (; iIntegerLength < iIntegerPartLength; ++iIntegerLength)
			{
				cWorkIntegerString.append('0');
			}
			cWorkIntegerString.append(cIntegerString);

			if (iFractionLength > 0)
			{
				cWorkFractionString = new StringBuilder();
				cWorkFractionString.append(cFractionString);
				for (; iFractionLength < iFractionPartLength; ++iFractionLength) {
					cWorkFractionString.append('0');
				}
				// append more zeros for last digits
				for (int i = 0; i < _iDigitPerUnit; ++i) {
					cWorkFractionString.append('0');
				}
			}

			int iIntegerDigitSize = (iIntegerLength + _iDigitPerUnit - 1) / _iDigitPerUnit;
			int iFractionDigitSize = (iFractionLength + _iDigitPerUnit - 1) / _iDigitPerUnit;
			iSize = iIntegerDigitSize + iFractionDigitSize;
			vecDigit = new int[iSize];

			int iTarget = iIntegerDigitSize - 1;
			int iEndPosition = cWorkIntegerString.length();
			for (int i = iIntegerLength; i > 0; i -= _iDigitPerUnit)
			{
				if (i > _iDigitPerUnit)
				{
					vecDigit[iTarget--] =
						Integer.parseInt(
							 cWorkIntegerString.substring(iEndPosition - _iDigitPerUnit,
														  iEndPosition));
					iEndPosition -= _iDigitPerUnit;
				}
				else
				{
					vecDigit[iTarget--] =
						Integer.parseInt(cWorkIntegerString.substring(0, iEndPosition));
				}
			}

			if (iFractionLength > 0)
			{
				iTarget = iIntegerDigitSize;
				int iBeginPosition = 0;
				for (int i = iFractionLength; i > 0; i -= _iDigitPerUnit)
				{
					vecDigit[iTarget++] =
						Integer.parseInt(
							 cWorkFractionString.substring(iBeginPosition,
														   iBeginPosition + _iDigitPerUnit));
					iBeginPosition += _iDigitPerUnit;
				}
			}
		}

		// precision as a type
		output_.writeInt(_precision);
		// scale as a type
		output_.writeInt(_scale);

		// integer part length
		output_.writeInt(iIntegerLength);
		// fraction part length
		output_.writeInt(iFractionLength);

		// negative flag
		output_.writeByte(bIsNegative);

		// size of digit unit
		output_.writeInt(iSize);

		// digits
		for (int i = 0; i < iSize; ++i)
		{
			output_.writeInt(vecDigit[i]);
		}
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.DECIMAL_DATA;
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object other_)
	{
		boolean result = false;
		if ((other_ instanceof DecimalData) == true)
		{
			DecimalData o = (DecimalData)other_;
			result = (getValue().equals(o.getValue()));
		}
		return result;
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new DecimalData(this);
	}

	/**
	 * 文字列に変換する
	 *
	 * @return	文字列
	 */
	public String toString()
	{
		return _value.toString();
	}
}

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
