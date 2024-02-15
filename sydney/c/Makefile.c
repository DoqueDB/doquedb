/*
 * Makefile.c --- 
 * 
 * Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

VERSIONINFODIR = version

SYD_SUBDIRS = \
	$(VERSIONINFODIR) \
	Kernel \
	Driver

CLIENT_SUBDIRS = \
	$(VERSIONINFODIR) \
	Kernel

UTIL_SUBDIRS = \
	Java \
	Utility

TEST_SUBDIRS = \
	Test

MODULE_SUBDIRS = \
	ModuleTest

SUBDIRS = \
	$(SYD_SUBDIRS) \
	$(UTIL_SUBDIRS) \
	$(TEST_SUBDIRS) 

CLIENTSUBDIRS = \
	$(CLIENT_SUBDIRS) \
	$(UTIL_SUBDIRS)

HDRDIR = ../include

PACKAGE_HDRS = \
	$(HDRDIR)/SyDefault.h \
	$(HDRDIR)/SyDLL.h \
	$(HDRDIR)/SyDynamicCast.h \
	$(HDRDIR)/SyFor.h \
	$(HDRDIR)/SyNameSpace.h \
	$(HDRDIR)/SyInclude.h \
	$(HDRDIR)/SyTypeName.h \
	$(HDRDIR)/SyTypes.h

#ifdef OS_RHLINUX6_0
.NOTPARALLEL:
#endif

#ifdef SYD_DLL
buildall:: sydney utility test
build-client:: client clientutility
#else
buildall:: sydney all utility test
build-client:: client clientall clientutility
#endif
buildmoduletest: moduletest
moduletestconf-r: tmoduleconf-r

#ifdef SYD_COVERAGE
sydney: sydobjlist-r sydall-r sydinstall-r sydinstalldll-r
client: sydclientobjlist-r sycclientall-r sydclientinstall-r sydclientinstalldll-r
#else
sydney: sydinstallh-r sydobjlist-r sydall-r sydinstall-r sydinstalldll-r
client: sydinstallh-r sydclientobjlist-r sydclientall-r sydclientinstall-r sydclientinstalldll-r
#endif
utility: uinstallh-r uobjlist-r uall-r
clientutility: clientuall-r
test: tall-r
moduletest: tmoduleall-r

PACKAGE_DIR = ..$(S)tools$(S)setup$(S)package

MOD_DIR = $(MODTOP)$(S)MODLIBPATH
UNA_DIR = $(UNATOP)$(S)UNALIBPATH

#ifdef SYD_OS_WINNT4_0
CP_R = xcopy /I /S /R /Y
#else
CP_R = cp -r
#endif

#ifdef DEBUG
EXTLIBPATH = lib_debug
#else
EXTLIBPATH = lib
#endif
#ifdef SYD_OS_WINNT4_0
/* EDITBIN.exe */
EDITBIN = EDITBIN.exe
/* version number of MSVC */
#ifdef SYD_C_MS10_0
MSVCVERSION = v10
#else
#ifdef SYD_C_MS8_0
MSVCVERSION = v8
#else
#ifdef SYD_C_MS7_1
MSVCVERSION = v7
#else
MSVCVERSION = v6
#endif
#endif
#endif
MSVC_DIR = $(SYDEXTLIB)$(S)msvc$(S)$(MSVCVERSION)
#endif
UNA_DATA_DIR = $(UNATOP)$(S)data$(S)
TERM_DATA_DIR = $(SYDTOP)$(S)lib$(S)$(OSNAME)$(S)mod$(S)data
MOD_DATA_UNADIC_DIR = $(UNA_DATA_DIR)$(S)unadic
MOD_DATA_TERM_DIR = $(TERM_DATA_DIR)$(S)term
MOD_DATA_TERMRSC7_DIR = $(TERM_DATA_DIR)$(S)termrsc7
ZLIB_DIR = $(ZLIBTOP)
BOOST_DIR = $(BOOSTTOP)$(S)lib
SYD_DIR = ..$(S)c.CONF
SQLI_DIR = ..$(S)Utility$(S)Sqli$(S)c.CONF
USERADD_DIR = ..$(S)Utility$(S)UserAdd$(S)c.CONF
USERDEL_DIR = ..$(S)Utility$(S)UserAdd$(S)c.CONF
USERPASSWORD_DIR = ..$(S)Utility$(S)UserAdd$(S)c.CONF
SYDSERVER_DIR = ..$(S)Utility$(S)SydServer$(S)c.CONF
#ifdef SYD_C_ICC
#ifdef SYD_ARCH64
INTEL_LIB_DIR = /opt/intel/cc/lib64
#else
INTEL_LIB_DIR = /opt/intel/cc/lib
#endif
#endif

