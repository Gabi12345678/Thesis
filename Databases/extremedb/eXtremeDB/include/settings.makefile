# 	Darwin:  Increase "number of processe per user" limit using "sudo sysctl -w kern.maxprocperuid=4096"
#                Make sure your system has JAVA_HOME variable pointing to /usr/libexec/java_home utility, add "export JAVA_HOME=$(/usr/libexec/java_home)"
#                MacOSX 10.13/XCode 10.1.10B61 (and later) requires to "export MACOSX_DEPLOYMENT_TARGET=10.9" to be able to linke Python/Lua included into eXtremeDB's source tree
#

# Build/Products root control
MCO_BUILD_ROOT  := $(MCO_ROOT)
MCO_PRODUCT_ROOT:= $(MCO_BUILD_ROOT)

# Build log control
#MCO_BUILD_LOG  :=on   # default
#MCO_BUILD_LOG  :=full
#MCO_BUILD_LOG  :=off
MCO_BUILD_LOG  :=$(if $(SHOW_BUILD_LOG),full,on)

$(if $(findstring on,$(MCO_BUILD_LOG)),$(info Analyzing build environment...))

# Extra checks control
MCO_EXTRA_CHECKS ?=
#MCO_EXTRA_CHECKS :=on

# Evaluation (limited) version of the runtime 
#MCO_ENABLE_EVAL_BINARIES:=yes
MCO_ENABLE_EVAL_BINARIES    ?=no

# Trace eXtremeDB libraries as dependencies of executables
MCO_COUNT_PREBUILT_AS_DEPENDENCIES ?= no

# Module production control
MCO_ENABLE_STATIC_BINARIES  :=STATIC
MCO_ENABLE_DYNAMIC_BINARIES :=DYNAMIC

# gcc/g++ overrides (works for gcc-family compilers, used in case if the build system has several gcc installed, ex. gcc/gcc253/gcc311/gcc456)
MCO_BUILD_GCC_OVERRIDE  ?= gcc
MCO_BUILD_GXX_OVERRIDE  ?= g++
MCO_HOST_GCC_OVERRIDE   ?= gcc
MCO_HOST_GXX_OVERRIDE   ?= g++
MCO_TARGET_GCC_OVERRIDE ?= gcc
MCO_TARGET_GXX_OVERRIDE ?= g++
MCO_CROSS_GCC_OVERRIDE  := $(MCO_BUILD_GCC_OVERRIDE)
MCO_CROSS_GXX_OVERRIDE  := $(MCO_BUILD_GXX_OVERRIDE)

# OpenSSL overrides
# MCO_FORCE_OPENSSL=auto       - (Default) try to use an OpenSSL installation pointed by MCO_OPENSSL_INCLUDE_PATH env. variable if it is set,
#                                otherwise try to use build systems OpenSSL headers if the built arch is the same as the target arch and the build sys. has necessary headers,
#                                finally use OpenSSL package from the source tree to build OpenSSL installation for the target system and use it to build eXtremeDB
# MCO_FORCE_OPENSSL=envpath    - Unconditionally forse to use $(MCO_OPENSSL_INCLUDE_PATH) to look for OpenSSL headers for the target
# MCO_FORCE_OPENSSL=buildsys   - Unconditionally force to use built system's OpenSSL installation
# MCO_FORCE_OPENSSL=sourcetree - Force to build the target-specific OpenSSL packgae from the source code archive located in target/sal/net
MCO_FORCE_OPENSSL?=auto
# MCO_FORCE_OPENSSL:=envpath
# MCO_FORCE_OPENSSL:=buildsys
# MCO_FORCE_OPENSSL:=sourcetree

# Python overrides
# MCO_FORCE_PYTHON=auto       - (Default) try to use a Python installation pointed by PYTHONBIN env. variable if it is set,
#                                otherwise try to find the Python by the path. Next check if the Python version is sufficient, it is runnable,
#                                the architecture matches and it has neccesary header file. Otherwise build own Python from the package in the source tree.
# MCO_FORCE_PYTHON=envpath    - Unconditionally force to use $(PYTHONBIN) to look for Python
# MCO_FORCE_PYTHON=buildsys   - Unconditionally force to use built system's Python installation
# MCO_FORCE_PYTHON=sourcetree - Force to build own Python packgae from the source code archive located in target/python

MCO_FORCE_PYTHON?=auto
# MCO_FORCE_PYTHON:=envpath
# MCO_FORCE_PYTHON:=buildsys
# MCO_FORCE_PYTHON:=sourcetree

# Should the REST service be added to the executables automatically?
MCO_FORCE_REST     :=
# MCO_FORCE_REST   :=YES

MCO_EXDB_RESTF_    :=
MCO_EXDB_RESTF_YES := MCO_CFG_REST_AUTOENABLE

# Backward Compatibility
#
# Data bus width override
# on/off/auto (default)
x64                  := on
#x64                 :=on
#x64                 :=off

# Features control (use on make's command line)
#
# MCO_SKIP_PERFMON  (yes/<not set>) - skip perfmon build
# MCO_SKIP_SEQ_LIB  (yes/<not set>) - disable sequences support
# MCO_SKIP_LUA_UDF  (yes/<not set>) - disable UDF for Lua support
# MCO_SKIP_XSQL_BIN (yes/<not set>) - skip xsql build
# JAVA              (off/<not set>) - skip Java-related code if set to off
# PYTHON            (off/<not set>) - skip Python-related code if set to off
# LUA               (off/<not set>) - skip Lua-related code if set to off
# FEEDHANDLER       (off/<not set>) - skip FeedHandler code if set to off

# Disable specific project groups by tags
#
# manually disable specific groups of projects (usually used from the command line)
MCO_DISABLE:=  feedhandler cluster ha
#MCO_DISABLE:=sequences
#MCO_DISABLE:=java python lua
#MCO_DISABLE:=net fsystem
#
# manually enable specific groups of projects (usually used from the command line)
MCO_ENABLE:= 
#MCO_ENABLE :=wchar
#MCO_ENABLE :=rtree
#MCO_ENABLE :=minicore # Minimization of eXtremeDB core. Enable this setting to decrease memory and code size confumption for the core. Most of the product's features will be disabled
#MCO_ENABLE :=largedb  # support for database >4G for 32-bit targets

# normally-disabled groups and old switches handling
MCO_SKIP_LUA_UDF_yes      :=luaudf
MCO_SKIP_PERFMON_yes      :=perfmon hvperf
MCO_SKIP_SEQ_LIB_yes      :=sequences
MCO_SKIP_XSQL_BIN_yes     :=xsql
JAVA_off                  :=java jni jdbc
PYTHON_off                :=python
LUA_off                   :=lua
FEEDHANDLER_off           :=feedhandler

MCO_ARCHIDS := arm64 arm ppc64le ppc64 ppcle ppc mips x86_64 x86 sparc64 ia64 microblaze m68k sh4 tilegx tile tricore

MCO_DISABLE_FEATURES_minicore:=ha cluster iot rest sql openssl zlib python java lua xsql sequences tlog luaudf perfmon feedhandler mvcc mcoloader odbc persistent ews httpview kdtree patriciatree patternsearch mvcc rtree events xml uda backup

MCO_DISABLE_AUTO:=obsolete licensing wchar minicore \
                  $(foreach archid,$(MCO_ARCHIDS),host-$(archid)-only) $(foreach archid,$(MCO_ARCHIDS),target-$(archid)-only) \
                  $(MCO_SKIP_PERFMON_$(MCO_SKIP_PERFMON)) $(MCO_SKIP_SEQ_LIB_$(MCO_SKIP_SEQ_LIB)) $(MCO_SKIP_XSQL_BIN_$(MCO_SKIP_XSQL_BIN)) \
                  $(JAVA_$(JAVA)) $(PYTHON_$(PYTHON)) $(LUA_$(LUA)) $(FEEDHANDLER_$(FEEDHANDLER)) $(MCO_DISABLE_FEATURES_$(filter minicore,$(MCO_ENABLE)))
COMMA:=,
MCO_DISABLED_TAGS:=$(filter-out $(subst $(COMMA), ,$(MCO_ENABLE)),$(subst $(COMMA), ,$(MCO_DISABLE_AUTO) $(MCO_DISABLE) $(MCO_CORE_MINIMIZATION_$(MCO_CORE_MINIMIZATION))))

# Enable/disable functionality implementation
MCO_EXDB_DISABLE_DEFINES_wchar      :=MCO_CFG_WCHAR_SUPPORT
MCO_EXDB_DISABLE_DEFINES_rtree      :=MCO_CFG_RTREE_SUPPORT
MCO_EXDB_DISABLE_DEFINES_sql        :=MCO_CFG_WRAPPER_SQL_SUPPORT
MCO_EXDB_DISABLE_DEFINES_sequences  :=MCO_CFG_WRAPPER_SEQUENCE_SUPPORT
MCO_EXDB_DISABLE_DEFINES_ha         :=MCO_CFG_WRAPPER_HA_SUPPORT
MCO_EXDB_DISABLE_DEFINES_cluster    :=MCO_CFG_WRAPPER_CLUSTER_SUPPORT
MCO_EXDB_DISABLE_DEFINES_tlog       :=MCO_CFG_WRAPPER_TL_SUPPORT
MCO_EXDB_DISABLE_DEFINES_luaudf     :=MCO_CFG_WRAPPER_LUAUDF_SUPPORT
MCO_EXDB_DISABLE_DEFINES_iot        :=MCO_CFG_WRAPPER_IOT_SUPPORT
MCO_EXDB_DISABLE_DEFINES_perfmon    :=MCO_CFG_WRAPPER_PERFMON_SUPPORT
MCO_EXDB_DISABLE_DEFINES_openssl    :=MCO_NET_USE_SSL=1
MCO_EXDB_DISABLE_DEFINES_zlib       :=MCO_NET_USE_ZLIB=1
MCO_EXDB_DISABLE_DEFINES_rest       :=MCO_CFG_REST_AUTOENABLE
MCO_EXDB_DISABLE_DEFINES_minicore   :=MCO_CFG_MINIMAL
MCO_EXDB_DISABLE_LIBRARIES_sql      :=mcosql mcosqlmini mcorsql
MCO_EXDB_DISABLE_LIBRARIES_sequences:=mcoseq mcoseqmath
MCO_EXDB_DISABLE_LIBRARIES_rest     :=mcorestdb mcorest mcorestperf mcorestsql mcoresttrace
MCO_EXDB_DISABLE_PRJFLAG_sql        :=F_SQL
MCO_EXDB_DISABLE_PRJFLAG_sequences  :=F_SEQUENCES
MCO_EXDB_DISABLE_PRJFLAG_ha         :=F_HA
MCO_EXDB_DISABLE_PRJFLAG_cluster    :=F_CLUSTER
MCO_EXDB_DISABLE_PRJFLAG_tlog       :=F_LOG
MCO_EXDB_DISABLE_PRJFLAG_iot        :=F_IOT
MCO_EXDB_DISABLE_PRJFLAG_rest       :=F_REST

# Explain make script decigions 
# MCO_EXPLAIN:=<tag>,<tag>
MCO_EXPLAIN_TAGS:=$(subst $(COMMA), ,$(MCO_EXPLAIN))

# note: UNICODE_SUPPORT is enabled explicitly because of MCO_CONFIG_OVERRIDE_WCHAR enabled
#       MCO_CONFIG_OVERRIDE_WCHAR - controls support of wchar/wstring datatype (ref. to wchar.h and wchar_t from the C runtime) in the core
#                                   controls support of wchar/wstring datatype in SQL
#       UNICODE_SUPPORT           - nchar/nstring datatype support for SQL (uses wchar_t and wchar.h from C runtime for implementation)
#       In case if the system does not have wchar_t support (no wchar.h) both of the switches need to be disabled (no wchar/wstring datatype, nchar/nstring enabled in the core)

# large database support
MCO_LARGE_DATABASE_SUPPORT_largedb  :=MCO_LARGE_DATABASE_SUPPORT
MCO_LARGE_DATABASE_SUPPORT_         :=

# AWS support
MCO_AWS_DEF_yes                     :=MCO_LIC_AWS
MCO_AWS_DEF_                        :=
MCO_AWS_GCC_PLATFORM_yes            :=-xc++ -lssl -lcrypto -lcurl
MCO_AWS_GCC_PLATFORM_               :=

