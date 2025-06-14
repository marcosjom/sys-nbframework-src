
################################
# MakefileFuncs is part of the NBFramework project.
# The goal of this script is to automatize the compilation
# of C/C++ projects by the definition of a Workspace, Project(s),
# Target(s), CodeGroup(s) and the rules the inclusion or exclusion
# from each compilation-task.
#
# The same definitions can generate "make" or "android-ndk" scripts.
################################

#
# Note about variables rules:
#
# The variables definied by "nbBuild*" methods for a workspace, target
# or codegroups are static, in the sense that their values will be the
# same even when the script is called multiple times with different
# parameters. This means no temporary values will interfiere when the
# script is called for different archs, as android-ndk does. Those
# variables starts with the "NB_*" prefix
#
# The call to "nbBuildWorkspaceRules" will produce temporary variables
# with prefix "_nbTmp*". Those tmp variables are suceptible to the scripts
# params and the compilation context. Those tmp vars are initialized to
# empty-string, to make safe when the script is called multiple times.
#
# This script will not generate input params variables; values should
# be stored in "NB_*" or "_nbTmp*" variables. Again, to make it safe to
# run this script multiple times.
#

#
# Note about "define defName ... endef":
# Basically it sets a multiple-lines variable.
# Use them as "$(eval $(call defName, params))".
#

#
# Cfg variables:
#
# NB_CFG_HOST				=
# NB_CFG_PRINT_INTERNALS	= [1, or any other than '1' is considered 'false']
# NB_CFG_PRINT_INFO			= [1, or any other than '1' is considered 'false']
# NB_CFG_DEBUG				= [1, or any other than '1' is considered 'false']
#
# Those variables can be passed as params to 'make', like:
# make NB_CFG_PRINT_INFO=1 NB_CFG_DEBUG=1

NB_COMMA := ,

define NB_SPACE
 
endef


define NB_TAB
	
endef

define NB_NEWLINE


endef

ifeq ($(NB_CFG_HOST),Android)
    ifeq ($(NDK_ROOT),)
        $(error NDK_ROOT must be specified, example: NDK_ROOT=/Applications/android-sdk-macosx/ndk-bundle)
    endif
    ifeq ($(ARCH),)
        $(error ARCH must be specified, example: ARCH=aarch64, or armv7a, arm, x86_64, i686)
    endif
    ifeq ($(ARCH),aarch64)
        ABI                := arm64-v8a
        PLATFORM_ARCH      := arm64
    endif
    ifeq ($(ARCH),armv7a)
        ABI                := armeabi-v7a
        PLATFORM_ARCH      := arm
    endif
    ifeq ($(ARCH),arm)
        ABI                := armeabi
        PLATFORM_ARCH      := arm
    endif
    ifeq ($(ARCH),x86_64)
        ABI                := x86_64
        PLATFORM_ARCH      := x86_64
    endif
    ifeq ($(ARCH),i686)
        ABI                := x86
        PLATFORM_ARCH      := x86
    endif
    ifeq ($(API),)
        $(error API must be specified, example: API=21)
    endif
    ifeq ($(HOST_TAG),)
        ifeq ($(OS),Windows_NT)
            HOST_TAG       := windows-x86_64
        else
            UNAME_S        := $(shell uname -s)
            ifeq ($(UNAME_S),Linux)
                HOST_TAG   := linux-x86_64
            endif
            ifeq ($(UNAME_S),Darwin)
                HOST_TAG   := darwin-x86_64
            endif
        endif
        ifeq ($(HOST_TAG),)
            $(error HOST_TAG was not specified nor could be detected, example: HOST_TAG=darwin-x86_64)
        endif
    endif
    NDK_PRIVATE_LIB_PATH   := $(NDK_ROOT)/toolchains/renderscript/prebuilt/$(HOST_TAG)/platform/$(PLATFORM_ARCH)
    NDK_LIB_PATH           := $(NDK_ROOT)/platforms/android-$(API)/arch-$(PLATFORM_ARCH)/usr/lib
    LD_FLAGS_PRE           := #-shared -Bsymbolic -z noexecstack -z relro -z now -nostdlib -L$(NDK_LIB_PATH)
    LD_FLAGS_POST          := $(NDK_PRIVATE_LIB_PATH)/libcompiler_rt.a -L$(NDK_PRIVATE_LIB_PATH) -lRSSupport -lm -lc
    LD                     := $(NDK_ROOT)/toolchains/llvm/prebuilt/$(HOST_TAG)/bin/ld.lld
    AR                     := $(NDK_ROOT)/toolchains/llvm/prebuilt/$(HOST_TAG)/bin/llvm-ar
    #
    CC                     := $(NDK_ROOT)/toolchains/llvm/prebuilt/$(HOST_TAG)/bin/clang
    CC_FLAGS_PRE           := -MP -target $(ARCH)-linux-android$(API) -fdata-sections -ffunction-sections -fstack-protector-strong -funwind-tables -no-canonical-prefixes --sysroot $(NDK_ROOT)/toolchains/llvm/prebuilt/$(HOST_TAG)/sysroot -Wno-invalid-command-line-argument -Wno-unused-command-line-argument -fno-addrsig -fpic -O2 -DNDEBUG -DANDROID -fPIC -Wa[NB_COMMA]--noexecstack -Wformat -Werror=format-security #missing comma after '-Wa'
    ifeq ($(NB_CFG_DEBUG),1)
        CC_FLAGS_POST      := -g
    else
        CC_FLAGS_POST      :=
    endif
    #
    CXX                    := $(NDK_ROOT)/toolchains/llvm/prebuilt/$(HOST_TAG)/bin/clang++
    CXX_FLAGS_PRE          := -MP -target $(ARCH)-linux-android$(API) -fdata-sections -ffunction-sections -fstack-protector-strong -funwind-tables -no-canonical-prefixes --sysroot $(NDK_ROOT)/toolchains/llvm/prebuilt/$(HOST_TAG)/sysroot -g -Wno-invalid-command-line-argument -Wno-unused-command-line-argument -fno-addrsig -fno-exceptions -fno-rtti -fpic -O2 -DNDEBUG -I$(NDK_ROOT)/sources/cxx-stl/llvm-libc++/include -I$(NDK_ROOT)/sources/cxx-stl/llvm-libc++abi/include -std=c++11 -DANDROID -fPIC -Wa[NB_COMMA]--noexecstack -Wformat -Werror=format-security #missing comma after '-Wa'
    ifeq ($(NB_CFG_DEBUG),1)
        CXX_FLAGS_POST     := -g
    else
        CXX_FLAGS_POST     :=
    endif
else
    #NB_CC_PREFIX
    ifneq ($(NB_CC_PREFIX),)
        LD                 := $(NB_CC_PREFIX)-ld
        AR                 := $(NB_CC_PREFIX)-ar
        CC                 := $(NB_CC_PREFIX)-gcc
        CXX                := $(NB_CC_PREFIX)-g++
        ifeq ($(NB_CFG_DEBUG),1)
            CC_FLAGS_PRE   :=
            CC_FLAGS_POST  := -g
        else
            CC_FLAGS_PRE   :=
            CC_FLAGS_POST  :=
        endif
        ifeq ($(NB_CFG_DEBUG),1)
            CXX_FLAGS_PRE  :=
            CXX_FLAGS_POST := -g
        else
            CXX_FLAGS_PRE  :=
            CXX_FLAGS_POST :=
        endif
    else
        #LD
        ifeq ($(LD),)
            LD             := ld
        endif
        #AR
        ifeq ($(AR),)
            AR             := ar
        endif
        #cc
        ifeq ($(CC),)
            CC             := gcc
        endif
        ifeq ($(NB_CFG_DEBUG),1)
            CC_FLAGS_PRE   :=
            CC_FLAGS_POST  := -g
        else
            CC_FLAGS_PRE   :=
            CC_FLAGS_POST  :=
        endif
        #CXX
        ifeq ($(CXX),)
            CXX            := g++
        endif
        ifeq ($(NB_CFG_DEBUG),1)
            CXX_FLAGS_PRE  :=
            CXX_FLAGS_POST := -g
        else
            CXX_FLAGS_PRE  :=
            CXX_FLAGS_POST :=
        endif
    endif
    #LD_FLAGS_PRE
    ifeq ($(LD_FLAGS_PRE),)
        LD_FLAGS_PRE       := --shared
    endif
    ifeq ($(LD_FLAGS_POST),)
        LD_FLAGS_POST      :=
    endif
endif

#------------------
# UNIVERSAL METHODS
#------------------

#
#Returns the provided name as a usable variable-name
#param $1: name
#
define nbNameAsVar
$(subst -,_,$(subst .,_,$(1)))
endef

#
#Returns the value of a var, or $2 if undefined, or $3 if empty
#param $1: name
#param $2: value if undefined
#param $3: value if empty
#
define nbVarVal
$(if $(findstring undefined,$(flavor $(1))),$(2),$(if $(strip $($(1))),$($(1)),$(3)))
endef

#
#Returns the provided list as a vertical list
#param $1: prefix-per-line (usually white spaces)
#param $2: list
#
define nbListToVertical
$(addprefix \$(NB_NEWLINE)$(1),$(2))
endef

#
#Returns the provided list-of-paths with a
#root-prefix added except on abosulte paths.
#param $1: root to add
#param $2: paths-list
#
define nbConcatRootToPaths
$(addprefix $(1),$(filter-out /%,$(2))) $(filter /%,$(2))
endef

#
#Returns 1 if the FLAG is in the list in the form of '-DMACRO' or '-DMACRO=*'
#param $1: macro to seacrh
#param $2: flag-list
#Note: extra speces on this code are importante, do not remove.
#
define nbFlagsHasMacro
$(strip $(if $(or $(filter -D$(1),$(2)),$(findstring $(NB_SPACE)-D$(1)=,$(NB_SPACE)$(2))),1,))
endef

