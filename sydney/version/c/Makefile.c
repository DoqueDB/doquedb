/*
 * Makefile.c --- resource
 * 
 * Copyright (c) 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * 製品バージョン
 * メジャーな変更があったときに3桁目または4桁目を増やす
 */
DIST_PRODUCTVERSION=1,0,0,0
ProductVersion_STR="1, 0, 0, 0\\0"

/*
 * 各モジュールのファイルバージョン
 */

SYDSERVER_VERSION=1
SQLI_VERSION=1
USERADD_VERSION=1
USERDEL_VERSION=1
USERPASSWORD_VERSION=1
CLIENT_VERSION=1
KERNEL_VERSION=1
MESSAGE_VERSION=1
BTREE_VERSION=1
BTREE2_VERSION=1
FILECOMMON_VERSION=1
FULLTEXT_VERSION=1
FULLTEXT2_VERSION=1
INVERTED_VERSION=1
LOB_VERSION=1
RECORD_VERSION=1
RECORD2_VERSION=1
VECTOR_VERSION=1
VECTOR2_VERSION=1
BITMAP_VERSION=1
PDO_VERSION=1
ARRAY_VERSION=1
KDTREE_VERSION=1
TRAGENT_VERSION=1

/*
 * ファイルバージョンより下位のバージョン
 * リリースしたソースに対して bugfix 版をリリースような場合に利用する
 *
 * 例) MINOR_VERSION=-bugfix1
 */
MINOR_VERSION=

/*
 * InternalName
 */
KERNEL_INTERNAL=Database Kernel
DRIVER_INTERNAL=Database File Driver
UTILITY_INTERNAL=Database Utility

/*
 * SydServerのDescription
 */
SYDSERVER_MODULE=Server
SYDSERVER_DESC_ORIGINAL=SydServer.exe

/*
 * SqliのDescription
 */
SQLI_MODULE=Utility Sqli Command
SQLI_DESC_ORIGINAL=Sqli.exe

/*
 * UserAddのDescription
 */
USERADD_MODULE=Utility UserAdd Command
USERADD_DESC_ORIGINAL=UserAdd.exe

/*
 * UserDelのDescription
 */
USERDEL_MODULE=Utility UserDel Command
USERDEL_DESC_ORIGINAL=UserDel.exe

/*
 * UserPasswdのDescription
 */
USERPASSWORD_MODULE=Utility UserPassword Command
USERPASSWORD_DESC_ORIGINAL=UserPassword.exe

/*
 * ClientのDescription
 */
CLIENT_MODULE=Client Module
CLIENT_DESC_ORIGINAL=TRMeister$(CLIENTSUFFIX).dll

/*
 * SyKernelのDescription
 */
KERNEL_MODULE=Kernel Module
KERNEL_DESC_ORIGINAL=SyKernel.dll

/*
 * MessageのDescription
 */
MESSAGE_MODULE=Message Module
MESSAGE_DESC_ORIGINAL=SyMesLNG$(CLIENTSUFFIX).dll

/*
 * BtreeのDescription
 */
BTREE_MODULE=Btree Module
BTREE_DESC_ORIGINAL=SyDrvBtr.dll

/*
 * Btree2のDescription
 */
BTREE2_MODULE=Btree2 Module
BTREE2_DESC_ORIGINAL=SyDrvBtr2.dll

/*
 * FileCommonのDescription
 */
FILECOMMON_MODULE=FileCommon Module
FILECOMMON_DESC_ORIGINAL=SyDrvCom.dll

/*
 * FullTextのDescription
 */
FULLTEXT_MODULE=FullText Module
FULLTEXT_DESC_ORIGINAL=SyDrvFts.dll

/*
 * FullText2のDescription
 */
FULLTEXT2_MODULE=FullText2 Module
FULLTEXT2_DESC_ORIGINAL=SyDrvFts2.dll

/*
 * InvertedのDescription
 */
INVERTED_MODULE=Inverted Module
INVERTED_DESC_ORIGINAL=SyDrvInv.dll

/*
 * LOBのDescription
 */
LOB_MODULE=Lob Module
LOB_DESC_ORIGINAL=SyDrvLob.dll

/*
 * RecordのDescription
 */
RECORD_MODULE=Record Module
RECORD_DESC_ORIGINAL=SyDrvRcd.dll

/*
 * Record2のDescription
 */
RECORD2_MODULE=Record2 Module
RECORD2_DESC_ORIGINAL=SyDrvRcd2.dll

/*
 * VectorのDescription
 */
VECTOR_MODULE=Vector Module
VECTOR_DESC_ORIGINAL=SyDrvVct.dll

