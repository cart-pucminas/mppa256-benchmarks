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
# Sources and Objects
#===============================================================================

# C Source Files
SRC += $(wildcard $(CURDIR)/*.c)

# Object Files
OBJ = $(SRC:.c=.$(OBJ_SUFFIX).o)

#===============================================================================

# Builds All Object Files
all: $(OBJ)
ifeq ($(VERBOSE), no)
	@echo [CC] $(BINDIR)/$(ELFBIN).$(OBJ_SUFFIX)
	@$(CC) $(LDFLAGS) -o $(BINDIR)/$(ELFBIN).$(OBJ_SUFFIX) $(OBJ) $(LIBS)
else
	$(CC) $(LDFLAGS) -o $(BINDIR)/$(ELFBIN).$(OBJ_SUFFIX) $(OBJ) $(LIBS)
endif

# Cleans All Object Files
clean:
ifeq ($(VERBOSE), no)
	@echo [CLEAN] $(OBJ)
	@rm -rf $(OBJ)
else
	rm -rf $(OBJ)
endif

# Cleans Everything
distclean: clean
ifeq ($(VERBOSE), no)
	@echo [CLEAN] $(BINDIR)/$(ELFBIN).$(OBJ_SUFFIX)
	@rm -rf $(BINDIR)/$(ELFBIN).$(OBJ_SUFFIX)
else
	rm -rf $(BINDIR)/$(ELFBIN)).$(OBJ_SUFFIX)
endif

# Builds a C Source file
%.$(OBJ_SUFFIX).o: %.c
ifeq ($(VERBOSE), no)
	@echo [CC] $@
	@$(CC) $(CFLAGS) $< -c -o $@
else
	$(CC) $(CFLAGS) $< -c -o $@
endif