MCO_EXDB_COMMONF :=MCO_CFG_WRAPPER_HA_SUPPORT MCO_CFG_WRAPPER_TL_SUPPORT MCO_CFG_WRAPPER_SEQUENCE_SUPPORT MCO_CFG_WRAPPER_SQL_SUPPORT MCO_CFG_WRAPPER_CLUSTER_SUPPORT \
                   MCO_CFG_WRAPPER_PERFMON_SUPPORT \
                   MCO_CFG_WRAPPER_LUAUDF_SUPPORT MCO_CFG_WRAPPER_IOT_SUPPORT MCO_CONFIG_OVERRIDE_WCHAR MCO_CFG_WCHAR_SUPPORT UNICODE_SUPPORT \
                   MCO_CONFIG_OVERRIDE_RTREE MCO_CFG_RTREE_SUPPORT MCO_NET_USE_SSL=1 \
                   MCO_NET_USE_ZLIB=1 MCO_CONFIG_OVERRIDE_CHECKLEVEL MCO_CFG_MINIMAL _GNU_SOURCE _SMP $(MCO_EXDB_RESTF_$(MCO_FORCE_REST)) \
                   $(MCO_LARGE_DATABASE_SUPPORT_$(findstring largedb,$(MCO_ENABLE))) \
                   $(MCO_AWS_DEF_$(MCO_LIC_AWS))

MCO_EXDB_OFFSF   :=
MCO_EXDB_DPTRF   :=MCO_CFG_USE_DIRECT_POINTERS
MCO_EXDB_WDBGF   :=MCO_CFG_CHECKLEVEL_1 MCO_CFG_CHECKLEVEL_2 MCO_CFG_CHECKLEVEL_3 MCO_CFG_CHECKLEVEL_PIN MCO_CFG_ODBC_TRACE MCO_DEBUG_DISK_CACHE MCO_FH_DEBUG
MCO_EXDB_NDBGF   :=
MCO_EXDB_WEVLF   :=MCO_CFG_TRLIMIT=10000000
MCO_EXDB_NEVLF   :=
MCO_EXDB_X32     :=
MCO_EXDB_X64     :=MCO_PLATFORM_X64 

MCO_SAMPLES_COMMON_DIR =$(abspath $(MCO_ROOT)/samples/native/common)
MCO_PUBLIC_INCLUDE_DIR =$(abspath $(MCO_ROOT)/include)
MCO_ALIAS_HOST   :=host
MCO_ALIAS_TARGET :=target

MCO_LUA_SRC_DIR         :=$(MCO_ROOT)/target/lua
MCO_LUA_SRC_PKG_DIR     :=LuaJIT-2.1.0-beta3

MCO_OPENSSL_SRC_DIR     :=$(MCO_ROOT)/target/sal/net
MCO_OPENSSL_SRC_PKG_DIR :=openssl-1.1.1c
MCO_OPENSSL_SRC_PKG_FILE:=openssl-1.1.1c.tar.gz
MCO_OPENSSL_DLOAD       :=wget https://www.openssl.org/source/$(MCO_OPENSSL_SRC_PKG_FILE)

MCO_PYTHON_SRC_DIR      :=$(MCO_ROOT)/target/python
MCO_PYTHON_SRC_PKG_DIR  :=Python-2.7.14
MCO_PYTHON_SRC_PKG_FILE :=Python-2.7.14.tgz
MCO_PYTHON_DLOAD        :=wget https://www.python.org/ftp/python/2.7.14/Python-2.7.14.tgz

# External libraries required to build eXtremeDB-enabled executables

# Default eXtremeDB platform-dependent drivers (for os-id or any for default)
MCO_EXDB_DEFAULT_MEMDEV_PRIVATE_any:=mcomconv
MCO_EXDB_DEFAULT_MEMDEV_SHARED_any :=mcomipc
MCO_EXDB_DEFAULT_STRDEV_any        :=mcofu98zip
MCO_EXDB_DEFAULT_SYNC_PRIVATE_any  :=mcoslnxp
MCO_EXDB_DEFAULT_SYNC_SHARED_any   :=mcoslnxp

MCO_EXDB_DEFAULT_MEMDEV_SHARED_Linux   :=
MCO_EXDB_DEFAULT_SYNC_PRIVATE_Linux    :=
MCO_EXDB_DEFAULT_SYNC_SHARED_Linux     :=
MCO_EXDB_DEFAULT_STRDEV_Linux          :=

MCO_EXDB_DEFAULT_MEMDEV_SHARED_autosar :=
MCO_EXDB_DEFAULT_SYNC_PRIVATE_autosar  :=mcosalhollow
MCO_EXDB_DEFAULT_SYNC_SHARED_autosar   :=
MCO_EXDB_DEFAULT_STRDEV_autosar        :=

MCO_EXDB_DEFAULT_SYNC_PRIVATE_SunOS:=mcossun
MCO_EXDB_DEFAULT_SYNC_SHARED_SunOS :=mcossun
MCO_EXDB_DEFAULT_SYNC_PRIVATE_HP-UX:=mcoshpux
MCO_EXDB_DEFAULT_SYNC_SHARED_HP-UX :=mcoshpux
MCO_EXDB_DEFAULT_SYNC_PRIVATE_AIX  :=mcosaixp
MCO_EXDB_DEFAULT_SYNC_SHARED_AIX   :=mcosaixp
MCO_EXDB_DEFAULT_SYNC_PRIVATE_QNX  :=mcosqnxp
MCO_EXDB_DEFAULT_SYNC_SHARED_QNX   :=mcosqnxp

MCO_EXDB_BUILD_MEMDEV_any          := conv alloc
MCO_EXDB_BUILD_MEMDEV_Linux        := ipc psx
MCO_EXDB_BUILD_MEMDEV_Darwin       := ipc psx
MCO_EXDB_BUILD_MEMDEV_uclinux      := ipc psx
MCO_EXDB_BUILD_MEMDEV_windows      := w32
MCO_EXDB_BUILD_MEMDEV_SunOS        := ipc
MCO_EXDB_BUILD_MEMDEV_QNX          := psx
MCO_EXDB_BUILD_MEMDEV_HP-UX        := ipc
MCO_EXDB_BUILD_MEMDEV_AIX          := ipc
MCO_EXDB_BUILD_MEMDEV_ecos         :=
MCO_EXDB_BUILD_MEMDEV_ucos         :=
MCO_EXDB_BUILD_MEMDEV_netos        := psx

MCO_EXDB_BUILD_SYNC_any            :=
MCO_EXDB_BUILD_SYNC_lynxos         := lynx lynxp
MCO_EXDB_BUILD_SYNC_Darwin         := lnx lnxp lnxs5
MCO_EXDB_BUILD_SYNC_Linux          := lnx lnxp lnxs5
MCO_EXDB_BUILD_SYNC_windows        := w32 w32n
MCO_EXDB_BUILD_SYNC_SunOS          := sun
MCO_EXDB_BUILD_SYNC_QNX            := qnxp
MCO_EXDB_BUILD_SYNC_HP-UX          := hpuxp
MCO_EXDB_BUILD_SYNC_AIX            := aixp
MCO_EXDB_BUILD_SYNC_ecos           := ecos
MCO_EXDB_BUILD_SYNC_netos          := ntos
MCO_EXDB_BUILD_SYNC_ucos           := ucs3

MCO_EXDB_BUILD_NWPIP_any           :=
MCO_EXDB_BUILD_NWPIP_Linux         :=psx
MCO_EXDB_BUILD_NWPIP_Darwin        :=psx
MCO_EXDB_BUILD_NWPIP_uclinux       :=psx
MCO_EXDB_BUILD_NWPIP_windows       :=w32
MCO_EXDB_BUILD_NWPIP_SunOS         :=psx
MCO_EXDB_BUILD_NWPIP_QNX           :=psx
MCO_EXDB_BUILD_NWPIP_HP-UX         :=psx
MCO_EXDB_BUILD_NWPIP_AIX           :=psx
MCO_EXDB_BUILD_NWPIP_ecos          :=
MCO_EXDB_BUILD_NWPIP_ucos          :=
MCO_EXDB_BUILD_NWPIP_netos         :=

MCO_EXDB_BUILD_NWTCP_any           :=
MCO_EXDB_BUILD_NWTCP_Linux         :=
MCO_EXDB_BUILD_NWTCP_Darwin        :=
MCO_EXDB_BUILD_NWTCP_uclinux       :=
MCO_EXDB_BUILD_NWTCP_windows       :=
MCO_EXDB_BUILD_NWTCP_SunOS         :=
MCO_EXDB_BUILD_NWTCP_QNX           :=
MCO_EXDB_BUILD_NWTCP_HP-UX         :=
MCO_EXDB_BUILD_NWTCP_AIX           :=
MCO_EXDB_BUILD_NWTCP_ecos          :=
MCO_EXDB_BUILD_NWTCP_ucos          :=
MCO_EXDB_BUILD_NWTCP_netos         :=

MCO_EXDB_BUILD_NWUDP_any           :=
MCO_EXDB_BUILD_NWUDP_Linux         :=
MCO_EXDB_BUILD_NWUDP_Darwin        :=
MCO_EXDB_BUILD_NWUDP_uclinux       :=
MCO_EXDB_BUILD_NWUDP_windows       :=
MCO_EXDB_BUILD_NWUDP_SunOS         :=
MCO_EXDB_BUILD_NWUDP_QNX           :=
MCO_EXDB_BUILD_NWUDP_HP-UX         :=
MCO_EXDB_BUILD_NWUDP_AIX           :=
MCO_EXDB_BUILD_NWUDP_ecos          :=
MCO_EXDB_BUILD_NWUDP_ucos          :=
MCO_EXDB_BUILD_NWUDP_netos         :=

MCO_EXDB_BUILD_NWQNXM_any          :=
MCO_EXDB_BUILD_NWQNXM_QNX          :=qnxm

MCO_EXDB_BUILD_FSYS_any            :=
MCO_EXDB_BUILD_FSYS_Linux          :=u98 uni u98zip u98ziplog u98test
MCO_EXDB_BUILD_FSYS_Darwin         :=u98 uni u98zip u98ziplog
MCO_EXDB_BUILD_FSYS_uclinux        :=u98
MCO_EXDB_BUILD_FSYS_windows        :=w32
MCO_EXDB_BUILD_FSYS_SunOS          :=u98 u98zip uni
MCO_EXDB_BUILD_FSYS_QNX            :=u98 u98zip uni
MCO_EXDB_BUILD_FSYS_HP-UX          :=u98 u98zip uni
MCO_EXDB_BUILD_FSYS_AIX            :=u98 u98zip uni
MCO_EXDB_BUILD_FSYS_ecos           :=ecos
MCO_EXDB_BUILD_FSYS_ucos           :=
MCO_EXDB_BUILD_FSYS_netos          :=
MCO_EXDB_BUILD_FSYS_lynxos         :=uni

# 1. Identificate the build system, ex. Linux, Darwin, CYGWIN_NT, MINGW32_NT, QNX, WIN32, SunOS, HP-UX, AIX 
#    Note: default to Linux x86_64
MCO_BUILD_OSID    :=$(or $(shell uname -s),$(error No OSID was reported by uname -s))
MCO_BUILD_ARCH_PART :=$(or $(shell uname -m),$(error No ARCHID was reported by uname -m))
MCO_BUILD_ARCHID  := $(or $(BUILD_ARCH), \
                      ,$(if $(or $(findstring aarch64,$(MCO_BUILD_ARCH_PART)),$(findstring arm64,$(MCO_BUILD_ARCH_PART))),arm64) \
                      ,$(if $(or $(findstring arm,$(MCO_BUILD_ARCH_PART)),$(findstring sa110,$(MCO_BUILD_ARCH_PART)),$(findstring ntoarm,$(MCO_BUILD_ARCH_PART))),arm) \
                      ,$(if $(or $(findstring ppc64le,$(MCO_BUILD_ARCH_PART)),$(findstring powerpc64le,$(MCO_BUILD_ARCH_PART))),ppc64le) \
                      ,$(if $(or $(findstring ppc64,$(MCO_BUILD_ARCH_PART)),$(findstring powerpc64,$(MCO_BUILD_ARCH_PART))),ppc64) \
                      ,$(if $(or $(findstring ppcle,$(MCO_BUILD_ARCH_PART)),$(findstring powerpcle,$(MCO_BUILD_ARCH_PART))),ppcle) \
                      ,$(if $(or $(findstring ntoppc,$(MCO_BUILD_ARCH_PART)),$(findstring ppc,$(MCO_BUILD_ARCH_PART)),$(findstring powerpc,$(MCO_BUILD_ARCH_PART))),ppc) \
                      ,$(if $(or $(findstring mips,$(MCO_BUILD_ARCH_PART)),$(findstring ntomips,$(MCO_BUILD_ARCH_PART))),mips) \
                      ,$(if $(findstring x86_64,$(MCO_BUILD_ARCH_PART)),x86_64) \
                      ,$(if $(or $(findstring pentium,$(MCO_BUILD_ARCH_PART)),$(findstring 86,$(MCO_BUILD_ARCH_PART))),x86) \
                      ,$(if $(findstring sun4u,$(MCO_BUILD_ARCH_PART)),sparc64) \
                      ,$(if $(findstring ia64,$(MCO_BUILD_ARCH_PART)),ia64) \
                      ,$(if $(findstring microblaze,$(MCO_BUILD_ARCH_PART)),microblaze) \
                      ,$(if $(or $(findstring m68k,$(MCO_BUILD_ARCH_PART)),$(findstring m68000,$(MCO_BUILD_ARCH_PART))),m68k) \
                      ,$(if $(findstring sh,$(MCO_BUILD_ARCH_PART)),sh4) \
                      ,$(if $(findstring tilegx,$(MCO_BUILD_ARCH_PART)),tilegx) \
                      ,$(if $(findstring tile,$(MCO_BUILD_ARCH_PART)),tile) \
                      ,$(if $(findstring tricore,$(MCO_BUILD_ARCH_PART)),tricore) \
                      ,$(error Failed to autodetect BUILD CPU architecture. Use BUILD_ARCH to override or set manually))