PACKAGE_SYDTEST_DIR = ..$(S)tools$(S)setup$(S)package_sydtest
SYDTEST_DIR = ..$(S)Test$(S)SydTest$(S)c.CONF
SYDTEST_EXTERNALSCORECALCULATOR_DIR = ..$(S)Test$(S)SydTest$(S)ExternalScoreCalculator$(S)c.CONF

#ifdef SYD_OS_WINNT4_0
COMMON_HDR_SRC_DIR = ..$(S)Kernel$(S)Common$(S)Common
SERVER_HDR_SRC_DIR = ..$(S)Kernel$(S)Server$(S)Server
#endif

#ifdef SYD_OS_WINNT4_0
INSTSRC_DIR = $(PACKAGE_DIR)$(S)olib$(S)windows
#endif
JAVADOC_SRC_DIR = ..$(S)Java$(S)JDBC$(S)docs$(S)api
JDBC_SRC_DIR = ..$(S)Java$(S)JDBC$(S)lib
JDBC_DIR = $(PACKAGE_NAME)$(S)jdbc
HIBERNATE_JAVADOC_SRC_DIR = ..$(S)Java$(S)Hibernate$(S)docs$(S)api
HIBERNATE_SRC_DIR = ..$(S)Java$(S)Hibernate$(S)lib
HIBERNATE_DIR = $(PACKAGE_NAME)$(S)hibernate
UTILITY_SRC_DIR = ..$(S)Utility$(S)SydAdmin$(S)lib
PASSWORD_SRC_DIR = $(PACKAGE_DIR)$(S)password

OBJ_DIR = $(PACKAGE_NAME)$(S)obj
HDR_DIR = $(PACKAGE_NAME)$(S)include
#ifdef SYD_OS_WINNT4_0
OBJ_BIN_DIR = $(OBJ_DIR)
OBJ_LIB_DIR = $(OBJ_DIR)
UTILITY_DIR = $(OBJ_BIN_DIR)$(S)java
LIB_DIR = $(PACKAGE_NAME)$(S)lib
ETC_DIR = $(OBJ_DIR)
VCLIB_DIR = $(PACKAGE_NAME)$(S)msvc
#else
OBJ_BIN_DIR = $(OBJ_DIR)$(S)bin
OBJ_LIB_DIR = $(OBJ_DIR)$(S)lib
UTILITY_DIR = $(OBJ_DIR)$(S)java
ETC_DIR = $(OBJ_DIR)$(S)etc
#endif
SAMPLE_DIR = $(PACKAGE_NAME)$(S)sample
DOC_DIR = $(PACKAGE_NAME)$(S)doc
LICENSE_DIR = $(PACKAGE_NAME)$(S)LICENSE

OBJ_DATA_DIR = $(OBJ_DIR)$(S)data
OBJ_DATA_UNADIC_DIR = $(OBJ_DATA_DIR)$(S)unadic
OBJ_DATA_TERM_DIR = $(OBJ_DATA_DIR)$(S)term
OBJ_DATA_TERMRSC7_DIR = $(OBJ_DATA_DIR)$(S)termrsc7

DIST_DIR = $(PACKAGE_NAME)$(S)dist
DIST_JDBC_DIR = $(DIST_DIR)$(S)jdbc
DIST_OBJ_DIR = $(DIST_DIR)$(S)obj

#ifdef SYD_OS_WINNT4_0
SH_SUFFIX = .bat
#else
SH_SUFFIX = .sh
#endif

#ifdef SYD_OS_WINNT4_0
ALL_DLL = *.exe
ALL_DLL = *.dll
ALL_BAT = *.bat
ALL_FILE = *.*
ALL_HDR = *.h
ALL_PDB = *.pdb
ALL_DOC = *.doc
ALL_XLS = *.xls
#else
#ifdef PURIFY
ALL_DLL = *.so*
#else
ALL_DLL = *.so
#endif
ALL_BAT = *.sh
ALL_FILE = *.*
ALL_HDR = *.h
ALL_PDB =
ALL_DOC = *.doc
ALL_XLS = *.xls
#endif

