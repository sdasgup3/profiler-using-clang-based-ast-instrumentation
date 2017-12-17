TARGET := test
CXX := g++
CXXFLAGS := -O3 -g
INCLUDES := ${HOME}/Github/FindingHavoc/collectingProfiles/instrumentor/include/
LIBS := ${HOME}/Github/FindingHavoc/collectingProfiles/instrumentor/build/
INSTR_SCRIPT := ${HOME}/Github/FindingHavoc/collectingProfiles/scripts/batchInstrument.sh

SRCS=$(shell find .  -name "*.cpp" | sort -V)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))

.PHONY: all
all: $(TARGET)

test: instrument_code build_objs

build_objs: $(OBJS)
		$(CXX) $(CXXFLAGS) -I $(INCLUDES)  $^ -L $(LIBS) -linstrumentor -lm -o $(TARGET)

%.o: %.cpp
		$(CXX) $(CXXFLAGS) -I $(INCLUDES) $< 	-c -o $@

instrument_code: $(SRCS)
		$(INSTR_SCRIPT)


.PHONY: clean format

clean:
		rm -rf $(OBJS) $(TARGET)
		git checkout *.cpp