#MCO_XARGS_TEST    := $(shell which xargs 2>/dev/null || echo none)
MCO_XARGS_TEST    := none

#    Adjust build osid if necessary 
MCO_BUILD_OSID    :=$(if $(findstring CYGWIN_NT,$(MCO_BUILD_OSID)),CYGWIN,$(MCO_BUILD_OSID))
MCO_BUILD_OSID    :=$(if $(findstring MINGW32_NT,$(MCO_BUILD_OSID)),MINGW,$(MCO_BUILD_OSID))
MCO_CROSS_OSID    :=$(MCO_BUILD_OSID)
MCO_BUILD_ARCH_SFX:=$(MCO_BUILD_OSID)-$(MCO_BUILD_ARCHID)

# Build's Python automation
# MCO_FORCE_PYTHON=auto       - (Default) try to use a Python installation pointed by PYTHONBIN env. variable if it is set,
#                                otherwise try to find the Python by the path. Next check if the Python version is sufficient, it is runnable,
#                                the architecture matches and it has neccesary header file. Otherwise build own Python from the package in the source tree.
# MCO_FORCE_PYTHON=envpath    - Unconditionally force to use $(PYTHONBIN) to look for Python
# MCO_FORCE_PYTHON=buildsys   - Unconditionally force to use built system's Python installation
# MCO_FORCE_PYTHON=sourcetree - Force to build own Python packgae from the source code archive located in target/python
MCO_BUILD_PYTHON_AUTOMATION := $(or \
$(if $(findstring auto,      $(MCO_FORCE_PYTHON)),$(or $(PYTHONBIN),$(shell which python),$(MCO_PYTHON_SRC_DIR)/$(MCO_PYTHON_SRC_PKG_DIR)-$(MCO_BUILD_ARCH_SFX)/bin/python2.7)),\
$(if $(findstring envpath,   $(MCO_FORCE_PYTHON)),$(PYTHONBIN)),\
$(if $(findstring buildsys,  $(MCO_FORCE_PYTHON)),$(shell which python)),\
$(if $(findstring sourcetree,$(MCO_FORCE_PYTHON)),$(MCO_PYTHON_SRC_DIR)/$(MCO_PYTHON_SRC_PKG_DIR)-$(MCO_BUILD_ARCH_SFX)/bin/python2.7),\
$(if $(PYTHONBIN),,$(error PYTHONBIN evn. variable enforced but has no value)),\
$(error An invalid value "$(MCO_FORCE_PYTHON)" of MCO_FORCE_PYTHON switch is given. Valid values are auto/envpath/buildsys/sourcetree))

#    Choose build-os utilities
MCO_BUILD_MCOCOMP_MINGW  :=$(MCO_PRODUCT_ROOT)\\$(if $(HOST_PLATFORM),build,host)\\bin\\mcocomp.exe
MCO_BUILD_MCOCOMP_windows:=$(MCO_PRODUCT_ROOT)\\$(if $(HOST_PLATFORM),build,host)\\bin\\mcocomp.exe
MCO_BUILD_MCOCOMP_CYGWIN :=$(MCO_PRODUCT_ROOT)/$(if $(HOST_PLATFORM),build,host)/bin/mcocomp.exe
MCO_BUILD_TONULL_windows :=>nul
MCO_BUILD_MAKE_AIX       :=gmake
MCO_BUILD_UNTAR_AIX      :=gtar --directory @output-dir@ -xaf @input@
MCO_BUILD_UNTAR_Darwin   :=tar -C @output-dir@ -xzf @input@

# default utilities set defined for *nixes, use MCO_BUILD_<tool>_<OSID> to override above this line
# note: if <build-platform>!=<host-platform> then use MCO_PRODUCT_ROOT/build/bin/mcocomp
MCO_BUILD_RMFILE         :=$(if $(findstring undefined, $(origin MCO_BUILD_RMFILE_$(MCO_BUILD_OSID))),rm -f,$(MCO_BUILD_RMFILE_$(MCO_BUILD_OSID)))
MCO_BUILD_RMDIR          :=$(if $(findstring undefined, $(origin MCO_BUILD_RMDIR_$(MCO_BUILD_OSID))) ,rm -rf,$(MCO_BUILD_RMDIR_$(MCO_BUILD_OSID)))
MCO_BUILD_MKDIR          :=$(if $(findstring undefined, $(origin MCO_BUILD_MKDIR_$(MCO_BUILD_OSID))) ,mkdir -p,$(MCO_BUILD_MKDIR_$(MCO_BUILD_OSID)))
MCO_BUILD_MAKE           :=$(if $(findstring undefined, $(origin MCO_BUILD_MAKE_$(MCO_BUILD_OSID)))  ,$(MAKE),$(MCO_BUILD_MAKE_$(MCO_BUILD_OSID)))
MCO_BUILD_ECHO           :=$(if $(findstring undefined, $(origin MCO_BUILD_ECHO_$(MCO_BUILD_OSID)))  ,echo,$(MCO_BUILD_ECHO_$(MCO_BUILD_OSID)))
MCO_BUILD_CAT            :=$(if $(findstring undefined, $(origin MCO_BUILD_CAT_$(MCO_BUILD_OSID)))   ,cat,$(MCO_BUILD_CAT_$(MCO_BUILD_OSID)))
MCO_BUILD_RENAME         :=$(if $(findstring undefined, $(origin MCO_BUILD_RENAME_$(MCO_BUILD_OSID))),mv,$(MCO_BUILD_RENAME_$(MCO_BUILD_OSID)))
MCO_BUILD_COPY           :=$(if $(findstring undefined, $(origin MCO_BUILD_COPY_$(MCO_BUILD_OSID)))  ,cp -f,$(MCO_BUILD_COPY_$(MCO_BUILD_OSID)))
MCO_BUILD_CPDIR          :=$(if $(findstring undefined, $(origin MCO_BUILD_CPDIR_$(MCO_BUILD_OSID))) ,cp -f -R,$(MCO_BUILD_CPDIR_$(MCO_BUILD_OSID)))
MCO_BUILD_MAKE_WRITABLE  :=$(if $(findstring undefined, $(origin MCO_BUILD_MAKE_WRITABLE_$(MCO_BUILD_OSID))) ,chmod -R u+w,$(MCO_BUILD_MAKE_WRITABLE_$(MCO_BUILD_OSID)))
MCO_BUILD_SEP            :=$(if $(findstring undefined, $(origin MCO_BUILD_SEP_$(MCO_BUILD_OSID)))   ,/,$(MCO_BUILD_SEP_$(MCO_BUILD_OSID)))
MCO_BUILD_TONULL         :=$(if $(findstring undefined, $(origin MCO_BUILD_TONULL_$(MCO_BUILD_OSID))),>/dev/null,$(MCO_BUILD_TONULL_$(MCO_BUILD_OSID)))
MCO_BUILD_INPARALLEL     :=$(if $(findstring undefined, $(origin MCO_BUILD_INPARALLEL_$(MCO_BUILD_OSID))), &,$(MCO_BUILD_INPARALLEL_$(MCO_BUILD_OSID)))
MCO_BUILD_MCOCOMP        :=$(if $(findstring undefined, $(origin MCO_BUILD_MCOCOMP_$(MCO_BUILD_OSID))),$(MCO_PRODUCT_ROOT)/$(if $(HOST_PLATFORM),build,host)/bin/mcocomp,$(MCO_BUILD_MCOCOMP_$(MCO_BUILD_OSID)))
MCO_BUILD_PYTHON         :=$(if $(findstring undefined, $(origin MCO_BUILD_PYTHON_$(MCO_BUILD_OSID))),$(MCO_BUILD_PYTHON_AUTOMATION),$(MCO_BUILD_PYTHON_$(MCO_BUILD_OSID)))
MCO_BUILD_MAKEARR_PY     :=$(if $(findstring undefined, $(origin MCO_BUILD_MAKEARR_PY_$(MCO_BUILD_OSID))),$(MCO_ROOT)/host/bin/make_arr.py,$(MCO_BUILD_MAKEARR_PY_$(MCO_BUILD_OSID)))
MCO_BUILD_JAVAC          :=$(if $(findstring undefined, $(origin MCO_BUILD_JAVAC_$(MCO_BUILD_OSID))),javac -g -classpath @classpath@ -d @output-dir@ -sourcepath @input-dir@ @input-files@,$(MCO_BUILD_JAVAC_$(MCO_BUILD_OSID)))
#MCO_BUILD_JAVAC         :=gjc -classpath @classpath@ -d @output@ -c -g -O @input@ 
MCO_BUILD_JAR            :=$(if $(findstring undefined, $(origin MCO_BUILD_JAR_$(MCO_BUILD_OSID))),jar cf @output@ -C @input@ .,$(MCO_BUILD_JAR_$(MCO_BUILD_OSID)))
MCO_BUILD_UNTAR          :=$(if $(findstring undefined, $(origin MCO_BUILD_UNTAR_$(MCO_BUILD_OSID))),tar --directory @output-dir@ -xaf @input@,$(MCO_BUILD_UNTAR_$(MCO_BUILD_OSID)))

# 2. Identificate the build-os default compiler family
MCO_OS_CFAM_Linux  :=gcc
MCO_OS_CFAM_windows:=wincc
MCO_OS_CFAM_CYGWIN :=gcc
MCO_OS_CFAM_QNX    :=qcc
MCO_OS_CFAM_SunOS  :=gcc
MCO_OS_CFAM_HP-UX  :=hpuxcc
MCO_OS_CFAM_Darwin :=clang
MCO_OS_CFAM_AIX    :=xlc
MCO_BUILD_CFAM     :=$(MCO_OS_CFAM_$(MCO_BUILD_OSID))
MCO_CROSS_CFAM     :=$(MCO_BUILD_CFAM)

# os-dependent libraries prefixes and suffixes
MCO_OS_LIB_STAT_PFX_any     :=lib
MCO_OS_LIB_DYNM_PFX_any     :=lib
MCO_OS_EXEC_PFX_any         :=
MCO_OS_LIB_STAT_SFX_any     :=.a
MCO_OS_LIB_DYNM_SFX_any     :=.so
MCO_OS_EXEC_SFX_any         :=
MCO_OS_LIB_JNI_SFX_any      :=.so
MCO_OS_LIB_LUA_SFX_any      :=.so

MCO_OS_LIB_JNI_SFX_Darwin   :=.jnilib
#MCO_OS_LIB_LUA_SFX_Darwin   :=.dylib

MCO_OS_LIB_STAT_PFX_windows :=
MCO_OS_LIB_DYNM_PFX_windows :=
MCO_OS_LIB_STAT_SFX_windows :=.lib
MCO_OS_LIB_DYNM_SFX_windows :=.dll
MCO_OS_EXEC_PFX_windows     :=.exe
MCO_OS_LIB_JNI_SFX_windows  :=.dll
MCO_OS_LIB_LUA_SFX_windows  :=.dll

# 3. Define host/target compiler family basing on HOST/TARGET_COMPILER_FAMILY and HOST/TARGET_OS
#    Default to build-os if not set
MCO_HOST_CFAM               :=$(or $(HOST_COMPILER_FAMILY),$(MCO_OS_CFAM_$(HOST_OS)),$(MCO_BUILD_CFAM))
MCO_TARGET_CFAM             :=$(or $(TARGET_COMPILER_FAMILY),$(MCO_OS_CFAM_$(TARGET_OS)),$(MCO_BUILD_CFAM))

# 4. Define host/target compilers
#    Common rules for all the compiler familys
MCO_CFAM_F_DEFINE_any       :=-D
MCO_CFAM_F_LIBRARY_any      :=-l
MCO_CFAM_F_PATHINC_any      :=-I
MCO_CFAM_F_PATHLIB_any      :=-L
MCO_CFAM_F_WDBG_any         :=-O0 -g
MCO_CFAM_F_NDBG_any         :=-O2

