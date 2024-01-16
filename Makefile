#CXX = clang++
CXX = g++

OUTFILE = out.bin
PRO_DIR = .
SOURCES := $(wildcard *.cpp $(PRO_DIR)/src/*.cpp)
SRCRCES += $(wildcard *.cpp $(PRO_DIR)/utils/*.cpp)

#OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
OBJS = $(addsuffix .o, $(basename $(SOURCES)))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -ldl -pthread -lrdkafka

CXXFLAGS = -std=c++11 -I$(PRO_DIR)/../../net/librdkafka/src -I$(PRO_DIR)/include -I$(PRO_DIR)/include/cppkafka -I$(PRO_DIR)/include/cppkafka/detail -I$(PRO_DIR)/include/cppkafka/utils
CXXFLAGS += -Wall -Wformat -O0 -ggdb
LIBS = -L$(PRO_DIR)/../../net/librdkafka/src


##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	# LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`
	LIBS += -L/usr/local/lib -L$(PRO_DIR)/lib
	LIBS += $(LINUX_GL_LIBS)
	# LIBS += -liconv

	# CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #Apple
	ECHO_MESSAGE = "MacOS"
	LIBS += -L/usr/lib -L/usr/local/lib -L$(PRO_DIR)/lib
	LIBS += -liconv
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(OUTFILE)
	@echo src = $(SOURCES)
	@echo obj = $(OBJS)
	@echo Build complete for $(ECHO_MESSAGE)

$(OUTFILE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(OUTFILE) $(OBJS)

cleanobj:
	rm -f $(OBJS)
