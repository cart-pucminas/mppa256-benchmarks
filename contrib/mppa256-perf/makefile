#
# MIT License
#
# Copyright(c) 2019 Pedro Henrique Penna <pedrohenriquepenna@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

#===============================================================================
# Build Options
#===============================================================================

# Verbose Build?
export VERBOSE ?= no

# Release Version?
export RELEASE ?= yes

#===============================================================================
# Directories
#===============================================================================

# Directories
export ROOTDIR    := $(CURDIR)
export BUILDDIR   := $(ROOTDIR)/build
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
export CFLAGS   = -std=c99 -fno-builtin
export CFLAGS  += -pedantic-errors
export CFLAGS  += -Wall -Wextra -Werror -Wa,--warn
export CFLAGS  += -Winit-self -Wswitch-default -Wfloat-equal
export CFLAGS  += -Wundef -Wshadow -Wuninitialized -Wlogical-op -Wvla
export CFLAGS  += -fno-stack-protector
export CFLAGS  += -I $(INCDIR)

# Additional Compiler Options
include $(BUILDDIR)/makefile.cflags

# Archiver Options
export ARFLAGS = rc

#===============================================================================
# Libraries
#===============================================================================

# Library Name
export LIBNAME = k1b-perf

#===============================================================================

# Builds everything.
all: all-ccluster all-iocluster

# Builds everything for Compute Cluster.
all-ccluster: make-dirs
	@$(MAKE) -C $(SRCDIR) all CLUSTER=ccluster

# Builds everything for IO DDR Cluster.
all-iocluster: make-dirs
	@$(MAKE) -C $(SRCDIR) all CLUSTER=iocluster

# Make Directories
make-dirs:
	@mkdir -p $(LIBDIR)

# Cleans object files.
clean:
	@$(MAKE) -C $(SRCDIR) clean

# Cleans everything.
distclean:
	@$(MAKE) -C $(SRCDIR) distclean
	@rm -rf $(LIBDIR)

#===============================================================================
# Install and Uninstall Rules
#===============================================================================

include $(BUILDDIR)/makefile.install
