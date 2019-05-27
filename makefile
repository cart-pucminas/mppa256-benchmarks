#
# Copyright (C) 2013-2019 The Engineers of CAP Bench
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

#===============================================================================
# Build Options
#===============================================================================

# Verbose Build?
export VERBOSE ?= no

# Release Version?
export RELEASE ?= no

# Target Benchmark Kernel
export KERNEL ?= TSP

#===============================================================================
# Directories
#===============================================================================

# Directories
export ROOTDIR    := $(CURDIR)
export BINDIR     := $(ROOTDIR)/bin
export BUILDDIR   := $(ROOTDIR)/build
export CONTRIBDIR := $(ROOTDIR)/contrib
export INCDIR     := $(ROOTDIR)/include
export LIBDIR     := $(ROOTDIR)/lib
export SRCDIR     := $(ROOTDIR)/src

#===============================================================================
# Toolchain Configuration
#===============================================================================

# Toolchain Directory
export TOOLCHAIN_DIR ?= /usr/local/k1tools

# Toolchain
export CC = $(TOOLCHAIN_DIR)/bin/k1-gcc
export LD = $(TOOLCHAIN_DIR)/bin/k1-gcc
export AR = $(TOOLCHAIN_DIR)/bin/k1-ar

# Compiler Options
export CFLAGS  = -std=c99 -fno-builtin
export CFLAGS += -pedantic-errors
export CFLAGS += -Wall -Wextra -Werror -Wa,--warn
export CFLAGS += -Winit-self -Wswitch-default -Wfloat-equal
export CFLAGS += -Wundef -Wshadow -Wuninitialized -Wlogical-op
export CFLAGS += -Wno-unused-function
export CFLAGS += -fno-stack-protector
export CFLAGS += -Wvla # -Wredundant-decls
export CFLAGS += -I $(INCDIR)
export CFLAGS += -march=k1b -mboard=developer
include $(BUILDDIR)/makefile.cflags

# Linker Options
export LDFLAGS = -march=k1b -mboard=developer

# Libraries.
export LIB_K1B_PERF := $(LIBDIR)

#===============================================================================
# Binaries and Images
#===============================================================================

# Binary
export ELFBIN := $(KERNEL).elf

# Image
export IMAGE := $(KERNEL).img

#===============================================================================

# Builds everything.
all: image

# Make Directories
make-dirs:
	@mkdir -p $(BINDIR)

# Builds binary.
binary: make-dirs contrib
	@$(MAKE) -C $(SRCDIR) all-$(KERNEL) CLUSTER="ccluster" LIB_K1B_PERF="$(LIBDIR)/k1b-perf.ccluster.a"
	@$(MAKE) -C $(SRCDIR) all-$(KERNEL) CLUSTER="iocluster" LIB_K1B_PERF="$(LIBDIR)/k1b-perf.iocluster.a"

# Build binary image.
image: binary
	@$(TOOLCHAIN_DIR)/bin/k1-create-multibinary \
		--boot $(BINDIR)/$(ELFBIN).iocluster    \
		--clusters $(BINDIR)/$(ELFBIN).ccluster \
		-T $(IMAGE) -f

# Cleans object files.
clean:
	@$(MAKE) -C $(SRCDIR) clean-$(KERNEL)

# Cleans everything.
distclean:
	@$(MAKE) -C $(SRCDIR) distclean
	@rm -fr $(BINDIR)

include $(BUILDDIR)/makefile.run

#===============================================================================
# Contrib Install and Uninstall Rules
#===============================================================================

include $(BUILDDIR)/makefile.contrib

#===============================================================================
# Run Rules
#===============================================================================