/* client is INCLUDED by package. */
package-client: tape-r copyobj-client copyobj-common copylocal-common
/* dm, sydtest is NOT included by package. */
package-dm: copyobj-dm
package-sydtest: copyobj-sydtest

#ifdef SYD_COVERAGE
package: copyobj copylocal
packageUAUD: copyobjUAUD copylocal
package-dist: copyobj-dist copylocal
package-old: copyobj-old copylocal
#else
package: copyobj copylocal
packageUAUD: copyobjUAUD copylocal
package-dist: copyobj-dist copylocal
package-old: copyobj-old copylocal
#endif
#ifdef SYD_OS_WINNT4_0
package-release:
#else
package-release:
	$(SED) '/^#start DEVENV/,/^#end DEVENV/d' < $(PACKAGE_NAME)$(S)conf$(SH_SUFFIX) > $(PACKAGE_NAME)$(S)conf$(SH_SUFFIX).new
	@$(RM) $(PACKAGE_NAME)$(S)conf$(SH_SUFFIX)
	@$(MV) $(PACKAGE_NAME)$(S)conf$(SH_SUFFIX).new $(PACKAGE_NAME)$(S)conf$(SH_SUFFIX)
#endif

copyobj: copyobj-server copyobj-common

copyobj-client:
	@$(MKDIR) $(OBJ_LIB_DIR)
#ifdef SYD_DLL
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYD_DIR)$(S)TRMeister$(CLIENTSUFFIX)$(D) $(OBJ_LIB_DIR)
#else
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYD_DIR)$(S)$(P)Trmeister$(CLIENTSUFFIX)$(D) $(OBJ_LIB_DIR)
#endif
#endif
#ifdef SYD_OS_WINNT4_0
	@$(MKDIR) $(LIB_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(SYD_DIR)$(S)TRMeister$(CLIENTSUFFIX).lib $(LIB_DIR)
#endif
	@$(MKDIR) $(HDR_DIR)$(S)mod
	@$(INSTALL) $(INSTALLINCLFLAGS) $(MODTOP)$(S)include$(S)$(ALL_HDR) $(HDR_DIR)$(S)mod
	@$(MKDIR) $(SAMPLE_DIR)$(S)sqli
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PACKAGE_DIR)$(S)sample$(S)windows$(S)sqli$(S)$(ALL_FILE) $(SAMPLE_DIR)$(S)sqli
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PACKAGE_DIR)$(S)sample$(S)windows$(S)sqli$(S)Makefile $(SAMPLE_DIR)$(S)sqli
#endif
#ifdef SYD_OS_LINUX
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PACKAGE_DIR)$(S)sample$(S)linux$(S)sqli$(S)$(ALL_FILE) $(SAMPLE_DIR)$(S)sqli
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PACKAGE_DIR)$(S)sample$(S)linux$(S)sqli$(S)Makefile $(SAMPLE_DIR)$(S)sqli
#endif
#ifdef SYD_OS_WINNT4_0
#ifndef _WIN64
	$(EDITBIN) /LARGEADDRESSAWARE $(OBJ_DIR)$(S)$(ALL_DLL) > NUL
#endif
#endif

copyobj-commonlib:
	@$(MKDIR) $(OBJ_LIB_DIR)