#    gcc/g++ family
MCO_CFAM_COMP_C_gcc         :=@platform@@gcc@ @debug_flags@ @arch_flags@ @defines@ -c @source_file@ -o @object_file@
MCO_CFAM_EXT_COMP_C_gcc     :=@platform@@gcc@ @arch_flags@ 
MCO_CFAM_COMP_CPP_gcc       :=@platform@@g++@ @debug_flags@ @arch_flags@ @defines@ -c @source_file@ -o @object_file@
MCO_CFAM_EXT_COMP_CPP_gcc   :=@platform@@g++@ @arch_flags@
MCO_CFAM_LINK_C_gcc         :=@platform@@gcc@ @debug_flags@ @link_flags_c@ @link_dynamic@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LINK_CPP_gcc       :=@platform@@g++@ @debug_flags@ @link_flags_cpp@ @link_dynamic@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LIBR_STAT_C_gcc    :=@platform@ar rc @output-file@ @object_files@ 
MCO_CFAM_LIBR_STAT_CPP_gcc  :=@platform@ar rc @output-file@ @object_files@
MCO_CFAM_EXT_LIBR_STAT_gcc  :=@platform@ar
MCO_CFAM_EXT_LIBR_DYNM_gcc  :=@platform@@gcc@ -shared @link_flags_cpp@
MCO_CFAM_LIBR_DYNM_C_gcc    :=@platform@@gcc@ -shared -rdynamic @debug_flags@ @link_flags_c@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LIBR_DYNM_CPP_gcc  :=@platform@@g++@ -shared -rdynamic @debug_flags@ @link_flags_cpp@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_PLATFORM_gcc        =$(if $($(1)_PLATFORM),$($(1)_PLATFORM)-)
#
# Arch switch table (x64=on/off/auto override implementation)
# MCO_ARCH_SWITCH_<arch>_<x64-value>_gcc:=<arch>
MCO_ARCH_SWITCH_x86_on_gcc     :=x86_64
MCO_ARCH_SWITCH_arm_on_gcc     :=arm64
MCO_ARCH_SWITCH_ppc_on_gcc     :=ppc64
MCO_ARCH_SWITCH_ppcle_on_gcc   :=ppc64le
MCO_ARCH_SWITCH_mips_on_gcc    :=mips64
MCO_ARCH_SWITCH_sparc_on_gcc   :=sparc64
MCO_ARCH_SWITCH_tricore_on_gcc :=tricore

MCO_ARCH_SWITCH_x86_64_off_gcc :=x86
MCO_ARCH_SWITCH_arm64_off_gcc  :=arm
MCO_ARCH_SWITCH_ppc64_off_gcc  :=ppc
MCO_ARCH_SWITCH_ppc64le_off_gcc:=ppcle
MCO_ARCH_SWITCH_mips64_off_gcc :=mips
MCO_ARCH_SWITCH_sparc64_off_gcc:=sparc
MCO_ARCH_SWITCH_tricore_off_gcc:=tricore

MCO_ARCHID_SWITCH_gcc=$(if $(subst auto,,$(x64)),$(or $(MCO_ARCH_SWITCH_$(2)_$(x64)_gcc),$(2)),$(2))

# MCO_ARCH_DBUS_WIDTH_<arch>_gcc:=32/64
MCO_ARCH_DBUS_WIDTH_x86_gcc       :=32
MCO_ARCH_DBUS_WIDTH_arm_gcc       :=32
MCO_ARCH_DBUS_WIDTH_ppc_gcc       :=32
MCO_ARCH_DBUS_WIDTH_ppcle_gcc     :=32
MCO_ARCH_DBUS_WIDTH_mips_gcc      :=32
MCO_ARCH_DBUS_WIDTH_sh4_gcc       :=32
MCO_ARCH_DBUS_WIDTH_sparc_gcc     :=32
MCO_ARCH_DBUS_WIDTH_microblaze_gcc:=32
MCO_ARCH_DBUS_WIDTH_tile_gcc      :=32
MCO_ARCH_DBUS_WIDTH_tilegx_gcc    :=32
MCO_ARCH_DBUS_WIDTH_m68k_gcc      :=16
MCO_ARCH_DBUS_WIDTH_x86_64_gcc    :=64
MCO_ARCH_DBUS_WIDTH_arm64_gcc     :=64
MCO_ARCH_DBUS_WIDTH_ppc64_gcc     :=64
MCO_ARCH_DBUS_WIDTH_ppc64le_gcc   :=64
MCO_ARCH_DBUS_WIDTH_mips64_gcc    :=64
MCO_ARCH_DBUS_WIDTH_sparc64_gcc   :=64
MCO_ARCH_DBUS_WIDTH_ia64_gcc      :=64
MCO_ARCH_DBUS_WIDTH_tricore_gcc   :=32

define MCO_CFAM_PROPS_get_gcc
# param 1 - HOST/TARGET
MCO_$(1)_DUMPMACHINE :=$(shell $(call MCO_CFAM_PLATFORM_gcc,$(1))$(MCO_$(1)_GCC_OVERRIDE) -dumpmachine)
MCO_$(1)_ARCH_PART   :=$(firstword $(subst -, ,$$(MCO_$(1)_DUMPMACHINE)))
endef 

define MCO_CFAM_PROPS_analyze_pass1_gcc
# param 1 - HOST/TARGET
MCO_$(1)_ARCHID      :=$(strip $(or $($(1)_ARCH) \
                      ,$(if $(findstring aarch64,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),arm64)) \
                      ,$(if $(or $(findstring arm,$(MCO_$(1)_ARCH_PART)),$(findstring sa110,$(MCO_$(1)_ARCH_PART))),$(call MCO_ARCHID_SWITCH_gcc,$(1),arm)) \
                      ,$(if $(or $(findstring ppc64le,$(MCO_$(1)_ARCH_PART)),$(findstring powerpc64le,$(MCO_$(1)_ARCH_PART))),$(call MCO_ARCHID_SWITCH_gcc,$(1),ppc64le)) \
                      ,$(if $(or $(findstring ppc64,$(MCO_$(1)_ARCH_PART)),$(findstring powerpc64,$(MCO_$(1)_ARCH_PART))),$(call MCO_ARCHID_SWITCH_gcc,$(1),ppc64)) \
                      ,$(if $(or $(findstring ppcle,$(MCO_$(1)_ARCH_PART)),$(findstring powerpcle,$(MCO_$(1)_ARCH_PART))),$(call MCO_ARCHID_SWITCH_gcc,$(1),ppcle)) \
                      ,$(if $(or $(findstring ppc,$(MCO_$(1)_ARCH_PART)),$(findstring powerpc,$(MCO_$(1)_ARCH_PART))),$(call MCO_ARCHID_SWITCH_gcc,$(1),ppc)) \
                      ,$(if $(findstring mips,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),mips)) \
                      ,$(if $(findstring x86_64,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),x86_64)) \
                      ,$(if $(findstring 86,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),x86)) \
                      ,$(if $(findstring pentium,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),x86)) \
                      ,$(if $(findstring sun4u,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),sparc64)) \
                      ,$(if $(findstring ia64,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),ia64)) \
                      ,$(if $(findstring microblaze,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),microblaze)) \
                      ,$(if $(or $(findstring m68k,$(MCO_$(1)_ARCH_PART)),$(findstring m68000,$(MCO_$(1)_ARCH_PART))),$(call MCO_ARCHID_SWITCH_gcc,$(1),m68k)) \
                      ,$(if $(findstring sh,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),sh4)) \
                      ,$(if $(findstring tilegx,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),tilegx)) \
                      ,$(if $(findstring tile,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),tile)) \
                      ,$(if $(findstring tricore,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),tricore)) \
                      ,x86 $(warning Failed to autodetect $(1) CPU architecture, default to x86. Use $(1)_ARCH to override or set manually)))

MCO_$(1)_OSID        :=$(strip $(or $($(1)_OS), $($(1)_OS_HINT) \
                      ,$(if $(findstring Linux,     $(MCO_BUILD_OSID)), \
                          $(or $(if $(findstring -uclinux,$(MCO_$(1)_DUMPMACHINE)),uclinux),$(if $(findstring -linux,$(MCO_$(1)_DUMPMACHINE)),Linux),$(MCO_BUILD_OSID))) \
                      ,$(if $(findstring CYGWIN,    $(MCO_BUILD_OSID)),$(or $(if $(findstring -uclinux,$(MCO_$(1)_DUMPMACHINE)),uclinux),$(if $(findstring tricore,$(MCO_$(1)_DUMPMACHINE)),autosar),CYGWIN)) \
                      ,$(if $(findstring MINGW32,   $(MCO_BUILD_OSID)),windows) \
                      ,$(if $(or $(findstring ucLinux,$($(1)_OS) $($(1)_OS_HINT)),$(findstring uCLinux,$($(1)_OS) $($(1)_OS_HINT))),uclinux) \
                      ,$(if $(findstring LynxOS,$($(1)_OS) $($(1)_OS_HINT)),lynxos) \
                      ,$(if $(findstring eCos,$($(1)_OS) $($(1)_OS_HINT)),ecos) \
                      ,$(if $(findstring NETOS,$($(1)_OS) $($(1)_OS_HINT)),netos) \
                      ,$(MCO_BUILD_OSID)$(warning No special setting for $(1) OS was detected. Default to $(MCO_BUILD_OSID), use $(1)_OS to override)))

endef 

define MCO_CFAM_PROPS_analyze_pass2_gcc
# param 1 - HOST/TARGET
MCO_$(1)_DBUS_WIDTH  :=$(strip $(or $($(1)_DBUS_WIDTH) \
                      ,$(MCO_ARCH_DBUS_WIDTH_$(MCO_$(1)_ARCHID)_gcc)
                      ,32 $(warning Failed to autodetect $(1) CPU data bus width, default to 32. Use $(1)_DBUS_WIDTH to override or set manually)))

MCO_$(1)_ARCH_SFX    :=$(MCO_$(1)_OSID)-$(MCO_$(1)_ARCHID)
endef 

#    qcc compiler
MCO_CFAM_COMP_C_qcc       :=qcc -lang-c   @platform@ @debug_flags@ @arch_flags@ @defines@ -c @source_file@ -o @object_file@
MCO_CFAM_EXT_COMP_C_qcc   :=qcc -lang-c   @platform@ @arch_flags@
MCO_CFAM_COMP_CPP_qcc     :=qcc -lang-c++ @platform@ @debug_flags@ @arch_flags@ @defines@ -c @source_file@ -o @object_file@
MCO_CFAM_EXT_COMP_CPP_qcc :=qcc -lang-c++ @platform@ @arch_flags@
MCO_CFAM_LINK_C_qcc       :=qcc -lang-c   @platform@ @debug_flags@ @link_flags_c@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LINK_CPP_qcc     :=qcc -lang-c++ @platform@ @debug_flags@ @link_flags_cpp@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_EXT_LIBR_STAT_qcc:=ar
MCO_CFAM_EXT_LIBR_DYNM_qcc:=qcc -lang-c++ -shared  @link_flags_cpp@
MCO_CFAM_LIBR_STAT_C_qcc  :=ar rc @output-file@ @object_files@ 
MCO_CFAM_LIBR_STAT_CPP_qcc:=ar rc @output-file@ @object_files@
MCO_CFAM_LIBR_DYNM_C_qcc  :=qcc -lang-c   -shared @platform@ @debug_flags@ @link_flags_c@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LIBR_DYNM_CPP_qcc:=qcc -lang-c++ -shared @platform@ @debug_flags@ @link_flags_cpp@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_PLATFORM_qcc      =$(if $($(1)_PLATFORM),-V$($(1)_PLATFORM))

define MCO_CFAM_PROPS_get_qcc
# param 1 - HOST/TARGET
MCO_$(1)_DUMPMACHINE :=$(shell qcc $(call MCO_CFAM_PLATFORM_qcc,$(1)) -dumpmachine)
MCO_$(1)_ARCH_PART   :=$(firstword $(subst -, ,$$(MCO_$(1)_DUMPMACHINE)))
endef

define MCO_CFAM_PROPS_analyze_pass1_qcc
# param 1 - HOST/TARGET
MCO_$(1)_ARCHID      :=$(strip $(or $($(1)_ARCH) \
                      ,$(if $(findstring ntoarm,$($(1)_PLATFORM)),arm) \
                      ,$(if $(findstring ntoppc,$($(1)_PLATFORM)),ppc) \
                      ,$(if $(findstring ntomips,$($(1)_PLATFORM)),mips) \
                      ,$(if $(findstring ntox86,$($(1)_PLATFORM)),x86) \
                      ,x86 $(warning Failed to autodetect $(1) CPU architecture, default to x86. Use $(1)_ARCH to override or set manually)))
MCO_$(1)_OSID        :=QNX
endef

define MCO_CFAM_PROPS_analyze_pass2_qcc
# param 1 - HOST/TARGET
$(if $(subst auto,,$(x64)),$(info "$(1): x64=$(x64) ignored"))

