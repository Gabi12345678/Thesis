$(if $(findstring on,$(MCO_BUILD_LOG)),$(info Setting up the build procedure...))

# Global include directory 
MCO_INCLUDE      :=$(MCO_ROOT)/include $(MCO_ROOT)/include/sql $(MCO_ROOT)/include/sql/odbc $(MCO_ROOT)/include/ha $(MCO_ROOT)/include/ws \
                   $(MCO_ROOT)/target/mcolib $(MCO_ROOT)/target/mcosql $(MCO_ROOT)/target/mcoha \
                   $(MCO_ROOT)/target/mcocluster $(MCO_ROOT)/target/mcoiotcomm $(MCO_ROOT)/target/mcorest/rest $(MCO_ROOT)/samples/native/common \
                   $(MCO_ROOT)/target/feedhandler/libfh/include $(MCO_ROOT)/target/feedhandler/libfh/private \
                   $(MCO_ROOT)/target/mcojser $(MCO_ROOT)/tests/thlib
# well-known ODBC projects
MCO_ODBC_PRJS              := mcoodbc mcoodbc_drv

MCO_DISABLED_TAGS_ARCHID:=$(filter-out host-$(MCO_HOST_ARCHID)-only,$(filter-out target-$(MCO_TARGET_ARCHID)-only,$(MCO_DISABLED_TAGS)))

# Disabled projects
MCO_DISABLED_PRJS:=$(strip $(foreach prj,$(MCO_PRJ_LIST),$(if $(filter $(MCO_DISABLED_TAGS_ARCHID),$(PRJ_$(prj)_TAGS)),$(prj))))
MCO_DISABLED_DEFS:=$(strip $(foreach tag,$(MCO_DISABLED_TAGS),$(MCO_EXDB_DISABLE_DEFINES_$(tag))))
MCO_DISABLED_LIBS:=$(strip $(foreach tag,$(MCO_DISABLED_TAGS),$(MCO_EXDB_DISABLE_LIBRARIES_$(tag))))

define DISABLE_PROJECT_FLAGS
$(if $(PRJ_$(1)_$(MCO_EXDB_DISABLE_PRJFLAG_$(2))), PRJ_$(1)_$(MCO_EXDB_DISABLE_PRJFLAG_$(2)):=)
endef
$(foreach prj,$(MCO_PRJ_LIST),$(foreach tag,$(MCO_DISABLED_TAGS),$(eval $(call DISABLE_PROJECT_FLAGS,$(prj),$(tag)))))

# Collect project disabled for automatic build (build by reference only)
MCO_SKIP_AUTO_BUILD_PRJS:=$(strip $(foreach prj,$(MCO_PRJ_LIST),$(if $(PRJ_$(prj)_SKIP_AUTO_BUILD),$(prj))))

# Filter-out disabled projects
MCO_PRJ_LIST_ENABLED           :=$(filter-out $(MCO_DISABLED_PRJS),$(MCO_PRJ_LIST))
PRJ_CUSTOM                     :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_CUSTOM))
PRJ_HOST_EXECUTABLE            :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_HOST_EXECUTABLE))
PRJ_TARGET_EXECUTABLE_DPTR     :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_EXECUTABLE_DPTR))
PRJ_TARGET_EXECUTABLE_OFFS     :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_EXECUTABLE_OFFS))
PRJ_TARGET_LIBRARY_DPTR_STATIC :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_LIBRARY_DPTR_STATIC))
PRJ_TARGET_LIBRARY_DPTR_DYNAMIC:=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_LIBRARY_DPTR_DYNAMIC))
PRJ_TARGET_LIBRARY_OFFS_STATIC :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_LIBRARY_OFFS_STATIC))
PRJ_TARGET_LIBRARY_OFFS_DYNAMIC:=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_LIBRARY_OFFS_DYNAMIC))
PRJ_TARGET_JAR                 :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_JAR))
PRJ_TARGET_CLASS               :=$(filter-out $(MCO_DISABLED_PRJS) $(MCO_SKIP_AUTO_BUILD_PRJS),$(PRJ_TARGET_CLASS))

