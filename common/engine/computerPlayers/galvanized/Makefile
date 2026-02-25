DEBUG=true
ifeq "$(DEBUG)" "true"
DEBUGOPTS=-g -O0
else
DEBUGOPTS=-O3 -DNDEBUG
endif
CC=g++
# To check and make sure stdafx is being used properly, add -H
CXXFLAGS=-fPIC -Wall -Wextra -pedantic -std=c++17 $(DEBUGOPTS)
sources=$(wildcard src/*.cpp)
objects=$(sources:src/%.cpp=obj/%.o)
headers=$(wildcard src/*.hpp)
library=dist/galvanized.so
resourcesrc=$(shell find resources/ -type f)
resourcedest=$(resourcesrc:resources/%=dist/%)
headerdependencies=$(objects:obj/%.o=obj/%.d)

# "|" signals an order-only prerequisite: the prereq must exist, but
# it won't cause the target to be recreated.
$(library): $(objects) Makefile | obj dist $(resourcedest)
	$(CC) $(CXXFLAGS) -shared -o $@ \
		-lluajit-5.1 $(objects) -lpthread

$(resourcedest) : dist/% : resources/% | dist
	mkdir -p $(dir $@);
	cp -r $< $@;

obj dist:
	if [[ ! -e $@ ]]; then mkdir $@; fi;

-include $(headerdependencies)

reformat:
	for file in $(sources) $(headers); do \
		clang-format -i -style="$$(<style)" "$$file" & \
		while [[ "$$(jobs | wc -l)" -ge "$$(grep processor /proc/cpuinfo | wc -l)" ]]; do \
			sleep .005; \
		done \
	done;

# Static pattern rules. Take each object in the targets, create a "base"
# with it specified by % (removing the literal part '.o'), then
# use that base to generate the prerequisite. This generates
# several rules.  For instance,  main.o : "main".o : "main".cpp
# then refer to the left-most prerequisite with $<, and the target
# with $@ like usual.
$(objects) : obj/%.o : src/%.cpp Makefile | obj
	$(CC) -c $(CXXFLAGS) -MMD \
	-I/usr/include/luajit-2.1 -Isrc/spdlog/include \
	$< -o $@

clean :
	rm -rf obj dist

echo :
	@echo sources $(sources)
	@echo objects $(objects)
	@echo headers $(headers) src/stdafx.hpp
	@echo library $(library)
	@echo resourcesrc $(resourcesrc)
	@echo resourcedest $(resourcedest)

.PHONY : echo clean reformat
