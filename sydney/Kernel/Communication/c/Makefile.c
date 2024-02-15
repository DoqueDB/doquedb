/*
 * Makefile.c --- Kernel/Communication/c
 * 
 * Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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

SUBDIRS =
CLIENTSUBDIRS =

/****************************************/
/* following variables MUST be defined  */

MODULE = Communication
TARGET_BASE = SyComm
SRCDIR = ..
HDRDIR = ../$(MODULE)
MESSAGE_DEFINITION =
MESSAGE_TARGET =

LOCAL_EXPORT_HDRDIR = ../../include/$(MODULE)
TOP_EXPORT_HDRDIR = ../../../include/$(MODULE)
TOP_INSTALL_DIR = ../../../c.CONF
PACKAGE_HDRDIR = $(TOP_INSTALL_DIR)/$(PACKAGE_NAME)/include/$(MODULE)

/* above variables MUST be defined      */
/****************************************/

/* headers created for message */
MESSAGE_HDRS =

/* headers installed in tape image */
PACKAGE_HDRS = \
	$(HDRDIR)/Module.h \
	$(HDRDIR)/AuthorizeMode.h \
	$(HDRDIR)/Crypt.h \
	$(HDRDIR)/CryptMode.h \
	$(HDRDIR)/Protocol.h \
	$(HDRDIR)/User.h \
	$(HDRDIR)/SerialIO.h \
	$(HDRDIR)/Socket.h \
	$(CRIPT_HDRS)

CRIPT_HDRS = \
	$(HDRDIR)/CryptCodec.h \
	$(HDRDIR)/NoCrypt.h

CRIPT_OBJS = \
	Crypt$O \
	CryptCodec$O \
	NoCrypt$O

/* headers installed in Kernel */
LOCAL_EXPORT_HDRS = \
	$(HDRDIR)/ClientConnection.h \
	$(HDRDIR)/Connection.h \
	$(HDRDIR)/ConnectionMasterID.h \
	$(HDRDIR)/ConnectionSlaveID.h \
	$(HDRDIR)/ConnectionSupplier.h \
	$(HDRDIR)/LocalClientConnection.h \
	$(HDRDIR)/Memory.h \
	$(HDRDIR)/RemoteClientConnection.h \
	$(HDRDIR)/ServerConnection.h \
	$(HDRDIR)/SocketDaemon.h

/* headers installed in TOP */
TOP_EXPORT_HDRS = \
	$(PACKAGE_HDRS) \
	$(MESSAGE_HDRS)

HDRS = \
	$(TOP_EXPORT_HDRS) \
	$(LOCAL_EXPORT_HDRS) \
	$(HDRDIR)/ConnectionKeeper.h \
	$(HDRDIR)/Local.h \
	$(HDRDIR)/LocalMemory.h \
	$(HDRDIR)/MemoryDaemon.h \
	$(HDRDIR)/ServerSocket.h

MESSAGE_OBJS =

CLIENT_OBJS = \
	$(MESSAGE_OBJS) \
	ClientConnection$O \
	Connection$O \
	ConnectionKeeper$O \
	ConnectionMasterID$O \
	ConnectionSlaveID$O \
	LocalClientConnection$O \
	LocalMemory$O \
	Memory$O \
	MemoryDaemon$O \
	RemoteClientConnection$O \
	SerialIO$O \
	Socket$O \
	$(CRIPT_OBJS)

OBJS = \
	$(CLIENT_OBJS) \
	Communication$O \
	ConnectionSupplier$O \
	ServerConnection$O \
	ServerSocket$O \
	SocketDaemon$O

#ifdef SYD_DLL
EXPORTFLAGS = \
	-DSYD_KERNEL_EXPORT_FUNCTION
#endif
EXTRACFLAGS = \
	$(EXPORTFLAGS)
EXTRALDFLAGS =
EXTRAPURIFYFLAGS =
EXTRAQUANTIFYFLAGS =
EXTRAPURECOVFLAGS =
EXTRALOCALCFLAGS =

/********************************************************/

TARGET = $(MODULE).ol
TARGET2 = $(MODULE)_C.ol

ALLTARGETS = \
	$(MESSAGE_TARGET) \
	$(TARGET)

/*
 * all
 */
AllTarget($(ALLTARGETS))
clientall:: all

ObjectListTarget($(TARGET), $(OBJS), $(TOP_INSTALL_DIR))
objlist:: clientobjlist
ObjectListTargetT(clientobjlist, $(TARGET2), $(CLIENT_OBJS), $(TOP_INSTALL_DIR))
	
/*
 * message
 */
/* MessageTarget($(MESSAGE_TARGET), $(MESSAGE_DEFINITION), $(BUILD_JAR)) */

/*
 * install library and header
 */
InstallHeaderTarget($(TOP_EXPORT_HDRS), $(TOP_EXPORT_HDRDIR))
InstallHeaderTarget($(LOCAL_EXPORT_HDRS), $(LOCAL_EXPORT_HDRDIR))

/*
 * install header for package
 */
TapeHeaderTarget($(PACKAGE_HDRS), $(PACKAGE_HDRDIR))

/*
 * clean
 */
CleanTarget($(ALLTARGETS))
CleanTarget($(OBJS))

#include "Makefile.h"

/*
    Copyright (c) 1999, 2023 Ricoh Company, Ltd.
    All rights reserved.
*/
