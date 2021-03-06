# See LICENSE file for license and copyright information

GIRARA_VERSION_MAJOR = 0
GIRARA_VERSION_MINOR = 2
GIRARA_VERSION_REV   = 8
VERSION = ${GIRARA_VERSION_MAJOR}.${GIRARA_VERSION_MINOR}.${GIRARA_VERSION_REV}

# Rules for the SOMAJOR and SOMINOR.
# Before a release perform the following checks against the last release:
# * If a function has been removed or the paramaters of a function have changed
#   bump SOMAJOR and set SOMINOR to 0.
# * If any of the exported datastructures have changed in a incompatible way
#   bump SOMAJOR and set SOMINOR to 0.
# * If a function has been added bump SOMINOR.
SOMAJOR = 3
SOMINOR = 0
SOVERSION = ${SOMAJOR}.${SOMINOR}

# pkg-config binary
PKG_CONFIG ?= pkg-config

# glib-compile-resource binary
GLIB_COMPILE_RESOURCES ?= glib-compile-resources

# libnotify
WITH_LIBNOTIFY ?= $(shell (${PKG_CONFIG} libnotify --atleast-version=0.7.0 && echo 1) || echo 0)

# libjson-c
WITH_JSON ?= $(shell (${PKG_CONFIG} json-c --exists && echo 1) || echo 0)

# paths
PREFIX ?= /usr
LIBDIR ?= ${PREFIX}/lib
INCLUDEDIR ?= ${PREFIX}/include

# locale directory
LOCALEDIR ?= ${PREFIX}/share/locale

# build directories
DEPENDDIR ?= .depend
BUILDDIR ?= build
BUILDDIR_RELEASE ?= ${BUILDDIR}/release
BUILDDIR_DEBUG ?= ${BUILDDIR}/debug
BUILDDIR_GCOV ?= ${BUILDDIR}/gcov
BINDIR ?= bin

# version checks
# If you want to disable any of the checks, set *_VERSION_CHECK to 0.

# GTK+
GTK_VERSION_CHECK ?= 1
GTK_MIN_VERSION = 3.20
GTK_PKG_CONFIG_NAME = gtk+-3.0
# glib
GLIB_VERSION_CHECK ?= 1
GLIB_MIN_VERSION = 2.50
GLIB_PKG_CONFIG_NAME = glib-2.0

# libs
ifeq (${GTK_INC}-${GTK_LIB},-)
PKG_CONFIG_LIBS += gtk+-3.0
else
INCS += ${GTK_INC}
LIBS += ${GTK_LIB}
endif

ifneq (${WITH_LIBNOTIFY},0)
ifeq (${LIBNOTIFY_INC}-${LIBNOTIFY_LIB},-)
PKG_CONFIG_LIBS += libnotify
else
INCS += ${LIBNOTIFY_INC}
LIBS += ${LIBNOTIFY_LIB}
endif
endif

ifneq (${WITH_JSON},0)
ifeq (${JSON_INC}-${JSON_LIB},-)
PKG_CONFIG_LIBS += json-c
else
INCS += ${JSON_INC}
LIBS += ${JSON_LIB}
endif
endif

ifneq (${PKG_CONFIG_LIBS},)
INCS += $(shell ${PKG_CONFIG} --cflags ${PKG_CONFIG_LIBS})
LIBS += $(shell ${PKG_CONFIG} --libs ${PKG_CONFIG_LIBS})
endif
LIBS += -lm

# pre-processor flags
CPPFLAGS += -D_FILE_OFFSET_BITS=64

# compiler flags
CFLAGS += -std=c11 -pedantic -Wall -Wextra -fPIC $(INCS)

# linker flags
LDFLAGS += -fPIC

# debug
DFLAGS = -O0 -g

# compiler
CC ?= gcc

# archiver
AR ?= ar

# strip
SFLAGS ?= -s

# soname
SONAME_FLAG ?= -soname
SHARED_FLAG ?= -shared

# set to something != 0 if you want verbose build output
VERBOSE ?= 0

# gettext package name
GETTEXT_PACKAGE ?= lib${PROJECT}-${SOMAJOR}

# msgfmt
MSGFMT ?= msgfmt

# gcov & lcov
GCOV_CFLAGS=-fprofile-arcs -ftest-coverage
GCOV_LDFLAGS=-fprofile-arcs
LCOV_OUTPUT=gcov
LCOV_EXEC=lcov
LCOV_FLAGS=--base-directory . --directory ${BUILDDIR_GCOV} --capture --rc \
					 lcov_branch_coverage=1 --output-file ${BUILDDIR_GCOV}/$(PROJECT).info
GENHTML_EXEC=genhtml
GENHTML_FLAGS=--rc lcov_branch_coverage=1 --output-directory ${LCOV_OUTPUT} ${BUILDDIR_GCOV}/$(PROJECT).info

# colors
COLOR ?= 1

# dist
TARFILE = ${PROJECTNV}-${VERSION}.tar.gz
