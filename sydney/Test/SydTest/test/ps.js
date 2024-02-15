//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
var fso = new ActiveXObject("Scripting.FileSystemObject");
var shell = new ActiveXObject("WScript.Shell");

// コマンドライン引数の取得
var args = new function() {
	var args = WSH.Arguments, result = [];
	for(var col = new Enumerator( args ); ! col.atEnd(); col.moveNext()) {
		result[ result.length ] = String( col.item() );
	}
	var named = {};
	for(var col = new Enumerator( args.Named ); ! col.atEnd(); col.moveNext()) {
		named[ String( col.item() ) ] = String( args.Named.Item( col.item() ) );
	}
	var unnamed = [];
	for(var col = new Enumerator( args.Unnamed ); ! col.atEnd(); col.moveNext()) {
		unnamed[ unnamed.length ] = String( col.item() );
	}
	result.named = named;
	result.unnamed = unnamed;
	
	result.toArray = function() {
		var result = [];
		for(var i = 0, l = this.length; i < l; i++) {
			result[ i ] = /((^".*"$)|(^'.*'$))/.test( this[i] ) ?
				this[i] : [ '"', this[i], '"' ].join("");
		}
		return result;
	};
	result.toString = function() {
		return this.toArray().join(" ");
	};
	return result;
}();

// cscriptで強制起動
if( /wscript\.exe$/i.test( WSH.FullName ) ) {
	shell.Run( [
		"cscript",
		/((^".*"$)|(^'.*'$))/.test( WSH.ScriptFullName ) ? WSH.ScriptFullName : [ '"', WSH.ScriptFullName, '"' ].join(""),
		args
	].join(" ") );
	WSH.Quit();
}

// ユーティリティ関数定義
var echo = function(s) {
	print( [ s, "\n" ].join("") );
}
var print = function(s) {
	WSH.StdOut.Write( s || "" );
}
var input = function() {
	if( arguments[0] ) print( arguments[0] );
	print( ">" );
	return WSH.StdIn.ReadLine();
}

Error.prototype.toString = function() {
	return this.description || this.message || this.number || this;
};

var $break = {};
var $continue = {};
Enumerator.prototype.each = function(iterator) {
	try {
		var i = 0;
		for(this.moveFirst(); ! this.atEnd(); this.moveNext()) {
			try {
				iterator( this.item(), i++ );
			} catch(e) {
				if( e != $continue ) throw e;
			}
		}
	} catch(e) {
		if( e != $break ) throw e;
	}
};

String.prototype.repeat = function(count) {
	var buf = [];
	for(var i = 0; i < count; i++) buf[buf.length] = this;
	return buf.join("");
};
String.prototype.align = function(align, size) {
	if( ! /^((left)|(right)|(center))$/i.test( align ) ) align = "left";
	if( isNaN(size) || size < 1 ) size = this.length;
	switch( align ) {
	case "left":
		return [ this, " ".repeat(size) ].join("").substr(0, size);
	case "right":
		return [ " ".repeat(size), this ].join("").slice(-1 * size);
	default:
		var pad = " ".repeat( size );
		var s = [ pad, this, pad ].join("");
		return s.substr( parseInt(s.length / 2) - parseInt(size / 2), size );
	}
};
Number.prototype.toTime = function() {
	var sec = this % 60;
	var min = Math.floor( this / 60 );
	var hour = Math.floor( min / 60 );
	min = hour ? min % 60 : min;
	return [
		hour,
		( "00" + min ).slice(-2),
		( "00" + sec ).slice(-2)
	].join(":");
};

var ProcMgr = function() {
	this.wmi = new ActiveXObject("WbemScripting.SWbemLocator").ConnectServer();
	var wshNet = new ActiveXObject("WScript.Network");
	this.currentUser = [ wshNet.UserDomain, wshNet.UserName ].join("\\");
};
ProcMgr.prototype = {
	list : function(id) {
		var query = "SELECT * FROM Win32_Process";
		query += id != null ? [ " WHERE ProcessId = ", id ].join("") : "";
		return new Enumerator( this.wmi.ExecQuery(query) );
	},
	kill : function(id) {
		this.list(id).each( function(proc) {
			proc.Terminate();
			throw $break;
		} );
	},
	exec : function(params) {
		if( params[0] && params[0].toLowerCase() == "--kill" && params[1] ) {
			// --kill
			this.kill( params[1] );
		} else if( params[0] && params[0].toLowerCase() == "--list" ) {
			// --list
			
			// -a オプション。全ユーザのプロセスを列挙
			var allUser = ( params[1] != null && /^-.*A/i.test( params[1] ) );
			// -d オプション。コマンドラインの情報を付加
			var detail = ( params[1] != null && /^-.*d/i.test( params[1] ) );
			
			var inited = false;
			var _self = this;
			this.list().each( function(proc) {
				if( ! inited ) {
					inited = true;
					echo( [
						"USER".align( "left", 10 ),
						"PEAK".align( "right", 8 ),
						"PID".align( "right", 5 ),
						"WK-SIZE".align( "right", 8 ),
						"TIME".align( "right", 10 ),
						"COMMAND".align( "left", 16 )
					].join(" ") );
				}
				var cmd = proc.CommandLine == null ? "(null)" : String( proc.CommandLine );
				var time = parseInt( ( Number(proc.KernelModeTime) + Number(proc.UserModeTime) ) / 10000 / 1000 );
				var info = proc.ExecMethod_( proc.Methods_.Item("GetOwner").Name, null );
				
				// プロセスオーナーのチェック
				if( ! allUser && [ info.Domain, info.User ].join("\\").toLowerCase() != _self.currentUser.toLowerCase() ) throw $continue;
				
				echo( [
					String(info.User || "(null)" ).align( "left", 10 ),
					String(Number(proc.PeakWorkingSetSize)).align( "right", 8 ),
					String(proc.ProcessId).align( "right", 5 ),
					String(Number(proc.WorkingSetSize) / 1024).align( "right", 8 ),
					String( time.toTime() ).align( "right", 10 ),
					[
						String(proc.Name),
						detail ? [ "(", cmd, ")" ].join("") : ""
					].join(" ")
				].join(" ") );
			} );
		} else {
			// no parameter -> --list
			this.exec( [ "--list" ] );
		}
	}
};

new ProcMgr().exec( args );
//
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