MCO_$(1)_DBUS_WIDTH  :=$(strip $(or $($(1)_DBUS_WIDTH),32))
MCO_$(1)_ARCH_SFX    :=$(MCO_$(1)_OSID)-$(MCO_$(1)_ARCHID)
endef

#    clang compiler (https://clang.llvm.org/docs/UsersManual.html#command-line-options)
MCO_CFAM_COMP_C_clang       :=clang   @platform@ @debug_flags@ @arch_flags@ @defines@ -c @source_file@ -o @object_file@
MCO_CFAM_EXT_COMP_C_clang   :=clang   @platform@ @arch_flags@ 
MCO_CFAM_COMP_CPP_clang     :=clang++ @platform@ @debug_flags@ @arch_flags@ @defines@ -c @source_file@ -o @object_file@
MCO_CFAM_EXT_COMP_CPP_clang :=clang++ @platform@ @arch_flags@
MCO_CFAM_LINK_C_clang       :=clang   @platform@ @debug_flags@ @link_flags_c@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LINK_CPP_clang     :=clang++ @platform@ @debug_flags@ @link_flags_cpp@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_EXT_LIBR_STAT_clang:=ar
MCO_CFAM_EXT_LIBR_DYNM_clang:=clang++ -shared -undefined dynamic_lookup @link_flags_cpp@
MCO_CFAM_LIBR_STAT_C_clang  :=ar rc @output-file@ @object_files@ 
MCO_CFAM_LIBR_STAT_CPP_clang:=ar rc @output-file@ @object_files@
MCO_CFAM_LIBR_DYNM_C_clang  :=clang   -shared -undefined dynamic_lookup @platform@ @debug_flags@ @link_flags_c@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LIBR_DYNM_CPP_clang:=clang++ -shared -undefined dynamic_lookup @platform@ @debug_flags@ @link_flags_cpp@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_PLATFORM_clang      =$(if $($(1)_PLATFORM),--target $($(1)_PLATFORM))

define MCO_CFAM_PROPS_get_clang
# param 1 - HOST/TARGET
MCO_$(1)_DUMPMACHINE :=$(shell clang $(call MCO_CFAM_PLATFORM_clang,$(1)) -dumpmachine)
MCO_$(1)_ARCH_PART   :=$(firstword $(subst -, ,$$(MCO_$(1)_DUMPMACHINE)))
endef

# note: using gcc code for clang
define MCO_CFAM_PROPS_analyze_pass1_clang
# param 1 - HOST/TARGET
MCO_$(1)_ARCHID      :=$(strip $(or $($(1)_ARCH) \
                      ,$(if $(findstring aarch64,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),arm64)) \
                      ,$(if $(findstring x86_64,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),x86_64)) \
                      ,$(if $(findstring x86,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),x86)) \
                      ,$(if $(findstring arm64,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),arm64)) \
                      ,$(if $(findstring arm,$(MCO_$(1)_ARCH_PART)),$(call MCO_ARCHID_SWITCH_gcc,$(1),arm)) \
                      ,x86 $(warning Failed to autodetect $(1) CPU architecture, default to x86. Use $(1)_ARCH to override or set manually)))
MCO_$(1)_OSID        :=Darwin
endef

define MCO_CFAM_PROPS_analyze_pass2_clang
# param 1 - HOST/TARGET
MCO_$(1)_DBUS_WIDTH  :=$(strip $(or $($(1)_DBUS_WIDTH) \
                      ,$(MCO_ARCH_DBUS_WIDTH_$(MCO_$(1)_ARCHID)_gcc)
                      ,32 $(warning Failed to autodetect $(1) CPU data bus width, default to 32. Use $(1)_DBUS_WIDTH to override or set manually)))
MCO_$(1)_ARCH_SFX    :=$(MCO_$(1)_OSID)-$(MCO_$(1)_ARCHID)
endef

#    xlc/xlc++ compiler (http://www-01.ibm.com/support/docview.wss?uid=swg27024742&aid=1)
MCO_CFAM_COMP_C_xlc       :=xlc   @platform@ @debug_flags@ @arch_flags@ @defines@ -c @source_file@ -o @object_file@ -MF @dependency_file@
MCO_CFAM_EXT_COMP_C_xlc   :=xlc   @platform@ @arch_flags@ 
MCO_CFAM_COMP_CPP_xlc     :=xlc++ @platform@ @debug_flags@ @arch_flags@ @defines@ -qrtti=dyna -c @source_file@ -o @object_file@ -MF @dependency_file@
MCO_CFAM_EXT_COMP_CPP_xlc :=xlc++ @platform@ @arch_flags@
MCO_CFAM_LINK_C_xlc       :=xlc   @platform@ @debug_flags@ @link_flags_c@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LINK_CPP_xlc     :=xlc++ @platform@ @debug_flags@ @link_flags_cpp@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_EXT_LIBR_STAT_xlc:=ar -Xany
MCO_CFAM_EXT_LIBR_DYNM_xlc:=xlc++ -G -qmkshrobj @link_flags_cpp@
MCO_CFAM_LIBR_STAT_C_xlc  :=ar -Xany rc @output-file@ @object_files@ 
MCO_CFAM_LIBR_STAT_CPP_xlc:=ar -Xany rc @output-file@ @object_files@
MCO_CFAM_LIBR_DYNM_C_xlc  :=xlc   -G -qmkshrobj @platform@ @debug_flags@ @link_flags_c@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_LIBR_DYNM_CPP_xlc:=xlc++ -G -qmkshrobj @platform@ @debug_flags@ @link_flags_cpp@ @object_files@ @libraries@ @platform_libs@ -o @output-file@
MCO_CFAM_PLATFORM_xlc      =$(if $($(1)_PLATFORM),-qarch=$($(1)_PLATFORM),-qarch=ppc)
#
# Arch switch table (x64=on/off/auto override implementation)
# MCO_ARCH_SWITCH_<arch>_<x64-value>_xlc:=<arch>
MCO_ARCH_SWITCH_powerpc_on_xlc:=powerpc64
MCO_ARCH_SWITCH_powerpc64_off_xlc:=powerpc

MCO_ARCHID_SWITCH_xlc=$(if $(subst auto,,$(x64)),$(or $(MCO_ARCH_SWITCH_$(2)_$(x64)_xlc),$(2)),$(2))
define MCO_CFAM_PROPS_get_xlc
# param 1 - HOST/TARGET
MCO_$(1)_ARCH_PART   :=powerpc
endef
define MCO_CFAM_PROPS_analyze_pass1_xlc
# param 1 - HOST/TARGET
MCO_$(1)_ARCHID      :=$(strip $(or $($(1)_ARCH),$(call MCO_ARCHID_SWITCH_xlc,$(1),powerpc)))
MCO_$(1)_OSID        :=AIX
endef
define MCO_CFAM_PROPS_analyze_pass2_xlc
# param 1 - HOST/TARGET
MCO_$(1)_DBUS_WIDTH  :=$(strip $(or $($(1)_DBUS_WIDTH),$(if $(subst powerpc,,$(MCO_$(1)_ARCHID)),64,32)))
MCO_$(1)_ARCH_SFX    :=$(MCO_$(1)_OSID)-$(MCO_$(1)_ARCHID)
endef

#    TODO: wincc compiler
#    TODO: icc/icpc compiler (https://software.intel.com/sites/default/files/m/d/4/1/d/8/icc.txt)
#    TODO: SunOS native compiler
#    TODO: HP-UX native compiler cc/aCC (https://support.hpe.com/hpsc/doc/public/display?docId=emr_na-c02652146)

# 5. Generate host/target arch, databus width and os-id props
#    Produce HOST/TARGET_ARCHID/OSID/ARCH_SFX
$(eval $(call MCO_CFAM_PROPS_get_$(MCO_HOST_CFAM),HOST)) 
$(eval $(call MCO_CFAM_PROPS_get_$(MCO_TARGET_CFAM),TARGET)) 
$(eval $(call MCO_CFAM_PROPS_analyze_pass1_$(MCO_HOST_CFAM),HOST)) 
$(eval $(call MCO_CFAM_PROPS_analyze_pass1_$(MCO_TARGET_CFAM),TARGET)) 
$(eval $(call MCO_CFAM_PROPS_analyze_pass2_$(MCO_BUILD_CFAM),BUILD)) 
$(eval $(call MCO_CFAM_PROPS_analyze_pass2_$(MCO_HOST_CFAM),HOST))  
$(eval $(call MCO_CFAM_PROPS_analyze_pass2_$(MCO_TARGET_CFAM),TARGET)) 

# Cross-platform architecture <build-arch><target-dbus-width>
# Need to be runable on the build system but has the same dbus width as the target
MCO_CROSS_DBUS_WIDTH          :=$(MCO_TARGET_DBUS_WIDTH)
MCO_CROSS_DBUS_WIDTH_SWITCH_32:=off
MCO_CROSS_DBUS_WIDTH_SWITCH_64:=on
MCO_CROSS_DBUS_WIDTH_SWITCH   :=$(MCO_CROSS_DBUS_WIDTH_SWITCH_$(MCO_CROSS_DBUS_WIDTH))
MCO_CROSS_ARCHID              :=$(or $(MCO_ARCH_SWITCH_$(MCO_BUILD_ARCHID)_$(MCO_CROSS_DBUS_WIDTH_SWITCH)_$(MCO_BUILD_CFAM)),$(MCO_BUILD_ARCHID))
MCO_CROSS_ARCH_SFX            :=$(MCO_BUILD_OSID)-$(MCO_CROSS_ARCHID)

# 6. Generate comiler-, os-, arch-, dbusw- dependent flags for host/target
MCO_CPROPS_F_MCOCOMP_any                :=-x32
MCO_CPROPS_F_MCOCOMP_x86_64             :=-x64

MCO_CPROPS_F_DEFS_xlc_AIX_any           :=-D_AIX
MCO_CPROPS_F_ARCH_xlc_AIX_any           :=-qmakedep=gcc -qpic -r -qthreaded -qasm=gcc -qwarn64
MCO_CPROPS_F_DEFS_xlc_AIX_powerpc       :=-D_PPC32 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_xlc_AIX_powerpc       :=-q32
MCO_CPROPS_F_LINK_xlc_AIX_powerpc       :=-q32
MCO_CPROPS_F_DEFS_xlc_AIX_powerpc64     :=--D_PPC32 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_xlc_AIX_powerpc64     :=-q64
MCO_CPROPS_F_LINK_xlc_AIX_powerpc64     :=-q64
MCO_CPROPS_PLATFORMLIB_xlc_AIX_any      :=-lrt -lm -lpthread $(USERLIB)

MCO_CPROPS_F_XWRN_clang_any_any         :=-Wextra -Wall -pedantic
MCO_CPROPS_F_DEFS_clang_Darwin_any      :=-D_MACOS -D_LINUX
MCO_CPROPS_F_ARCH_clang_Darwin_any      :=-I/usr/include/malloc -MMD
MCO_CPROPS_PLATFORMLIB_clang_Darwin_any:=-lpthread -lm -ldl $(USERLIB)
MCO_CPROPS_F_DEFS_clang_Darwin_x86      :=-D_Ix86 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_clang_Darwin_x86      :=-m32
MCO_CPROPS_F_LINK_clang_Darwin_x86      :=-m32
MCO_CPROPS_F_DEFS_clang_Darwin_x86_64   :=-D_Ix86 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_clang_Darwin_x86_64   :=-m64
MCO_CPROPS_F_LINK_clang_Darwin_x86_64   :=-m64

MCO_CPROPS_F_ARCH_gcc_any_any           :=-fPIC -Wall -Wno-unused-variable -Wno-unused-function -MMD
MCO_CPROPS_F_XWRN_gcc_any_any           :=-Wextra -Wall -pedantic
MCO_CPROPS_PLATFORMLIB_gcc_any_any      :=$(MCO_EXT_LIBS) $(USERLIB)