#
#Returns '1' for each flag found in the form of '-DMACRO' or '-DMACRO=*'
#param $1: macros list to search
#param $2: flags-list
#
define nbFlagsHasAnyMacros
$(strip $(foreach macro,$(1),$(if $(call nbFlagsHasMacro,$(macro),$(2)),has_$(macro),)))
endef

#
#Returns '1' for each flag not-found in the form of '-DMACRO' or '-DMACRO=*'
#param $1: macros list to search
#param $2: flags-list
#
define nbFlagsMissingAnyMacros
$(strip $(foreach macro,$(1),$(if $(call nbFlagsHasMacro,$(macro),$(2)),,missing_$(macro))))
endef
    
#
#Returns the provided list-of-paths with a
#prefix and a root-prefix added except on abosulte paths.
#param $1, prefix to add
#param $2, root to add
#param $3, paths-list
#
define nbConcatPrefixAndRootToPaths
$(addprefix $(1)$(2),$(filter-out /%,$(3))) $(addprefix $(1),$(filter /%,$(3)))
endef

#
#ToDo: use 'filter' function instead of 'nbFindWord'
#
#Returns a list of 1s for each time the word is found in the provided list.
#param $1, word to find
#param $2, list to search
#
define nbFindWord
$(strip $(foreach word,$(2),$(if $(subst $(1),,$(word)),, 1)))
endef

#-------------------------
# CALL
#-------------------------
#NB_CFG
#NB_CFG_PRINT_INTERNALS         := 1 #any other than '1' is considered 'false'
#NB_CFG_PRINT_INFO              := 1 #any other than '1' is considered 'false'

#Note: musts be used as "$(eval $(call nbCall,__funcName__))"
define nbCall
    #Print internals
    $(if $(findstring 1,$(NB_CFG_PRINT_INTERNALS)),$(eval $(info $(call $(1)))),)
    #Print info
    $(if $(findstring 1,$(NB_CFG_PRINT_INFO)),
        $(if $(findstring nbBuild,$(1)),
            $(eval $(call $(1)PrintInfo))
        )
    )
    #
    $(call $(1))
endef

#-------------------------
# WORKSPACE
#-------------------------

ifeq ($$(NB_CFG_HOST),Android)
define nbInitWorkspacePath
NB_WORKSPACE_OUT_DIR_BIN  := bin/make/$(if $(findstring 1,$(DEBUG)),debug,release)/$(NB_CFG_HOST)/$(ABI)
    NB_WORKSPACE_OUT_DIR_OBJS := tmp/make/$(if $(findstring 1,$(DEBUG)),debug,release)/$(NB_CFG_HOST)/$(ABI)
endef
else
ifeq ($(OS),Windows_NT)
define nbInitWorkspacePath
NB_WORKSPACE_OUT_DIR_BIN  := bin/make/$(if $(findstring 1,$(DEBUG)),debug,release)/$(OS)/$(PROCESSOR_ARCHITEW6432)
    NB_WORKSPACE_OUT_DIR_OBJS := tmp/make/$(if $(findstring 1,$(DEBUG)),debug,release)/$(OS)/$(PROCESSOR_ARCHITEW6432)
endef
else
define nbInitWorkspacePath
NB_WORKSPACE_OUT_DIR_BIN  := bin/make/$(if $(findstring 1,$(DEBUG)),debug,release)/$(shell uname -s)/$(shell uname -p)/$(CC)
    NB_WORKSPACE_OUT_DIR_OBJS := tmp/make/$(if $(findstring 1,$(DEBUG)),debug,release)/$(shell uname -s)/$(shell uname -p)/$(CC)
endef
endif
endif

define nbInitWorkspace
    #############
    # initWorkspace
    #############
    #
    #NB_CFG_HOST is '$(NB_CFG_HOST)'
    #
    NB_WORKSPACE_REL_DIR        := $(dir $(lastword $(MAKEFILE_LIST)))
    NB_WORKSPACE_BLD_CFG        := $(if $(findstring 1,$(DEBUG)),debug,release)
    NB_WORKSPACE_PROJECTS_      :=
    $(nbInitWorkspacePath)
    #
endef

define nbUndefineWorkspace
    $(info Undefining worspace.)\
    $(foreach proj,$(NB_WORKSPACE_PROJECTS_),\
        $(call nbUndefineProject,$(proj))\
    )
    NB_WORKSPACE_REL_DIR:=
    NB_WORKSPACE_BLD_CFG:=
    NB_WORKSPACE_PROJECTS_:=
endef

define nbBuildWorkspaceRulesAndroid
    ################
    # buildWorkspace
    ################
    $(eval _nbTmpAllTargetsNames:=)\
    $(eval _nbTmpTargetsDisabled:=)\
    $(if $(findstring undefined,$(flavor NB_TARGET_TO_COMPILE)),\
       $(error 'NB_TARGET_TO_COMPILE' is required for Android compilation [undefined]),\
    )\
    $(if $(strip $(NB_TARGET_TO_COMPILE)),,\
       $(error 'NB_TARGET_TO_COMPILE' is required for Android compilation [empty]),\
    )\
    $(foreach project,$(NB_WORKSPACE_PROJECTS_),\
      $(foreach target,$($(call nbNameAsVar,$(project))_NB_PROJECT_TARGETS_),\
        $(if $(findstring undefined,$(flavor NB_TARGET_TO_COMPILE)),
          #########
          # Context '$(target)' (autogenerated, undefined filter)
          #########
          $(call nbBuildWorkspaceRulesTarget,$(call nbNameAsVar,$(target)))
          ,\
          $(if $(strip $(NB_TARGET_TO_COMPILE)),\
            $(if $(filter $(strip $(target)),$(NB_TARGET_TO_COMPILE)),
                #########
                # Context '$(target)' (autogenerated, filtering by '$(NB_TARGET_TO_COMPILE)')
                #########
                $(call nbBuildWorkspaceRulesTarget,$(call nbNameAsVar,$(target)))
              ,
                #########
                # Context '$(target)' (ignoring, filtering by '$(NB_TARGET_TO_COMPILE)')
                #########
            )
          ,
            #########
            # Context '$(target)' (autogenerated, empty filter)
            #########
            $(call nbBuildWorkspaceRulesTarget,$(call nbNameAsVar,$(target)))
         )\
        )\
      )\
    )\
    #
	#echo '$(if $(_nbTmpTargetsDisabled),Warning: these targets were disabled until missing files are included: $(strip $(_nbTmpTargetsDisabled)).,All targets rules were available.)'
    #
endef