#ifdef SYD_DLL
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYD_DIR)$(S)$(P)SyMesEng$(CLIENTSUFFIX)$(D) $(OBJ_LIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYD_DIR)$(S)$(P)SyMesJpn$(CLIENTSUFFIX)$(D) $(OBJ_LIB_DIR)
#endif
	@$(INSTALL) $(INSTALLBINFLAGS) $(MOD_DIR)$(S)$P$(ALL_DLL) $(OBJ_LIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(UNA_DIR)$(S)$(ALL_DLL) $(OBJ_LIB_DIR)
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(ZLIB_DIR)$(S)$(ALL_DLL) $(OBJ_LIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)olib$(S)windows$(S)$(ALL_DLL) $(OBJ_LIB_DIR)
#ifdef SYD_C_MS8_0
	@$(MKDIR) $(VCLIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(MSVC_DIR)$(S)$(ALL_EXE) $(VCLIB_DIR)
#else
	@$(INSTALL) $(INSTALLBINFLAGS) $(MSVC_DIR)$(S)$(EXTLIBPATH)$(S)$(ALL_DLL) $(OBJ_LIB_DIR)
#endif
	@$(INSTALL) $(INSTALLBINFLAGS) $(MOD_DIR)$(S)$(MODLIBNAME).pdb $(OBJ_LIB_DIR)
	@$(MKDIR) $(LIB_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(SYD_DIR)$(S)SyMesEng$(CLIENTSUFFIX)$(L) $(LIB_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(SYD_DIR)$(S)SyMesJpn$(CLIENTSUFFIX)$(L) $(LIB_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(MOD_DIR)$(S)$(MODLIBNAME).lib $(LIB_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(UNA_DIR)$(S)$(UNALIBNAME).lib $(LIB_DIR)
#endif
#ifdef SYD_C_ICC
	@$(INSTALL) $(INSTALLBINFLAGS) $(INTEL_LIB_DIR)$(S)libimf$(D) $(OBJ_LIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(INTEL_LIB_DIR)$(S)libirc$(D) $(OBJ_LIB_DIR)
#ifdef SYD_C_ICC14
	@$(INSTALL) $(INSTALLBINFLAGS) $(INTEL_LIB_DIR)$(S)libirng$(D) $(OBJ_LIB_DIR)
#else
	@$(INSTALL) $(INSTALLBINFLAGS) $(INTEL_LIB_DIR)$(S)libcxaguard$(D).5 $(OBJ_LIB_DIR)
#endif
	@$(INSTALL) $(INSTALLBINFLAGS) $(INTEL_LIB_DIR)$(S)libintlc$(D).5 $(OBJ_LIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(INTEL_LIB_DIR)$(S)libsvml$(D) $(OBJ_LIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(INTEL_LIB_DIR)$(S)libiomp5$(D) $(OBJ_LIB_DIR)
#endif
	/* the parameter expansions below do not affect windows dll */
#ifndef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(BOOST_DIR)$(S)$(BOOSTDLL:-l%=lib%.so).$(BOOSTVERSION:v%=%) $(OBJ_LIB_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(BOOST_DIR)$(S)$(BOOST_FILESYSTEM_DLL:-l%=lib%.so).$(BOOSTVERSION:v%=%) $(OBJ_LIB_DIR)
#endif

copyobj-common: copyobj-commonlib
	@$(MKDIR) $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(SQLI_DIR)$(S)Sqli$E $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(USERADD_DIR)$(S)UserAdd$E $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(USERDEL_DIR)$(S)UserDel$E $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(USERPASSWORD_DIR)$(S)UserPassword$E $(OBJ_BIN_DIR)
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(SQLI_DIR)$(S)Sqli.pdb $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(USERADD_DIR)$(S)UserAdd.pdb $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(USERDEL_DIR)$(S)UserDel.pdb $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(USERPASSWORD_DIR)$(S)UserPassword.pdb $(OBJ_BIN_DIR)
#endif
	@$(INSTALL) $(INSTALLLIBFLAGS) $(PACKAGE_DIR)$(S)README.txt $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLLIBFLAGS) $(PACKAGE_DIR)$(S)README_*.txt $(PACKAGE_NAME)
	@$(MKDIR) $(DOC_DIR)
	@$(CP_R) $(PACKAGE_DIR)$(S)doc$(S)* $(DOC_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)conf$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)install-client$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(MKDIR) $(LICENSE_DIR)
	@$(CP_R) $(PACKAGE_DIR)$(S)LICENSE$(S)* $(LICENSE_DIR)
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)trload$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)trunload$(SH_SUFFIX) $(PACKAGE_NAME)
#else
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)Sqli$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)load$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)unload$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)UserAdd$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)UserDel$(SH_SUFFIX) $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)UserPassword$(SH_SUFFIX) $(PACKAGE_NAME)
#endif
	@$(MKDIR) $(JDBC_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(JDBC_SRC_DIR)$(S)doquedb.jar $(JDBC_DIR)
	@$(MKDIR) $(HIBERNATE_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(HIBERNATE_SRC_DIR)$(S)dqdialect.jar $(HIBERNATE_DIR)
#ifdef SYD_OS_WINNT4_0
	@$(MKDIR) $(OBJ_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(INSTSRC_DIR)$(S)instsrc.exe $(OBJ_DIR)
#ifndef _WIN64
	$(EDITBIN) /LARGEADDRESSAWARE $(OBJ_DIR)$(S)$(ALL_DLL) > NUL
#endif
#endif

copyobj-server:
	@$(MKDIR) $(OBJ_LIB_DIR)
#ifdef SYD_DLL
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYD_DIR)$(S)$(ALL_DLL) $(OBJ_LIB_DIR)
#ifndef _WIN64
#endif
#endif
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(UNA_DIR)$(S)$(ALL_PDB) $(OBJ_LIB_DIR)
#endif
	@$(MKDIR) $(OBJ_BIN_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYDSERVER_DIR)$(S)SydServer$E $(OBJ_BIN_DIR)
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYDSERVER_DIR)$(S)SydServer.pdb $(OBJ_BIN_DIR)
#endif
	@$(MKDIR) $(OBJ_DATA_DIR)
	@$(MKDIR) $(OBJ_DATA_UNADIC_DIR)
	@$(MKDIR) $(OBJ_DATA_UNADIC_DIR)$(S)norm
	@$(MKDIR) $(OBJ_DATA_UNADIC_DIR)$(S)stem
	@$(MKDIR) $(OBJ_DATA_UNADIC_DIR)$(S)una
  
	@$(MKDIR) $(OBJ_DATA_TERM_DIR)
	@$(MKDIR) $(OBJ_DATA_TERMRSC7_DIR)

	@$(INSTALL) $(INSTALLINCLFLAGS) $(MOD_DATA_UNADIC_DIR)$(S)unaparam.dat $(OBJ_DATA_UNADIC_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(MOD_DATA_UNADIC_DIR)$(S)norm$(S)$(ALL_FILE) $(OBJ_DATA_UNADIC_DIR)$(S)norm
	@$(INSTALL) $(INSTALLINCLFLAGS) $(MOD_DATA_UNADIC_DIR)$(S)stem$(S)$(ALL_FILE) $(OBJ_DATA_UNADIC_DIR)$(S)stem
	@$(INSTALL) $(INSTALLINCLFLAGS) $(MOD_DATA_UNADIC_DIR)$(S)una$(S)$(ALL_FILE) $(OBJ_DATA_UNADIC_DIR)$(S)una
  
	@$(INSTALL) $(INSTALLINCLFLAGS) $(MOD_DATA_TERM_DIR)$(S)$(ALL_FILE) $(OBJ_DATA_TERM_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(MOD_DATA_TERMRSC7_DIR)$(S)$(ALL_FILE) $(OBJ_DATA_TERMRSC7_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(filter-out %$(S)installadmin.sh, $(shell ls $(PACKAGE_DIR)$(S)$(OSNAME)$(S)$(ALL_BAT))) $(PACKAGE_NAME)
#ifdef SYD_ARCH64
#ifndef SYD_OS_SOLARIS
	@$(CP) $(PACKAGE_DIR)$(S)$(OSNAME)64$(S)$(ALL_BAT) $(PACKAGE_NAME)
#endif
#endif
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PACKAGE_DIR)$(S)README.txt $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PACKAGE_DIR)$(S)README_jp.txt $(PACKAGE_NAME)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PACKAGE_DIR)$(S)exclude.txt $(PACKAGE_NAME)
#endif

copyobjUAUD: copyobj
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)conf.bat-UAUD $(PACKAGE_NAME)
	@$(RM) $(PACKAGE_NAME)$(S)conf.bat
	@$(MV) $(PACKAGE_NAME)$(S)conf.bat-UAUD $(PACKAGE_NAME)$(S)conf.bat

