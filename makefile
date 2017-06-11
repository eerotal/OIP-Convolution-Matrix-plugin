#
#  Copyright 2017 Eero Talus
#
#  This file is part of Open Image Pipeline.
#
#  Open Image Pipeline is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Open Image Pipeline is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Open Image Pipeline.  If not, see <http://www.gnu.org/licenses/>.
#

# Set some compilation options.
CC=gcc
CCFLAGS=-Wall -Wpedantic -Wextra -pedantic-errors -std=gnu11 -fPIC -shared
LFLAGS=-lm -lfreeimage -loipimgutil -loipbuildinfo

# Enable debugging options.
DEBUG=0
ifeq ($(DEBUG), 1)
$(info [INFO]: Enabling debugging options.)
CCFLAGS+=-fsanitize=address -g
endif

# Set some project configuration defaults. These can be
# overridden with the project-config file.
NAME=plugin
SRCDIR=src
BINDIR=bin

# Check whether the project-config file exists and include it if it does.
ifneq ($(wildcard project-config),)
$(info [INFO]: Loading project configuration from the file project-config)
include project-config
endif

# Check whether the build-config file exists.
ifeq ($(wildcard build-config),)
$(error [ERROR]: No build-config file found! Please run config-build-env.sh first)
configure
endif

# Include the build-config file.
$(info [INFO]: Loading build configuration from the file build-config)
include build-config

# Setup some path variables.
OIP_PLUGIN_DIR=$(OIPDIR)/plugins

INCLUDES+=-I$(OIPDIR)/oipcore/src/oipimgutil
INCLUDES+=-I$(OIPDIR)/oipcore/src/oipbuildinfo
INCLUDES+=-I$(OIPDIR)/oipcore/src/oip

LIBS+=-L$(OIPDIR)/oipcore/src/oipimgutil/bin/
LIBS+=-L$(OIPDIR)/oipcore/src/oipbuildinfo/bin/

# Check if the OIP directory path is specified and throw an error if not.
ifndef OIPDIR
$(error [ERROR]: OIP directory not defined. There's something wrong with the build-config file)
endif

# Include the OIP build-config file.
include $(OIPDIR)/build-config

# Compile the plugin.
compile: $(SRCDIR)/*.c
	@mkdir -p $(BINDIR)
	@echo -n "[INFO]: Compiling "$(NAME)"..."
	@$(CC) $(CCFLAGS) $(SRCDIR)/*.c -o $(BINDIR)/lib$(NAME).so $(INCLUDES) $(LIBS) $(LFLAGS)
	@echo " Done."

# Copy the plugin to the OIP plugins directory.
install:
	@echo -n "[INFO]: Copying 'lib"$(NAME)".so' to '"$(OIP_PLUGIN_DIR)"'..."
	@cp $(BINDIR)/lib$(NAME).so $(OIP_PLUGIN_DIR)
	@echo " Done."

# Delete the compiled .so files.
clean:
	rm -rf $(BINDIR)

# Delete the compiled .so files and the build-config file.
clean-all: clean
	rm -f build-config
	
# Count the lines of code in this project.
LOC:
	wc -l $(SRCDIR)/*.c

