// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// XA_HeurCommit.java -- XA_HeurCommitクラス
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

package jp.co.ricoh.doquedb.jdbcx.exception;

import javax.transaction.xa.XAException;

/**
 * <code>javax.transaction.xa.XAException</code>を継承し、
 * XA_HeurCommitを生成する
 *
 */
public class XA_HeurCommit extends XAException {

	private String message;

	/**
	 * XA_HeurCommitを生成する。
	 *
	 * @param	message_
	 * 			エラーメッセージ
	 */
	public XA_HeurCommit(String message_) {
		super(XA_HEURCOM);
		this.message = message_;
	}

	/**
	 * XA_HeurCommitのエラーメッセージを取得。
	 *
	 * @return	エラーメッセージ
	 *
	 */
	public String getMessage() {
		return this.message;
	}
}

//
//Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//All rights reserved.
//