# Leave only projects from the current source tree level and the levels down below (all targets required by the projects will be build automatically)
mco_filter_current_level_and_deeper=$(if $(wildcard $(CURDIR)$(MCO_BUILD_SEP)/$(subst $(CURDIR)$(MCO_BUILD_SEP),,$(PRJ_$(1)_DIR)$(MCO_BUILD_SEP))),$(1))
BUILD_CUSTOM                     :=$(foreach prj,$(PRJ_CUSTOM),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_HOST_EXECUTABLE            :=$(foreach prj,$(PRJ_HOST_EXECUTABLE),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_EXECUTABLE_DPTR     :=$(foreach prj,$(PRJ_TARGET_EXECUTABLE_DPTR),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_EXECUTABLE_OFFS     :=$(foreach prj,$(PRJ_TARGET_EXECUTABLE_OFFS),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_LIBRARY_DPTR_STATIC :=$(foreach prj,$(PRJ_TARGET_LIBRARY_DPTR_STATIC),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_LIBRARY_DPTR_DYNAMIC:=$(foreach prj,$(PRJ_TARGET_LIBRARY_DPTR_DYNAMIC),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_LIBRARY_OFFS_STATIC :=$(foreach prj,$(PRJ_TARGET_LIBRARY_OFFS_STATIC),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_LIBRARY_OFFS_DYNAMIC:=$(foreach prj,$(PRJ_TARGET_LIBRARY_OFFS_DYNAMIC),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_JAR                 :=$(foreach prj,$(PRJ_TARGET_JAR),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_TARGET_CLASS               :=$(foreach prj,$(PRJ_TARGET_CLASS),$(call mco_filter_current_level_and_deeper,$(prj)))
BUILD_PRJ_LIST                   :=$(foreach prj,$(MCO_PRJ_LIST_ENABLED),$(call mco_filter_current_level_and_deeper,$(prj)))

# TODO: Obsolete make interface (used for tests only, not suitable for parallel build need to be removed completely)
# PRJ_TYPE       - type of the project, APP or LIB or NONE to prevent build
# PRJ_TITLE      - name of the sample, name of an executable file
ifeq "$(PRJ_TYPE)" "APP"
# add mcoodbc and mcoodbc_drv forcefully to maintain right libaries location
MCO_PRJ_LIST               +=$(PRJ_TITLE)
BUILD_PRJ_LIST             :=$(PRJ_TITLE)
PRJ_$(PRJ_TITLE)_TAGS      :=tests $(TEST_TAGS) $(if $(findstring odbc,$(TEST_TAGS)),,$(if $(findstring YES,$(PRJ_F_ODBC)),odbc))
PRJ_$(PRJ_TITLE)_RUNTIME   :=YES

# PRJ_SHOW_BUILD - Show build process, YES or NO or AUTO
MCO_BUILD_LOG              :=$(if $(findstring undefined,$(origin SHOW_BUILD_LOG)),$(if $(findstring on,$(PRJ_SHOW_BUILD)),full,on),$(if $(findstring on,$(SHOW_BUILD_LOG)),full,on))

# PRJ_DIRECTPTR  - Direct Pointers feature, YES or NO or AUTO
PRJ_TARGET_EXECUTABLE_DPTR   :=$(if $(findstring YES,$(PRJ_DIRECTPTR)),$(PRJ_TITLE))
PRJ_TARGET_EXECUTABLE_OFFS   :=$(if $(findstring YES,$(PRJ_DIRECTPTR)),,$(PRJ_TITLE))
BUILD_TARGET_EXECUTABLE_DPTR :=$(if $(findstring YES,$(PRJ_DIRECTPTR)),$(PRJ_TITLE))
BUILD_TARGET_EXECUTABLE_OFFS :=$(if $(findstring YES,$(PRJ_DIRECTPTR)),,$(PRJ_TITLE))

# PRJ_DBNAMES    - names of the sample's databases
PRJ_$(PRJ_TITLE)_DATABASES :=$(if $(findstring mcoperf.mco,$(PRJ_SCHEMAS)),mcoperfmon_db )$(subst mcoperfmon_db,,$(PRJ_DBNAMES))

# PRJ_SCHEMAS    - database schema files
PRJ_$(PRJ_TITLE)_SCHEMAS   :=$(if $(findstring mcoperf.mco,$(PRJ_SCHEMAS)),$(MCO_PUBLIC_INCLUDE_DIR)/mcoperf.mco )$(addprefix $(CURDIR)/,$(subst mcoperf.mco,,$(PRJ_SCHEMAS)))

# PRJ_MEMDEV     - memory access device, CONV(PRIVATE?) or SHARED or NONE
PRJ_$(PRJ_TITLE)_MEMDEV    :=$(if $(findstring CONV,$(PRJ_MEMDEV)),PRIVATE,$(if $(findstring PRIVATE,$(PRJ_MEMDEV)),PRIVATE,$(if $(findstring SHARED,$(PRJ_MEMDEV)),SHARED)))

# PRJ_STORAGE    - runtime configuration, MEMORY for inmem or PERSISTENT or NONE for samples without a DB
PRJ_$(PRJ_TITLE)_STORAGE   :=$(if $(findstring NONE,$(PRJ_STORAGE)),,$(if $(findstring MEMORY,$(PRJ_STORAGE)),TRANSIENT,$(subst ,,$(PRJ_STORAGE))))

# PRJ_STORAGEDEV - storage device, AUTO or NONE or platform-dependent name (see mcof* libraries)
PRJ_$(PRJ_TITLE)_STORAGEDEV:=$(if $(findstring NONE,$(PRJ_STORAGEDEV)),,$(subst ,,$(PRJ_STORAGEDEV)))

# PRJ_TRANSMGR   - transaction manager, MURSIW or MVCC or NONE
PRJ_$(PRJ_TITLE)_TRANSMGR  :=$(if $(findstring undefined,$(origin PRJ_TRANSMGR)),,$(PRJ_TRANSMGR))

# PRJ_SYNC       - sync. primitives, AUTO or NONE or platform-dependent name (see mcos* libraries)
PRJ_$(PRJ_TITLE)_SYNC      :=$(if $(findstring NONE,$(PRJ_SYNC)),,$(PRJ_SYNC))

# PRJ_F_CLUSTER  - Cluster feature, TCP or NO
PRJ_$(PRJ_TITLE)_F_CLUSTER :=$(if $(findstring undefined,$(origin PRJ_F_CLUSTER)),,$(if $(findstring NO,$(PRJ_F_CLUSTER)),,$(PRJ_F_CLUSTER)))

# PRJ_F_SEQUENCES- Sequences feature, YES (ordinary version), RLE or NO
PRJ_$(PRJ_TITLE)_F_SEQUENCES:=$(if $(findstring undefined,$(origin PRJ_F_SEQUENCES)),,$(if $(findstring NO,$(PRJ_F_SEQUENCES)),,$(PRJ_F_SEQUENCES)))

# PRJ_F_SQL      - SQL feature, LOCAL, REMOTE or NO
PRJ_$(PRJ_TITLE)_F_SQL     :=$(if $(findstring undefined,$(origin PRJ_F_SQL)),,$(if $(findstring NO,$(PRJ_F_SQL)),,$(PRJ_F_SQL)))

# PRJ_HA         - HA feature, TCP or UDP or PIPES or NONE
PRJ_$(PRJ_TITLE)_F_HA      :=$(if $(findstring undefined,$(origin PRJ_HA)),,$(if $(findstring NONE,$(PRJ_HA)),,$(PRJ_HA)))

# PRJ_F_LOG      - LOG feature, YES or NO
PRJ_$(PRJ_TITLE)_F_LOG     :=$(if $(findstring undefined,$(origin PRJ_F_LOG)),,$(if $(findstring NO,$(PRJ_F_LOG)),,YES))

# PRJ_F_SERI     - Serialization feature, YES or NO
PRJ_$(PRJ_TITLE)_F_SERI    :=$(if $(findstring undefined,$(origin PRJ_F_SERI)),,$(if $(findstring NO,$(PRJ_F_SERI)),,YES))

# PRJ_F_DBCALC   - DBCALC feature, YES or NO
PRJ_$(PRJ_TITLE)_F_DBCALC  :=$(if $(findstring undefined,$(origin PRJ_F_DBCALC)),,$(if $(findstring NO,$(PRJ_F_DBCALC)),,YES))

# PRJ_F_UDA      - UDA feature, YES, MCOAPI or NO
PRJ_$(PRJ_TITLE)_F_UDA     :=$(if $(findstring undefined,$(origin PRJ_F_UDA)),,$(if $(findstring NO,$(PRJ_F_UDA)),,YES))

# PRJ_F_XML      - XML feature, YES or NO
PRJ_$(PRJ_TITLE)_F_XML     :=$(if $(findstring undefined,$(origin PRJ_F_XML)),,$(if $(findstring NO,$(PRJ_F_XML)),,YES))

# PRJ_F_JSER     - JSON serialization feature, YES or NO
PRJ_$(PRJ_TITLE)_F_JSER    :=$(if $(findstring undefined,$(origin PRJ_F_JSER)),,$(if $(findstring NO,$(PRJ_F_JSER)),,YES))

# PRJ_F_CPP      - generate C++ interface, YES or NO
PRJ_$(PRJ_TITLE)_F_CPP     :=$(if $(findstring undefined,$(origin PRJ_F_CPP)),,$(if $(findstring NO,$(PRJ_F_CPP)),,YES))

# PRJ_F_HVPERFMON- Support for permon in HttpViewer

# PRJ_F_HV       - HttpViewer feature, YES or NO
PRJ_$(PRJ_TITLE)_F_HV      :=$(if $(findstring undefined,$(origin PRJ_F_HV)),,$(if $(findstring NO,$(PRJ_F_HV)),,YES))

# PRJ_F_PERFMON  - Performance monitor support
PRJ_$(PRJ_TITLE)_F_PERFMON :=$(if $(findstring undefined,$(origin PRJ_F_PERFMON)),,$(if $(findstring NO,$(PRJ_F_PERFMON)),,YES))

# PRJ_RMFILES    - list of files to remove for distclean command except executable, database and generated files

# PRJ_F_BACKUP
PRJ_$(PRJ_TITLE)_F_BACKUP  :=$(if $(findstring undefined,$(origin PRJ_F_BACKUP)),,$(if $(findstring NO,$(PRJ_F_BACKUP)),,YES))

# PRJ_F_CRYPT
PRJ_$(PRJ_TITLE)_F_CRYPT   :=$(if $(findstring undefined,$(origin PRJ_F_CRYPT)),,$(if $(findstring NO,$(PRJ_F_CRYPT)),,YES))

# PRJ_F_IOT
PRJ_$(PRJ_TITLE)_F_IOT     :=$(if $(findstring undefined,$(origin PRJ_F_IOT)),,$(if $(findstring NO,$(PRJ_F_IOT)),,YES))

# PRJ_F_MCOCOMP
PRJ_$(PRJ_TITLE)_F_MCOCOMP :=$(if $(findstring undefined,$(origin PRJ_F_MCOCOMP)),,$(if $(findstring NO,$(PRJ_F_MCOCOMP)),,YES))

# PRJ_F_NATIVE
PRJ_$(PRJ_TITLE)_F_NATIVE  :=$(if $(findstring undefined,$(origin PRJ_F_NATIVE)),,$(if $(findstring NO,$(PRJ_F_NATIVE)),,YES))

# PRJ_F_ODBC
PRJ_$(PRJ_TITLE)_F_ODBC    :=$(if $(findstring undefined,$(origin PRJ_F_ODBC)),,$(if $(findstring NO,$(PRJ_F_ODBC)),,YES))

# PRJ_F_ODBCDRV
PRJ_$(PRJ_TITLE)_F_ODBCDRV :=$(if $(findstring undefined,$(origin PRJ_F_ODBCDRV)),,$(if $(findstring NO,$(PRJ_F_ODBCDRV)),,YES))

# PRJ_F_USE_DYNAMIC_LIBRARIES
PRJ_$(PRJ_TITLE)_F_USE_DYNAMIC_LIBRARIES :=$(if $(findstring undefined,$(origin PRJ_F_USE_DYNAMIC_LIBRARIES)),,$(if $(findstring NO,$(PRJ_F_USE_DYNAMIC_LIBRARIES)),,YES))

# PRJ_RUNTIME
PRJ_$(PRJ_TITLE)_RUNTIME :=$(if $(findstring undefined,$(origin PRJ_RUNTIME)),YES,$(if $(findstring YES,$(PRJ_RUNTIME)),YES,))

# PRJ_F_REST
PRJ_$(PRJ_TITLE)_F_REST    :=$(if $(findstring undefined,$(origin PRJ_F_REST)),,$(if $(findstring NO,$(PRJ_F_REST)),,YES))

# PRJ_F_SMARTPTR
PRJ_$(PRJ_TITLE)_F_SMARTPTR:=$(if $(findstring undefined,$(origin PRJ_F_SMARTPTR)),,$(if $(findstring NO,$(PRJ_F_SMARTPTR)),,YES))

# PRJ_SUFFIX
#PRJ_$(PRJ_TITLE)_RESULT_NAME_BASE:=$(if $(PRJ_SUFFIX),$(PRJ_TITLE)_$(PRJ_SUFFIX))

# PRJ_MCOCOMPFLAGS - custom mcocomp flags
PRJ_$(PRJ_TITLE)_MCOCOMPFLAGS :=$(if $(findstring undefined,$(origin PRJ_MCOCOMPFLAGS)),,$(PRJ_MCOCOMPFLAGS))

# USERLIB
PRJ_$(PRJ_TITLE)_ADDITIONAL_LIBRARIES:=$(USERLIB)

# PRJ_SOURCES    - source files except generated
PRJ_$(PRJ_TITLE)_SRC       :=$(addprefix $(CURDIR)/,$(PRJ_SOURCES)) $(if $(findstring file,$(origin TEST_COMMENT)),$(MCO_ROOT)/tests/thlib/thlib.c,$(MCO_SAMPLES_COMMON_DIR)/common.c $(if $(PRJ_$(PRJ_TITLE)_F_SQL),$(MCO_SAMPLES_COMMON_DIR)/commonSQL.c) $(if $(PRJ_$(PRJ_TITLE)_F_CLUSTER),$(MCO_SAMPLES_COMMON_DIR)/commoncl.c) $(if $(PRJ_$(PRJ_TITLE)_F_HA),$(MCO_SAMPLES_COMMON_DIR)/commonha.c))

PRJ_$(PRJ_TITLE)_DIR       :=$(CURDIR)

PRJ_$(PRJ_TITLE)_INCLUDE   :=$(PRJ_INCLUDE)
endif

ifneq "$(PRJ_TYPE)" "APP"
# TestSuite control overrides
# param 1 - executable project name
define APPLY_TESTSUITE_BUILD_CONTROLS
$(if $(findstring undefined,$(origin PRJ_STORAGE))    ,,PRJ_$(1)_STORAGE:=$(if $(findstring PERSISTENT,$(PRJ_STORAGE)),PERSISTENT)$(if $(findstring MEMORY,$(PRJ_STORAGE)),TRANSIENT)$(if $(findstring NONE,$(PRJ_STORAGE)),))
$(if $(findstring undefined,$(origin PRJ_STORAGE))    ,,PRJ_$(1)_F_MCOCOMP_FORCE:=$(if $(findstring PERSISTENT,$(PRJ_STORAGE)),PERSISTENT)$(if $(findstring TRANSIENT,$(PRJ_STORAGE)),TRANSIENT)$(if $(findstring NONE,$(PRJ_STORAGE)),))
$(if $(findstring undefined,$(origin PRJ_STORAGE))    ,,PRJ_$(1)_STORAGEDEV:=$(if $(findstring NONE,$(PRJ_STORAGE)),,AUTO))
$(if $(findstring undefined,$(origin PRJ_MEMDEV))     ,,PRJ_$(1)_MEMDEV:=$(if $(findstring CONV,$(PRJ_MEMDEV)),PRIVATE,SHARED))
$(if $(findstring undefined,$(origin PRJ_TRANSMGR))   ,,PRJ_$(1)_TRANSMGR:=$(PRJ_TRANSMGR))
$(if $(findstring undefined,$(origin PRJ_F_SEQUENCES)),,PRJ_$(1)_F_SEQUENCES:=$(if $(findstring NO,$(PRJ_F_SEQUENCES)),)$(if $(findstring RLE,$(PRJ_F_SEQUENCES)),RLE)$(if $(findstring YES,$(PRJ_F_SEQUENCES)),YES))
$(if $(findstring undefined,$(origin PRJ_DIRECTPTR)),,BUILD_TARGET_EXECUTABLE_DPTR:=$(subst $(1),,$(BUILD_TARGET_EXECUTABLE_DPTR))$(if $(findstring YES,$(PRJ_DIRECTPTR)), $(1)))
$(if $(findstring undefined,$(origin PRJ_DIRECTPTR)),,  PRJ_TARGET_EXECUTABLE_DPTR:=$(subst $(1),,$(  PRJ_TARGET_EXECUTABLE_DPTR))$(if $(findstring YES,$(PRJ_DIRECTPTR)), $(1)))
$(if $(findstring undefined,$(origin PRJ_DIRECTPTR)),,BUILD_TARGET_EXECUTABLE_OFFS:=$(subst $(1),,$(BUILD_TARGET_EXECUTABLE_OFFS))$(if $(findstring  NO,$(PRJ_DIRECTPTR)), $(1)))
$(if $(findstring undefined,$(origin PRJ_DIRECTPTR)),,  PRJ_TARGET_EXECUTABLE_OFFS:=$(subst $(1),,$(  PRJ_TARGET_EXECUTABLE_OFFS))$(if $(findstring  NO,$(PRJ_DIRECTPTR)), $(1)))
$(if $(findstring undefined,$(origin PRJ_SUFFIX))     ,,PRJ_$(1)_RESULT_NAME_BASE:=$(1)_$(PRJ_SUFFIX))
$(if $(findstring undefined,$(origin TEST_NAME))      ,,PRJ_$(1)_TAGS            :=test $(TEST_TAGS) $(PRJ_$(1)_TAGS) $(if $(findstring odbc,$(TEST_TAGS)),,$(if $(findstring YES,$(PRJ_F_ODBC)),odbc)))
$(if $(findstring undefined,$(origin PRJ_MCOCOMPFLAGS)),,PRJ_$(1)_MCOCOMPFLAGS:=$(PRJ_MCOCOMPFLAGS))
endef
$(foreach prj,$(BUILD_TARGET_EXECUTABLE_DPTR) $(BUILD_TARGET_EXECUTABLE_OFFS),$(eval $(call APPLY_TESTSUITE_BUILD_CONTROLS,$(prj))))
endif

# Separate ODBC-related projects
MCO_ODBC_PRJS:=$(strip $(MCO_ODBC_PRJS) $(filter-out $(MCO_ODBC_PRJS),$(foreach prj,$(MCO_PRJ_LIST),$(if $(filter odbc,$(PRJ_$(prj)_TAGS)),$(prj)))))
MCO_ODBC_TARGETS:=\
$(foreach limtype, $(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),wevl) nevl,$(foreach dbgtype,wdbg ndbg,$(foreach modtype,$(MCO_ENABLE_STATIC_BINARIES) $(MCO_ENABLE_DYNAMIC_BINARIES),$(foreach rttype,OFFS DPTR,$(foreach i,$(strip $(filter $(MCO_ODBC_PRJS),$(BUILD_TARGET_LIBRARY_$(rttype)_$(modtype)))),$(addsuffix $(if $(findstring wdbg,$(dbgtype)),_debug)$(if $(findstring STATIC,$(modtype)),$(MCO_TARGET_LIB_STAT_SFX),$(MCO_TARGET_LIB_DYNM_SFX)),$(addprefix $(MCO_ROOT)/odbc/bin$(if $(findstring DPTR,$(rttype)),.dptr)$(if $(findstring DYNAMIC,$(modtype)),.so)$(if $(findstring wevl,$(limtype)),.eval)/lib,$(or $(PRJ_$(i)_RESULT_NAME_BASE),$(i)))))))))
MCO_ODBC_SAMPLES:=\
$(foreach rttype,OFFS DPTR,$(addprefix $(MCO_ROOT)/odbc/bin$(if $(findstring DPTR,$(rttype)),.dptr)/,$(strip $(foreach prj,$(filter $(MCO_ODBC_PRJS),$(BUILD_TARGET_EXECUTABLE_$(rttype))),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj))))))

# Make build targets
PRJ_ALL_BUILD_TARGETS:=$(subst $(MCO_ROOT),$(MCO_PRODUCT_ROOT),$(strip $(foreach limtype,$(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),wevl) nevl,$(foreach rttype,OFFS DPTR,$(addprefix $(MCO_ROOT)/target/bin$(if $(findstring DPTR,$(rttype)),.dptr)$(if $(findstring wevl,$(limtype)),.eval)/,$(strip $(foreach prj,$(filter-out $(MCO_ODBC_PRJS),$(BUILD_TARGET_EXECUTABLE_$(rttype))),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj))))))) $(addprefix $(MCO_ROOT)/host/bin/,$(strip $(foreach prj,$(BUILD_HOST_EXECUTABLE),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj))))) $(addprefix $(MCO_ROOT)/target/bin/,$(addsuffix .jar,$(strip $(foreach prj,$(BUILD_TARGET_JAR),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj)))))) $(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),$(addprefix $(MCO_ROOT)/target/bin.eval/,$(addsuffix .jar,$(strip $(foreach prj,$(BUILD_TARGET_JAR),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj))))))) $(foreach limtype, $(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),wevl) nevl,$(foreach dbgtype,wdbg ndbg,$(foreach modtype,$(MCO_ENABLE_STATIC_BINARIES) $(MCO_ENABLE_DYNAMIC_BINARIES),$(foreach rttype,OFFS DPTR,$(foreach i,$(strip $(filter-out $(MCO_ODBC_PRJS),$(BUILD_TARGET_LIBRARY_$(rttype)_$(modtype)))),$(addsuffix $(if $(findstring wdbg,$(dbgtype)),_debug)$(if $(findstring STATIC,$(modtype)),$(MCO_TARGET_LIB_STAT_SFX),$(MCO_TARGET_LIB_$(if $(findstring extremedbjni,$(i)),JNI,DYNM)_SFX)),$(addprefix $(MCO_ROOT)/target/bin$(if $(findstring DPTR,$(rttype)),.dptr)$(if $(findstring DYNAMIC,$(modtype)),.so)$(if $(findstring wevl,$(limtype)),.eval)/lib,$(or $(PRJ_$(i)_RESULT_NAME_BASE),$(i))))))))) $(foreach cl,$(strip $(BUILD_TARGET_CLASS)),$(PRJ_$(cl)_DIR)/$(or $(PRJ_$(cl)_RESULT_NAME_BASE),$(cl)).class) $(foreach prj,$(BUILD_CUSTOM),$(PRJ_$(prj)_PRODUCTS)) $(MCO_ODBC_TARGETS) $(MCO_ODBC_SAMPLES) ))