copyobj-old: copyobj
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)$(ALL_BAT)-old $(PACKAGE_NAME)

copyobj-dist: copyobj
	@$(MKDIR) $(DIST_DIR)
	@$(MKDIR) $(DIST_JDBC_DIR)
	@$(RMALL) $(DIST_OBJ_DIR)
	@$(MV) $(PACKAGE_NAME)$(S)conf$(SH_SUFFIX) $(DIST_DIR)
	@$(MV) $(PACKAGE_NAME)$(S)exclude.txt $(DIST_DIR)
	@$(MV) $(PACKAGE_NAME)$(S)install$(SH_SUFFIX) $(DIST_DIR)
	@$(MV) $(PACKAGE_NAME)$(S)setup$(SH_SUFFIX) $(DIST_DIR)
	@$(MV) $(PACKAGE_NAME)$(S)trmeister$(SH_SUFFIX) $(DIST_DIR)
	@$(MV) $(PACKAGE_NAME)$(S)uninstall$(SH_SUFFIX) $(DIST_DIR)
	@$(MV) $(PACKAGE_NAME)$(S)unsetup$(SH_SUFFIX) $(DIST_DIR)
	@$(MV) $(JDBC_DIR)$(S)doquedb.jar $(DIST_JDBC_DIR)
	@$(MV) $(OBJ_DIR) $(DIST_OBJ_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_DIR)$(S)$(OSNAME)$(S)install$(SH_SUFFIX)-dist $(DIST_DIR)
	@$(RM) $(DIST_DIR)$(S)install$(SH_SUFFIX)
	@$(MV) $(DIST_DIR)$(S)install$(SH_SUFFIX)-dist $(DIST_DIR)$(S)install$(SH_SUFFIX)

