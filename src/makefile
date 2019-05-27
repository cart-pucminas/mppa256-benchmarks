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
# Target Configuration
#===============================================================================

ifneq ($(CLUSTER),)
include $(BUILDDIR)/makefile.$(CLUSTER)
endif

#===============================================================================
# Build Configuration
#===============================================================================

# Suffix for Objects
export OBJ_SUFFIX ?= $(CLUSTER)

export LIBS += $(LIB_K1B_PERF)

#===============================================================================
# Sources and Objects
#===============================================================================

# C Source Files
export SRC += $(wildcard $(CURDIR)/common/*.c)

#===============================================================================
# Default Build Rules
#===============================================================================

# Builds everything.
all: all-$(KERNEL)

# Cleans object files.
clean: all-$(KERNEL)

# Cleans everything.
distclean:
	@find $(SRCDIR) -name "*.o" -exec rm -rf {} \;
	@rm -f $(BINDIR)/*.elf $(ROOTDIR)/*.img

#===============================================================================
# FPU Kernel Build Rules
#===============================================================================

# Builds FPU Kernel.
all-FPU:
	@$(MAKE) -C FPU all

# Cleans object files.
clean-FPU:
	@$(MAKE) -C FPU clean

#===============================================================================
# MM Kernel Build Rules
#===============================================================================

# Builds MM Kernel.
all-MM:
	@$(MAKE) -C MM all

# Cleans object files.
clean-MM:
	@$(MAKE) -C MM clean

#===============================================================================
# GF Kernel Build Rules
#===============================================================================

# Builds GF Kernel.
all-GF:
	@$(MAKE) -C GF all

# Cleans object files.
clean-GF:
	@$(MAKE) -C GF clean
#
# TSP Kernel Build Rules
#===============================================================================

# Builds MM Kernel.
all-TSP:
	@$(MAKE) -C TSP all

# Cleans object files.
clean-TSP:
	@$(MAKE) -C TSP clean