/*
 * Vector2のDescription
 */
VECTOR2_MODULE=Vector2 Module
VECTOR2_DESC_ORIGINAL=SyDrvVct2.dll

/*
 * BitmapのDescription
 */
BITMAP_MODULE=Bitmap Module
BITMAP_DESC_ORIGINAL=SyDrvBmp.dll

/*pdo*/
PDO_MODULE=PHP_PDO Module
PDO_DESC_ORIGINAL=php_pdo_trmeister.dll

/*
 * ArrayのDescription
 */
ARRAY_MODULE=Array Module
ARRAY_DESC_ORIGINAL=SyDrvAry.dll

/*
 * KdTreeのDescription
 */
KDTREE_MODULE=KDTree Module
KDTREE_DESC_ORIGINAL=SyDrvKtr.dll

/*
 * TRAgentのDescription
 */
TRAGENT_MODULE=TRMeister Agent
TRAGENT_DESC_ORIGINAL=TRAgent.exe
	
/*
 * SyKernelVersion.h関係
 */
TOP_EXPORT_HDRDIR = ../../include
TOP_EXPORT_HDRS = \
	SyKernelVersion.h

/*
 * リリースによらず固定だがオブジェクトに依存するものは
 * 下記の .pl ファイルを作成するエントリに記載されている
 *
 * オブジェクトに依存しないものは ../src/VerInfo.c に直接記載されている
 */

CONF_STR=CONF

FILETYPE_NUM_EXE=0x1L
FILETYPE_NUM_DLL=0x2L

#ifdef SYD_OS_WINNT4_0
TARGETFILES= \
	$(TOP_EXPORT_HDRS) \
	SydServer.rc \
	Sqli.rc \
	UserAdd.rc \
	UserDel.rc \
	UserPassword.rc \
	Client.rc \
	Kernel.rc \
	Message.rc \
	Btree.rc \
	Btree2.rc \
	FileCommon.rc \
	FullText.rc \
	FullText2.rc \
	Inverted.rc \
	Lob.rc \
	Record.rc \
	Record2.rc \
	Vector.rc \
	Vector2.rc \
	Bitmap.rc \
	Pdo.rc \
	Array.rc \
	KdTree.rc \
	TRAgent.rc
#else
TARGETFILES= \
	$(TOP_EXPORT_HDRS)
#endif

#ifdef SYD_OS_WINNT4_0
PERLSCRIPTS = \
	SydServer.pl \
	Sqli.pl \
	UserAdd.pl \
	UserDel.pl \
	UserPassword.pl \
	Client.pl \
	Kernel.pl \
	Message.pl \
	Btree.pl \
	Btree2.pl \
	FileCommon.pl \
	FullText.pl \
	FullText2.pl \
	Inverted.pl \
	Lob.pl \
	Record.pl \
	Record2.pl \
	Vector.pl \
	Vector2.pl \
	Bitmap.pl \
	SyKernelVersion.pl \
	Pdo.pl \
	Array.pl \
	KdTree.pl \
	TRAgent.pl	
#else
PERLSCRIPTS = \
	SyKernelVersion.pl
#endif

PERLFLAGS = -p

.SUFFIXES: .rc .pl

.pl.rc:
	-$(RM) $@
	$(PERL) $(PERLFLAGS) common.pl ../src/VerInfo.rc.master | $(PERL) $(PERLFLAGS) $*.pl > $*.tmp
	$(MV) $*.tmp $@

SyKernelVersion.h: ../src/SyKernelVersion.h.master SyKernelVersion.pl
	-$(RM) $@
	$(PERL) $(PERLFLAGS) SyKernelVersion.pl ../src/SyKernelVersion.h.master > SyKernelVersion.tmp
	$(MV) SyKernelVersion.tmp $@

/*
 * all
 */
AllTarget(Makefile $(TARGETFILES))
clientall:: all

CleanTarget($(TARGETFILES))
CleanTarget($(PERLSCRIPTS))

common.pl: ../c/Makefile.c
	-$(RM) $@
	echo # DO NOT EDIT THIS FILE > $@
	echo s/DIST_PRODUCTVERSION/$(DIST_PRODUCTVERSION)/g; >> $@
	echo s/ProductVersion_STR/$(ProductVersion_STR)/g; >> $@
	echo s/CONF_STR/$(CONF_STR)/g; >> $@

