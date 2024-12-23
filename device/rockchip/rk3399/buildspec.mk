#
# Copyright (C) 2007 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

######################################################################
# This is a do-nothing template file.  To use it, copy it to a file
# named "buildspec.mk" in the root directory, and uncomment or change
# the variables necessary for your desired configuration.  The file
# "buildspec.mk" should never be checked in to source control.
######################################################################

TARGET_BOARD_PLATFORM ?= rk3399
TARGET_BOARD_PLATFORM_GPU := mali-t860

# Choose a product to build for.  Look in the products directory for ones
# that work.
ifndef TARGET_PRODUCT
TARGET_PRODUCT:=rk3399
endif

# Choose a variant to build.  If you don't pick one, the default is eng.
# User is what we ship.  Userdebug is that, with a few flags turned on
# for debugging.  Eng has lots of extra tools for development.
ifndef TARGET_BUILD_VARIANT
#TARGET_BUILD_VARIANT:=user
TARGET_BUILD_VARIANT:=userdebug
#TARGET_BUILD_VARIANT:=eng
endif

# Choose a targeted release.  If you don't pick one, the default is the
# soonest future release.
ifndef TARGET_PLATFORM_RELEASE
#TARGET_PLATFORM_RELEASE:=OPR1
endif

# Choose additional targets to always install, even when building
# minimal targets like "make droid".  This takes simple target names
# like "Browser" or "MyApp", the names used by LOCAL_MODULE or
# LOCAL_PACKAGE_NAME.  Modules listed here will always be installed in
# /system, even if they'd usually go in /data.
ifndef CUSTOM_MODULES
#CUSTOM_MODULES:=
endif

# Set this to debug or release if you care.  Otherwise, it defaults to release.
ifndef TARGET_BUILD_TYPE
#TARGET_BUILD_TYPE:=release
endif

# Uncomment this if you want the host tools built in debug mode.  Otherwise
# it defaults to release.
ifndef HOST_BUILD_TYPE
#HOST_BUILD_TYPE:=debug
endif

# Turn on debugging for selected modules.  If DEBUG_MODULE_<module-name> is set
# to a non-empty value, the appropriate HOST_/TARGET_CUSTOM_DEBUG_CFLAGS
# will be added to LOCAL_CFLAGS when building the module.
#DEBUG_MODULE_ModuleName:=true

# Specify the extra CFLAGS to use when building a module whose
# DEBUG_MODULE_ variable is set.  Host and device flags are handled
# separately.
#HOST_CUSTOM_DEBUG_CFLAGS:=
#TARGET_CUSTOM_DEBUG_CFLAGS:=

# Choose additional locales, like "en_US" or "it_IT", to add to any
# built product.  Any locales that appear in CUSTOM_LOCALES but not in
# the locale list for the selected product will be added to the end
# of PRODUCT_LOCALES.
ifndef CUSTOM_LOCALES
#CUSTOM_LOCALES:=
endif

# If you have a special place to put your ouput files, set this, otherwise
# it goes to <build-root>/out
#OUT_DIR:=/tmp/stuff

# If you want to always set certain system properties, add them to this list.
# E.g., "ADDITIONAL_BUILD_PROPERTIES += ro.prop1=5 prop2=value"
# This mechanism does not currently support values containing spaces.
#ADDITIONAL_BUILD_PROPERTIES +=

# If you want to reduce the system.img size by several meg, and are willing to
# lose access to CJK (and other) character sets, define NO_FALLBACK_FONT:=true
ifndef NO_FALLBACK_FONT
#NO_FALLBACK_FONT:=true
endif

# OVERRIDE_RUNTIMES allows you to locally override PRODUCT_RUNTIMES.
#
# To only build ART, use "runtime_libart_default"
# To use Dalvik but also include ART, use "runtime_libdvm_default runtime_libart"
# To use ART but also include Dalvik, use "runtime_libart_default runtime_libdvm"
ifndef OVERRIDE_RUNTIMES
#OVERRIDE_RUNTIMES:=runtime_libart_default
#OVERRIDE_RUNTIMES:=runtime_libdvm_default runtime_libart
#OVERRIDE_RUNTIMES:=runtime_libart_default runtime_libdvm
endif

# when the build system changes such that this file must be updated, this
# variable will be changed.  After you have modified this file with the new
# changes (see buildspec.mk.default), update this to the new value from
# buildspec.mk.default.
BUILD_ENV_SEQUENCE_NUMBER := 13