MCO_CPROPS_F_DEFS_gcc_Linux_any         :=-D_LINUX
MCO_CPROPS_PLATFORMLIB_gcc_Linux_any    :=$(MCO_AWS_GCC_PLATFORM_$(MCO_LIC_AWS)) -lpthread -lm -ldl
# No need to link complex hierarchy of shared libraries thus no need to use this switch
#MCO_CPROPS_F_DYNLINK_gcc_Linux_any      :=-Wl,--copy-dt-needed-entries
MCO_CPROPS_F_DEFS_gcc_Linux_x86         :=-D_Ix86 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_x86         :=-m32
MCO_CPROPS_F_LINK_gcc_Linux_x86         :=-m32
MCO_CPROPS_F_DEFS_gcc_Linux_x86_64      :=-D_Ix86 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_x86_64      :=-m64
MCO_CPROPS_F_LINK_gcc_Linux_x86_64      :=-m64
MCO_CPROPS_F_DEFS_gcc_Linux_arm         :=-D_ARM32 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_DEFS_gcc_Linux_arm64       :=-D_ARM32 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_DEFS_gcc_Linux_ppc         :=-D_PPC32 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_ppc         :=-m32
MCO_CPROPS_F_LINK_gcc_Linux_ppc         :=-m32
MCO_CPROPS_F_DEFS_gcc_Linux_ppcle       :=-D_PPC32 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_ppcle       :=-m32
MCO_CPROPS_F_LINK_gcc_Linux_ppcle       :=-m32
MCO_CPROPS_F_DEFS_gcc_Linux_ppc64       :=-D_PPC32 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_ppc64       :=-m64 -fpermissive
MCO_CPROPS_F_LINK_gcc_Linux_ppc64       :=-m64
MCO_CPROPS_F_DEFS_gcc_Linux_ppc64le     :=-D_PPC32 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_ppc64le     :=-m64 -fpermissive
MCO_CPROPS_F_LINK_gcc_Linux_ppc64le     :=-m64
MCO_CPROPS_F_DEFS_gcc_Linux_mips        :=-D_MIPS=32 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_mips        :=-mips2
MCO_CPROPS_F_LINK_gcc_Linux_mips        :=-mips2
MCO_CPROPS_F_DEFS_gcc_Linux_mips64      :=-D_MIPS=64 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_Linux_mips64      :=
MCO_CPROPS_F_LINK_gcc_Linux_mips64      :=
MCO_CPROPS_F_DEFS_gcc_Linux_sh4         :=-D_SH4 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_DEFS_gcc_Linux_tile        :=-D_TILE=tile $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_DEFS_gcc_Linux_tilegx      :=-D_TILE=tilegx $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_DEFS_gcc_Linux_microblaze  :=-D_MICROBLAZE $(addprefix -U,$(MCO_EXDB_X64))

MCO_CPROPS_F_DEFS_gcc_CYGWIN_any        :=-D_LINUX -D_CYGWIN
MCO_CPROPS_F_DEFS_gcc_CYGWIN_x86        :=-D_Ix86 $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_CYGWIN_x86        :=-m32
MCO_CPROPS_F_LINK_gcc_CYGWIN_x86        :=-m32
MCO_CPROPS_F_DEFS_gcc_CYGWIN_x86_64     :=-D_Ix86 $(addprefix -D,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_CYGWIN_x86_64     :=-m64
MCO_CPROPS_F_LINK_gcc_CYGWIN_x86_64     :=-m64
MCO_CPROPS_PLATFORMLIB_gcc_CYGWIN_any   :=-lpthread -ldl -lkernel32

MCO_CPROPS_F_DEFS_gcc_autosar_any        :=-D_AUTOSAR -fno-common -W -ffunction-sections -fdata-sections -fshort-double -Wno-unused-parameter -fstack-usage
MCO_CPROPS_F_DEFS_gcc_autosar_tricore    :=-D_TRICORE $(addprefix -U,$(MCO_EXDB_X64))
MCO_CPROPS_F_ARCH_gcc_autosar_tricore    :=-mcpu=tc39xx -fstack-usage
MCO_CPROPS_F_LINK_gcc_autosar_tricore    :=-fno-common -W -ffunction-sections -fdata-sections -fshort-double -std=c++11 -Wno-unused-parameter -mcpu=tc39xx
MCO_CPROPS_PLATFORMLIB_gcc_autosar_any  :=
MCO_CPROPS_PLATFORMLIB_gcc_autosar_tricore:=

MCO_CPROPS_F_DEFS_gcc_uclinux_any       :=-D_UCLINUX
MCO_CPROPS_F_LINK_gcc_uclinux_any       :=-elf2flt
MCO_CPROPS_PLATFORMLIB_gcc_uclinux_any  :=-lc

MCO_CPROPS_F_DEFS_gcc_ecos_any          :=-D_ECOS -DMCO_CFG_USE_EXCEPTIONS=0
MCO_CPROPS_F_ARCH_gcc_ecos_any          :=-fno-exceptions
MCO_CPROPS_F_LINK_gcc_ecos_any          :=-fno-exceptions

MCO_CPROPS_F_DEFS_gcc_lynxos_any        :=-D__LYNX -DMCO_CONFIG_OVERRIDE_WCHAR
MCO_CPROPS_F_LINK_gcc_lynxos_any        := -mthreads -L$(ENV_PREFIX)/lib/thread

MCO_CPROPS_F_DEFS_gcc_netos_any         :=-D_NETOS

MCO_CPROPS_F_DEFS_qcc_QNX_any           :=-D_QNX
MCO_CPROPS_F_ARCH_qcc_QNX_any           :=-MMD
MCO_CPROPS_PLATFORMLIB_qcc_QNX_any      :=-lm
MCO_CPROPS_SOCKETLIB_qcc_QNX_any        :=-lsocket

MCO_CPROPS_F_DEFS_gcc_SunOS_any         :=-D_SOLARIS -DMCO_STRICT_ALIGNMENT
MCO_CPROPS_F_ARCH_gcc_SunOS_any         :=-MMD
MCO_CPROPS_PLATFORMLIB_qcc_QNX_any      :=-lpthread -lnsl -lm
MCO_CPROPS_SOCKETLIB_qcc_QNX_any        :=-lsocket

# TODO: support LynxOS, eCos, ucLinux, uCos, windows
# SOCKETLIB      = ws2_32.lib

#    Define host/target compilers and props
define CONSTRUCT_PROPS
# param1 : HOST/TARGET
MCO_$(1)_F_MCOCOMP    :=$(or $(MCO_CPROPS_F_MCOCOMP_$(MCO_$(1)_ARCHID)),$(MCO_CPROPS_F_MCOCOMP_any))
MCO_$(1)_F_DEFINE     :=$(or $(MCO_CFAM_F_DEFINE_$(MCO_$(1)_CFAM)),$(MCO_CFAM_F_DEFINE_any))
MCO_$(1)_F_LIBRARY    :=$(or $(MCO_CFAM_F_LIBRARY_$(MCO_$(1)_CFAM)),$(MCO_CFAM_F_LIBRARY_any))
MCO_$(1)_F_PATHINC    :=$(or $(MCO_CFAM_F_PATHINC_$(MCO_$(1)_CFAM)),$(MCO_CFAM_F_PATHINC_any))
MCO_$(1)_F_PATHLIB    :=$(or $(MCO_CFAM_F_PATHLIB_$(MCO_$(1)_CFAM)),$(MCO_CFAM_F_PATHLIB_any))
MCO_$(1)_F_WDBG       :=$(or $(MCO_CFAM_F_WDBG_$(MCO_$(1)_CFAM)),$(MCO_CFAM_F_WDBG_any))
MCO_$(1)_F_NDBG       :=$(or $(MCO_CFAM_F_NDBG_$(MCO_$(1)_CFAM)),$(MCO_CFAM_F_NDBG_any))
MCO_$(1)_LIB_STAT_PFX :=$(or $(MCO_OS_LIB_STAT_PFX_$(MCO_$(1)_OSID)),$(MCO_OS_LIB_STAT_PFX_any))
MCO_$(1)_LIB_DYNM_PFX :=$(or $(MCO_OS_LIB_DYNM_PFX_$(MCO_$(1)_OSID)),$(MCO_OS_LIB_DYNM_PFX_any))
MCO_$(1)_EXEC_PFX     :=$(or $(MCO_OS_EXEC_PFX_$(MCO_$(1)_OSID)),$(MCO_OS_EXEC_PFX_any))
MCO_$(1)_LIB_STAT_SFX :=$(or $(MCO_OS_LIB_STAT_SFX_$(MCO_$(1)_OSID)),$(MCO_OS_LIB_STAT_SFX_any))
MCO_$(1)_LIB_DYNM_SFX :=$(or $(MCO_OS_LIB_DYNM_SFX_$(MCO_$(1)_OSID)),$(MCO_OS_LIB_DYNM_SFX_any))
MCO_$(1)_EXEC_SFX     :=$(or $(MCO_OS_EXEC_SFX_$(MCO_$(1)_OSID)),$(MCO_OS_EXEC_SFX_any))
MCO_$(1)_PLATFORM     :=$(call MCO_CFAM_PLATFORM_$(MCO_$(1)_CFAM),$(1))

MCO_$(1)_F_DEFS       :=$(MCO_CPROPS_F_DEFS_$(MCO_$(1)_CFAM)_any_any) \
                        $(MCO_CPROPS_F_DEFS_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_any) \
                        $(MCO_CPROPS_F_DEFS_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_$(MCO_$(1)_ARCHID))
MCO_$(1)_F_ARCH       :=$(MCO_CPROPS_F_ARCH_$(MCO_$(1)_CFAM)_any_any) \
                        $(MCO_CPROPS_F_ARCH_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_any) \
                        $(MCO_CPROPS_F_ARCH_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_$(MCO_$(1)_ARCHID)) \
                        $($(1)_FLAGS)
MCO_$(1)_F_XWRN_on    :=$(MCO_CPROPS_F_XWRN_$(MCO_$(1)_CFAM)_any_any) \
                        $(MCO_CPROPS_F_XWRN_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_any) \
                        $(MCO_CPROPS_F_XWRN_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_$(MCO_$(1)_ARCHID))
MCO_$(1)_F_LINK       :=$(MCO_CPROPS_F_LINK_$(MCO_$(1)_CFAM)_any_any) \
                        $(MCO_CPROPS_F_LINK_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_any) \
                        $(MCO_CPROPS_F_LINK_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_$(MCO_$(1)_ARCHID)) \
                        $($(1)_FLAGS)
MCO_$(1)_F_DYNLINK    :=$(MCO_CPROPS_F_DYNLINK_$(MCO_$(1)_CFAM)_any_any) \
                        $(MCO_CPROPS_F_DYNLINK_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_any) \
                        $(MCO_CPROPS_F_DYNLINK_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_$(MCO_$(1)_ARCHID))
MCO_$(1)_PLATFORMLIB  :=$(MCO_CPROPS_PLATFORMLIB_$(MCO_$(1)_CFAM)_any_any) \
                        $(MCO_CPROPS_PLATFORMLIB_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_any) \
                        $(MCO_CPROPS_PLATFORMLIB_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_$(MCO_$(1)_ARCHID))
MCO_$(1)_SOCKETLIB    :=$(MCO_CPROPS_SOCKETLIB_$(MCO_$(1)_CFAM)_any_any) \
                        $(MCO_CPROPS_SOCKETLIB_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_any) \
                        $(MCO_CPROPS_SOCKETLIB_$(MCO_$(1)_CFAM)_$(MCO_$(1)_OSID)_$(MCO_$(1)_ARCHID))

MCO_$(1)_LIB_JNI_SFX :=$(or $(MCO_OS_LIB_JNI_SFX_$(MCO_$(1)_OSID)),$(MCO_OS_LIB_JNI_SFX_any))
MCO_$(1)_LIB_LUA_SFX :=$(or $(MCO_OS_LIB_LUA_SFX_$(MCO_$(1)_OSID)),$(MCO_OS_LIB_LUA_SFX_any))
endef

$(eval $(call CONSTRUCT_PROPS,BUILD))
$(eval $(call CONSTRUCT_PROPS,HOST))
$(eval $(call CONSTRUCT_PROPS,TARGET))
$(eval $(call CONSTRUCT_PROPS,CROSS))

MCO_TEMPLATE_TO_COMMAND=$(strip \
			$(subst @gcc@,$(MCO_$(1)_GCC_OVERRIDE), \
			$(subst @g++@,$(MCO_$(1)_GXX_OVERRIDE), \
            $(subst @platform@,$(MCO_$(1)_PLATFORM), \
			$(subst @defines@,$(MCO_$(1)_F_DEFS) @defines@, \
			$(subst @arch_flags@,$(MCO_$(1)_F_ARCH) $(MCO_$(1)_F_XWRN_$(MCO_EXTRA_CHECKS)), \
			$(subst @link_flags_c@,$(MCO_$(1)_F_LINK), \
			$(subst @link_flags_cpp@,$(MCO_$(1)_F_LINK), \
			$(subst @platform_libs@,$(MCO_$(1)_PLATFORMLIB), \
			$(2))))))))))

define CONSTRUCT_COMPILERS
# param1 : HOST/TARGET
MCO_$(1)_COMP_C       :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_COMP_C_$(MCO_$(1)_CFAM)))
MCO_$(1)_EXT_COMP_C   :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_EXT_COMP_C_$(MCO_$(1)_CFAM)))
MCO_$(1)_COMP_CPP     :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_COMP_CPP_$(MCO_$(1)_CFAM)))
MCO_$(1)_EXT_COMP_CPP :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_EXT_COMP_CPP_$(MCO_$(1)_CFAM)))
MCO_$(1)_LINK_C       :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_LINK_C_$(MCO_$(1)_CFAM)))
MCO_$(1)_LINK_CPP     :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_LINK_CPP_$(MCO_$(1)_CFAM)))
MCO_$(1)_EXT_LIBR_STAT:=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_EXT_LIBR_STAT_$(MCO_$(1)_CFAM)))
MCO_$(1)_EXT_LIBR_DYNM:=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_EXT_LIBR_DYNM_$(MCO_$(1)_CFAM)))
MCO_$(1)_LIBR_STAT_C  :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_LIBR_STAT_C_$(MCO_$(1)_CFAM)))
MCO_$(1)_LIBR_STAT_CPP:=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_LIBR_STAT_CPP_$(MCO_$(1)_CFAM)))
MCO_$(1)_LIBR_DYNM_C  :=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_LIBR_DYNM_C_$(MCO_$(1)_CFAM)))
MCO_$(1)_LIBR_DYNM_CPP:=$(call MCO_TEMPLATE_TO_COMMAND,$(1),$(MCO_CFAM_LIBR_DYNM_CPP_$(MCO_$(1)_CFAM)))
endef