copylocal: copylocal-server copylocal-common

copylocal-common:
	@$(MKDIR) $(JDBC_DIR)$(S)javadoc
	@$(MKDIR) $(HIBERNATE_DIR)$(S)javadoc
	@$(MKDIR) $(OBJ_DIR)
	@$(MKDIR) $(UTILITY_DIR)
	@$(CP_R) $(JAVADOC_SRC_DIR) $(JDBC_DIR)$(S)javadoc
	@$(CP_R) $(HIBERNATE_JAVADOC_SRC_DIR) $(HIBERNATE_DIR)$(S)javadoc
	@$(INSTALL) $(INSTALLBINFLAGS) $(UTILITY_SRC_DIR)$(S)load.jar $(UTILITY_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) $(UTILITY_SRC_DIR)$(S)unload.jar $(UTILITY_DIR)
copylocal-server:
	@$(MKDIR) $(ETC_DIR)
	@$(INSTALL) $(INSTALLINCLFLAGS) $(PASSWORD_SRC_DIR)$(S)*.pswd $(ETC_DIR)

copyobj-dm:
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(COMMON_HDR_SRC_DIR)$(S)ExceptionMessage.h $(HDR_DIR)$(S)Common
	@$(INSTALL) $(INSTALLBINFLAGS) $(COMMON_HDR_SRC_DIR)$(S)Message.h $(HDR_DIR)$(S)Common
	@$(INSTALL) $(INSTALLBINFLAGS) $(COMMON_HDR_SRC_DIR)$(S)MessageStream.h $(HDR_DIR)$(S)Common
	@$(INSTALL) $(INSTALLBINFLAGS) $(COMMON_HDR_SRC_DIR)$(S)MessageStreamBuffer.h $(HDR_DIR)$(S)Common
	@$(MKDIR) $(HDR_DIR)$(S)Server
	@$(INSTALL) $(INSTALLBINFLAGS) $(SERVER_HDR_SRC_DIR)$(S)Singleton.h $(HDR_DIR)$(S)Server
	@$(INSTALL) $(INSTALLBINFLAGS) $(SERVER_HDR_SRC_DIR)$(S)Module.h $(HDR_DIR)$(S)Server
	@$(INSTALL) $(INSTALLBINFLAGS) $(HDRDIR)$(S)SyDynamicCast.h $(HDR_DIR)
	@$(INSTALL) $(INSTALLBINFLAGS) SyKernel$(L) $(LIB_DIR)
#endif

copyobj-sydtest:
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYDTEST_DIR)$(S)SydTest$E $(OBJ_BIN_DIR)
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYDTEST_EXTERNALSCORECALCULATOR_DIR)$(S)$(ALL_DLL) $(OBJ_BIN_DIR)
#else
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYDTEST_EXTERNALSCORECALCULATOR_DIR)$(S)$(ALL_DLL) $(OBJ_LIB_DIR)
#endif
	@$(INSTALL) $(INSTALLBINFLAGS) $(PACKAGE_SYDTEST_DIR)$(S)$(OSNAME)$(S)$(ALL_BAT) $(PACKAGE_NAME)
