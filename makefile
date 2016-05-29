# Environment
MKDIR=mkdir
CPP=g++

# Object Directory
BUILDDIR=build
DEBUG_DIR=$(BUILDDIR)/Debug
RELEASE_DIR=$(BUILDDIR)/Release

# Object Files
DEBUG_OBJECTFILES= \
	$(DEBUG_DIR)/QueryCollection.o \
	$(DEBUG_DIR)/main.o
RELEASE_OBJECTFILES= \
	$(RELEASE_DIR)/QueryCollection.o \
	$(RELEASE_DIR)/main.o

# Binary location
TARGET=bin

# CC Compiler Flags
CPPFLAGS= -Iinclude -I/usr/local/include -std=c++11 -MMD -MP -MF

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/local/lib -L/usr/lib/i386-linux-gnu -lopencv_core -lopencv_highgui -lboost_filesystem -lboost_program_options -lopencv_imgproc -lopencv_features2d -lopencv_flann -lopencv_nonfree -lboost_system -lavcodec -lavformat -lavutil -lswscale

# Flags for debug vs release
DEBUG_FLAGS= -g
RELEASE_FLAGS= -O2

# Build debug
.PHONY: debug
debug: CPPFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_DIR)/bin/rankcollection

# Build release
.PHONY: release
release: CPPFLAGS += $(RELEASE_FLAGS)
release: $(RELEASE_DIR)/bin/rankcollection

# Debug
$(DEBUG_DIR)/bin/rankcollection: $(DEBUG_OBJECTFILES)
	$(MKDIR) -p $(DEBUG_DIR)/$(TARGET)
	$(CPP) $(CPPFLAGS) -o $(DEBUG_DIR)/$(TARGET)/rankcollection $^ $(LDLIBSOPTIONS)
$(DEBUG_DIR)/main.o: main.cpp
	$(MKDIR) -p $(DEBUG_DIR)/$(TARGET)
	$(CPP) $(CPPFLAGS) $(LDLIBSOPTIONS) -o $(DEBUG_DIR)/main.o -c main.cpp
$(DEBUG_DIR)/QueryCollection.o: QueryCollection.cpp
	$(MKDIR) -p $(DEBUG_DIR)/$(TARGET)
	$(CPP) $(CPPFLAGS) $(LDLIBSOPTIONS) -o $(DEBUG_DIR)/QueryCollection.o -c QueryCollection.cpp

# Release
$(RELEASE_DIR)/bin/rankcollection: $(RELEASE_OBJECTFILES)
	$(MKDIR) -p $(RELEASE_DIR)/$(TARGET)
	$(CPP) $(CPPFLAGS) -o $(RELEASE_DIR)/$(TARGET)/rankcollection $^ $(LDLIBSOPTIONS)
$(RELEASE_DIR)/main.o: main.cpp
	$(MKDIR) -p $(RELEASE_DIR)/$(TARGET)
	$(CPP) $(CPPFLAGS) $(LDLIBSOPTIONS) -o $(RELEASE_DIR)/main.o -c main.cpp
$(RELEASE_DIR)/QueryCollection.o: QueryCollection.cpp
	$(MKDIR) -p $(RELEASE_DIR)/$(TARGET)
	$(CPP) $(CPPFLAGS) $(LDLIBSOPTIONS) -o $(RELEASE_DIR)/QueryCollection.o -c QueryCollection.cpp

# Clean objects
.PHONY: clean
clean:
	rm -rf $(BUILDDIR)