$(eval $(call CONSTRUCT_COMPILERS,BUILD))
$(eval $(call CONSTRUCT_COMPILERS,HOST))
$(eval $(call CONSTRUCT_COMPILERS,TARGET))
$(eval $(call CONSTRUCT_COMPILERS,CROSS))

# Set default eXtremeDB drivers for HOST/TARGET
define CONSTRUCT_EXDB_DEFAULTS
# Param 1 = HOST/TARGET
MCO_$(1)_DEFAULT_MEMDEV_PRIVATE:=$(or $(MCO_EXDB_DEFAULT_MEMDEV_PRIVATE_$(MCO_$(1)_OSID)),$(MCO_EXDB_DEFAULT_MEMDEV_PRIVATE_any))
MCO_$(1)_DEFAULT_MEMDEV_SHARED :=$(or $(MCO_EXDB_DEFAULT_MEMDEV_SHARED_$(MCO_$(1)_OSID)),$(MCO_EXDB_DEFAULT_MEMDEV_SHARED_any))
MCO_$(1)_DEFAULT_STRDEV        :=$(or $(MCO_EXDB_DEFAULT_STRDEV_$(MCO_$(1)_OSID)),$(MCO_EXDB_DEFAULT_STRDEV_any))
MCO_$(1)_DEFAULT_SYNC_PRIVATE  :=$(or $(MCO_EXDB_DEFAULT_SYNC_PRIVATE_$(MCO_$(1)_OSID)),$(MCO_EXDB_DEFAULT_SYNC_PRIVATE_any))
MCO_$(1)_DEFAULT_SYNC_SHARED   :=$(or $(MCO_EXDB_DEFAULT_SYNC_SHARED_$(MCO_$(1)_OSID)),$(MCO_EXDB_DEFAULT_SYNC_SHARED_any))
MCO_$(1)_MEMDEV_IMPLEMENTATIONS:=$(MCO_EXDB_BUILD_MEMDEV_any) $(MCO_EXDB_BUILD_MEMDEV_$(MCO_$(1)_OSID))
MCO_$(1)_SYNC_IMPLEMENTATIONS  :=$(MCO_EXDB_BUILD_SYNC_any) $(MCO_EXDB_BUILD_SYNC_$(MCO_$(1)_OSID))
MCO_$(1)_NWPIP_IMPLEMENTATIONS :=$(MCO_EXDB_BUILD_NWPIP_any) $(MCO_EXDB_BUILD_NWPIP_$(MCO_$(1)_OSID))
MCO_$(1)_NWTCP_IMPLEMENTATIONS :=$(MCO_EXDB_BUILD_NWTCP_any) $(MCO_EXDB_BUILD_NWTCP_$(MCO_$(1)_OSID))
MCO_$(1)_NWUDP_IMPLEMENTATIONS :=$(MCO_EXDB_BUILD_NWUDP_any) $(MCO_EXDB_BUILD_NWUDP_$(MCO_$(1)_OSID))
MCO_$(1)_NWQNXM_IMPLEMENTATIONS:=$(MCO_EXDB_BUILD_NWQNXM_any) $(MCO_EXDB_BUILD_NWQNXM_$(MCO_$(1)_OSID))
MCO_$(1)_FSYS_IMPLEMENTATIONS  :=$(MCO_EXDB_BUILD_FSYS_any) $(MCO_EXDB_BUILD_FSYS_$(MCO_$(1)_OSID))
endef
$(eval $(call CONSTRUCT_EXDB_DEFAULTS,BUILD))
$(eval $(call CONSTRUCT_EXDB_DEFAULTS,HOST))
$(eval $(call CONSTRUCT_EXDB_DEFAULTS,TARGET))

# Define eXtremeDB components
# core libraries
MCO_CORE_                   :=
MCO_CORE_YES                :=mcolib
# transient/persistent core extention
MCO_VT_                     :=
MCO_VT_TRANSIENT            :=mcovtmem
MCO_VT_PERSISTENT           :=mcovtdsk
# transaction manager components and libraries to resolve cross refs
MCO_TM_                     :=
MCO_TM_MURSIW               :=mcotmursiw mcolib
MCO_TM_MURSIW_READ          :=mcotread mcolib
MCO_TM_MVCC                 :=mcotmvcc mcolib
MCO_TM_EXCLUSIVE            :=mcotexcl mcolib
# SQL support components and mcocomp's additonal flags
MCO_SQL_                    :=
MCO_SQL_LOCAL               :=mcosql mcobackup mcoseri mcolib mcoseq mcoseqmath @socketlib@
MCO_SQL_REMOTE              :=mcosql mcorsql mcobackup mcoseri mcolib mcoseq mcoseqmath @socketlib@
MCO_SQL_REMOTE_STANDALONE   :=mcosqlmini mcorsql mcorsqlstub mcouwrt
MCO_SQL_MCOCOMPF_                    :=
MCO_SQL_MCOCOMPF_LOCAL               := -sql
MCO_SQL_MCOCOMPF_REMOTE              := -sql
MCO_SQL_MCOCOMPF_REMOTE_STANDALONE   := -sql
# HA components and flags
MCO_mcohalib              :=mcoha mcorest mcoews mcoseri mcolib
MCO_HA_                   :=
MCO_HA_TCP                :=@mcohalib@ mconwtcp @socketlib@
MCO_HA_UDP                :=@mcohalib@ mconwudp @socketlib@
MCO_HA_PIPES              :=@mcohalib@ mconwpipes
MCO_HA_CFLAGS_            :=
MCO_HA_CFLAGS_TCP         :=CFG_TCP
MCO_HA_CFLAGS_UDP         :=CFG_UDP
MCO_HA_CFLAGS_TCPUDP      :=CFG_TCP
MCO_HA_CFLAGS_PIPES       :=CFG_PIPES
# TL components and flags
MCO_TL_                   :=
MCO_TL_YES                :=mcolog mcoseri mcolib
# Serialization component
MCO_SERI_                 :=
MCO_SERI_YES              :=mcoseri mcolib
# DBCalc
MCO_DBCALC_               :=
MCO_DBCALC_YES            :=mcodbcalc mcolib
# Iterator component
MCO_ITER_                 :=
MCO_ITER_YES              :=mcoiter mcolib
# JSON serialization component
MCO_JSER_                 :=
MCO_JSER_YES              := mcojser mcouda mcolib mcoseq mcoseqmath
# UDA component
MCO_UDA_                  :=
MCO_UDA_YES               :=mcouda mcolib mcoseq mcoseqmath
MCO_UDA_MCOAPI            :=mcouda mcolib mcoseq mcoseqmath
MCO_UDA_MCOCOMP_          :=
MCO_UDA_MCOCOMP_YES       := -sa -f
MCO_UDA_MCOCOMP_MCOAPI    := -f
# Perfomance monitor
MCO_PERFMON_              :=
MCO_PERFMON_YES           :=mcoperf mcoseri mcolib $(PERFMON_ADD_LIB)
# HTTP Viewer component ( With- and without SQL support)
MCO_HV__                  :=
MCO_HV__LOCAL             :=
MCO_HV__REMOTE            :=
MCO_HV__REMOTE_STANDALONE :=
MCO_HV_YES_               :=mcohv mcoews mconet mcouda mcolib mcoseq mcoseqmath @socketlib@
MCO_HV_YES_LOCAL          :=mcohv_sql mcoews mconet mcouda mcolib mcoseq mcoseqmath @socketlib@
MCO_HV_YES_REMOTE         :=mcohv_sql mcoews mconet mcouda mcolib mcoseq mcoseqmath @socketlib@
MCO_HV_YES_REMOTE_STANDALONE  :=mcohv_sql mcoews mconet mcouda mcolib mcoseq mcoseqmath @socketlib@
# Cluster component
MCO_CLUSTER_              :=
MCO_CLUSTER_TCP           :=mcocluster mcorest mcorestdb mcoews mcouda mcoseri mcocltcp mcolib mcoseq mcoseqmath mconet @socketlib@
MCO_CLUSTER_MPI           :=mcocluster mcorest mcorestdb mcoews mcouda mcoseri mcoclmpi mcolib mcoseq mcoseqmath mconet @socketlib@
# Sequentces
MCO_SEQ_                  :=
MCO_SEQ_YES               :=mcoseq mcoseqmath
MCO_SEQ_RLE               :=mcoseqrle mcoseqrlemath
# Mcocomp library
MCO_MCOCOMP_              :=
MCO_MCOCOMP_YES           :=mcocomp
# Incremental backup
MCO_BACKUP_               :=
MCO_BACKUP_YES            :=mcobackup
# REST interface
MCO_REST_                 :=
MCO_REST_YES              :=mcorestdb mcorest mcoews mcouda mcolib mcoseq mcoseqmath mconet @socketlib@
MCO_REST_YES_PERFMON_YES  :=mcorestperf
MCO_REST_YES_SQL_YES      :=mcorestsql
MCO_REST_YES_TRACE_YES    :=mcoresttrace
# ODBC interface. Link ODBC object code
MCO_ODBC_                 :=
MCO_ODBC_YES              :=mcoodbc mcosql mcobackup mcoseri mcorsql @socketlib@
# ODBC interface. Link ODBC driver shared library itself
MCO_ODBCDRV_              :=
MCO_ODBCDRV_YES           :=mcoodbc_drv
# Cryptografy
MCO_CRYPT__               :=
MCO_CRYPT_YES_            :=mcocryptstub
MCO_CRYPT_YES_YES         :=mcocryptaes
# C runtime usage
MCO_UTILS_any             :=
MCO_UTILS_YES_any         :=mcouwrt
MCO_UTILS_YES_autosar     :=mcounrt
# Trace component
MCO_TRACE_                :=
MCO_TRACE_NO              :=
MCO_TRACE_YES             :=mcotrace
# IOT component
MCO_IOT_                  :=
MCO_IOT_YES               :=mcoiotrepl mcoiotcomm mcorest mcorestdb mcoews mcouda mcoseri mcolib mcoseq mcoseqmath mconet @socketlib@
# FeedHandler component
MCO_FHLIB_                :=
MCO_FHLIB_YES             :=fh mcosql mcorsql mcobackup mcoseri mcoseq mcoseqmath mcoews mconet mcoloader mcotrace @socketlib@
# SAL libraries
MCO_SAL_                  :=
MCO_SAL_YES_any           :=mcosallatches mcosalatomic mcosaltimer mcosalsmp mcosalmem
MCO_SAL_YES_autosar       :=mcosalhollow
MCO_SAL_DLOAD_            :=
MCO_SAL_DLOAD_YES_any     :=mcosaldload
MCO_SAL_DLOAD_YES_autosar :=mcosalhollow
MCO_SAL_NET_              :=
MCO_SAL_NET_YES_any       :=mconet
MCO_SAL_NET_YES_autosar   :=mcosalhollow

MCO_ENFORCE_MCOCOMP_          :=
MCO_ENFORCE_MCOCOMP_TRANSIENT :=-transient
MCO_ENFORCE_MCOCOMP_PERSISTENT:=-persistent

MCO_USE_DEBUG_            :=$(if $(findstring on,$(DEBUG))$(findstring on,$(MCO_DEBUG)),_debug)
MCO_USE_DEBUG_YES         :=_debug

MCO_DYNM_PATH_            :=
MCO_DYNM_PATH_YES         :=.so