define nbBuildWorkspaceRulesDefault
    ################
    # buildWorkspace
    ################
    $(eval _nbTmpAllTargetsNames:=)\
    $(eval _nbTmpTargetsDisabled:=)\
    $(foreach project,$(NB_WORKSPACE_PROJECTS_),\
      $(foreach target,$($(call nbNameAsVar,$(project))_NB_PROJECT_TARGETS_),\
        $(if $(findstring undefined,$(flavor NB_TARGET_TO_COMPILE)),
          #########
          # Context '$(target)' (autogenerated, undefined filter)
          #########
          $(call nbBuildWorkspaceRulesTarget,$(call nbNameAsVar,$(target)))
          ,\
          $(if $(strip $(NB_TARGET_TO_COMPILE)),\
            $(if $(filter $(strip $(target)),$(NB_TARGET_TO_COMPILE)),
                #########
                # Context '$(target)' (autogenerated, filtering by '$(NB_TARGET_TO_COMPILE)')
                #########
                $(call nbBuildWorkspaceRulesTarget,$(call nbNameAsVar,$(target)))
              ,
                #########
                # Context '$(target)' (ignoring, filtering by '$(NB_TARGET_TO_COMPILE)')
                #########
            )
          ,
            #########
            # Context '$(target)' (autogenerated, empty filter)
            #########
            $(call nbBuildWorkspaceRulesTarget,$(call nbNameAsVar,$(target)))
         )\
        )\
      )\
    )\
    #
    all: $(strip $(subst ",,$(_nbTmpAllTargetsNames))) end-of-target
    #
    end-of-target:
	echo '$(if $(_nbTmpTargetsDisabled),Warning: these targets were disabled until missing files are included: $(strip $(_nbTmpTargetsDisabled)).,All targets rules were available.)'
    #
endef

define nbBuildWorkspaceRules
    $(if $(findstring AndroidNdkBuild,$(NB_CFG_HOST)),\
      $(call nbBuildWorkspaceRulesAndroid)\
      ,\
      $(call nbBuildWorkspaceRulesDefault)\
    )\
    $(call nbUndefineWorkspace)
endef

#populates the list of dependencies once; it validates circular references.
#param $1, target var name
define nbBuildWorkspaceRulesTarget
    $(eval nbDefsTargetFlags_:=)\
    $(eval nbDefsTargetObjs_:=)\
    $(eval nbDefsTargetOutDirs_:=)\
    $(eval nbDefsTargetLibs_:=)\
    $(eval nbDefsTargetLibsPaths_:=)\
    $(eval nbDefsTargetBuildsStatic_:=)\
    $(eval nbDefsTargetBuildsShared_:=)\
    $(eval nbDefsTargetBuildsExe_:=)\
    \
    $(eval _nbTmp$(1)SubTargets:=)\
    $(eval _nbTmp$(1)RecursionVisited:=$(1))\
    \
    $(eval _nbTmp$(1)RecursiveFlags:=)\
    $(eval _nbTmp$(1)RecursionVisited:=$(1))\
    $(call nbDefsTargetRulesFlagsRecursiveFromEnvVars,$(1),$(1))\
    $(eval _nbTmp$(1)RecursionVisited:=$(1))\
    $(call nbDefsTargetRulesFlagsRecursiveFromRulesMatch,1,$(1),$(1))\
    $(info _nbTmp$(1)RecursiveFlags := $(_nbTmp$(1)RecursiveFlags))
    \
    $(if $($(1)_NB_TARGET_IS_DISABLED_),\
        $(eval _nbTmpTargetsDisabled += $($(1)_NB_TARGET_NAME))\
    ,\
        #Target dependencies: $(_nbTmp$(1)SubTargets)
        #Target flags: $(nbDefsTargetFlags_)
        $(eval _nbTmp$(1)RecursionVisited:=$(1))\
        $(call nbDefsTargetCodeGroupsRules,$(1),$(1),_nbTmp$(1)SubTargets,nbDefsTargetFlags_,nbDefsTargetObjs_,nbDefsTargetOutDirs_,nbDefsTargetLibs_,nbDefsTargetLibsPaths_,nbDefsTargetBuildsStatic_,nbDefsTargetBuildsShared_,nbDefsTargetBuildsExe_)\
        $(call nbDefsTargetRootRules,$(1),nbDefsTargetFlags_,nbDefsTargetObjs_,nbDefsTargetOutDirs_,nbDefsTargetLibs_,nbDefsTargetLibsPaths_,nbDefsTargetBuildsStatic_,nbDefsTargetBuildsShared_,nbDefsTargetBuildsExe_)\
        $(if $(filter "$($(1)_NB_TARGET_NAME)",$(_nbTmpAllTargetsNames)),,$(eval _nbTmpAllTargetsNames += "$($(1)_NB_TARGET_NAME)"))\
    )
endef

#Print
define nbBuildWorkspaceRulesPrintInfo
    $(info ------------)
    $(info - WORSPACE -)
    $(info ------------)
    $(info NB_WORKSPACE_REL_DIR        := $(NB_WORKSPACE_REL_DIR))
    $(info NB_WORKSPACE_BLD_CFG        := $(NB_WORKSPACE_BLD_CFG))
    $(info NB_WORKSPACE_PROJECTS_      := $(strip $(NB_WORKSPACE_PROJECTS_)))
endef

#-------------------------
# PROJECT
#-------------------------

define nbInitProject
    #############
    # initProject
    #############
    NB_PROJECT_REL_DIR          := $(dir $(lastword $(MAKEFILE_LIST)))
    NB_PROJECT_NAME             :=
    NB_PROJECT_CFLAGS           :=
    NB_PROJECT_CXXFLAGS         :=
    NB_PROJECT_LDFLAGS          :=
    NB_PROJECT_INCLUDES         :=
    NB_PROJECT_LIBS_PATHS       :=
    NB_PROJECT_DEPEND_FLAGS     :=
    NB_PROJECT_DEPEND_LIBS      :=
    NB_PROJECT_TARGETS_         :=
endef

define nbBuildProjectRules
    $(if $(findstring undefined,$(flavor NB_PROJECT_REL_DIR)),
        $(error Mising 'nbInitProject' before 'nbBuildProjectRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]'))'),
    )$(if $(strip $(NB_PROJECT_REL_DIR)),,
        $(error Mising 'nbInitProject' before 'nbBuildProjectRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]'))'),
    )$(if $(findstring undefined,$(flavor NB_PROJECT_NAME)),
        $(error 'NB_PROJECT_NAME' required before 'nbBuildProjectRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]'))'),
    )$(if $(strip $(NB_PROJECT_NAME)),,
        $(error 'NB_PROJECT_NAME' required before 'nbBuildProjectRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]'))')
    )
    ###############
    # buildProject: /$(NB_PROJECT_NAME)
    ###############
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_REL_DIR          := $(NB_PROJECT_REL_DIR)
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_NAME             := $(NB_PROJECT_NAME)
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_CFLAGS           := $(strip $(NB_PROJECT_CFLAGS))
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_CXXFLAGS         := $(strip $(NB_PROJECT_CXXFLAGS))
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_LDFLAGS          := $(strip $(NB_PROJECT_LDFLAGS))
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_INCLUDES         := $(strip $(NB_PROJECT_INCLUDES))
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_LIBS_PATHS       := $(strip $(NB_PROJECT_LIBS_PATHS))
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_DEPEND_FLAGS     := $(strip $(NB_PROJECT_DEPEND_FLAGS))
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_DEPEND_LIBS      := $(strip $(NB_PROJECT_DEPEND_LIBS))
    $(call nbNameAsVar,$(NB_PROJECT_NAME))_NB_PROJECT_TARGETS_         := $(strip $(NB_PROJECT_TARGETS_))
    NB_WORKSPACE_PROJECTS_      += $(call nbNameAsVar,$(NB_PROJECT_NAME))
    #############
    # initProject
    #############
    NB_PROJECT_REL_DIR          := $(dir $(lastword $(MAKEFILE_LIST)))
    NB_PROJECT_NAME             :=
    NB_PROJECT_CFLAGS           :=
    NB_PROJECT_CXXFLAGS         :=
    NB_PROJECT_LDFLAGS          :=
    NB_PROJECT_INCLUDES         :=
    NB_PROJECT_LIBS_PATHS       :=
    NB_PROJECT_DEPEND_FLAGS     :=
    NB_PROJECT_DEPEND_LIBS      :=
    NB_PROJECT_TARGETS_         :=
    #
endef

#1, project var-name
define nbUndefineProject
    $(info Undefining project '$(1)'.)\
    $(foreach target,$($(1)_NB_PROJECT_TARGETS_),\
        $(call nbUndefineTarget,$(target))\
    )
    $(1)_NB_PROJECT_REL_DIR:=
    $(1)_NB_PROJECT_NAME:=
    $(1)_NB_PROJECT_CFLAGS:=
    $(1)_NB_PROJECT_CXXFLAGS:=
    $(1)_NB_PROJECT_LDFLAGS:=
    $(1)_NB_PROJECT_INCLUDES:=
    $(1)_NB_PROJECT_LIBS_PATHS:=
    $(1)_NB_PROJECT_DEPEND_LIBS:=
    $(1)_NB_PROJECT_TARGETS_:=
endef


#Print
define nbBuildProjectRulesPrintInfo
    $(info -----------)
    $(info - PROJECT: /$(NB_PROJECT_NAME))
    $(info -----------)
    $(info NB_PROJECT_REL_DIR          := $(NB_PROJECT_REL_DIR))
    $(info NB_PROJECT_NAME             := $(NB_PROJECT_NAME))
    $(info NB_PROJECT_CFLAGS           := $(strip $(NB_PROJECT_CFLAGS)))
    $(info NB_PROJECT_CXXFLAGS         := $(strip $(NB_PROJECT_CXXFLAGS)))
    $(info NB_PROJECT_LDFLAGS          := $(strip $(NB_PROJECT_LDFLAGS)))
    $(info NB_PROJECT_INCLUDES         := $(strip $(NB_PROJECT_INCLUDES)))
    $(info NB_PROJECT_LIBS_PATHS       := $(strip $(NB_PROJECT_LIBS_PATHS)))
    $(info NB_PROJECT_DEPEND_FLAGS     := $(strip $(NB_PROJECT_DEPEND_FLAGS)))
    $(info NB_PROJECT_DEPEND_LIBS      := $(strip $(NB_PROJECT_DEPEND_LIBS)))
    $(info NB_PROJECT_TARGETS_         := $(strip $(NB_PROJECT_TARGETS_)))
endef

#-------------------------
# TARGET
#-------------------------

#Init
define nbInitTarget
    #############
    # initTarget
    #############
    NB_TARGET_REL_DIR           := $(NB_PROJECT_REL_DIR)
    NB_TARGET_NAME              :=
    NB_TARGET_PREFIX            :=
    NB_TARGET_SUFIX             :=
    NB_TARGET_TYPE              :=
    NB_TARGET_CFLAGS            := $(NB_PROJECT_CFLAGS)
    NB_TARGET_CXXFLAGS          := $(NB_PROJECT_CXXFLAGS)
    NB_TARGET_LDFLAGS           := $(NB_PROJECT_LDFLAGS)
    NB_TARGET_INCLUDES          := $(NB_PROJECT_INCLUDES)
    NB_TARGET_FRAMEWORKS        :=
    NB_TARGET_STATIC_LIBS       :=
    NB_TARGET_SHARED_LIBS       :=
    NB_TARGET_LIBS              :=
    NB_TARGET_LIBS_PATHS        := $(NB_PROJECT_LIBS_PATHS)
    NB_TARGET_DEPENDS           :=
    NB_TARGET_FLAGS_ENABLES     :=
    NB_TARGET_CODEGROUPS_       :=
    #
endef

