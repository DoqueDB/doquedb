// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AdminException.java -- 
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


package jp.co.ricoh.sydney.admin.util.exception;

public class AdminException extends Throwable
{
	/**
	 *
	 */
	private static final long serialVersionUID = 1L;
	private int		_appcode = 0;
	private String	_appmsg = new String();

	private String	_cuaseObjectName = "";

	/**
	 *
	 */
	public AdminException()
	{
		super();
	}

	/**
	 * @param message
	 */
	public AdminException(String	message_)
	{
		super(message_);
		this._appmsg = message_;
	}

	/**
	 * @param cause
	 */
	public AdminException(Throwable	cause_)
	{
		super(cause_);
	}

	/**
	 * @param message
	 * @param cause
	 */
	public AdminException(	String		message_,
							Throwable	cause_)
	{
		super(message_, cause_);
	}

	public AdminException(int	appcode_)
	{
		this._appcode = appcode_;
	}

	/**
	 * Returns a String that represents the value of this object.
	 * @return a string representation of the receiver
	 */
	public String toString()
	{
		return super.toString();
	}

	public int getCode()
	{
		return this._appcode;
	}

	public String getMessage()
	{
		return this._appmsg;
	}

	public void setCauseObject(String	cuaseObjectName_)
	{
		this._cuaseObjectName = cuaseObjectName_;
	}

	public String getCauseObject()
	{
		return this._cuaseObjectName;
	}
}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