# OpenSSL 
MCO_BUILD_OPENSSL111:=$(and $(shell which openssl),$(findstring 1.1.1,$(shell openssl version)))
MCO_TARGET_OPENSSL_INCLUDE:=$(or \
$(if $(or $(findstring envpath,$(MCO_FORCE_OPENSSL)),$(and $(findstring auto,$(MCO_FORCE_OPENSSL)),$(wildcard $(MCO_OPENSSL_INCLUDE_PATH)/openssl/ssl.h))),$(MCO_OPENSSL_INCLUDE_PATH)),\
$(if $(or $(findstring buildsys,$(MCO_FORCE_OPENSSL)),$(and $(findstring auto,$(MCO_FORCE_OPENSSL)),$(filter $(MCO_BUILD_ARCH_SFX),$(MCO_TARGET_ARCH_SFX)),$(MCO_BUILD_OPENSSL111),$(wildcard /usr/include/openssl/ssl.h)$(wildcard /usr/local/include/openssl/ssl.h))),/usr/include /usr/local/include),\
$(if $(or $(findstring sourcetree,$(MCO_FORCE_OPENSSL)),$(findstring auto,$(MCO_FORCE_OPENSSL))),$(MCO_OPENSSL_SRC_DIR)/openssl-$(MCO_TARGET_ARCH_SFX)/include),\
$(MCO_OPENSSL_SRC_DIR)/openssl-$(MCO_TARGET_ARCH_SFX)/include)
MCO_TARGET_OPENSSL_DEPENDENCY:=$(if $(findstring openssl,$(MCO_DISABLED_TAGS)),,$(if $(findstring $(MCO_OPENSSL_SRC_DIR)/openssl-$(MCO_TARGET_ARCH_SFX)/include,$(MCO_TARGET_OPENSSL_INCLUDE)),$(MCO_TARGET_OPENSSL_INCLUDE)/openssl/ssl.h))

ifneq (x$(findstring openssl,$(MCO_EXPLAIN_TAGS))x,xx)
$(info [$(if $(and $(shell which openssl),$(findstring 1.1.1,$(shell openssl version))),X, )] - OpenSSL 1.1.1 exists on the build system) 
$(info [$(if $(findstring envpath,$(MCO_FORCE_OPENSSL)),X, )] - Forced to use MCO_OPENSSL_INCLUDE_PATH env. variable)
$(info [$(if $(findstring buildsys,$(MCO_FORCE_OPENSSL)),X, )] - Forced to use build system OpenSSL installation)
$(info [$(if $(findstring sourcetree,$(MCO_FORCE_OPENSSL)),X, )] - Forced to build and use own OpenSSL installation)
$(info [$(if $(and $(findstring auto,$(MCO_FORCE_OPENSSL)),$(wildcard $(MCO_OPENSSL_INCLUDE_PATH)/openssl/ssl.h)),X, )] - auto-detection enabled, MCO_OPENSSL_INCLUDE_PATH env. variable is set and development headers are present)
$(info [$(if $(and $(findstring auto,$(MCO_FORCE_OPENSSL)),$(filter $(MCO_BUILD_ARCH_SFX),$(MCO_TARGET_ARCH_SFX)),$(MCO_BUILD_OPENSSL111),$(wildcard /usr/include/openssl/ssl.h)$(wildcard /usr/local/include/openssl/ssl.h)),X, )] - auto-detection enabled, the build system match to the target system and there are usable development headers in the local OpenSSL installation)
$(info [$(if $(findstring auto,$(MCO_FORCE_OPENSSL)),X, )] - auto-detection enabled, build and use own OpenSSL installation if all above failed)
$(info [$(if $(findstring $(MCO_FORCE_OPENSSL),auto envpath buildsys sourcetree), ,X)] - Invalid value of MCO_FORCE_OPENSSL detected, build and use own OpenSSL installation)
$(info MCO_TARGET_OPENSSL_INCLUDE=$(MCO_TARGET_OPENSSL_INCLUDE))
$(info )
endif

# explain python
ifneq (x$(findstring python,$(MCO_EXPLAIN_TAGS))x,xx)
$(info [$(if $(wildcard $(MCO_BUILD_PYTHON)),X, )] Python does exist on the build system)
$(info [$(if $(or $(filter $(MCO_BUILD_ARCH_SFX),$(MCO_TARGET_ARCH_SFX)),$(and $(findstring x86_64,$(MCO_BUILD_ARCHID)),$(findstring x86,$(MCO_TARGET_ARCHID)))),X, )] Target and build-system arch. signatures are the same [$(if $(filter $(MCO_BUILD_ARCH_SFX),$(MCO_TARGET_ARCH_SFX)),+,-)] or Build system is x86_64 and the target is x86 [$(if $(and $(findstring x86_64,$(MCO_BUILD_ARCHID)),$(findstring x86,$(MCO_TARGET_ARCHID))),+,-)] (python is runnable and good for the target usage))
$(info [$(if $(and $(wildcard $(MCO_BUILD_PYTHON)),$(shell $(MCO_BUILD_PYTHON) -c 'import sys; print("passed" if sys.maxsize == 2**$(MCO_TARGET_DBUS_WIDTH)/2-1 else "")')),X, )] DBus width match (otherwise not good for target usage))
$(info [$(if $(and $(wildcard $(MCO_BUILD_PYTHON)),$(findstring 2.7.,$(shell $(MCO_BUILD_PYTHON) -c "from platform import python_version; print python_version()"))),X, )] Build python is 2.7.x (otherwise not good for host/target/cross usage))
$(info [$(if $(and $(wildcard $(MCO_BUILD_PYTHON)),$(wildcard $(shell $(MCO_BUILD_PYTHON) -c "from distutils.sysconfig import get_python_inc;print get_python_inc()")/Python.h)),X, )] Dev. headers are present (otherwise not good for target usage))
$(info )
endif

# if there is no python in the build system or existing python is not good then build one later
MCO_BUILD_PYTHON:=$(or $(if $(and $(wildcard $(MCO_BUILD_PYTHON)),$(findstring 2.7.,$(shell $(MCO_BUILD_PYTHON) -c "from platform import python_version; print python_version()"))),$(MCO_BUILD_PYTHON)),$(MCO_PYTHON_SRC_DIR)/$(MCO_PYTHON_SRC_PKG_DIR)-$(MCO_BUILD_ARCH_SFX)/bin/python2.7)

ifeq (x$(findstring python,$(MCO_DISABLED_TAGS))x,xx)
# Target-side python defs. Use build system python if ...
# ... (1) the target arch signature is the same as the build arch signature or the build system is x86_64 and the target is x86 (x86_64 is capable to run x86 executables)
# ... (2) build systems has python installed 
# ...     (2.1) the build system python matches target's data bus width
# ...     (2.2) python's version is 2.7.x
# ...     (2.3) the build has python dev. package (Python.h header exists)
# ... (3) or the script will build own python for the build system (so version will be 2.7.x and dev. headers present) (takes time)
# ...     (3.1) Databus width of the build system matches to the one on the target
# ... (4) Otherwise the script will build own python for the target (takes time)
MCO_BUILD_PYTHON_VERSION_IS     :=$(strip $(if $(wildcard $(MCO_BUILD_PYTHON)),$(shell $(MCO_BUILD_PYTHON) -c "from platform import python_version; print python_version()")))
MCO_BUILD_PYTHON_VERSION_IS_27x :=$(findstring 2.7.,$(MCO_BUILD_PYTHON_VERSION_IS))
MCO_BUILD_PYTHON_VERSION_IS_2714:=$(findstring 2.7.14,$(MCO_BUILD_PYTHON_VERSION_IS))

MCO_TARGET_PYTHON :=$(strip $(if \
     $(and \
         $(or \
            $(if $(subst $(MCO_BUILD_ARCH_SFX),,$(MCO_TARGET_ARCH_SFX)),,equal1),\
            $(and \
                $(if $(subst x86_64,,$(MCO_BUILD_ARCHID)),,equal2),\
                $(if $(subst x86,,$(MCO_TARGET_ARCHID)),,equal3)\
            )\
         ),\
         $(strip $(if $(wildcard $(MCO_BUILD_PYTHON)),\
             $(and\
                 $(MCO_BUILD_PYTHON_VERSION_IS_27x),\
                 $(shell $(MCO_BUILD_PYTHON) -c 'import sys; print("passed" if sys.maxsize == 2**$(MCO_TARGET_DBUS_WIDTH)/2-1 else "")'),\
                 $(wildcard $(shell $(MCO_BUILD_PYTHON) -c "from distutils.sysconfig import get_python_inc;print get_python_inc()")/Python.h)\
             ),\
             $(if $(subst $(MCO_TARGET_DBUS_WIDTH),,$(MCO_BUILD_DBUS_WIDTH)),,ok)))\
     ),\
     $(MCO_BUILD_PYTHON),\
     $(MCO_PYTHON_SRC_DIR)/$(MCO_PYTHON_SRC_PKG_DIR)-$(MCO_TARGET_ARCH_SFX)/bin/python2.7))

ifeq (x$(MCO_TARGET_PYTHON)x,x$(and $(MCO_BUILD_PYTHON),$(wildcard $(MCO_BUILD_PYTHON)))x)
MCO_TARGET_PYTHON_INCLUDE:=$(shell $(MCO_BUILD_PYTHON) -c "from distutils.sysconfig import get_python_inc;print get_python_inc()")
else
MCO_TARGET_PYTHON_INCLUDE:=$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(MCO_PYTHON_SRC_DIR)/$(MCO_PYTHON_SRC_PKG_DIR)-$(MCO_TARGET_ARCH_SFX))/include/python2.7
endif

# Cross-platform python (<build-arch>+<target-dbus-width>) is necessary if the build system is not capable to run target's python. 
# if <target-python> != <build-python>
#     then check <build-python> version if is 2.7.1.4
#             then if <cross-arch> != <build-arch>
#                     then build <cross-python>
#                     else use <build-python>
#             else build <cross-python>
#     else use <build-python>
MCO_CROSS_PYTHON:=$(strip \
	$(if $(subst $(MCO_TARGET_PYTHON),,$(MCO_BUILD_PYTHON)),\
        $(if $(MCO_BUILD_PYTHON_VERSION_IS_2714),\
             $(if $(subst $(MCO_CROSS_ARCH_SFX),,$(MCO_BUILD_ARCH_SFX)),\
                  $(MCO_PYTHON_SRC_DIR)/$(MCO_PYTHON_SRC_PKG_DIR)-$(MCO_CROSS_ARCH_SFX)/bin/python2.7,\
                  $(MCO_BUILD_PYTHON)\
              ),\
             $(MCO_PYTHON_SRC_DIR)/$(MCO_PYTHON_SRC_PKG_DIR)-$(MCO_CROSS_ARCH_SFX)/bin/python2.7\
         ),\
        $(MCO_BUILD_PYTHON)\
     )\
)

ifneq (x$(findstring python,$(MCO_EXPLAIN_TAGS))x,xx)
$(info BUILD_PYTHON =$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(MCO_BUILD_PYTHON)))
$(info CROSS_PYTHON =$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(MCO_CROSS_PYTHON)))
$(info TARGET_PYTHON=$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(MCO_TARGET_PYTHON)))
$(info TARGET_PYTHON_INCLUDE=$(MCO_TARGET_PYTHON_INCLUDE))
$(info )
endif
else
ifneq (x$(findstring python,$(MCO_EXPLAIN_TAGS))x,xx)
$(info BUILD_PYTHON =$(MCO_BUILD_PYTHON))
$(info )
endif
endif

# Common utilities used in makefiles
mco_makefile_dir = $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
mco_makefile_file= $(addprefix $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/,$(1))
mco_project_dir  = $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
mco_project_file = $(addprefix $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/,$(1))

define mco_uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

# Dump the command and it's output to the console or to the log file depending on the logging setting
# param 1 - project
# param 2 - message
# param 3 - log file
define mco_rule_start
$(if $(findstring full,$(MCO_BUILD_LOG)),,$(if $(findstring off,$(MCO_BUILD_LOG)),,@$(MCO_BUILD_ECHO) "$(2)"))
	$(if $(findstring full,$(MCO_BUILD_LOG)),,$(if $(findstring off,$(MCO_BUILD_LOG)),,$(if $(3),@$(MCO_BUILD_ECHO) "">$(3))))
endef
# param 1 - project
# param 2 - command to run
# param 3 - log file
define mco_rule_run
$(if $(findstring full,$(MCO_BUILD_LOG)),$(2),$(if $(findstring off,$(MCO_BUILD_LOG)),@$(2) $(MCO_BUILD_TONULL) 2>&1,@$(if $(3),$(MCO_BUILD_ECHO) "$(2)">>$(3) && )$(2)$(if $(3),>>$(3),$(MCO_BUILD_TONULL)) 2>&1))
endef
# param 1 - project
define mco_rule_finish
$(if $(findstring off,$(MCO_BUILD_LOG)),@$(MCO_BUILD_ECHO)$(MCO_BUILD_TONULL))
endef

$(if $(findstring on,$(MCO_BUILD_LOG)),$(info Loading cascade make system...))
default: all

