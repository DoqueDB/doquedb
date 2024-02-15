// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Xid.java -- JDBCX の Xidクラス
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

package jp.co.ricoh.doquedb.jdbcx;

/**
 * Xid インタフェースは、X/Open トランザクション識別子 XID 構造の Java マッピングです。
 * このインタフェースは、3 つのアクセス用メソッドを指定しており、
 * それぞれグローバルトランザクションの形式識別子、グローバルトランザクション識別子、
 * およびブランチの修飾子を取得するために使用します。
 * Xid インタフェースは、トランザクションマネージャとリソースマネージャで使用されます。
 * このインタフェースは、アプリケーションプログラムからは見えません。
 *
 */
public class Xid implements javax.transaction.xa.Xid
{
	private byte[] _globalTransactionID;

	private int _formatID;

	private byte[] _branchQualifier;

	/**
	 * XIDの生成。
	 *
	 * @param	globalTransactionID_
	 * 			グローバルトランザクション識別子
	 * @param	branchQualifier_
	 * 			ランザクションブランチの識別子
	 * @param	formatID_
	 * 			形式識別子
	 */
	public Xid(byte[] globalTransactionID_, byte[] branchQualifier_, int formatID_)
	{
		this._globalTransactionID = globalTransactionID_;
		this._branchQualifier = branchQualifier_;
		this._formatID = formatID_;
	}

	/**
	 * XID のトランザクションブランチの識別子部分をバイトの配列として取得します。
	 *
	 * @return トランザクションブランチの識別子。
	 *
	 */
	public byte[] getBranchQualifier() {
		return this._branchQualifier;
	}

	/**
	 * XID の形式識別子部分を取得します。
	 *
	 * @return 形式識別子。
	 *
	 */
	public int getFormatId() {
		return this._formatID;
	}

	/**
	 * XID のグローバルトランザクション識別子部分をバイトの配列として取得します。
	 *
	 * @return グローバルトランザクション識別子。
	 *
	 */
	public byte[] getGlobalTransactionId() {
		return this._globalTransactionID;
	}

	/**
	 * XIDオブジェクトの比較。
	 *
	 * @param	obj_
	 * 			比較するオブジェクト
	 * @return 等しい場合はtrue、そうでない場合は false
	 *
	 */
	public boolean equals(Object obj_) {
		// オブジェクトがXidの場合
		if (obj_ instanceof Xid) {

			Xid xid = (Xid) obj_;
			// フォーマットIDが違う
			if (this._formatID != xid.getFormatId()) {
				return false;
			}

			// グローバルトランザクション識別子の比較
			byte[] globalTransactionID = xid.getGlobalTransactionId();
			int globalTransactionIDLen = globalTransactionID == null ? 0 : globalTransactionID.length;
			int thisGlobalTransactionIDLen = this._globalTransactionID == null ? 0 : this._globalTransactionID.length;

			// 長さが一緒の場合、内容が一緒か調べる
			if (globalTransactionIDLen == thisGlobalTransactionIDLen) {

				for (int i = 0; i < globalTransactionIDLen; i++) {
					if (globalTransactionID[i] != this._globalTransactionID[i]) {
						return false;
					}
				}
			}
			else {
				return false;
			}

			// トランザクションブランチ識別子の比較
			byte[] branchQualifier = xid.getBranchQualifier();
			int branchQualifierLen = branchQualifier == null ? 0 : branchQualifier.length;
			int thisBranchQualifierLen = this._branchQualifier == null ? 0 : this._branchQualifier.length;

			// 長さが一緒の場合、内容が一緒か調べる
			if (branchQualifierLen == thisBranchQualifierLen) {

					for (int i = 0; i < branchQualifierLen; i++) {
						if (branchQualifier[i] != this._branchQualifier[i]) {
							return false;
						}
					}
			} else {
				return false;
			}

			return true;

		}
		// オブジェクトがXidではない
		else {
			return false;
		}
	}

}

//
//Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//All rights reserved.
//
