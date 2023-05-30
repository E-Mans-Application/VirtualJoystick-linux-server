

# variables

# where we put all our binaries and other similar files
BUILDDIR:= target

# the files to compile
# ADD NEWLY CREATED FILES HERE:
override SRC :=	\
		src/main.cpp \
		\
		src/CommandLineParser.cpp \
		\
		src/IOException.cpp \
		src/Command.cpp \
		src/Gamepad.cpp \
		src/Socket.cpp

# the corresponding binary files
# they are obtained by replacing (syntactically) all instances of .cpp by .o
override OBJ :=	$(SRC:%.cpp=$(BUILDDIR)/%.o)
override TESTS_OBJ := $(TESTS:%.cpp=$(BUILDDIR)/%.o)

# name of the project
NAME := virtual_gamepad

# compiler
CXX:= g++ # it is g++ by default, but good to know the option
# pre-compilation flags
override CPPFLAGS += -I include
# compile flags
override CXXFLAGS += -Wall -std=c++17

# linker (g++ is fine, just in case we want it to be custom)
LD := $(CXX)
# libraries used in the project
override LDLIBS += 
# flags for the linkage
override LDFLAGS +=

# lmsfhqsdmjf
override RM := rm -rf

# the default rule for the makefile (must be the first one)
all: $(NAME)

debug: override CPPFLAGS += -D_DEBUG
debug: override CXXFLAGS += -g
debug: $(NAME)

# main rule for the project
# the $^ sign means that all dependencies are compiled
# the -o option refers to the linkage step
# $@ means the resulting binary will be named NAME
$(NAME): $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@ $(LDLIBS) 

# cleans up everything nicely
clean:
	$(RM) $(BUILDDIR) $(NAME) $(NAME).exe

# rules in order to correctly take into account the .hpp files
$(BUILDDIR)/%.o: override CPPFLAGS += -MT $@ -MMD -MP -MF $(@:.o=.d)
$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(COMPILE.cc) $< -o $@


# the targets that do not generate binaries
.PHONY := all clean debug test

-include $(OBJ:.o=.d)