#ifdef SYD_OS_WINNT4_0
	@$(INSTALL) $(INSTALLBINFLAGS) $(SYDTEST_DIR)$(S)SydTest.pdb $(OBJ_BIN_DIR)
#endif

MOD_BUILDDIR = ..$(S)..$(S)..$(S)mod$(S)$(MODVERSION)
UNA_BUILDDIR = ..$(S)..$(S)..$(S)nlp$(S)una$(S)$(UNAVERSION)

mod:
	BEGINLOCAL(mod) \				@@\
		cd $(MOD_BUILDDIR) LDELIM(mod) \	@@\
		$(MKCONFDIR) CONF LDELIM(mod) \		@@\
		cd c.CONF LDELIM(mod) \			@@\
		$(MAKE) conf-r installh-r LDELIM(mod) \	@@\
		$(MAKE) reconf-r buildall package \	@@\
	ENDLOCAL(mod)

clean-mod:
	BEGINLOCAL(clean-mod) \				@@\
		cd $(MOD_BUILDDIR)$(S)c.CONF LDELIM(clean-mod) \@@\
		$(MAKE) clean-r \			@@\
	ENDLOCAL(clean-mod)

una:
	BEGINLOCAL(una) \				@@\
		cd $(UNA_BUILDDIR) LDELIM(una) \	@@\
		$(MKCONFDIR) CONF LDELIM(una) \		@@\
		cd c.CONF LDELIM(una) \			@@\
		$(MAKE) conf-r installh-r LDELIM(una) \	@@\
		$(MAKE) reconf-r buildall package \	@@\
	ENDLOCAL(una)

clean-una:
	BEGINLOCAL(clean-una) \				@@\
		cd $(UNA_BUILDDIR)$(S)c.CONF LDELIM(clean-una) \ @@\
		$(MAKE) clean-r \			@@\
	ENDLOCAL(clean-una)

RTarget(sydinstallh, installh, $(SYD_SUBDIRS))
RTarget(sydobjlist, objlist, $(SYD_SUBDIRS))
RTarget(sydall, all, $(SYD_SUBDIRS))
RTarget(sydinstall, install, $(SYD_SUBDIRS))
RTarget(syddll, dll, $(SYD_SUBDIRS))
RTarget(sydinstalldll, installdll, $(SYD_SUBDIRS))

RTarget(uinstallh, installh, $(UTIL_SUBDIRS))
RTarget(uobjlist, objlist, $(UTIL_SUBDIRS))
RTarget(uall, all, $(UTIL_SUBDIRS))
RTarget(tall, all, $(TEST_SUBDIRS))
RTarget(tmoduleall, all, $(MODULE_SUBDIRS))

ConfRTargetBase(tmoduleconf, conf, $(MODULE_SUBDIRS))

tools::
	-@$(RM) $(BUILD_JAR)
	BEGINLOCAL(tools) \		@@\
		cd $(SYDBUILDDIR) && \	@@\
		$(MAKE) all \		@@\
	ENDLOCAL(tools)

/* RTarget(sydclientinstallh, clientinstallh, $(CLIENT_SUBDIRS)) */
RTarget(sydclientobjlist, clientobjlist, $(CLIENT_SUBDIRS))
RTarget(sydclientall, clientall, $(CLIENT_SUBDIRS))
RTarget(sydclientinstall, clientinstall, $(CLIENT_SUBDIRS))
RTarget(sydclientdll, clientdll, $(CLIENT_SUBDIRS))
RTarget(sydclientinstalldll, clientinstalldll, $(CLIENT_SUBDIRS))
RTarget(clientuall, clientall, $(UTIL_SUBDIRS))

/*
 * install header for package
 *
 *		The headers are used by client's codes.
 */
TapeHeaderTarget($(PACKAGE_HDRS), $(HDR_DIR))

/********************************************************/

#ifndef SYD_DLL
KERNEL_BASE = SyKernel
#if defined(OS_WINDOWSNT4_0) || defined(OS_WINDOWS98)
CLIENT_BASE = TRMeister$(CLIENTSUFFIX)
#else
CLIENT_BASE = Trmeister$(CLIENTSUFFIX)
#endif
TOP_INSTALL_DIR = .