SydServer.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(SYDSERVER_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(SYDSERVER_MODULE)/g; >> $@
	echo s/ORIGINAL/$(SYDSERVER_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(UTILITY_INTERNAL)/g; >> $@

Sqli.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(SQLI_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(SQLI_MODULE)/g; >> $@
	echo s/ORIGINAL/$(SQLI_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(UTILITY_INTERNAL)/g; >> $@

UserAdd.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(USERADD_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(USERADD_MODULE)/g; >> $@
	echo s/ORIGINAL/$(USERADD_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(UTILITY_INTERNAL)/g; >> $@

UserDel.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(USERDEL_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(USERDEL_MODULE)/g; >> $@
	echo s/ORIGINAL/$(USERDEL_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(UTILITY_INTERNAL)/g; >> $@

UserPassword.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(USERPASSWORD_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(USERPASSWORD_MODULE)/g; >> $@
	echo s/ORIGINAL/$(USERPASSWORD_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(UTILITY_INTERNAL)/g; >> $@

Client.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(CLIENT_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(CLIENT_MODULE)/g; >> $@
	echo s/ORIGINAL/$(CLIENT_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(KERNEL_INTERNAL)/g; >> $@

Kernel.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(KERNEL_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(KERNEL_MODULE)/g; >> $@
	echo s/ORIGINAL/$(KERNEL_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(KERNEL_INTERNAL)/g; >> $@

Message.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(MESSAGE_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(MESSAGE_MODULE)/g; >> $@
	echo s/ORIGINAL/$(MESSAGE_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(KERNEL_INTERNAL)/g; >> $@

Btree.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(BTREE_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(BTREE_MODULE)/g; >> $@
	echo s/ORIGINAL/$(BTREE_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Btree2.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(BTREE2_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(BTREE2_MODULE)/g; >> $@
	echo s/ORIGINAL/$(BTREE2_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

FileCommon.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(FILECOMMON_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(FILECOMMON_MODULE)/g; >> $@
	echo s/ORIGINAL/$(FILECOMMON_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

FullText.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(FULLTEXT_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(FULLTEXT_MODULE)/g; >> $@
	echo s/ORIGINAL/$(FULLTEXT_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

FullText2.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(FULLTEXT2_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(FULLTEXT2_MODULE)/g; >> $@
	echo s/ORIGINAL/$(FULLTEXT2_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Inverted.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(INVERTED_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(INVERTED_MODULE)/g; >> $@
	echo s/ORIGINAL/$(INVERTED_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Lob.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(LOB_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(LOB_MODULE)/g; >> $@
	echo s/ORIGINAL/$(LOB_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Record.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(RECORD_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(RECORD_MODULE)/g; >> $@
	echo s/ORIGINAL/$(RECORD_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Record2.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(RECORD2_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(RECORD2_MODULE)/g; >> $@
	echo s/ORIGINAL/$(RECORD2_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Vector.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(VECTOR_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(VECTOR_MODULE)/g; >> $@
	echo s/ORIGINAL/$(VECTOR_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Vector2.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(VECTOR2_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(VECTOR2_MODULE)/g; >> $@
	echo s/ORIGINAL/$(VECTOR2_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Bitmap.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(BITMAP_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(BITMAP_MODULE)/g; >> $@
	echo s/ORIGINAL/$(BITMAP_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Pdo.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(PDO_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(PDO_MODULE)/g; >> $@
	echo s/ORIGINAL/$(PDO_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

Array.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(ARRAY_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(ARRAY_MODULE)/g; >> $@
	echo s/ORIGINAL/$(ARRAY_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

KdTree.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(KDTREE_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(KDTREE_MODULE)/g; >> $@
	echo s/ORIGINAL/$(KDTREE_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(DRIVER_INTERNAL)/g; >> $@

TRAgent.pl: common.pl ../c/Makefile.c
	-$(RM) $@
	echo s/VERSION_NUM/$(TRAGENT_VERSION)/g; >> $@
	echo s/FILETYPE_NUM/$(FILETYPE_NUM_DLL)/g; >> $@
	echo s/MODULE_NAME/$(TRAGENT_MODULE)/g; >> $@
	echo s/ORIGINAL/$(TRAGENT_DESC_ORIGINAL)/g; >> $@
	echo s/INTERNAL_NAME/$(UTILITY_INTERNAL)/g; >> $@
	
SyKernelVersion.pl: ../c/Makefile.c
	-$(RM) $@
#ifdef SYD_OS_WINNT4_0
	echo s/VERSION_NUM/$(KERNEL_VERSION)/g; >> $@
	echo s/MINOR_VERSION/$(MINOR_VERSION)/g; >> $@
#else
	echo "s/VERSION_NUM/$(KERNEL_VERSION)/g;" >> $@
	echo "s/MINOR_VERSION/$(MINOR_VERSION)/g;" >> $@
#endif

InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))

/* no makefile.h */
MAKEFILE_H =