BUILD_TARGET_EXECUTABLE_DPTR_TARGET_DIR :=$(subst  ,,$(foreach prj,$(BUILD_TARGET_EXECUTABLE_DPTR),$(if $(findstring $(PRJ_$(prj)_DIR),$(subst /target/,@,$(PRJ_$(prj)_DIR))),,$(prj))))
BUILD_TARGET_EXECUTABLE_OFFS_TARGET_DIR :=$(subst  ,,$(foreach prj,$(BUILD_TARGET_EXECUTABLE_OFFS),$(if $(findstring $(PRJ_$(prj)_DIR),$(subst /target/,@,$(PRJ_$(prj)_DIR))),,$(prj))))
BUILD_TARGET_EXECUTABLE_DPTR_SAMPLES_DIR:=$(subst  ,,$(foreach prj,$(BUILD_TARGET_EXECUTABLE_DPTR),$(if $(findstring $(PRJ_$(prj)_DIR),$(subst /samples/,@,$(PRJ_$(prj)_DIR))),,$(prj))))
BUILD_TARGET_EXECUTABLE_OFFS_SAMPLES_DIR:=$(subst  ,,$(foreach prj,$(BUILD_TARGET_EXECUTABLE_OFFS),$(if $(findstring $(PRJ_$(prj)_DIR),$(subst /samples/,@,$(PRJ_$(prj)_DIR))),,$(prj))))

BUILD_CUSTOM_TARGET_DIR                 :=$(subst  ,,$(foreach prj,$(BUILD_CUSTOM),$(if $(findstring $(PRJ_$(prj)_DIR),$(subst /target/,@,$(PRJ_$(prj)_DIR))),,$(PRJ_$(prj)_PRODUCTS))))
BUILD_CUSTOM_HOST_DIR                   :=$(subst  ,,$(foreach prj,$(BUILD_CUSTOM),$(if $(findstring $(PRJ_$(prj)_DIR),$(subst /host/,@,$(PRJ_$(prj)_DIR))),,$(PRJ_$(prj)_PRODUCTS))))
BUILD_CUSTOM_SAMPLES_DIR                :=$(subst  ,,$(foreach prj,$(BUILD_CUSTOM),$(if $(findstring $(PRJ_$(prj)_DIR),$(subst /samples/,@,$(PRJ_$(prj)_DIR))),,$(PRJ_$(prj)_PRODUCTS))))

PRJ_HOST_BUILD_TARGETS  :=$(subst $(MCO_ROOT),$(MCO_PRODUCT_ROOT),$(strip $(addprefix $(MCO_ROOT)/host/bin/,$(strip $(foreach prj,$(BUILD_HOST_EXECUTABLE),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj)))) $(BUILD_CUSTOM_HOST_DIR))))

PRJ_TARGET_BUILD_TARGETS:=$(subst $(MCO_ROOT),$(MCO_PRODUCT_ROOT),$(strip $(addprefix $(MCO_ROOT)/target/bin/,$(addsuffix .jar,$(strip $(foreach prj,$(BUILD_TARGET_JAR),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj)))))) $(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),$(addprefix $(MCO_ROOT)/target/bin.eval/,$(addsuffix .jar,$(strip $(foreach prj,$(BUILD_TARGET_JAR),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj))))))) $(foreach limtype, $(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),wevl) nevl,$(foreach dbgtype,wdbg ndbg,$(foreach modtype,$(MCO_ENABLE_STATIC_BINARIES) $(MCO_ENABLE_DYNAMIC_BINARIES),$(foreach rttype,OFFS DPTR,$(foreach i,$(strip $(filter-out $(MCO_ODBC_PRJS),$(BUILD_TARGET_LIBRARY_$(rttype)_$(modtype)))),$(addsuffix $(if $(findstring wdbg,$(dbgtype)),_debug)$(if $(findstring STATIC,$(modtype)),$(MCO_TARGET_LIB_STAT_SFX),$(or $(if $(findstring extremedbjni,$(i)),$(MCO_TARGET_LIB_JNI_SFX)),$(MCO_TARGET_LIB_DYNM_SFX))),$(addprefix $(MCO_ROOT)/target/bin$(if $(findstring DPTR,$(rttype)),.dptr)$(if $(findstring DYNAMIC,$(modtype)),.so)$(if $(findstring wevl,$(limtype)),.eval)/lib,$(or $(PRJ_$(i)_RESULT_NAME_BASE),$(i))))))))) $(BUILD_CUSTOM_TARGET_DIR)) $(strip $(foreach rttype,OFFS DPTR,$(addprefix $(MCO_ROOT)/target/bin$(if $(findstring DPTR,$(rttype)),.dptr)/,$(foreach prj,$(filter-out $(MCO_ODBC_PRJS),$(BUILD_TARGET_EXECUTABLE_$(rttype)_TARGET_DIR)),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj)))))) $(MCO_ODBC_TARGETS))

PRJ_SAMPLES_BUILD_TARGETS:=$(subst $(MCO_ROOT),$(MCO_PRODUCT_ROOT),$(strip $(foreach rttype,OFFS DPTR,$(addprefix $(MCO_ROOT)/target/bin$(if $(findstring DPTR,$(rttype)),.dptr)/,$(foreach prj,$(filter-out $(MCO_ODBC_PRJS),$(BUILD_TARGET_EXECUTABLE_$(rttype)_SAMPLES_DIR)),$(or $(PRJ_$(prj)_RESULT_NAME_BASE),$(prj))))) $(foreach cl,$(strip $(BUILD_TARGET_CLASS)),$(PRJ_$(cl)_DIR)/$(or $(PRJ_$(cl)_RESULT_NAME_BASE),$(cl)).class) $(BUILD_CUSTOM_SAMPLES_DIR)) $(MCO_ODBC_SAMPLES))

.PHONY: all clean distclean host target samples

define ANALYZE_PROJECT_PROPS
# param 1 - project name
# param 2 - object directory suffix
# param 3 - host/target