#Build (public)
define nbBuildTargetRules
    $(if $(findstring undefined,$(flavor NB_PROJECT_REL_DIR)),\
        $(error Mising 'nbInitProject' before 'nbBuildTargetRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))'),\
    )\
    $(if $(strip $(NB_PROJECT_REL_DIR)),,\
        $(error Mising 'nbInitProject' before 'nbBuildTargetRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))'),\
    )\
    $(if $(findstring undefined,$(flavor NB_PROJECT_NAME)),\
        $(error 'NB_PROJECT_NAME' required before 'nbBuildTargetRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))'),\
    )\
    $(if $(strip $(NB_PROJECT_NAME)),,\
        $(error 'NB_PROJECT_NAME' required before 'nbBuildTargetRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))')\
    )\
    $(if $(findstring undefined,$(flavor NB_TARGET_NAME)),\
        $(error 'NB_TARGET_NAME' required before 'nbBuildTargetRules'(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))),\
    )\
    $(if $(strip $(NB_TARGET_NAME)),,\
        $(error 'NB_TARGET_NAME' required before 'nbBuildTargetRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))')\
    )\
    $(if $(findstring undefined,$(flavor $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME)),,\
        $(if $(strip $($(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME)),\
            $(warning '$(NB_TARGET_NAME)' already in use as target name at 'nbBuildTargetRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))'; each target in the workspace must have an unique 'NB_TARGET_NAME'; have in mind that two similar names can be normalized internally to the same variable name after replacing undesired chars; found '$(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME := $($(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME)'.)\
        ,\
            \
        )\
    )\
    ###############
    # buildTarget: /$(NB_PROJECT_NAME)/$(NB_TARGET_NAME)
    ###############
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_REL_DIR        := $(NB_TARGET_REL_DIR)
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME           := $(NB_TARGET_NAME)
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_PREFIX         := $(NB_TARGET_PREFIX)
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_SUFIX          := $(NB_TARGET_SUFIX)
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_TYPE           := $(NB_TARGET_TYPE)
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_CFLAGS         := $(strip $(NB_TARGET_CFLAGS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_CXXFLAGS       := $(strip $(NB_TARGET_CXXFLAGS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_LDFLAGS        := $(strip $(NB_TARGET_LDFLAGS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_INCLUDES       := $(strip $(NB_TARGET_INCLUDES))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_FRAMEWORKS     := $(strip $(NB_TARGET_FRAMEWORKS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_STATIC_LIBS    := $(strip $(NB_TARGET_STATIC_LIBS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_SHARED_LIBS    := $(strip $(NB_TARGET_SHARED_LIBS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_LIBS           := $(strip $(NB_TARGET_LIBS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_LIBS_PATHS     := $(strip $(NB_TARGET_LIBS_PATHS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_DEPENDS        := $(strip $(NB_TARGET_DEPENDS))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_FLAGS_ENABLES  := $(strip $(NB_TARGET_FLAGS_ENABLES))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_CODEGROUPS_    := $(strip $(NB_TARGET_CODEGROUPS_))
    $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_IS_DISABLED_   :=
    NB_PROJECT_TARGETS_ += $(call nbNameAsVar,$(NB_TARGET_NAME))
    #
    #############
    # initTarget
    #############
    NB_TARGET_REL_DIR           := $(NB_PROJECT_REL_DIR)
    NB_TARGET_NAME              :=
    NB_TARGET_PREFIX            :=
    NB_TARGET_SUFIX             :=
    NB_TARGET_TYPE              :=
    NB_TARGET_CFLAGS            := $(NB_PROJECT_CFLAGS)
    NB_TARGET_CXXFLAGS          := $(NB_PROJECT_CXXFLAGS)
    NB_TARGET_LDFLAGS           := $(NB_PROJECT_LDFLAGS)
    NB_TARGET_INCLUDES          := $(NB_PROJECT_INCLUDES)
    NB_TARGET_FRAMEWORKS        :=
    NB_TARGET_STATIC_LIBS       :=
    NB_TARGET_SHARED_LIBS       :=
    NB_TARGET_LIBS              :=
    NB_TARGET_LIBS_PATHS        := $(NB_PROJECT_LIBS_PATHS)
    NB_TARGET_DEPENDS           :=
    NB_TARGET_FLAGS_ENABLES     :=
    NB_TARGET_CODEGROUPS_       :=
    #
    #
endef


#1, target var-name
define nbUndefineTarget
    $(info Undefining target '$(1)'.)\
    $(foreach codegrp,$($(1)_NB_TARGET_CODEGROUPS_),\
        $(call nbUndefineCodeGroup,$(1),$(codegrp))\
    )
    $(1)_NB_TARGET_REL_DIR:=
    $(1)_NB_TARGET_NAME:=
    $(1)_NB_TARGET_PREFIX:=
    $(1)_NB_TARGET_SUFIX:=
    $(1)_NB_TARGET_TYPE:=
    $(1)_NB_TARGET_CFLAGS:=
    $(1)_NB_TARGET_CXXFLAGS:=
    $(1)_NB_TARGET_LDFLAGS:=
    $(1)_NB_TARGET_INCLUDES:=
    $(1)_NB_TARGET_FRAMEWORKS:=
    $(1)_NB_TARGET_STATIC_LIBS:=
    $(1)_NB_TARGET_SHARED_LIBS:=
    $(1)_NB_TARGET_LIBS:=
    $(1)_NB_TARGET_LIBS_PATHS:=
    $(1)_NB_TARGET_DEPENDS:=
    $(1)_NB_TARGET_FLAGS_ENABLES:=
    $(1)_NB_TARGET_CODEGROUPS_:=
    $(1)_NB_TARGET_IS_DISABLED_:=
endef

#Print
define nbBuildTargetRulesPrintInfo
    $(info ----------)
    $(info - TARGET: /$(NB_PROJECT_NAME)/$(NB_TARGET_NAME))
    $(info ----------)
    $(info NB_TARGET_REL_DIR           := $(NB_TARGET_REL_DIR))
    $(info NB_TARGET_NAME              := $(NB_TARGET_NAME))
    $(info NB_TARGET_PREFIX            := $(NB_TARGET_PREFIX))
    $(info NB_TARGET_SUFIX             := $(NB_TARGET_SUFIX))
    $(info NB_TARGET_TYPE              := $(NB_TARGET_TYPE))
    $(info NB_TARGET_CFLAGS            := $(strip $(NB_TARGET_CFLAGS)))
    $(info NB_TARGET_CXXFLAGS          := $(strip $(NB_TARGET_CXXFLAGS)))
    $(info NB_TARGET_LDFLAGS           := $(strip $(NB_TARGET_LDFLAGS)))
    $(info NB_TARGET_INCLUDES          := $(strip $(NB_TARGET_INCLUDES)))
    $(info NB_TARGET_FRAMEWORKS        := $(strip $(NB_TARGET_FRAMEWORKS)))
    $(info NB_TARGET_STATIC_LIBS       := $(strip $(NB_TARGET_STATIC_LIBS)))
    $(info NB_TARGET_SHARED_LIBS       := $(strip $(NB_TARGET_SHARED_LIBS)))
    $(info NB_TARGET_LIBS              := $(strip $(NB_TARGET_LIBS)))
    $(info NB_TARGET_LIBS_PATHS        := $(strip $(NB_TARGET_LIBS_PATHS)))
    $(info NB_TARGET_DEPENDS           := $(strip $(NB_TARGET_DEPENDS)))
    $(info NB_TARGET_CODEGROUPS_       := $(strip $(NB_TARGET_CODEGROUPS_)))
endef
    
#populates the list of flags dependencies found (*_FLAGS_REQUIRED, *_FLAGS_FORBIDDEN and *_FLAGS_ENABLES) from make user-input.
#param $1, root-target var-name
#param $2, target var-name
#
define nbDefsTargetRulesFlagsRecursiveFromEnvVars
    $(foreach flag,$($(2)_NB_TARGET_FLAGS_ENABLES),\
        $(if $(call nbFlagsHasMacro,$(flag),$(_nbTmp$(1)RecursiveFlags)),\
            \
        ,\
            $(if $(filter undefined,$(flavor $(flag))),\
                \
            ,\
                $(info $(1)/$(1)/$(2) flag-enables '$(flag)' found as env var: '$($(flag))' (env-step).)\
                $(eval _nbTmp$(1)RecursiveFlags += -D$(flag)=$($(flag)))\
            )\
        )\
    )
    $(foreach codegrp,$($(2)_NB_TARGET_CODEGROUPS_),\
        $(foreach flag,$($(2)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_REQUIRED),\
            $(if $(call nbFlagsHasMacro,$(flag),$(_nbTmp$(1)RecursiveFlags)),\
                \
            ,\
                $(if $(filter undefined,$(flavor $(flag))),\
                ,\
                    $(info $(1)/$(1)/$(2) flag-required '$(flag)' found as env var: '$($(flag))' (env-step).)\
                    $(eval _nbTmp$(1)RecursiveFlags += -D$(flag)=$($(flag)))\
                )\
            )\
        )\
        $(foreach flag,$($(2)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_FORBIDDEN),\
            $(if $(call nbFlagsHasMacro,$(flag),$(_nbTmp$(1)RecursiveFlags)),\
                \
            ,\
                $(if $(filter undefined,$(flavor $(flag))),\
                ,\
                    $(info $(1)/$(1)/$(2) flag-forbidden '$(flag)' found as env var: '$($(flag))' (env-step).)\
                    $(eval _nbTmp$(1)RecursiveFlags += -D$(flag)=$($(flag)))\
                )\
            )\
        )\
        $(foreach flag,$($(2)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_ENABLES),\
            $(if $(call nbFlagsHasMacro,$(flag),$(_nbTmp$(1)RecursiveFlags)),\
                \
            ,\
                $(if $(filter undefined,$(flavor $(flag))),\
                ,\
                    $(info $(1)/$(1)/$(2) flag-enables '$(flag)' found as env var: '$($(flag))' (env-step).)\
                    $(eval _nbTmp$(1)RecursiveFlags += -D$(flag)=$($(flag)))\
                )\
            )\
        )\
    )\
    $(foreach subtarget,$($(2)_NB_TARGET_DEPENDS),\
        $(if $(filter $(call nbNameAsVar,$(subtarget)),$(_nbTmp$(1)RecursionVisited)),\
            $(info $(1)/$(1)/$(2) skipping subtarget '$(1)/$(2)/$(subtarget)' already-anlzd (env-step).)\
        ,\
            $(info $(1)/$(1)/$(2) analyzing subtarget '$(1)/$(2)/$(subtarget)' (env-step).)\
            $(eval _nbTmp$(1)RecursionVisited += $(call nbNameAsVar,$(subtarget)))\
            $(call nbDefsTargetRulesFlagsRecursiveFromEnvVars,$(1),$(call nbNameAsVar,$(subtarget)))\
        )\
    )
endef

#populates the list of flags dependencies found (*_FLAGS_REQUIRED, *_FLAGS_FORBIDDEN and *_FLAGS_ENABLES) from code groups that match rules
#param $1, recursive-call-depth
#param $2, root-target var-name
#param $3, target var-name
#
define nbDefsTargetRulesFlagsRecursiveFromRulesMatch
    $(if $(filter $(2),$(3)),\
        $(eval _nbTmp$(2)RecursionVisited:=$(2))\
        $(eval _nbTmp$(2)FlagAdded:=)\
    ,\
        \
    )\
    $(foreach flag,$($(3)_NB_TARGET_FLAGS_ENABLES),\
        $(if $(call nbFlagsHasMacro,$(flag),$(_nbTmp$(2)RecursiveFlags)),\
            \
        ,\
            $(if $(filter undefined,$(flavor $(flag))),\
                $(info $(1)/$(2)/$(3) flag-enables '$(flag)' enabled with empty value (rules-anlz step).)\
                $(eval _nbTmp$(2)RecursiveFlags += -D$(flag))\
                $(eval _nbTmp$(2)FlagAdded := 1)\
            ,\
                $(info $(1)/$(2)/$(3) flag-enables '$(flag)' enabled with env-var value: '$($(flag))' (rules-anlz step).)\
                $(eval _nbTmp$(2)RecursiveFlags += -D$(flag)=$($(flag)))\
                $(eval _nbTmp$(2)FlagAdded := 1)\
            )\
        )\
    )\
    $(foreach codegrp,$($(3)_NB_TARGET_CODEGROUPS_),\
        $(if $(call nbCodeGroupRuleMatch2,$($(3)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_REQUIRED),$($(3)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_FORBIDDEN),$(_nbTmp$(2)RecursiveFlags)),\
            $(info $(1)/$(2)/$(3)/$(codegrp) matches rules (will be compiled for this root-target).)
            $(foreach flag,$($(3)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_ENABLES),\
                $(if $(call nbFlagsHasMacro,$(flag),$(_nbTmp$(2)RecursiveFlags)),\
                    \
                ,\
                    $(if $(filter undefined,$(flavor $(flag))),\
                        $(info $(1)/$(2)/$(3)/$(codegrp) flag-enables '$(flag)' enabled with empty value (rules-anlz step).)\
                        $(eval _nbTmp$(2)RecursiveFlags += -D$(flag))\
                        $(eval _nbTmp$(2)FlagAdded := 1)\
                    ,\
                        $(info $(1)/$(2)/$(3)/$(codegrp) flag-enables '$(flag)' enabled with env-var value: '$($(flag))' (rules-anlz step).)\
                        $(eval _nbTmp$(2)RecursiveFlags += -D$(flag)=$($(flag)))\
                        $(eval _nbTmp$(2)FlagAdded := 1)\
                    )\
                )\
            )\
        ,\
            $(info $(1)/$(2)/$(3)/$(codegrp) doesnt matches rules (wont be compiled for this root-target).)\
        )\
    )\
    $(foreach subtarget,$($(3)_NB_TARGET_DEPENDS),\
        $(if $(filter $(call nbNameAsVar,$(subtarget)),$(_nbTmp$(2)RecursionVisited)),
            $(info $(1)/$(2)/$(3) skipping subtarget '$(2)/$(3)/$(subtarget)' already-anlzd (rules-anlz step).)\
        ,\
            $(info $(1)/$(2)/$(3) analyzing subtarget '$(2)/$(3)/$(subtarget)' (rules-anlz step).)\
            $(eval _nbTmp$(2)RecursionVisited += $(call nbNameAsVar,$(subtarget)))\
            $(call nbDefsTargetRulesFlagsRecursiveFromRulesMatch,$(1)_$(1),$(2),$(call nbNameAsVar,$(subtarget)))\
        )\
    )\
    $(if $(and $(filter $(2),$(3)),$(_nbTmp$(2)FlagAdded)),\
        $(call nbDefsTargetRulesFlagsRecursiveFromRulesMatch,$(1)_$(1),$(2),$(3))\
    ,\
        \
    )
endef

#returns the rules for target's codeGroups compilation
#param $1, root-target's var name
#param $2, target's var name
#param $3, list for dependencies targets names
#param $4, list for flags set
#param $5, list for objs (build-rules) added
#param $6, list for unique output dirs
#param $7, list for unique libs to link
#param $8, list for unique libs paths
#param $9, list for static-libs (build-rules) added
#param $10, list for shared-libs (build-rules) added
#param $11, list for exes (build-rules) added
#
define nbDefsTargetCodeGroupsRules
    $(if $(findstring "$(NB_WORKSPACE_OUT_DIR_BIN)",$($(6))),,$(eval $(6) += "$(NB_WORKSPACE_OUT_DIR_BIN)"))\
    $(if $(findstring "$(NB_WORKSPACE_OUT_DIR_OBJS)",$($(6))),,$(eval $(6) += "$(NB_WORKSPACE_OUT_DIR_OBJS)"))\
    $(if $(findstring "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)",$($(6))),,$(eval $(6) += "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)"))\
    $(foreach reqLib,$($(2)_NB_TARGET_LIBS),\
        $(if $(findstring "$(reqLib)",$($(7))),,$(eval $(7) += "$(reqLib)"))\
    )\
    $(foreach reqLibPath,$($(2)_NB_TARGET_LIBS_PATHS),\
        $(if $(findstring "$(reqLibPath)",$($(8))),,$(eval $(8) += "$(reqLibPath)"))\
    )\
    $(foreach subtarget,$($(2)_NB_TARGET_DEPENDS),\
        $(if $(filter $(call nbNameAsVar,$(subtarget)),$(_nbTmp$(1)RecursionVisited)),\
            \
        ,\
             $(eval _nbTmp$(1)RecursionVisited += $(call nbNameAsVar,$(subtarget)))\
             $(if $(call nbFindWord,undefined,$(flavor $(call nbNameAsVar,$(subtarget))_NB_TARGET_NAME)),\
                $(warning Sub-target '$(subtarget)' not found; target '$($(1)_NB_TARGET_NAME)' will be disabled until dependencies are included.)\
                $(eval $(1)_NB_TARGET_IS_DISABLED_ := 1)\
             ,\
                #Subtarget '$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)'; found '$(call nbFindWord,$($(1)_NB_TARGET_TYPE),static shared)' found '$(call nbFindWord,$($(2)_NB_TARGET_TYPE),static shared)'
                #Subtarget and '$(and $(call nbFindWord,$($(1)_NB_TARGET_TYPE),static),$(call nbFindWord,$($(2)_NB_TARGET_TYPE),static))'
                $(if $(and $(call nbFindWord,$($(1)_NB_TARGET_TYPE),static),$(call nbFindWord,$($(2)_NB_TARGET_TYPE),static)),\
                    #Subtarget ignored '$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)'; parent and child targtes are static libraries.
                ,\
                    $(eval nbDefsTargetObjs_$(call nbNameAsVar,$(subtarget))_ :=)\
                    $(call nbDefsTargetCodeGroupsRules,$(1),$(call nbNameAsVar,$(subtarget)),,$(4),nbDefsTargetObjs_$(call nbNameAsVar,$(subtarget))_,$(6),$(7),$(8),$(9),$(10),$(11))\
                    $(call nbDefsTargetSubTargetRules,$(1),$(call nbNameAsVar,$(subtarget)),$(4),$(5),nbDefsTargetObjs_$(call nbNameAsVar,$(subtarget))_,$(6),$(7),$(8),$(9),$(10),$(11))\
                )\
            )\
        )\
    )\
    $(foreach codegrp,$($(2)_NB_TARGET_CODEGROUPS_),\
        $(if $(call nbCodeGroupRuleMatch2,$($(2)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_REQUIRED),$($(2)_$(call nbNameAsVar,$(codegrp))_NB_CODE_GRP_FLAGS_FORBIDDEN),$(_nbTmp$(1)RecursiveFlags)),\
            #+++ CodeGroup must compile: $(1)/$(2)/$(codegrp)$(NB_NEWLINE)
            $(call nbDefsTargetCodeGroupRules,$(1),$(2),$(codegrp),$(4),$(5),$(6),$(7),$(8),$(9),$(10),$(11))
        )\
    )
endef

#returns the rules for codeGroup compilation
#param $1, root-target var-name
#param $2, codegroup's target var-name
#param $3, codegroup var-name
#param $4, list for flags set
#param $5, list for objs (build-rules) added
#param $6, list for unique output dirs
#param $7, list for unique libs to link
#param $8, list for unique libs paths
#param $9, list for static-libs (build-rules) added
#param $10, list for shared-libs (build-rules) added
#param $11, list for exes (build-rules) added
#
define nbDefsTargetCodeGroupRulesAndroid
    $(foreach reqLib,$($(2)_$(3)_NB_CODE_GRP_LIBS),\
        $(if $(filter $(reqLib),$($(7))),,$(eval $(7) += $(reqLib)))\
    )$(foreach reqLibPath,$($(2)_$(3)_NB_CODE_GRP_LIBS_PATHS),\
        $(if $(findstring "$(reqLibPath)",$($(8))),,$(eval $(8) += "$(reqLibPath)"))\
    )\
    $(if $(findstring static,$($(2)_NB_TARGET_TYPE)),\

        #
        #code-group '$(1)/$(2)/$(3)'
        #
        LOCAL_PATH               := .
        include $$(CLEAR_VARS)
        LOCAL_MODULE             := $(1)_$(2)_$(3)
        LOCAL_CFLAGS             := -Os -fPIC -ffunction-sections -fdata-sections $($(2)_$(3)_NB_CODE_GRP_CFLAGS) $(_nbTmp$(1)RecursiveFlags)
        LOCAL_CPPFLAGS           := -Os -fPIC -ffunction-sections -fdata-sections $($(2)_$(3)_NB_CODE_GRP_CXXFLAGS) $(_nbTmp$(1)RecursiveFlags)
        LOCAL_C_INCLUDES         := $(call nbListToVertical,                                ,$(call nbConcatRootToPaths,$($(2)_$(3)_NB_CODE_GRP_REL_DIR),$($(2)_$(3)_NB_CODE_GRP_INCLUDES)))
        LOCAL_SRC_FILES          := $(call nbListToVertical,                                ,$(call nbConcatRootToPaths,$($(2)_$(3)_NB_CODE_GRP_REL_DIR),$($(2)_$(3)_NB_CODE_GRP_SRCS)))
        include $$(BUILD_STATIC_LIBRARY)
        $(eval $(9)              += $(1)_$(2)_$(3))
        
    ,\
    $(if\
        $(findstring shared,$($(2)_NB_TARGET_TYPE)),\

        #
        #code-group '$(1)/$(2)/$(3)'
        #
        LOCAL_PATH               := .
        include $$(CLEAR_VARS)
        LOCAL_MODULE             := $(2)
        LOCAL_CFLAGS             := -Os -fPIC -ffunction-sections -fdata-sections $($(2)_$(3)_NB_CODE_GRP_CFLAGS) $(_nbTmp$(1)RecursiveFlags)
        LOCAL_CPPFLAGS           := -Os -fPIC -ffunction-sections -fdata-sections $($(2)_$(3)_NB_CODE_GRP_CXXFLAGS) $(_nbTmp$(1)RecursiveFlags)
        LOCAL_LDFLAGS            :=
        LOCAL_C_INCLUDES         := $(call nbListToVertical,                                ,$(call nbConcatRootToPaths,$($(2)_$(3)_NB_CODE_GRP_REL_DIR),$($(2)_$(3)_NB_CODE_GRP_INCLUDES)))
        LOCAL_SRC_FILES          := $(call nbListToVertical,                                ,$(call nbConcatRootToPaths,$($(2)_$(3)_NB_CODE_GRP_REL_DIR),$($(2)_$(3)_NB_CODE_GRP_SRCS)))
        LOCAL_LDLIBS             := $(addprefix -l,$($(2)_NB_TARGET_LIBS))
        LOCAL_SHARED_LIBRARIES   := $($(1)_NB_TARGET_SHARED_LIBS) $(strip $($(10)))
        LOCAL_STATIC_LIBRARIES   := $($(1)_NB_TARGET_STATIC_LIBS) $(strip $($(9)))
        include $$(BUILD_SHARED_LIBRARY)
        $(eval $(10)             += $(2))
        
        ,$(error\
                unexpected NB_TARGET_TYPE: '$($(1)_NB_TARGET_TYPE)' for '$(1)' at nbDefsTargetRootRules)\
            )\
    )

endef
            
#returns the rules for codeGroup compilation
#param $1, root-target var-name
#param $2, codegroup's target var-name
#param $3, codegroup var-name
#param $4, list for flags set
#param $5, list for objs (build-rules) added
#param $6, list for unique output dirs
#param $7, list for unique libs to link
#param $8, list for unique libs paths
#param $9, list for static-libs (build-rules) added
#param $10, list for shared-libs (build-rules) added
#param $11, list for exes (build-rules) added
#
define nbDefsTargetCodeGroupRulesDefault
    $(if $(findstring "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)",$($(6))),,$(eval $(6) += "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)"))\
    $(foreach srcDir,$(dir $($(2)_$(3)_NB_CODE_GRP_SRCS)),\
        $(if $(findstring "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_$(3)_NB_CODE_GRP_NAME)/$(srcDir)",$($(6))),,$(eval $(6) += "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_$(3)_NB_CODE_GRP_NAME)/$(srcDir)"))\
    )\
    $(foreach reqLib,$($(2)_$(3)_NB_CODE_GRP_LIBS),\
        $(if $(filter $(reqLib),$($(7))),,$(eval $(7) += $(reqLib)))\
    )\
    $(foreach reqLibPath,$($(2)_$(3)_NB_CODE_GRP_LIBS_PATHS),\
        $(if $(findstring "$(reqLibPath)",$($(8))),,$(eval $(8) += "$(reqLibPath)"))\
    )\

    #
    #code-group compile rule (by context's base-path and .o extension)
    #
    $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_$(3)_NB_CODE_GRP_NAME)/%.o: $($(2)_NB_TARGET_REL_DIR)%
	$$(if $$(findstring .cpp, $$(suffix $$<))\
           , $$(subst [NB_COMMA],$$(NB_COMMA),$$(strip $(CXX) $(CXX_FLAGS_PRE) $($(2)_$(3)_NB_CODE_GRP_CXXFLAGS) -c $$< -o $$@\
             $$(call nbConcatPrefixAndRootToPaths,-I,$($(2)_NB_TARGET_REL_DIR),$($(2)_$(3)_NB_CODE_GRP_INCLUDES)) $(CXX_FLAGS_POST)))\
             $(_nbTmp$(1)RecursiveFlags)\
           , $$(subst [NB_COMMA],$$(NB_COMMA),$$(strip $(CC)  $(CC_FLAGS_PRE)  $($(2)_$(3)_NB_CODE_GRP_CFLAGS)   -c $$< -o $$@\
             $$(call nbConcatPrefixAndRootToPaths,-I,$($(2)_NB_TARGET_REL_DIR),$($(2)_$(3)_NB_CODE_GRP_INCLUDES)) $(CC_FLAGS_POST)))\
             $(_nbTmp$(1)RecursiveFlags)\
        )
    #
    $(eval $(5) += $(addprefix $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_$(3)_NB_CODE_GRP_NAME)/,$(addsuffix .o,$($(2)_$(3)_NB_CODE_GRP_SRCS))))
    
endef


#returns the rules for codeGroup compilation
#param $1, root-target var-name
#param $2, codegroup's target var-name
#param $3, codegroup var-name
#param $4, list for flags set
#param $5, list for objs (build-rules) added
#param $6, list for unique output dirs
#param $7, list for unique libs to link
#param $8, list for unique libs paths
#param $9, list for static-libs (build-rules) added
#param $10, list for shared-libs (build-rules) added
#param $11, list for exes (build-rules) added
#
define nbDefsTargetCodeGroupRules
    $(if $(findstring AndroidNdkBuild,$(NB_CFG_HOST)),\
      $(call nbDefsTargetCodeGroupRulesAndroid,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8),$(9),$(10),$(11))\
    ,\
      $(call nbDefsTargetCodeGroupRulesDefault,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8),$(9),$(10),$(11))\
    )
endef


#returns the rules for codeGroup compilation
#param $1, target's var-name
#param $2, list for flags set
#param $3, list for objs (build-rules) added
#param $4, list for unique output dirs
#param $5, list for unique libs to link
#param $6, list for unique libs paths
#param $7, list for static-libs (build-rules) added
#param $8, list for shared-libs (build-rules) added
#param $9, list for exes (build-rules) added
#
define nbDefsTargetRootRulesAndroid
    $(if $(findstring static,$($(1)_NB_TARGET_TYPE)),\
      \
    ,\
      $(if\
        $(findstring shared,$($(1)_NB_TARGET_TYPE)),\
          \
        ,\
          $(error unexpected NB_TARGET_TYPE: '$($(1)_NB_TARGET_TYPE)' for '$(1)' at nbDefsTargetRootRules)\
        )\
    )
endef

#returns the rules for codeGroup compilation
#param $1, target's var-name
#param $2, list for flags set
#param $3, list for objs (build-rules) added
#param $4, list for unique output dirs
#param $5, list for unique libs to link
#param $6, list for unique libs paths
#param $7, list for static-libs (build-rules) added
#param $8, list for shared-libs (build-rules) added
#param $9, list for exes (build-rules) added
#
define nbDefsTargetRootRulesDefault
    #
    #target rule (by name)
    #
    $($(1)_NB_TARGET_NAME): $(NB_WORKSPACE_OUT_DIR_BIN)/$($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX)

    #
    #target rule (by output name)
    #
    $(if $(strip $($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_SUFIX)),\
        $($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX): $(NB_WORKSPACE_OUT_DIR_BIN)/$($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX)\
    ,)

    #
    #target rule (by output file)
    #
    $(if $(filter static,$($(1)_NB_TARGET_TYPE)),
        $(NB_WORKSPACE_OUT_DIR_BIN)/$($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX): .folders.$($(1)_NB_TARGET_NAME) .cfgs.$($(1)_NB_TARGET_NAME) $($(3)) end-of-target
	    $$(AR) rvs $$@ $$(filter %.o, $$^)
    ,$(if\
        $(filter shared,$($(1)_NB_TARGET_TYPE)),
            $(NB_WORKSPACE_OUT_DIR_BIN)/$($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX): .folders.$($(1)_NB_TARGET_NAME) .cfgs.$($(1)_NB_TARGET_NAME) $($(3)) end-of-target
	        $$(LD) $(LD_FLAGS_PRE) $($(1)_NB_TARGET_LDFLAGS) $$(filter %.o, $$^) $(addprefix -L,$($(6))) $(addprefix -l,$($(7))) $(addprefix -l,$($(8))) $(addprefix -l,$(subst ",,$($(5)))) -o $$@ $(LD_FLAGS_POST)
        ,$(if\
            $(filter exe,$($(1)_NB_TARGET_TYPE)),
                $(NB_WORKSPACE_OUT_DIR_BIN)/$($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX): .folders.$($(1)_NB_TARGET_NAME) .cfgs.$($(1)_NB_TARGET_NAME) $($(3)) end-of-target
	            $$(CC) $$(filter %.o, $$^) $(addprefix -L,$($(6))) $(addprefix -l,$($(7))) $(addprefix -l,$($(8))) $(addprefix -l,$(subst ",,$($(5)))) $(addprefix -framework ,$($(1)_NB_TARGET_FRAMEWORKS)) -o $$@
            ,$(error\
                unexpected NB_TARGET_TYPE: '$($(1)_NB_TARGET_TYPE)' for '$(1)' at nbDefsTargetRootRules)\
            )\
        )\
    )

    #
    #target's folders rule
    #
    .folders.$($(1)_NB_TARGET_NAME):$(foreach reqDir,$($(4)),$(NB_NEWLINE)$(NB_TAB)mkdir -p $(reqDir))
    #
    
    #
    #target's configs rule
    #
    .cfgs.$($(1)_NB_TARGET_NAME):
	echo '$(strip $(_nbTmp$(1)RecursiveFlags))' > $(NB_WORKSPACE_OUT_DIR_BIN)/$($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX).build.txt
	echo '//Built by nbFramework/MakefileFuncs.mk.\n//Build date "$(shell date)"\n\n//$(words $(strip $(_nbTmp$(1)RecursiveFlags))) flags defined for all sources:\n$(strip $(_nbTmp$(1)RecursiveFlags))\n' > $(NB_WORKSPACE_OUT_DIR_BIN)/$($(1)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)$($(1)_NB_TARGET_SUFIX).build.h
        
endef


#returns the rules for codeGroup compilation
#param $1, target's var-name
#param $2, list for flags set
#param $3, list for objs (build-rules) added
#param $4, list for unique output dirs
#param $5, list for unique libs to link
#param $6, list for unique libs paths
#param $7, list for static-libs (build-rules) added
#param $8, list for shared-libs (build-rules) added
#param $9, list for exes (build-rules) added

#
define nbDefsTargetRootRules
    $(if $(findstring AndroidNdkBuild,$(NB_CFG_HOST)),$(call nbDefsTargetRootRulesAndroid,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8),$(9)),$(call nbDefsTargetRootRulesDefault,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8),$(9)))
endef


#returns the rules for codeGroup compilation
#param $1, root target's var-name
#param $2, target's var-name
#param $3, list for flags set
#param $4, list for parents objs
#param $5, list for objs (build-rules) added
#param $6, list for unique output dirs
#param $7, list for unique libs to link
#param $8, list for unique libs paths
#param $9, list for static-libs (build-rules) added
#param $10, list for shared-libs (build-rules) added
#param $11, list for exes (build-rules) added
#
define nbDefsTargetSubTargetRulesAndroid
    $(if $(findstring static,$($(2)_NB_TARGET_TYPE)),\
      \
    ,\
      $(if\
        $(findstring shared,$($(2)_NB_TARGET_TYPE)),
          \
        ,\
          $(error unexpected NB_TARGET_TYPE: '$($(1)_NB_TARGET_TYPE)' for '$(1)' at nbDefsTargetRootRules)\
      )\
    )
endef

#returns the rules for codeGroup compilation
#param $1, root target's var-name
#param $2, target's var-name
#param $3, list for flags set
#param $4, list for parents objs
#param $5, list for objs (build-rules) added
#param $6, list for unique output dirs
#param $7, list for unique libs to link
#param $8, list for unique libs paths
#param $9, list for static-libs (build-rules) added
#param $10, list for shared-libs (build-rules) added
#param $11, list for exes (build-rules) added
#
define nbDefsTargetSubTargetRulesDefault
    #
    #sub-target rule (by name)
    #
    $($(1)_NB_TARGET_NAME)_$($(2)_NB_TARGET_NAME): $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX)
    
    #
    #sub-target rule (by output name)
    #
    $(if $(strip $($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_SUFIX)),\
        $($(2)_NB_TARGET_PREFIX)$($(1)_NB_TARGET_NAME)_$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX): $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX)\
    ,)

    #
    #sub-target rule (by output file)
    #
    $(if $(filter static,$($(2)_NB_TARGET_TYPE)),
        $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX): .cfgs.$($(1)_NB_TARGET_NAME)_$($(2)_NB_TARGET_NAME) $($(5)) end-of-target
	    $$(AR) rvs $$@ $$(filter %.o, $$^)

        $(eval $(4) += $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX))
        $(eval $(9) += $($(2)_NB_TARGET_NAME))
        $(if $(findstring "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)",$($(8))),,$(eval $(8) += "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)"))
        
    ,$(if\
        $(filter shared,$($(2)_NB_TARGET_TYPE)),
            $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX): .cfgs.$($(1)_NB_TARGET_NAME)_$($(2)_NB_TARGET_NAME) $($(5)) end-of-target
	        $$(LD) $(LD_FLAGS_PRE) $($(2)_NB_TARGET_LDFLAGS) $$(filter %.o, $$^) $(addprefix -L,$($(8))) $(addprefix -l,$(subst ",,$($(7)))) -o $$@ $(LD_FLAGS_POST)

            $(eval $(4) += $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX))
            $(eval $(10) += $($(2)_NB_TARGET_NAME))
            $(if $(findstring "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)",$($(8))),,$(eval $(8) += "$(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)"))
        
        ,$(if\
            $(filter exe,$($(2)_NB_TARGET_TYPE)),
                $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX): .cfgs.$($(1)_NB_TARGET_NAME)_$($(2)_NB_TARGET_NAME) $($(5)) end-of-target
	            $$(CC) $$(filter %.o, $$^) $(addprefix -L,$($(8))) $(addprefix -l,$(subst ",,$($(7)))) $(addprefix -framework ,$($(2)_NB_TARGET_FRAMEWORKS)) -o $$@
	            $(eval $(11) += $($(2)_NB_TARGET_NAME))
            ,$(error\
                unexpected NB_TARGET_TYPE: '$($(2)_NB_TARGET_TYPE)' for '$(2)' at 'nbDefsTargetSubTargetRules')\
            )\
        )\
    )

    #
    #target's configs rule
    #
    .cfgs.$($(1)_NB_TARGET_NAME)_$($(2)_NB_TARGET_NAME):
	echo '$(strip $(_nbTmp$(1)RecursiveFlags))' > $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX).build.txt
	echo '//Built by nbFramework/MakefileFuncs.mk.\n//Build date "$(shell date)"\n\n//$(words $(strip $($(3)))) flags defined for all sources:\n$(strip $(_nbTmp$(1)RecursiveFlags))' > $(NB_WORKSPACE_OUT_DIR_OBJS)/$($(1)_NB_TARGET_NAME)/$($(2)_NB_TARGET_NAME)/$($(2)_NB_TARGET_PREFIX)$($(2)_NB_TARGET_NAME)$($(2)_NB_TARGET_SUFIX).build.h
        
endef


#returns the rules for codeGroup compilation
#param $1, root target's var-name
#param $2, target's var-name
#param $3, list for flags set
#param $4, list for parents objs
#param $5, list for objs (build-rules) added
#param $6, list for unique output dirs
#param $7, list for unique libs to link
#param $8, list for unique libs paths
#param $9, list for static-libs (build-rules) added
#param $10, list for shared-libs (build-rules) added
#param $11, list for exes (build-rules) added
#
define nbDefsTargetSubTargetRules
    $(if $(findstring AndroidNdkBuild,$(NB_CFG_HOST)),\
      $(call nbDefsTargetSubTargetRulesAndroid,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8),$(9),$(10),$(11))\
    ,\
      $(call nbDefsTargetSubTargetRulesDefault,$(1),$(2),$(3),$(4),$(5),$(6),$(7),$(8),$(9),$(10),$(11))\
    )
endef
    
#ToDo: remove after implemented inside 'nbDefsTargetRootRules'
#ifeq (AndroidNdkBuild,$(NB_CFG_HOST))
#define nbBuildTargetRulesCmdsStatic
#    $$(AR) crsD $$@ $$(filter %.o, $$^)
#endef
#endif

#Build (private, non-Android)
ifneq (AndroidNdkBuild,$(NB_CFG_HOST))
    define nbBuildTargetRulesValues
    endef
endif

#-------------------------
# CODE_GRP
#-------------------------

#Init
define nbInitCodeGrp
    #############
    # initCodeGrp
    #############
    NB_CODE_GRP_REL_DIR         := $(NB_TARGET_REL_DIR)
    NB_CODE_GRP_NAME            :=
    NB_CODE_GRP_CFLAGS          := $(strip $(NB_TARGET_CFLAGS))
    NB_CODE_GRP_CXXFLAGS        := $(strip $(NB_TARGET_CXXFLAGS))
    NB_CODE_GRP_INCLUDES        := $(strip $(NB_TARGET_INCLUDES))
    NB_CODE_GRP_FLAGS_REQUIRED  :=
    NB_CODE_GRP_FLAGS_FORBIDDEN :=
    NB_CODE_GRP_FLAGS_ENABLES   :=
    NB_CODE_GRP_LIBS            :=
    NB_CODE_GRP_LIBS_PATHS      :=
    NB_CODE_GRP_SRCS            :=
    #
endef


#Build (public)
define nbBuildCodeGrpRules
    $(if $(findstring undefined,$(flavor NB_PROJECT_REL_DIR)),\
        $(error Mising 'nbInitProject' before 'nbBuildCodeGrpRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))'),\
    )\
    $(if $(strip $(NB_PROJECT_REL_DIR)),,\
        $(error Mising 'nbInitProject' before 'nbBuildCodeGrpRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))'),\
    )\
    $(if $(findstring undefined,$(flavor NB_PROJECT_NAME)),\
        $(error 'NB_PROJECT_NAME' required before 'nbBuildCodeGrpRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))'),\
    )\
    $(if $(strip $(NB_PROJECT_NAME)),,\
        $(error 'NB_PROJECT_NAME' required before 'nbBuildCodeGrpRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))')\
    )\
    $(if $(findstring undefined,$(flavor NB_TARGET_NAME)),\
        $(error 'NB_TARGET_NAME' required before 'nbBuildCodeGrpRules'(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))),\
    )\
    $(if $(strip $(NB_TARGET_NAME)),,\
        $(error 'NB_TARGET_NAME' before 'nbBuildCodeGrpRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))')\
    )\
    $(if $(findstring undefined,$(flavor $(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME)),,
        $(if $(strip $($(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME)),\
            $(warning '$(NB_TARGET_NAME)' already in use as target name at 'nbBuildTargetRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]'))'; each target in the workspace must have an unique 'NB_TARGET_NAME'; have in mind that two similar names can be normalized internally to the same variable name after replacing undesired chars; found '$(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME := $($(call nbNameAsVar,$(NB_TARGET_NAME))_NB_TARGET_NAME)'.)\
        ,\
            \
        )\
    )\
    $(if $(findstring undefined,$(flavor NB_CODE_GRP_NAME)),\
        $(error 'NB_CODE_GRP_NAME' required before 'nbBuildCodeGrpRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))'),\
    )\
    $(if $(strip $(NB_CODE_GRP_NAME)),,\
        $(error 'NB_CODE_GRP_NAME' required before 'nbBuildCodeGrpRules(/$(call nbVarVal,NB_PROJECT_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_TARGET_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]')/$(call nbVarVal,NB_CODE_GRP_NAME,'[undefined]','[empty]'))')\
    )\
    ###############
    # buildCodeGrp: /$(NB_PROJECT_NAME)/$(NB_TARGET_NAME)/$(NB_CODE_GRP_NAME)
    ###############
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_REL_DIR         := $(NB_CODE_GRP_REL_DIR)
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_NAME            := $(NB_CODE_GRP_NAME)
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_CFLAGS          := $(strip $(NB_CODE_GRP_CFLAGS))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_CXXFLAGS        := $(strip $(NB_CODE_GRP_CXXFLAGS))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_INCLUDES        := $(strip $(NB_CODE_GRP_INCLUDES))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_FLAGS_REQUIRED  := $(strip $(NB_CODE_GRP_FLAGS_REQUIRED))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_FLAGS_FORBIDDEN := $(strip $(NB_CODE_GRP_FLAGS_FORBIDDEN))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_FLAGS_ENABLES   := $(strip $(NB_CODE_GRP_FLAGS_ENABLES))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_LIBS            := $(strip $(NB_CODE_GRP_LIBS))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_LIBS_PATHS      := $(strip $(NB_CODE_GRP_LIBS_PATHS))
    $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))_NB_CODE_GRP_SRCS            := $(call nbListToVertical,                                                ,$(NB_CODE_GRP_SRCS))
    NB_TARGET_CODEGROUPS_ += $(call nbNameAsVar,$(NB_CODE_GRP_NAME))
    #
    #############
    # initCodeGrp
    #############
    NB_CODE_GRP_REL_DIR         := $(NB_TARGET_REL_DIR)
    NB_CODE_GRP_NAME            :=
    NB_CODE_GRP_CFLAGS          := $(strip $(NB_TARGET_CFLAGS))
    NB_CODE_GRP_CXXFLAGS        := $(strip $(NB_TARGET_CXXFLAGS))
    NB_CODE_GRP_INCLUDES        := $(strip $(NB_TARGET_INCLUDES))
    NB_CODE_GRP_FLAGS_REQUIRED  :=
    NB_CODE_GRP_FLAGS_FORBIDDEN :=
    NB_CODE_GRP_FLAGS_ENABLES   :=
    NB_CODE_GRP_LIBS            :=
    NB_CODE_GRP_LIBS_PATHS      :=
    NB_CODE_GRP_SRCS            :=
    #
endef

#1, target var-name
#2, code group var-name
define nbUndefineCodeGroup
    $(info Undefining code-group '$(1)/$(2)'.)
    $(1)_$(2)_NB_CODE_GRP_REL_DIR:=
    $(1)_$(2)_NB_CODE_GRP_NAME:=
    $(1)_$(2)_NB_CODE_GRP_CFLAGS:=
    $(1)_$(2)_NB_CODE_GRP_CXXFLAGS:=
    $(1)_$(2)_NB_CODE_GRP_INCLUDES:=
    $(1)_$(2)_NB_CODE_GRP_FLAGS_REQUIRED:=
    $(1)_$(2)_NB_CODE_GRP_FLAGS_FORBIDDEN:=
    $(1)_$(2)_NB_CODE_GRP_FLAGS_ENABLES:=
    $(1)_$(2)_NB_CODE_GRP_LIBS:=
    $(1)_$(2)_NB_CODE_GRP_LIBS_PATHS:=
    $(1)_$(2)_NB_CODE_GRP_SRCS:=
endef

#Print
define nbBuildCodeGrpRulesPrintInfo
    $(info --------------)
    $(info - CODE GROUP: /$(NB_PROJECT_NAME)/$(NB_TARGET_NAME)/$(NB_CODE_GRP_NAME))
    $(info --------------)
    $(info NB_CODE_GRP_REL_DIR         := $(NB_CODE_GRP_REL_DIR))
    $(info NB_CODE_GRP_NAME            := $(NB_CODE_GRP_NAME))
    $(info NB_CODE_GRP_CFLAGS          := $(strip $(NB_CODE_GRP_CFLAGS)))
    $(info NB_CODE_GRP_CXXFLAGS        := $(strip $(NB_CODE_GRP_CXXFLAGS)))
    $(info NB_CODE_GRP_INCLUDES        := $(strip $(NB_CODE_GRP_INCLUDES)))
    $(info NB_CODE_GRP_FLAGS_REQUIRED  := $(strip $(NB_CODE_GRP_FLAGS_REQUIRED)))
    $(info NB_CODE_GRP_FLAGS_FORBIDDEN := $(strip $(NB_CODE_GRP_FLAGS_FORBIDDEN)))
    $(info NB_CODE_GRP_FLAGS_ENABLES   := $(strip $(NB_CODE_GRP_FLAGS_ENABLES)))
    $(info NB_CODE_GRP_LIBS            := $(strip $(NB_CODE_GRP_LIBS)))
    $(info NB_CODE_GRP_LIBS_PATHS      := $(strip $(NB_CODE_GRP_LIBS_PATHS)))
    $(info NB_CODE_GRP_SRCS            := count:$(words $(NB_CODE_GRP_SRCS)))
endef

#
#Returns 1 if all variables from the required-list-of-names are set (or is empty) and none of the forbidden list-of-names is set.
#param $1, required var-names list
#param $2, forbidden var-names list
#param $3, list for already-defined-variables
define nbCodeGroupRuleMatch2
$(strip \
    $(if $(strip $(1)), \
        $(if $(call nbFlagsMissingAnyMacros,$(1),$(3)),\
            \
        ,\
            $(if $(strip $(2)), \
                $(if $(call nbFlagsHasAnyMacros,$(2),$(3)),\
                ,1)\
            ,1)\
        )\
    ,\
        $(if $(strip $(2)), \
            $(if $(call nbFlagsHasAnyMacros,$(2),$(3)),\
            ,1)\
        ,1)\
    )\
)
endef

#
#Returns the description of why ther rules didint matched, or empty if the rule matched
#param $1, required var-names list
#param $2, forbidden var-names list
#param $3, list for already-defined-variables
define nbCodeGroupRuleMissmatchReason2
$(strip \
    $(if $(strip $(1)),$(call nbFlagsMissingAnyMacros,$(1),$(3)),)\
    $(if $(strip $(2)),$(call nbFlagsHasAnyMacros,$(2),$(3)),)\
)
endef






#ToDo: remove
#Build (private, NDK module)
#define nbBuildCodeGrpRulesAndroidNdkBuild
#    LOCAL_PATH               := .
#    include $$(CLEAR_VARS)
#    LOCAL_MODULE             := $(call nbNameAsVar,$(NB_TARGET_NAME)_$(NB_CODE_GRP_NAME))
#    LOCAL_CFLAGS             := $(NB_CODE_GRP_CFLAGS)
#    LOCAL_CPPFLAGS           := $(NB_CODE_GRP_CXXFLAGS)
#    LOCAL_C_INCLUDES         := $(call nbListToVertical,                                ,$(call nbConcatRootToPaths,$(NB_CODE_GRP_REL_DIR),$(NB_CODE_GRP_INCLUDES)))
#    LOCAL_SRC_FILES          := $(call nbListToVertical,                                ,$(call nbConcatRootToPaths,$(NB_CODE_GRP_REL_DIR),$(NB_CODE_GRP_SRCS)))
#    LOCAL_LDLIBS             := $(addprefix -l,$(NB_TARGET_LIBS))
#    LOCAL_SHARED_LIBRARIES   := $(NB_TARGET_SHARED_LIBS)
#    LOCAL_STATIC_LIBRARIES   := $(strip $(foreach target,$(NB_TARGET_DEPENDS), $(NB_TARGET_CODEGROUPS_$(target))))
#    include $$($(if $(findstring shared,$(NB_TARGET_TYPE)),BUILD_SHARED_LIBRARY,BUILD_STATIC_LIBRARY))
#endef
