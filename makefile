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
CCFLAGS=-Wall -Wpedantic -std=gnu99 -fPIC -shared
LFLAGS=-lm -lfreeimage -loipimgutil
NAME=convolution

# Enable memory debugging options.
ifeq ($(MEMDEBUG), true)
$(info Enabling memory debugging options.)
CCFLAGS+=-fsanitize=address
endif

# Check whether the build-config file exists.
ifeq ($(wildcard build-config),)
$(error No build-config file found! Please run config-build-env.sh first)
endif

# Include the build-config file.
include build-config

# Setup some path variables.
SRCDIR=src
BINDIR=bin
OIP_PLUGIN_DIR=$(OIPDIR)/plugins

INCLUDES=-I$(OIPDIR)/src/imgutil/ -I$(OIPDIR)/src/headers
LIBS=-L$(OIPDIR)/src/imgutil/bin/

compile: $(SRCDIR)/*.c
	mkdir -p $(BINDIR)
	$(CC) $(CCFLAGS) $(SRCDIR)/*.c -o $(BINDIR)/lib$(NAME).so $(INCLUDES) $(LIBS) $(LFLAGS)

install:
	cp $(BINDIR)/lib$(NAME).so $(OIP_PLUGIN_DIR)

clean:
	rm -rf $(BINDIR)

clean-hard: clean
	rm -f build-config
	
LOC:
	wc -l $(SRCDIR)/*.c