/* CAUTION: The order of following ol files is very important */
KERNEL_OL = \
	$(COMMON_OL) \
	$(TOP_INSTALL_DIR)$(S)Utility.ol \
	$(TOP_INSTALL_DIR)$(S)Lock.ol \
	$(TOP_INSTALL_DIR)$(S)Trans.ol \
	$(TOP_INSTALL_DIR)$(S)Buffer.ol \
	$(TOP_INSTALL_DIR)$(S)Version.ol \
	$(TOP_INSTALL_DIR)$(S)Checkpoint.ol \
	$(TOP_INSTALL_DIR)$(S)PhysicalFile.ol \
	$(TOP_INSTALL_DIR)$(S)LogicalLog.ol \
	$(TOP_INSTALL_DIR)$(S)LogicalFile.ol \
	$(DRIVER_OL) \
	$(TOP_INSTALL_DIR)$(S)Statement.ol \
	$(TOP_INSTALL_DIR)$(S)Schema.ol \
	$(TOP_INSTALL_DIR)$(S)Plan.ol \
	$(TOP_INSTALL_DIR)$(S)Analysis.ol \
	$(TOP_INSTALL_DIR)$(S)Execution.ol \
	$(TOP_INSTALL_DIR)$(S)Opt.ol \
	$(TOP_INSTALL_DIR)$(S)Admin.ol \
	$(TOP_INSTALL_DIR)$(S)Server.ol \
	$(TOP_INSTALL_DIR)$(S)Communication.ol \
	$(INTERFACE_OL)

COMMON_OL = \
	$(TOP_INSTALL_DIR)/Message.ol \
	$(TOP_INSTALL_DIR)/Exception.ol \
	$(TOP_INSTALL_DIR)/Os.ol \
	$(TOP_INSTALL_DIR)/Common.ol

INTERFACE_OL = \
	$(TOP_INSTALL_DIR)$(S)Client.ol \
	$(TOP_INSTALL_DIR)$(S)Client2.ol

CLIENT_OL = \
	$(COMMON_OL) \
	$(TOP_INSTALL_DIR)$(S)Communication_C.ol \
	$(INTERFACE_OL)

#ifndef SYD_CPU_SPARC
OLD_DRIVER_OL = \
	$(TOP_INSTALL_DIR)$(S)Btree.ol \
	$(TOP_INSTALL_DIR)$(S)Vector.ol
#else
OLD_DRIVER_OL =
#endif
DRIVER_OL = \
	$(TOP_INSTALL_DIR)$(S)Array.ol \
	$(TOP_INSTALL_DIR)$(S)Bitmap.ol \
	$(TOP_INSTALL_DIR)$(S)Btree2.ol \
	$(TOP_INSTALL_DIR)$(S)FileCommon.ol \
	$(TOP_INSTALL_DIR)$(S)FullText.ol \
	$(TOP_INSTALL_DIR)$(S)Inverted.ol \
	$(TOP_INSTALL_DIR)$(S)Lob.ol \
	$(TOP_INSTALL_DIR)$(S)Record.ol \
	$(TOP_INSTALL_DIR)$(S)Vector2.ol \
	$(OLD_DRIVER_OL)

EXTRALOCALDLLFLAGS =  \
	$(LPATHOP)$(MOD_DIR) \
	$(LPATHOP)$(ZLIB_DIR)

LDLIBS = \
	$(MODLIBS) \
	$(ZLIBDLL)

KERNEL_TARGET = $P$(KERNEL_BASE)$L
CLIENT_TARGET = $P$(CLIENT_BASE)$L

ALLTARGETS = \
	$(KERNEL_TARGET) \
	$(CLIENT_TARGET)

AllTarget($(ALLTARGETS))

LibraryTarget5($(KERNEL_TARGET), $(KERNEL_OL))
InstallLibraryTarget($(KERNEL_TARGET), $(TOP_INSTALL_DIR))
LibraryTarget5($(CLIENT_TARGET), $(CLIENT_OL))
InstallLibraryTarget($(CLIENT_TARGET), $(TOP_INSTALL_DIR))

CleanTarget($(KERNEL_TARGET) $(KERNEL_DLLTARGET))
CleanTarget($(CLIENT_TARGET) $(CLIENT_DLLTARGET))

#endif

/* no makefile.h */
MAKEFILE_H =

/*
  Copyright (c) 2001, 2023 Ricoh Company, Ltd.
  All rights reserved.
*/