# assigne intermediate directory
PRJ_$(1)$(2)_INTERMEDIATE_DIR := $(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/obj-$(1)$(2)

# PRJ_<prj>_DEBUG     - use debug libraries for the executable/shared module
# YES                 -> link debug version of eXtremeDB libraries
# <not set>           -> link non-debug version of eXtremeDB libraries
$(if $(findstring undefined, $(origin MCO_USE_DEBUG_$(PRJ_$(1)_DEBUG))),$(error Variable PRJ_$(1)_DEBUG. Invalid value "$(PRJ_$(1)_DEBUG)"))

# PRJ_<prj>_STORAGE - Storage
# TRANSIENT           -> mcovtmem
# PERSISTENT          -> mcovtdsk 
# <not set>           -> no storage lib on the linker line 
$(if $(findstring undefined, $(origin MCO_VT_$(PRJ_$(1)_STORAGE))),$(error Variable PRJ_$(1)_STORAGE. Invalid value "$(PRJ_$(1)_STORAGE)"))
PRJ_$(1)$(2)_LIBS_VT:=$(MCO_VT_$(PRJ_$(1)_STORAGE))
# TODO: get rid of following, need for backward compatibility only
PRJ_$(1)$(2)_FLAGS_C_VT  :=$(if $(findstring PERSISTENT,$(PRJ_$(1)_STORAGE)),DISK_DATABASE)
PRJ_$(1)$(2)_FLAGS_CPP_VT:=$(if $(findstring PERSISTENT,$(PRJ_$(1)_STORAGE)),DISK_DATABASE)

# PRJ_<prj>_MEMDEV - Memory device
# PRIVATE             -> default setting for platform+private memory driver
# SHARED              -> default setting for platform+shared  memory driver
# <other>             -> put the value to the linker line as the memory driver
# <not set>           -> no memory driver on the linker line
PRJ_$(1)$(2)_LIBS_MEMDEV:=$(if $(PRJ_$(1)_MEMDEV),$(if $(findstring undefined,$(origin MCO_$(3)_DEFAULT_MEMDEV_$(PRJ_$(1)_MEMDEV))),$(PRJ_$(1)_MEMDEV),$(MCO_$(3)_DEFAULT_MEMDEV_$(PRJ_$(1)_MEMDEV))))

# PRJ_<prj>_STORAGEDEV - Storage device
# AUTO                -> default fs wrapper for the platform
# <other>             -> put the value to the linker line as the fs driver
# <not set>           -> no storage driver on the linker line
PRJ_$(1)$(2)_LIBS_STRDEV:=$(if $(and $(PRJ_$(1)_STORAGEDEV),$(or $(findstring PERSISTENT,$(PRJ_$(1)_STORAGE)),$(findstring YES,$(PRJ_$(1)_F_FHLIB)),$(findstring YES,$(PRJ_$(1)_F_LOG)))),$(if $(findstring AUTO,$(PRJ_$(1)_STORAGEDEV)),$(MCO_$(3)_DEFAULT_STRDEV),$(PRJ_$(1)_STORAGEDEV)))

# PRJ_<prj>_SYNC - Sync. implementation library
# AUTO                -> default setting for platform+PRJ_<prj>_MEMDEV sync. implementation library
# <other>             -> put the value to the linker line as the sync. implementation library
# <not set>           -> no sync. implementation library on the linker line
PRJ_$(1)$(2)_LIBS_SYNC:=$(if $(PRJ_$(1)_SYNC),$(if $(findstring AUTO,$(PRJ_$(1)_SYNC)),$(MCO_$(3)_DEFAULT_SYNC_$(PRJ_$(1)_MEMDEV)),$(PRJ_$(1)_SYNC)))

# PRJ_<prj>_TRANSMGR - Transaction Manager
# MURSIW          -> MURSIW TM implemenation library 
# MURSIW_READ     -> Read-optimized MURSIW TM implemenation library 
# MVCC            -> MVCC TM implemenation library
# EXCLUSIVE       -> Exclusive TM implemenation library
# <not set>       -> no TM library on the linker line
$(if $(findstring undefined, $(origin MCO_TM_$(PRJ_$(1)_TRANSMGR))),$(error Variable PRJ_$(1)_TRANSMGR. Invalid value "$(PRJ_$(1)_TRANSMGR)"))
PRJ_$(1)$(2)_LIBS_TMGR:=$(MCO_TM_$(PRJ_$(1)_TRANSMGR))

# PRJ_<prj>_F_SEQUENCES - Enable sequences support
# YES           -> Enable sequences support
# RLE           -> Enable sequences support (RLE)
# <not set>     -> Disable sequences support
$(if $(findstring undefined, $(origin MCO_SEQ_$(PRJ_$(1)_F_SEQUENCES))),$(error Variable PRJ_$(1)_F_SEQUENCES. Invalid value "$(PRJ_$(1)_F_SEQUENCES)"))
PRJ_$(1)$(2)_LIBS_SEQUENCES:=$(MCO_SEQ_$(PRJ_$(1)_F_SEQUENCES))

# PRJ_<prj>_F_SQL - SQL setting 
# LOCAL               -> local SQL engine
# REMOTE              -> RSQL engine + default socket library for the platform
# REMOTE_STANDALONE   -> RSQL client engine only + default socket library for the platform
# <not set>           -> no SQL feature on the linker line
$(if $(findstring undefined, $(origin MCO_SQL_$(PRJ_$(1)_F_SQL))),$(error Variable PRJ_$(1)_F_SQL. Invalid value "$(PRJ_$(1)_F_SQL)"))
PRJ_$(1)$(2)_LIBS_SQL:=$(MCO_SQL_$(PRJ_$(1)_F_SQL))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_SQL:=$(MCO_SQL_MCOCOMPF_$(PRJ_$(1)_F_SQL))

# PRJ_<prj>_F_HA - High Availaibility setting
# TCP, UDP, PIPES     -> set NW implementation library + default socket library for the platform
# <not set>           -> no HA feature on the linker line
$(if $(findstring undefined, $(origin MCO_HA_$(firstword $(PRJ_$(1)_F_HA)))),$(error Variable PRJ_$(1)_F_HA. Invalid value "$(firstword $(PRJ_$(1)_F_HA))"))
PRJ_$(1)$(2)_FLAGS_C_HA  :=$(MCO_HA_CFLAGS_$(firstword $(PRJ_$(1)_F_HA)))
PRJ_$(1)$(2)_FLAGS_CPP_HA:=$(MCO_HA_CFLAGS_$(firstword $(PRJ_$(1)_F_HA)))
$(eval PRJ_$(1)$(2)_LIBS_HA_TMP :=$(foreach c,$(PRJ_$(1)_F_HA),$(MCO_HA_$(c))))
PRJ_$(1)$(2)_LIBS_HA     :=$(if $(findstring @mcohalib@,$(PRJ_$(1)$(2)_LIBS_HA_TMP)),$(MCO_mcohalib)) $(subst @mcohalib@,,$(subst @socketlib@,,$(PRJ_$(1)$(2)_LIBS_HA_TMP)))$(if $(findstring @socketlib@,$(PRJ_$(1)$(2)_LIBS_HA_TMP)), @socketlib@)

# PRJ_<prj>_F_LOG - Enable TL feature
# YES                 -> enable TL
# <not set>           -> no TL feature on the linker line
$(if $(findstring undefined, $(origin MCO_TL_$(PRJ_$(1)_F_LOG))),$(error Variable PRJ_$(1)_F_LOG. Invalid value "$(PRJ_$(1)_F_LOG)"))
PRJ_$(1)$(2)_LIBS_LOG    :=$(MCO_TL_$(PRJ_$(1)_F_LOG))

# PRJ_<prg>_F_SERI - Enable serialization libraries
# YES                 -> enable serialization
# <not set>           -> no serialization libs on the linker line
$(if $(findstring undefined, $(origin MCO_SERI_$(PRJ_$(1)_F_SERI))),$(error Variable PRJ_$(1)_F_SERI. Invalid value "$(PRJ_$(1)_F_SERI)"))
PRJ_$(1)$(2)_LIBS_SERI   :=$(MCO_SERI_$(PRJ_$(1)_F_SERI))
PRJ_$(1)$(2)_FLAGS_C_SERI  :=$(if $(PRJ_$(1)_F_SERI),MCO_HAS_SERIALIZATION=1)
PRJ_$(1)$(2)_FLAGS_CPP_SERI:=$(if $(PRJ_$(1)_F_SERI),MCO_HAS_SERIALIZATION=1)

# PRJ_<prj>_F_DBCALC - Enable DBCalc feature
# YES                 -> enable dbcalc
# <not set>           -> no dbcalc lib on the linker line
$(if $(findstring undefined, $(origin MCO_DBCALC_$(PRJ_$(1)_F_DBCALC))),$(error Variable PRJ_$(1)_F_DBCALC. Invalid value "$(PRJ_$(1)_F_DBCALC)"))
PRJ_$(1)$(2)_LIBS_DBCALC:=$(MCO_DBCALC_$(PRJ_$(1)_F_DBCALC))

# PRJ_<prj>_F_ITER - Enable iterator support
# YES                 -> enable iterator
# <not set>           -> no iterator lib on the linker line
$(if $(findstring undefined, $(origin MCO_ITER_$(PRJ_$(1)_F_ITER))),$(error Variable PRJ_$(1)_F_ITER. Invalid value "$(PRJ_$(1)_F_ITER)"))
PRJ_$(1)$(2)_LIBS_ITER:=$(MCO_ITER_$(PRJ_$(1)_F_ITER))

# PRJ_<prj>_F_UDA - Enable UDA support
# YES                 -> enable UDA
# <not set>           -> no UDA lib on the linker line
$(if $(findstring undefined, $(origin MCO_UDA_$(PRJ_$(1)_F_UDA))),$(error Variable PRJ_$(1)_F_UDA. Invalid value "$(PRJ_$(1)_F_UDA)"))
PRJ_$(1)$(2)_LIBS_UDA:=$(MCO_UDA_$(PRJ_$(1)_F_UDA))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_UDA:=$(MCO_UDA_MCOCOMP_$(PRJ_$(1)_F_UDA))

# PRJ_<prj>_F_XML - Enable XML support
# YES                 -> enable XML API
# <not set>           -> do not generate XML api for project's database
$(if $(PRJ_$(1)_F_XML),$(if $(findstring $(PRJ_$(1)_F_XML),YES),,$(error Variable PRJ_$(1)_F_XML. Invalid value "$(PRJ_$(1)_F_XML)")))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_XML:=$(if $(PRJ_$(1)_F_XML), -x)

# PRJ_<prj>_F_JSER - Enable JSON serialization support
# YES                 -> enable JSON API
# <not set>           -> no jser lib on the linker line, do not generate JSON api for project's database
$(if $(PRJ_$(1)_F_JSER),$(if $(findstring $(PRJ_$(1)_F_JSER),YES),,$(error Variable PRJ_$(1)_F_JSER. Invalid value "$(PRJ_$(1)_F_JSER)")))
PRJ_$(1)$(2)_LIBS_JSER:=$(MCO_JSER_$(PRJ_$(1)_F_JSER))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_JSER:=$(if $(PRJ_$(1)_F_JSER), -json)

# PRJ_<prj>_F_CPP - Generate CPP interface for project's database
# YES                 -> enable CPP API
# <not set>           -> do not generate CPP interface for project's database
# TODO: change F_CPP to F_DBINTFCPP
$(if $(PRJ_$(1)_F_CPP),$(if $(findstring $(PRJ_$(1)_F_CPP),YES),,$(error Variable PRJ_$(1)_F_CPP. Invalid value "$(PRJ_$(1)_F_CPP)")))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_CPP:=$(if $(PRJ_$(1)_F_CPP), -hpp)

# PRJ_<prj>_F_SMARTPTR - Enable smart pointers CPP API for project's database
# YES                 -> enable smart ptr CPP API
# <not set>           -> do not generate smart ptr CPP interface for project's database
# TODO: change F_SMAPTPTR to F_DBINTFSMARTPTR
$(if $(PRJ_$(1)_F_SMARTPTR),$(if $(findstring $(PRJ_$(1)_F_SMARTPTR),YES),,$(error Variable PRJ_$(1)_F_SMARTPTR. Invalid value "$(PRJ_$(1)_F_SMARTPTR)")))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_SMARTPTR:=$(if $(PRJ_$(1)_F_SMARTPTR), -smartptr)

# PRJ_<prj>_F_PERFMON - Enable PerfMon feature
# YES                 -> Enable PerfMon feature
# <not set>           -> No PerfMon feature
$(if $(findstring undefined, $(origin MCO_PERFMON_$(PRJ_$(1)_F_PERFMON))),$(error Variable PRJ_$(1)_F_PERMON. Invalid value "$(PRJ_$(1)_F_PERFMON)"))
PRJ_$(1)$(2)_LIBS_PERFMON:=$(MCO_PERFMON_$(PRJ_$(1)_F_PERFMON))

# PRJ_<prj>_F_HV - Enable HttpViewer feature
# YES                 -> Enable HttpViewer feature
# <not set>           -> No HttpViewer feature
$(if $(findstring undefined, $(origin MCO_HV_$(PRJ_$(1)_F_HV)_$(PRJ_$(1)_F_SQL))),$(error Variable PRJ_$(1)_F_HV. Invalid value "$(PRJ_$(1)_F_HV)"))
PRJ_$(1)$(2)_LIBS_HV:=$(if $(MCO_HV_$(PRJ_$(1)_F_HV)_$(PRJ_$(1)_F_SQL)),$(if $(PRJ_$(1)_F_PERFMON),mcohvperf)) $(MCO_HV_$(PRJ_$(1)_F_HV)_$(PRJ_$(1)_F_SQL))

# PRJ_<prj>_F_CLUSTER - Enable Cluster feature
# TCP, MPI            -> Enable Cluster with TCP/MPI transport library
# <not set>           -> No cluster feature
# TODO: change PRJ_<prj>_F_CLUSTER to PRJ_<prj>_CLUSTER
$(if $(findstring undefined, $(origin MCO_CLUSTER_$(PRJ_$(1)_F_CLUSTER))),$(error Variable PRJ_$(1)_F_CLUSTER. Invalid value "$(PRJ_$(1)_F_CLUSTER)"))
PRJ_$(1)$(2)_LIBS_CLUSTER:=$(MCO_CLUSTER_$(PRJ_$(1)_F_CLUSTER))

# PRJ_<prj>_F_MCOCOMP - mcocomp library
# YES           -> Add mcocomp library to the linkers line
# <not set>     -> Do not use mcocomp library
$(if $(findstring undefined, $(origin MCO_MCOCOMP_$(PRJ_$(1)_F_MCOCOMP))),$(error Variable PRJ_$(1)_F_MCOCOMP. Invalid value "$(PRJ_$(1)_F_MCOCOMP)"))
PRJ_$(1)$(2)_LIBS_MCOCOMP:=$(MCO_MCOCOMP_$(PRJ_$(1)_F_MCOCOMP))

# PRJ_<prj>_F_NATIVE - MCOCOMP's native ordering
# YES           -> Do not reorder classes fields in the schema
# <not set>     -> Optimize fields layout
$(if $(PRJ_$(1)_F_NATIVE),$(if $(findstring $(PRJ_$(1)_F_NATIVE),YES),,$(error Variable PRJ_$(1)_F_NATIVE. Invalid value "$(PRJ_$(1)_F_NATIVE)")))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_NATIVE:=$(if $(PRJ_$(1)_F_NATIVE), -nosort)

# PRJ_<prj>_F_BACKUP - Enable incremental backup feature
# YES           -> Enable incremantal backup feature
# <not set>     -> Disable incremental backup feature
$(if $(findstring undefined, $(origin MCO_BACKUP_$(PRJ_$(1)_F_BACKUP))),$(error Variable PRJ_$(1)_F_BACKUP. Invalid value "$(PRJ_$(1)_F_BACKUP)"))
PRJ_$(1)$(2)_LIBS_BACKUP:=$(MCO_BACKUP_$(PRJ_$(1)_F_BACKUP))

# PRJ_<prj>_F_REST - Enable REST interface feature
# YES           -> Enable REST feature
# <not set>     -> Disable 
$(if $(findstring undefined, $(origin MCO_REST_$(PRJ_$(1)_F_REST))),$(error Variable PRJ_$(1)_F_REST. Invalid value "$(PRJ_$(1)_F_REST)"))
PRJ_$(1)$(2)_LIBS_REST:= \
    $(MCO_REST_$(or $(PRJ_$(1)_F_REST),$(if $(findstring $(1),$(PRJ_TARGET_EXECUTABLE_DPTR) $(PRJ_TARGET_EXECUTABLE_OFFS)),$(if $(MCO_FORCE_REST),YES)))_PERFMON_$(PRJ_$(1)_F_PERFMON)) \
    $(MCO_REST_$(or $(PRJ_$(1)_F_REST),$(if $(findstring $(1),$(PRJ_TARGET_EXECUTABLE_DPTR) $(PRJ_TARGET_EXECUTABLE_OFFS)),$(if $(MCO_FORCE_REST),YES)))_SQL_$(if $(PRJ_$(1)_F_SQL),YES)) \
    $(MCO_REST_$(or $(PRJ_$(1)_F_REST),$(if $(findstring $(1),$(PRJ_TARGET_EXECUTABLE_DPTR) $(PRJ_TARGET_EXECUTABLE_OFFS)),$(if $(MCO_FORCE_REST),YES)))_TRACE_$(or $(PRJ_$(1)_F_FH),$(PRJ_$(1)_RUNTIME))) \
    $(MCO_REST_$(or $(PRJ_$(1)_F_REST),$(if $(findstring $(1),$(PRJ_TARGET_EXECUTABLE_DPTR) $(PRJ_TARGET_EXECUTABLE_OFFS)),$(if $(MCO_FORCE_REST),YES))))

# PRJ_<prj>_F_MCOCOMP_FORCE - force the database classes to be persistent or transient
# TRANSIENT     -> force all database classes to be transient
# PERSISTENT    -> force all database classes to be persistent
# <not set>     -> as mentioned in the schema
$(if $(findstring undefined, $(origin MCO_ENFORCE_MCOCOMP_$(PRJ_$(1)_F_MCOCOMP_FORCE))),$(error Variable PRJ_$(1)_F_MCOCOMP_FORCE. Invalid value "$(PRJ_$(1)_F_MCOCOMP_FORCE)"))
PRJ_$(1)$(2)_FLAGS_MCOCOMP_ENFORCE:=$(MCO_ENFORCE_MCOCOMP_$(PRJ_$(1)_F_MCOCOMP_FORCE))

# PRJ_<prj>_F_IOT - Enable IOT support
# YES                 -> enable IOT
# <not set>           -> no IOT libraries
$(if $(findstring undefined, $(origin MCO_IOT_$(PRJ_$(1)_F_IOT))),$(error Variable PRJ_$(1)_F_IOT. Invalid value "$(PRJ_$(1)_F_IOT)"))
PRJ_$(1)$(2)_LIBS_IOT    :=$(MCO_IOT_$(PRJ_$(1)_F_IOT))

# PRJ_<prj>_F_ODBC - Link ODBC library in object code version
# YES           -> Enable 
# <not set>     -> Disable 
$(if $(findstring undefined, $(origin MCO_ODBC_$(PRJ_$(1)_F_ODBC))),$(error Variable PRJ_$(1)_F_ODBC. Invalid value "$(PRJ_$(1)_F_ODBC)"))
PRJ_$(1)$(2)_LIBS_ODBC:=$(MCO_ODBC_$(PRJ_$(1)_F_ODBC))

# PRJ_<prj>_F_ODBCDRV - Link ODBC driver in shared library version
# YES           -> Enable 
# <not set>     -> Disable 
$(if $(findstring undefined, $(origin MCO_ODBCDRV_$(PRJ_$(1)_F_ODBCDRV))),$(error Variable PRJ_$(1)_F_ODBCDRV. Invalid value "$(PRJ_$(1)_F_ODBCDRV)"))
PRJ_$(1)$(2)_LIBS_ODBCDRV:=$(MCO_ODBCDRV_$(PRJ_$(1)_F_ODBCDRV))

# PRJ_<prj>_F_FHLIB - Enable FeedHandler support
# YES                 -> enable 
# <not set>           -> disable
$(if $(findstring undefined, $(origin MCO_FHLIB_$(PRJ_$(1)_F_FHLIB))),$(error Variable PRJ_$(1)_F_FHLIB. Invalid value "$(PRJ_$(1)_F_FHLIB)"))
PRJ_$(1)$(2)_LIBS_FHLIB:=$(MCO_FHLIB_$(PRJ_$(1)_F_FHLIB))

# PRJ_<prj>_RUNTIME - eXtremeDB Runtime (depends on PRJ_<prj>_F_CLUSTER,PRJ_<prj>_HA,PRJ_<prj>_SQL
# YES                 -> add mcocore libs to the linker line
# <not set>           -> no core libs
$(if $(findstring undefined, $(origin MCO_CORE_$(PRJ_$(1)_RUNTIME))),$(error Variable PRJ_$(1)_RUNTIME. Invalid value "$(PRJ_$(1)_RUNTIME)"))
PRJ_$(1)$(2)_LIBS_CORE :=$(MCO_CORE_$(PRJ_$(1)_RUNTIME))
PRJ_$(1)$(2)_LIBS_UTILS:=$(or $(MCO_UTILS_$(or $(PRJ_$(1)_F_CRT),$(PRJ_$(1)_RUNTIME))_$(MCO_TARGET_OSID)),$(MCO_UTILS_$(or $(PRJ_$(1)_F_CRT),$(PRJ_$(1)_RUNTIME))_any))
PRJ_$(1)$(2)_LIBS_TRACE:=$(MCO_TRACE_$(or $(MCO_FORCE_TRACE),$(PRJ_$(1)_F_TRACE),$(PRJ_$(1)_F_FH),$(PRJ_$(1)_RUNTIME)))
PRJ_$(1)$(2)_LIBS_SAL  :=$(or $(MCO_SAL_NET_$(or $(if $(PRJ_$(1)_F_CLUSTER),YES),$(if $(PRJ_$(1)_F_HA),YES),$(if $(findstring REMOTE,$(PRJ_$(1)_F_SQL)),YES))_$(MCO_TARGET_OSID)),$(MCO_SAL_NET_$(or $(if $(PRJ_$(1)_F_CLUSTER),YES),$(if $(PRJ_$(1)_F_HA),YES),$(if $(findstring REMOTE,$(PRJ_$(1)_F_SQL)),YES))_any)) \
	                 $(or $(MCO_SAL_$(or $(PRJ_$(1)_RUNTIME),$(if $(findstring STANDALONE,$(PRJ_$(1)_F_SQL)),YES),$(PRJ_$(1)_F_FHLIB))_$(MCO_TARGET_OSID)),$(MCO_SAL_$(or $(PRJ_$(1)_RUNTIME),$(if $(findstring STANDALONE,$(PRJ_$(1)_F_SQL)),YES),$(PRJ_$(1)_F_FHLIB))_any)) \
			 $(or $(MCO_SAL_DLOAD_$(or $(if $(PRJ_$(1)_F_SQL),YES), $(PRJ_$(1)_F_FHLIB))_$(MCO_TARGET_OSID)),$(MCO_SAL_DLOAD_$(or $(if $(PRJ_$(1)_F_SQL),YES), $(PRJ_$(1)_F_FHLIB))_any))

# PRJ_<prj>_F_CRYPT - Enable cryptografy feature (depends on PRJ_<prj>_RUNTIME)
# YES           -> Enable support
# <not set>     -> no support for the feature
$(if $(findstring undefined, $(origin MCO_CRYPT_$(PRJ_$(1)_RUNTIME)_$(PRJ_$(1)_F_CRYPT))),$(error Variable PRJ_$(1)_F_CRYPT. Invalid value "$(PRJ_$(1)_F_CRYPT)"))
PRJ_$(1)$(2)_LIBS_CRYPT:=$(MCO_CRYPT_$(or $(PRJ_$(1)_F_FHLIB),$(PRJ_$(1)_RUNTIME))_$(PRJ_$(1)_F_CRYPT))

PRJ_$(1)$(2)_LIBRARIES_SET:=$$(strip $$(filter-out $$(MCO_DISABLED_LIBS),$$(PRJ_$(1)$(2)_LIBS_FHLIB) $$(PRJ_$(1)$(2)_LIBS_ODBC) $$(PRJ_$(1)$(2)_LIBS_ODBCDRV) $$(PRJ_$(1)$(2)_LIBS_CORE) $$(PRJ_$(1)$(2)_LIBS_VT) $$(PRJ_$(1)$(2)_LIBS_MEMDEV) $$(PRJ_$(1)$(2)_LIBS_SYNC) $$(PRJ_$(1)$(2)_LIBS_TMGR) $$(PRJ_$(1)$(2)_LIBS_REST) $$(PRJ_$(1)$(2)_LIBS_HV) $$(PRJ_$(1)$(2)_LIBS_SQL) $$(PRJ_$(1)$(2)_LIBS_HA) $$(PRJ_$(1)$(2)_LIBS_LOG) $$(PRJ_$(1)$(2)_LIBS_STRDEV) $$(PRJ_$(1)$(2)_LIBS_SERI) $$(PRJ_$(1)$(2)_LIBS_DBCALC) $$(PRJ_$(1)$(2)_LIBS_ITER) $$(PRJ_$(1)$(2)_LIBS_JSER) $$(PRJ_$(1)$(2)_LIBS_UDA) $$(PRJ_$(1)$(2)_LIBS_PERFMON) $$(PRJ_$(1)$(2)_LIBS_HVPERFMON) $$(PRJ_$(1)$(2)_LIBS_CLUSTER) $$(PRJ_$(1)$(2)_LIBS_SEQUENCES) $$(PRJ_$(1)$(2)_LIBS_MCOCOMP) $$(PRJ_$(1)$(2)_LIBS_BACKUP) $$(PRJ_$(1)$(2)_LIBS_IOT) $$(PRJ_$(1)$(2)_LIBS_CRYPT) $$(PRJ_$(1)$(2)_LIBS_UTILS) $$(PRJ_$(1)$(2)_LIBS_TRACE) $$(PRJ_$(1)$(2)_LIBS_SAL)))
endef

define CONSTRUCT_TARGETS_START
all: $(if $(MCO_PACKAGE_ROOT),samples,$(PRJ_ALL_BUILD_TARGETS))
	$(call mco_rule_start,,$$(@) is done,)
	$(call mco_rule_finish,)

host: $(PRJ_HOST_BUILD_TARGETS)
	$(call mco_rule_start,,$$(@) is done,)
	$(call mco_rule_finish,)

target: $(PRJ_TARGET_BUILD_TARGETS)
	$(call mco_rule_start,,$$(@) is done,)
	$(call mco_rule_finish,)

tools: $(PRJ_HOST_BUILD_TARGETS) $(PRJ_TARGET_BUILD_TARGETS)
	$(call mco_rule_start,,$$(@) is done,)
	$(call mco_rule_finish,)

samples: $(PRJ_SAMPLES_BUILD_TARGETS)
	$(call mco_rule_start,,$$(@) is done,)
	$(call mco_rule_finish,)
endef

define FUNC
CHUNK:=$(words 1,5,$(1))
TAIL :=$(subst $(CHUNK),,$(1))
$(warning CHUNK:$(CHUNK))
$(if $(TAIL),$(eval $(call FUNC,$(TAIL))))	
endef

define CONSTRUCT_TARGET_SCHEMA_TO_SOURCES
# param 1 - project name
# param 2 - database name
# param 3 - schema file
# param 4 - object directory
# param 5 - mcocomp flags
$(addprefix $(4),$(foreach f,c cpp hpp,$(2).$(f))):$(addprefix $(4),$(2).h)

$(if $(findstring on,$(MCO_BUILD_LOG)),PRJ_$(1)_BUILD_LOGS += $(4)/$(notdir $(3)).mco.build-log)
$(if $(findstring on,$(MCO_BUILD_LOG)),$(4)/$(notdir $(3)).mco.build-log: $(addprefix $(4),$(2).h))

$(addprefix $(4),$(2).h) :$(if $(findstring yes,$(MCO_COUNT_PREBUILT_AS_DEPENDENCIES)),$(MCO_BUILD_MCOCOMP)) $(3)
	$$(if $$(wildcard $(4)*),,@$(MCO_BUILD_MKDIR) "$(4)")
	$(call mco_rule_start ,$(1),MCOCOMP: $(1) $(notdir $(3)),$(4)/$(notdir $(3)).mco.build-log)
	$(call mco_rule_run   ,$(1),$(MCO_BUILD_MCOCOMP) $(5) -p$(4) -f$(3),$(4)/$(notdir $(3)).mco.build-log)
	$(call mco_rule_finish,$(1))
endef

define CONSTRUCT_FILE_ASSOCIATION
$(1)=$(2)
endef

define CONSTRUCT_TARGET_C_SOURCE_TO_OBJECT
# param 1 - project name
# param 2 - source file
# param 3 - object file
# param 4 - external C flags
# param 5 - HOST/TARGET
$(if $(findstring on,$(MCO_BUILD_LOG)),PRJ_$(1)_BUILD_LOGS += $(3).build-log)
$(if $(findstring on,$(MCO_BUILD_LOG)),$(3).build-log: $(3))

$(3): $(2)
	$$(if $$(wildcard $(dir $(3))/*),,@$(MCO_BUILD_MKDIR) "$(dir $(3))")
	$(call mco_rule_start,$(1),     CC: $(1) $(notdir $(2)),$(3).build-log)
	$(call mco_rule_run,$(1),$(strip $(subst @debug_flags@,$(if $(findstring -wdbg, $(3)),$(MCO_$(5)_F_WDBG),$(MCO_$(5)_F_NDBG)),$(subst @defines@,$(addprefix $(MCO_$(5)_F_DEFINE),$(filter-out $(MCO_DISABLED_DEFS),$(MCO_EXDB_COMMONF) $(if $(findstring -wdbg, $(3)),$(MCO_EXDB_WDBGF),$(MCO_EXDB_NDBGF)) $(if $(findstring -dptr,$(3)),$(MCO_EXDB_DPTRF),$(MCO_EXDB_OFFSF)) $(if $(findstring -wevl, $(3)),$(MCO_EXDB_WEVLF),$(MCO_EXDB_NEVLF)))) $$(addprefix $(MCO_$(5)_F_DEFINE),$(filter-out $(MCO_DISABLED_DEFS),$(4))) $(addprefix $(MCO_$(5)_F_PATHINC),$(dir $(3)) $(dir $(2)) $(MCO_INCLUDE) $(PRJ_$(1)_INCLUDE)), $(subst @source_file@,$$(<), $(subst @dependency_file@,$$(@:.o=.d),$(subst @object_file@,$$(@),$(MCO_$(5)_COMP_C))))))),$(3).build-log)
	$(call mco_rule_finish,$(1))
endef

define CONSTRUCT_TARGET_CPP_SOURCE_TO_OBJECT
# param 1 - project name
# param 2 - source file
# param 3 - object file
# param 4 - external CPP flags
# param 5 - HOST/TARGET
$(if $(findstring on,$(MCO_BUILD_LOG)),PRJ_$(1)_BUILD_LOGS += $(3).build-log)
$(if $(findstring on,$(MCO_BUILD_LOG)),$(3).build-log: $(3))

$(3): $(2)
	$$(if $$(wildcard $(dir $(3))/*),,@$(MCO_BUILD_MKDIR) "$(dir $(3))")
	$(call mco_rule_start,$(1),    CPP: $(1) $(notdir $(2)),$(3).build-log)
	$(call mco_rule_run,$(1),$(strip $(subst @debug_flags@,$(if $(findstring -wdbg, $(3)),$(MCO_$(5)_F_WDBG),$(MCO_$(5)_F_NDBG)),$(subst @defines@,$(addprefix $(MCO_$(5)_F_DEFINE),$(filter-out $(MCO_DISABLED_DEFS),$(MCO_EXDB_COMMONF) $(if $(findstring -wdbg, $(3)),$(MCO_EXDB_WDBGF),$(MCO_EXDB_NDBGF)) $(if $(findstring -dptr,$(3)),$(MCO_EXDB_DPTRF),$(MCO_EXDB_OFFSF)) $(if $(findstring -wevl, $(3)),$(MCO_EXDB_WEVLF),$(MCO_EXDB_NEVLF)))) $$(addprefix $(MCO_$(5)_F_DEFINE),$(filter-out $(MCO_DISABLED_DEFS),$(4))) $(addprefix $(MCO_$(5)_F_PATHINC),$(dir $(3)) $(dir $(2)) $(MCO_INCLUDE) $(PRJ_$(1)_INCLUDE)), $(subst @source_file@,$$(<), $(subst @dependency_file@,$$(@:.o=.d),$(subst @object_file@,$$(@),$(MCO_$(5)_COMP_CPP))))))),$(3).build-log)
	$(call mco_rule_finish,$(1))
endef

define CONSTRUCT_TARGET_BINARY 
# param 1 - project name
# param 2 - object directory suffix
# param 3 - host/target

# collect all the required libraries
PRJ_$(1)$(2)_LIBRARIES:=$(addprefix $(MCO_$(3)_F_LIBRARY),$(addsuffix $(MCO_USE_DEBUG_$(if $(findstring wdbg,$(2)),YES)),$(subst @socketlib@,,$(PRJ_$(1)$(2)_LIBRARIES_SET)))) $(if $(findstring @socketlib@,$(PRJ_$(1)$(2)_LIBRARIES_SET)),$(MCO_$(3)_SOCKETLIB))

# make the binary to depend on all the required libraries (exclude @socketlib@ mark)
PRJ_$(1)$(2)_DEPENDENCIES_LIBS:=$(strip $(if $(findstring YES,$(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES)),$(addprefix $(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(3))/bin$(if $(findstring -dptr,$(2)),.dptr).so/$(MCO_$(3)_LIB_DYNM_PFX),$(addsuffix $(MCO_$(3)_LIB_DYNM_SFX),$(addsuffix $(MCO_USE_DEBUG_$(if $(findstring wdbg,$(2)),YES)),$(subst @socketlib@,,$(filter-out $(MCO_ODBC_PRJS),$(PRJ_$(1)$(2)_LIBRARIES_SET)))))),$(addprefix $(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(3))/bin$(if $(findstring -dptr,$(2)),.dptr)/$(MCO_$(3)_LIB_STAT_PFX),$(addsuffix $(MCO_$(3)_LIB_STAT_SFX),$(addsuffix $(MCO_USE_DEBUG_$(if $(findstring wdbg,$(2)),YES)),$(subst @socketlib@,,$(filter-out $(MCO_ODBC_PRJS),$(PRJ_$(1)$(2)_LIBRARIES_SET))))))) \
$(if $(findstring YES,$(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES)),$(addprefix $(MCO_PRODUCT_ROOT)/odbc/bin$(if $(findstring -dptr,$(2)),.dptr).so/$(MCO_$(3)_LIB_DYNM_PFX),$(addsuffix $(MCO_$(3)_LIB_DYNM_SFX),$(addsuffix $(MCO_USE_DEBUG_$(if $(findstring wdbg,$(2)),YES)),$(filter $(MCO_ODBC_PRJS),$(PRJ_$(1)$(2)_LIBRARIES_SET))))),$(addprefix $(MCO_PRODUCT_ROOT)/odbc/bin$(if $(findstring -dptr,$(2)),.dptr)/$(MCO_$(3)_LIB_STAT_PFX),$(addsuffix $(MCO_$(3)_LIB_STAT_SFX),$(addsuffix $(MCO_USE_DEBUG_$(if $(findstring wdbg,$(2)),YES)),$(filter $(MCO_ODBC_PRJS),$(PRJ_$(1)$(2)_LIBRARIES_SET)))))))

# associate PRJ_<project><objdirsuffix>_FILE_IO_<path>/source.c -> <path>/obj-<project><objdirsuffix>/source.o
$(foreach f,$(filter %.c %.cpp,$(PRJ_$(1)_SRC)),  $(eval $(call CONSTRUCT_FILE_ASSOCIATION, PRJ_$(1)$(2)_FILE_IO_$(subst :,/,$(f)), $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(basename $(notdir $(f))).o)))

# associate PRJ_<project><objdirsuffix>_FILE_IO_<path>/obj-<project><objdirsuffix>/dbsource.c -> <path>/obj-<project><objdirsuffix>/dbsource.o
$(foreach d,$(PRJ_$(1)_DATABASES),$(eval $(call CONSTRUCT_FILE_ASSOCIATION, PRJ_$(1)$(2)_FILE_IO_$(subst :,/,$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).c), $(PRJ_$(1)_INTERMEDIATE_DIR)/$(d).o)))

# associate PRJ_<project><objdirsuffix>_FILE_OI_<path>/obj-<project><objdirsuffix>/source.o -> <path>/source.c
$(foreach f,$(filter %.c %.cpp,$(PRJ_$(1)_SRC)),$(eval $(call CONSTRUCT_FILE_ASSOCIATION, PRJ_$(1)$(2)_FILE_OI_$(subst :,/,$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(basename $(notdir $(f))).o),$(f) ) ) )

# associate PRJ_<project><objdirsuffix>_FILE_OI_<path>/obj-<project><objdirsuffix>/dbsource.o -> <path>/obj-<project><objdirsuffix>/dbsource.c
$(foreach d,$(PRJ_$(1)_DATABASES),$(eval $(call CONSTRUCT_FILE_ASSOCIATION, PRJ_$(1)$(2)_FILE_OI_$(subst :,/,$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).o),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).c ) ) )

# process the project source code file to objects files
$(foreach f,$(filter %.c,$(PRJ_$(1)_SRC)),  $(eval $(call CONSTRUCT_TARGET_C_SOURCE_TO_OBJECT  ,$(1),$(f),$(PRJ_$(1)$(2)_FILE_IO_$(subst :,/,$(f))),$$(strip $$(PRJ_$(1)$(2)_FLAGS_C_VT) $$(PRJ_$(1)$(2)_FLAGS_C_HA) $$(PRJ_$(1)$(2)_FLAGS_C_SERI) $$(PRJ_$(1)_DEFINES)),$(3))))
$(foreach f,$(filter %.cpp,$(PRJ_$(1)_SRC)),$(eval $(call CONSTRUCT_TARGET_CPP_SOURCE_TO_OBJECT,$(1),$(f),$(PRJ_$(1)$(2)_FILE_IO_$(subst :,/,$(f))),$$(strip $$(PRJ_$(1)$(2)_FLAGS_CPP_VT) $$(PRJ_$(1)$(2)_FLAGS_CPP_HA) $$(PRJ_$(1)$(2)_FLAGS_CPP_SERI) $$(PRJ_$(1)_DEFINES)),$(3))))

# process the generated database source files to generated database object files
$(foreach d,$(PRJ_$(1)_DATABASES),          $(eval $(call CONSTRUCT_TARGET_C_SOURCE_TO_OBJECT  ,$(1),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).c,$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).o,$$(strip $$(PRJ_$(1)$(2)_FLAGS_C_VT) $$(PRJ_$(1)$(2)_FLAGS_C_HA) $$(PRJ_$(1)$(2)_FLAGS_C_SERI) $$(PRJ_$(1)_DEFINES)),$(3))))

# add dependency of all the project source code and generated database source code files from the generated database headers
$(if $(PRJ_$(1)_DATABASES),$(filter-out $(MCO_SAMPLES_COMMON_DIR)%,$(filter %.c,$(PRJ_$(1)_SRC)) $(filter %.cpp,$(PRJ_$(1)_SRC))) $(foreach d,$(PRJ_$(1)_DATABASES),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).c): $(addprefix $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/,$(addsuffix .h,$(PRJ_$(1)_DATABASES))))

# process the project schemas to generated database source code files and headers
$(foreach i,1 2 3 4 5 6 7 8 9 10,$(if $(word $(i),$(PRJ_$(1)_DATABASES)), $(eval $(call CONSTRUCT_TARGET_SCHEMA_TO_SOURCES,$(1),$(word $(i),$(PRJ_$(1)_DATABASES)),$(word $(i),$(PRJ_$(1)_SCHEMAS)),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/,$(MCO_TARGET_F_MCOCOMP) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_ENFORCE) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_CPP) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_NATIVE) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_SMARTPTR) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_SQL) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_UDA) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_XML) $$(PRJ_$(1)$(2)_FLAGS_MCOCOMP_JSER) $$(PRJ_$(1)_MCOCOMPFLAGS)))))

# make a list of the project object files 
PRJ_$(1)$(2)_OBJECTS  := $(foreach f,$(PRJ_$(1)_SRC),$$(PRJ_$(1)$(2)_FILE_IO_$(subst :,/,$(f)))) $(foreach d,$(PRJ_$(1)_DATABASES),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).o)

# execute all source code files dependecies if they were pre-generated
-include $$(PRJ_$(1)$(2)_OBJECTS:%.o=%.d)

# call customs makefuncs from the project makefile
$(foreach f,$(PRJ_$(1)_MAKEFUNC),$(eval $(call $(f),$(PRJ_$(1)_DIR),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/)))

# process the object files to the binary
$(MCO_PRODUCT_ROOT)/$(if $(filter $(1),$(MCO_ODBC_PRJS)),odbc,$(MCO_ALIAS_$(3)))/bin$(if $(findstring -dptr,$(2)),.dptr)$(if $(findstring -dynm,$(2)),.so)$(if $(findstring -wevl,$(2)),.eval)/$(if $(findstring -stat,$(2)),$(MCO_$(3)_LIB_STAT_PFX))$(if $(findstring -dynm,$(2)),$(MCO_$(3)_LIB_DYNM_PFX))$(if $(findstring -exec,$(2)),$(MCO_$(3)_EXEC_PFX))$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1))$(if $(findstring -wdbg,$(2)),$(if $(findstring -exec,$(2)),,_debug))$(if $(findstring -stat,$(2)),$(MCO_$(3)_LIB_STAT_SFX))$(if $(findstring -dynm,$(2)),$(or $(if $(findstring extremedbjni,$(1)),$(MCO_$(3)_LIB_JNI_SFX)),$(MCO_$(3)_LIB_DYNM_SFX)))$(if $(findstring -exec,$(2)),$(MCO_$(3)_EXEC_SFX)): $$(PRJ_$(1)$(2)_OBJECTS) $(if $(findstring yes,$(MCO_COUNT_PREBUILT_AS_DEPENDENCIES)),$$(PRJ_$(1)$(2)_DEPENDENCIES_LIBS)) $(PRJ_$(1)_ADDITIONAL_DEPENDENCIES)
	@$(MCO_BUILD_MKDIR) $$(dir $$(@))
	@$(MCO_BUILD_MKDIR) $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)
	$(call mco_rule_start ,$(1),   LINK: $$(subst $(MCO_PRODUCT_ROOT),,$$(@)),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$$(notdir $$(@)).build-log)
	$$(if $$(wildcard $$(filter %.mco.build-log,$(filter $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/%,$(PRJ_$(1)_BUILD_LOGS)))),@$(MCO_BUILD_CAT) $$(wildcard $$(filter %.mco.build-log,$(filter $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/%,$(PRJ_$(1)_BUILD_LOGS))))>>$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$$(notdir $$(@)).build-log)
	$$(if $$(wildcard $$(filter-out %.mco.build-log,$(filter $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/%,$(PRJ_$(1)_BUILD_LOGS)))),@$(MCO_BUILD_CAT) $$(wildcard $$(filter-out %.mco.build-log,$(filter $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/%,$(PRJ_$(1)_BUILD_LOGS))))>>$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$$(notdir $$(@)).build-log)
	$(call mco_rule_run   ,$(1),$(if $(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES),LD_LIBRARY_PATH=$${LD_LIBRARY_PATH}:$(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(3))/bin$(if $(findstring -dptr,$(2)),.dptr)$(strip $(MCO_DYNM_PATH_$(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES))):$(MCO_PRODUCT_ROOT)/odbc/bin$(if $(findstring -dptr,$(2)),.dptr)$(strip $(MCO_DYNM_PATH_$(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES))) )$(strip $(subst @link_dynamic@,$(if $(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES),$(MCO_$(3)_F_DYNLINK)),$(subst @debug_flags@,$(if $(findstring -wdbg, $(3)),$(MCO_$(3)_F_WDBG),$(MCO_$(3)_F_NDBG)),$(subst @libraries@,$(MCO_$(3)_F_PATHLIB)$(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(3))/bin$(if $(findstring -dptr,$(2)),.dptr)$(strip $(MCO_DYNM_PATH_$(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES)))/ $(MCO_$(3)_F_PATHLIB)$(MCO_PRODUCT_ROOT)/odbc/bin$(if $(findstring -dptr,$(2)),.dptr)$(strip $(MCO_DYNM_PATH_$(PRJ_$(1)_F_USE_DYNAMIC_LIBRARIES)))/ $$(PRJ_$(1)$(2)_LIBRARIES) $$(PRJ_$(1)_ADDITIONAL_LIBRARIES),$(subst @output-file@,$$(@),$(subst @object_files@,$$(filter %.o,$$(^)),$(if $(filter %.cpp,$(PRJ_$(1)_SRC))$(PRJ_$(1)_F_SQL),$(if $(findstring -stat,$(2)),$(MCO_$(3)_LIBR_STAT_CPP))$(if $(findstring -dynm,$(2)),$(MCO_$(3)_LIBR_DYNM_CPP))$(if $(findstring -exec,$(2)),$(MCO_$(3)_LINK_CPP)),$(if $(findstring -stat,$(2)),$(MCO_$(3)_LIBR_STAT_C))$(if $(findstring -dynm,$(2)),$(MCO_$(3)_LIBR_DYNM_C))$(if $(findstring -exec,$(2)),$(MCO_$(3)_LINK_C))))))))),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$$(notdir $$(@)).build-log)
	$(call mco_rule_finish,$(1))

# generate evaluation target for executables.
ifneq "$(findstring -exec,$(2))" ""
ifeq "$(MCO_ENABLE_EVAL_BINARIES)" "yes"
$(MCO_PRODUCT_ROOT)/$(if $(filter $(1),$(MCO_ODBC_PRJS)),odbc,$(MCO_ALIAS_$(3)))/bin$(if $(findstring -dptr,$(2)),.dptr).eval/$(MCO_$(3)_EXEC_PFX)$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1))$(if $(findstring -wdbg,$(2)),$(if $(findstring -exec,$(2)),,_debug))$(MCO_$(3)_EXEC_SFX): $(MCO_PRODUCT_ROOT)/$(if $(filter $(1),$(MCO_ODBC_PRJS)),odbc,$(MCO_ALIAS_$(3)))/bin$(if $(findstring -dptr,$(2)),.dptr)/$(MCO_$(3)_EXEC_PFX)$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1))$(if $(findstring -wdbg,$(2)),$(if $(findstring -exec,$(2)),,_debug))$(MCO_$(3)_EXEC_SFX)
	@$(MCO_BUILD_MKDIR) $$(dir $$(@))
	@$(MCO_BUILD_COPY) $$(<) $$(@)
endif
endif

# Add all intermediate files and directories to the list
# PRJ_$(1)_BUILD_LOGS constructed and collected for each of the object files individualy  
PRJ_$(1)_PRODUCTS          += $(MCO_PRODUCT_ROOT)/$(if $(filter $(1),$(MCO_ODBC_PRJS)),odbc,$(MCO_ALIAS_$(3)))/bin$(if $(findstring -dptr,$(2)),.dptr)$(if $(findstring -dynm,$(2)),.so)$(if $(findstring -wevl,$(2)),.eval)/$(if $(findstring -stat,$(2)),$(MCO_$(3)_LIB_STAT_PFX))$(if $(findstring -dynm,$(2)),$(MCO_$(3)_LIB_DYNM_PFX))$(if $(findstring -exec,$(2)),$(MCO_$(3)_EXEC_PFX))$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1))$(if $(findstring -wdbg,$(2)),$(if $(findstring -exec,$(2)),,_debug))$(if $(findstring -stat,$(2)),$(MCO_$(3)_LIB_STAT_SFX))$(if $(findstring -dynm,$(2)),$(or $(if $(findstring extremedbjni,$(1)),$(MCO_$(3)_LIB_JNI_SFX)),$(MCO_$(3)_LIB_DYNM_SFX)))$(if $(findstring -exec,$(2)),$(MCO_$(3)_EXEC_SFX))
PRJ_$(1)_INTERMEDIATE_DIRS +=$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)
PRJ_$(1)_INTERMEDIATES     +=$(foreach f,$(filter %.c %.cpp,$(PRJ_$(1)_SRC)),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(basename $(notdir $(f))).o $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(basename $(notdir $(f))).d)
PRJ_$(1)_INTERMEDIATES     +=$(foreach d,$(PRJ_$(1)_DATABASES),$(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).h $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).hpp $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).c $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).o $(PRJ_$(1)$(2)_INTERMEDIATE_DIR)/$(d).d)

endef

define CONSTRUCT_JAVA_JAR
# param 1 - project name
# param 2 - HOST/TARGET
# param 3 - side-platform-id
PRJ_$(1)_BUILD_LOGS        +=$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/$(1).jar.build-log
PRJ_$(1)_INTERMEDIATE_DIRS +=$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/classes
PRJ_$(1)_INTERMEDIATES     +=$(addprefix $(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/classes/,$(PRJ_$(1)_SRC:%.java=%.class))
PRJ_$(1)_PRODUCTS          += $(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(2))/bin/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).jar

$(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(2))/bin/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).jar: $(PRJ_$(1)_SRC)
	@$(MCO_BUILD_MKDIR) $$(dir $$(@)) $(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR)/classes)
	$(call mco_rule_start ,$(1),    JAR: $$(subst $(MCO_PRODUCT_ROOT),,$$(@)),$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR)/$(1).jar.build-log))
	$(call mco_rule_run   ,$(1),$(subst @classpath@,.,$(subst @output-dir@,$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR)/classes),$(subst @input-dir@,$(PRJ_$(1)_DIR)/java,$(subst @input-files@,$(PRJ_$(1)_SRC),$(MCO_BUILD_JAVAC))))),$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR)/$(1).jar.build-log))
	$(call mco_rule_run   ,$(1),$(subst @output@,$$(@),$(subst @input@,$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR)/classes), $(MCO_BUILD_JAR))),$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR)/$(1).jar.build-log))
	$(call mco_rule_finish,$(1))

$(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(2))/bin.eval/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).jar: $(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(2))/bin/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).jar
	@$(MCO_BUILD_MKDIR) $$(dir $$(@))
	@$(MCO_BUILD_COPY) $(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(2))/bin/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).jar $(MCO_PRODUCT_ROOT)/$(MCO_ALIAS_$(2))/bin.eval/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).jar
endef

define CONSTRUCT_JAVA_CLASS
# param 1 - project name
# param 2 - HOST/TARGET
# param 3 - side-platform-id
PRJ_$(1)_BUILD_LOGS        +=$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/$(1).class.build-log
# TODO: a single java-file may produce many class-files. Need to figure out products names somehow
#PRJ_$(1)_PRODUCTS          +=$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).class
PRJ_$(1)_PRODUCTS          +=$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/*.class

$(subst $(MCO_ROOT),$(MCO_PRODUCT_ROOT),$(PRJ_$(1)_DIR))/$(or $(PRJ_$(1)_RESULT_NAME_BASE),$(1)).class: $(PRJ_$(1)_SRC)
	@$(MCO_BUILD_MKDIR) $$(dir $$(@)) $(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))
	$(call mco_rule_start ,$(1),  CLASS: $$(subst $(MCO_PRODUCT_ROOT),,$$(@)),$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/$(1).class.build-log)
	$(call mco_rule_run   ,$(1),$(subst @classpath@,$(MCO_PRODUCT_ROOT)/target/bin/extremedb.jar:.,$(subst @output-dir@,$(subst $(MCO_ROOT),$(MCO_PRODUCT_ROOT),$(PRJ_$(1)_DIR)),$(subst @input-dir@,$(PRJ_$(1)_DIR),$(subst @input-files@,$(PRJ_$(1)_SRC),$(MCO_BUILD_JAVAC))))),$(subst $(MCO_ROOT),$(MCO_BUILD_ROOT),$(PRJ_$(1)_DIR))/$(1).class.build-log)
	@$(if $(and $(subst $(MCO_PRODUCT_ROOT),,$(MCO_ROOT)),$(wildcard $(PRJ_$(1)_DIR)/*.sh $(PRJ_$(1)_DIR)/*.bat)),$(MCO_BUILD_COPY) $(wildcard $(PRJ_$(1)_DIR)/*.sh $(PRJ_$(1)_DIR)/*.bat) $$(dir $$(@)))
	$(call mco_rule_finish,$(1))
endef

# construct common targets for the beginning of the make-script (all for default target)
$(eval $(call CONSTRUCT_TARGETS_START))

# analyze project properites
$(foreach i,$(PRJ_HOST_ANALYZE_ONLY),     $(eval $(call ANALYZE_PROJECT_PROPS,$(i),-$(MCO_HOST_ARCH_SFX)$(PRJ_$(i)_BUILD_SFX),HOST)))
$(foreach i,$(PRJ_TARGET_ANALYZE_ONLY),   $(eval $(call ANALYZE_PROJECT_PROPS,$(i),-$(MCO_TARGET_ARCH_SFX)$(PRJ_$(i)_BUILD_SFX),TARGET)))
$(foreach limtype,$(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),wevl) nevl, $(foreach dbgtype,wdbg ndbg, $(foreach modtype,$(MCO_ENABLE_STATIC_BINARIES) $(MCO_ENABLE_DYNAMIC_BINARIES), $(foreach rttype, OFFS DPTR, $(foreach i,$(PRJ_TARGET_LIBRARY_$(rttype)_$(modtype)), $(eval $(call ANALYZE_PROJECT_PROPS,$(i),-$(MCO_TARGET_ARCH_SFX)-$(if $(findstring STATIC,$(modtype)),stat,dynm)-$(if $(findstring DPTR,$(rttype)),dptr,offs)-$(dbgtype)-$(limtype),TARGET)) )))))
$(foreach i,$(PRJ_HOST_EXECUTABLE),       $(eval $(call ANALYZE_PROJECT_PROPS,$(i),-$(MCO_HOST_ARCH_SFX)-exec-$(if $(MCO_USE_DEBUG_$(PRJ_$(i)_DEBUG)),w,n)dbg,HOST)))
$(foreach i,$(PRJ_TARGET_EXECUTABLE_DPTR),$(eval $(call ANALYZE_PROJECT_PROPS,$(i),-$(MCO_TARGET_ARCH_SFX)-exec-dptr-$(if $(MCO_USE_DEBUG_$(PRJ_$(i)_DEBUG)),w,n)dbg,TARGET)))
$(foreach i,$(PRJ_TARGET_EXECUTABLE_OFFS),$(eval $(call ANALYZE_PROJECT_PROPS,$(i),-$(MCO_TARGET_ARCH_SFX)-exec-offs-$(if $(MCO_USE_DEBUG_$(PRJ_$(i)_DEBUG)),w,n)dbg,TARGET)))

# construct library targets
$(foreach limtype,$(if $(findstring yes,$(MCO_ENABLE_EVAL_BINARIES)),wevl) nevl, \
$(foreach dbgtype,wdbg ndbg, \
$(foreach modtype,$(MCO_ENABLE_STATIC_BINARIES) $(MCO_ENABLE_DYNAMIC_BINARIES), \
$(foreach rttype, OFFS DPTR, \
$(foreach i,$(PRJ_TARGET_LIBRARY_$(rttype)_$(modtype)), \
$(eval $(call CONSTRUCT_TARGET_BINARY,$(i),-$(MCO_TARGET_ARCH_SFX)-$(if $(findstring STATIC,$(modtype)),stat,dynm)-$(if $(findstring DPTR,$(rttype)),dptr,offs)-$(dbgtype)-$(limtype),TARGET)) \
)))))

# construct executable targets
$(foreach i,$(PRJ_HOST_EXECUTABLE),       $(eval $(call CONSTRUCT_TARGET_BINARY,$(i),-$(MCO_HOST_ARCH_SFX)-exec-$(if $(MCO_USE_DEBUG_$(PRJ_$(i)_DEBUG)),w,n)dbg,HOST)))
$(foreach i,$(PRJ_TARGET_EXECUTABLE_DPTR),$(eval $(call CONSTRUCT_TARGET_BINARY,$(i),-$(MCO_TARGET_ARCH_SFX)-exec-dptr-$(if $(MCO_USE_DEBUG_$(PRJ_$(i)_DEBUG)),w,n)dbg,TARGET)))
$(foreach i,$(PRJ_TARGET_EXECUTABLE_OFFS),$(eval $(call CONSTRUCT_TARGET_BINARY,$(i),-$(MCO_TARGET_ARCH_SFX)-exec-offs-$(if $(MCO_USE_DEBUG_$(PRJ_$(i)_DEBUG)),w,n)dbg,TARGET)))
$(foreach i,$(PRJ_TARGET_JAR),            $(eval $(call CONSTRUCT_JAVA_JAR,$(i),TARGET)))
$(foreach i,$(PRJ_TARGET_CLASS),          $(eval $(call CONSTRUCT_JAVA_CLASS,$(i),TARGET)))

# construct custom rules
$(foreach i,$(PRJ_GENERATE_CUSTOM_RULES),$(eval $(call $(i),1,2)))

# construct common targets for the ending of the make-script (all the vaiables has values)
ifneq "none" "$(MCO_XARGS_TEST)"

define step

	@$(MCO_BUILD_ECHO) $(1) >> $(2)
endef

# Collect all build-logs, intermediate and product files and directories
MCO_BUILD_LOGS        :=$(strip $(foreach prj,$(BUILD_PRJ_LIST),$(PRJ_$(prj)_BUILD_LOGS)))
MCO_PRODUCTS          :=$(strip $(foreach prj,$(BUILD_PRJ_LIST),$(PRJ_$(prj)_PRODUCTS)))
MCO_PRODUCT_DIRS      :=$(strip $(foreach prj,$(BUILD_PRJ_LIST),$(PRJ_$(prj)_PRODUCT_DIRS)))
MCO_INTERMEDIATES     :=$(strip $(foreach prj,$(BUILD_PRJ_LIST),$(PRJ_$(prj)_INTERMEDIATES)))
MCO_INTERMEDIATE_DIRS :=$(strip $(foreach prj,$(BUILD_PRJ_LIST),$(PRJ_$(prj)_INTERMEDIATE_DIRS)))
#MCO_INTERMEDIATES     +=$(filter %.d, $(MCO_INTERMEDIATES:%.o=%.d))
# separate products,build-logs and intermediates located out of dedicated directories
MCO_PRODUCTS_LONE     :=$(filter-out $(addsuffix /%,$(MCO_PRODUCT_DIRS)),$(MCO_PRODUCTS))
MCO_INTERMEDIATES_LONE:=$(filter-out $(addsuffix /%,$(MCO_INTERMEDIATE_DIRS)),$(MCO_INTERMEDIATES))
MCO_BUILD_LOGS_LONE   :=$(filter-out $(addsuffix /%,$(MCO_INTERMEDIATE_DIRS)),$(MCO_BUILD_LOGS))

define GENERATE_CLEANUP_TARGETS
clean: 
	$(call mco_rule_start,,Cleaning up...,)
	@$(MCO_BUILD_ECHO) $(firstword $(MCO_INTERMEDIATES)) > $(MCO_BUILD_ROOT)/mco-tmp-list
	$(foreach L,$(wordlist 2,$(words $(MCO_INTERMEDIATES)),$(MCO_INTERMEDIATES)),$(call step,$(L),$(MCO_BUILD_ROOT)/mco-tmp-list))
	@$(MCO_BUILD_CAT) $(MCO_BUILD_ROOT)/mco-tmp-list | xargs @$(MCO_BUILD_RMFILE)
	@$(MCO_BUILD_RMFILE) $(MCO_BUILD_ROOT)/mco-tmp-list
	$(call mco_rule_finish,)

distclean: 
	$(call mco_rule_start,,Cleaning up...,)
	@$(MCO_BUILD_ECHO) $(firstword $(MCO_PRODUCT_DIRS)) > $(MCO_BUILD_ROOT)/mco-tmp-list
	$(foreach L,$(wordlist 2,$(words $(MCO_PRODUCT_DIRS)),$(MCO_PRODUCT_DIRS)),$(call step,$(L),$(MCO_BUILD_ROOT)/mco-tmp-list))
	$(foreach L,$(MCO_INTERMEDIATE_DIRS),$(call step,$(L),$(MCO_BUILD_ROOT)/mco-tmp-list))
	$(foreach L,$(MCO_PRODUCTS_LONE),$(call step,$(L),$(MCO_BUILD_ROOT)/mco-tmp-list))
	$(foreach L,$(MCO_BUILD_LOGS_LONE),$(call step,$(L),$(MCO_BUILD_ROOT)/mco-tmp-list))
	@(MCO_BUILD_CAT) $(MCO_BUILD_ROOT)/mco-tmp-list) | xargs @(MCO_BUILD_RMFILE)
	@(MCO_BUILD_RMFILE) $(MCO_BUILD_ROOT)/mco-tmp-list
	$(call mco_rule_finish,)
endef

$(eval $(call GENERATE_CLEANUP_TARGETS))

else

define GENERATE_LONE_FILES
PRJ_$(1)_INTERMEDIATES_LONE:=$(filter-out $(addsuffix /%,$(PRJ_$(1)_INTERMEDIATE_DIRS)),$(PRJ_$(1)_INTERMEDIATES))
PRJ_$(1)_BUILD_LOGS_LONE   :=$(filter-out $(addsuffix /%,$(PRJ_$(1)_INTERMEDIATE_DIRS)),$(PRJ_$(1)_BUILD_LOGS))
PRJ_$(1)_PRODUCTS_LONE     :=$(filter-out $(addsuffix /%,$(PRJ_$(1)_DIRS)),$(PRJ_$(1)_PRODUCTS))
endef
$(foreach prj,$(BUILD_PRJ_LIST),$(eval $(call GENERATE_LONE_FILES,$(prj))))

define clean_step

	$(if $(PRJ_$(1)_INTERMEDIATES)     ,$(call mco_rule_run,,$(MCO_BUILD_RMFILE) $(PRJ_$(1)_INTERMEDIATES) $(MCO_BUILD_INPARALLEL),))
	$(if $(PRJ_$(1)_INTERMEDIATE_DIRS) ,$(call mco_rule_run,,$(MCO_BUILD_RMDIR)  $(PRJ_$(1)_INTERMEDIATE_DIRS)  $(MCO_BUILD_INPARALLEL),))
endef

define distclean_step

	$(if $(PRJ_$(1)_PRODUCT_DIRS)      ,$(call mco_rule_run,,$(MCO_BUILD_RMDIR)  $(PRJ_$(1)_PRODUCT_DIRS)       $(MCO_BUILD_INPARALLEL),))
	$(if $(PRJ_$(1)_INTERMEDIATE_DIRS) ,$(call mco_rule_run,,$(MCO_BUILD_RMDIR)  $(PRJ_$(1)_INTERMEDIATE_DIRS)  $(MCO_BUILD_INPARALLEL),))
	$(if $(PRJ_$(1)_INTERMEDIATES_LONE),$(call mco_rule_run,,$(MCO_BUILD_RMFILE) $(PRJ_$(1)_INTERMEDIATES_LONE) $(MCO_BUILD_INPARALLEL),))
	$(if $(PRJ_$(1)_BUILD_LOGS_LONE)   ,$(call mco_rule_run,,$(MCO_BUILD_RMFILE) $(PRJ_$(1)_BUILD_LOGS_LONE)    $(MCO_BUILD_INPARALLEL),))
	$(if $(PRJ_$(1)_PRODUCTS_LONE)     ,$(call mco_rule_run,,$(MCO_BUILD_RMFILE) $(PRJ_$(1)_PRODUCTS_LONE)      $(MCO_BUILD_INPARALLEL),))
endef

define GENERATE_CLEANUP_TARGETS
clean: 
	$(call mco_rule_start,,Cleaning up...,)
	$(foreach prj,$(BUILD_PRJ_LIST),$(call clean_step,$(prj)))
	$(call mco_rule_finish,)

distclean: 
	$(call mco_rule_start,,Cleaning up...,)
	$(foreach prj,$(BUILD_PRJ_LIST),$(call distclean_step,$(prj)))
	$(call mco_rule_finish,)
endef

$(eval $(call GENERATE_CLEANUP_TARGETS))
endif

$(if $(findstring on,$(MCO_BUILD_LOG)),$(info Processing...))
