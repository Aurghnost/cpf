$(VERBOSE).SILENT: # commenter pour afficher les commandes, décommenter pour cacher
##########################
# PARAMETRES MODIFIABLES #
##########################

# Compilateurs C et C++
CPREPROC = gcc -E
CC = gcc
CPPC = g++
LINKER = ld

# Répertoires des sources :
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
TEMPLATE_DIR = templates
LD_DIR = ldscripts

# Répertoires des builds (a priori inutile d'y toucher)
BUILD_ROOT = build
DOC_DIR = docs
LIB_DIR = lib 
OBJ_DIR := $(BUILD_ROOT)/obj
BIN_DIR := $(BUILD_ROOT)/bin
TESTS_BUILD = $(BUILD_ROOT)/tests

GLOBAL_INCLUDES = # a priori toujours vide

LINKER_SCRIPT_TEMPLATE=link.template.ld

# Fichiers générés dans BIN_DIR
ELF_NAME = main
LIB_NAME = libbase
LINKER_SCRIPT = $(BUILD_ROOT)/link.ld

# Extensions
EXT_TO_COMPILE = cpp c
EXT_OTHERS = h hpp tpp # extensions de shaders : frag vert geom tesc tese comp glsl

# Liens vers bibliothèques à ajouter au projet
LDLIBS = # -lm : pour la bibliothèque <math.h>

LDFLAGS = # évite que des bibliothèques dynamiques inutilisées soient chargés par défaut

CFLAGS = # -pipe -MMD -MP
CPPFLAGS = # -pipe -MMD -MP

# Drapeaux des compilateurs C et C++
# Optimisations pour le processeur
# CFLAGS += -masm=intel -mfpmath=sse  -mfma -m64 # Y a les extensions MMX, SSE1, SSE2 et AVX
# CFLAGS += -mmmx -msse -msse2 -mavx2 # extensions d'instructions SIMD sur les flottants
# CFLAGS += -mrdrnd -mrdseed # extensions pour les générations aléatoires physiques

# CPPFLAGS += -masm=intel -mfpmath=sse  -mfma -m64 # Y a les extensions MMX, SSE1, SSE2 et AVX
# CPPFLAGS += -mmmx -msse -msse2 -mavx2 # extensions d'instructions SIMD sur les flottants
# CPPFLAGS += -mrdrnd -mrdseed # extensions pour les générations aléatoires physiques

###############################################
# LE CODE CI-DESSOUS NE DOIT PAS ETRE MODIFIE #
###############################################

DEFAULT_FLAGS = -Wall -Wextra -Wconversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -Wdouble-promotion
DEFAULT_FLAGS += -I$(INC_DIR) -I$(TEMPLATE_DIR) $(foreach i, $(GLOBAL_INCLUDES), -include $(i))
DEFAULT_LDLIBS = # Rien pour l'instant...

# Choix des drapeaux selon le mode de compilation :
ifeq ($(BUILD), release)
BUILD_MOD = RELEASE
DEFAULT_FLAGS += -DNDEBUG
DEFAULT_FLAGS += -O3
DEFAULT_FLAGS += -march=native # fast permet les optis mathématiques même si moins bien dans le cas général
else
BUILD_MOD = DEBUG
DEFAULT_FLAGS += -O0 -fsanitize=address,undefined,leak -g3
LDFLAGS += -fsanitize=address,undefined,leak
endif

# Récupération des données nécessaires à la construction des binaires
ELF = $(BIN_DIR)/$(ELF_NAME)
SRC = $(foreach e,$(EXT_TO_COMPILE),$(shell find ./$(SRC_DIR)/ -name "*.$(e)"))
SRC := $(SRC:./%=%)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJ := $(OBJ:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
LIB = $(wildcard $(LIB_DIR)/*.a)
TESTS = $(foreach e,$(EXT_TO_COMPILE),$(shell find ./$(TEST_DIR)/ -name "*.test.$(e)"))
TESTS := $(TESTS:./%=%)
TESTS := $(TESTS:$(TEST_DIR)/%.test.c=$(TESTS_BUILD)/%.c.exe)
TESTS := $(TESTS:$(TEST_DIR)/%.test.cpp=$(TESTS_BUILD)/%.cpp.exe)
TESTS_OBJ = $(filter-out %/main.o, $(OBJ))

all: $(ELF)

test: $(LINKER_SCRIPT) $(TESTS) | $(TESTS_BUILD)
	@echo "Running all tests :"
	$(foreach t, $(TESTS), ./$(t);)

run: $(ELF)
	$(ELF)

$(TESTS_BUILD)/%.c.exe: $(TEST_DIR)/%.test.c $(TESTS_OBJ) | $(TESTS_BUILD)
	@mkdir -p $(dir $@)
	$(CC) $(LINKER_SCRIPT) $(DEFAULT_FLAGS) $< -o $@ $(TESTS_OBJ) $(CFLAGS) $(DEFAULT_LDLIBS) $(foreach l,$(LDLIBS),-l$(l)) $(LDFLAGS)

$(TESTS_BUILD)/%.cpp.exe: $(TEST_OUT_DIR)/%.test.cpp $(TESTS_OBJ) | $(TESTS_BUILD)
	@mkdir -p $(dir $@)
	$(CPPC) $(LINKER_SCRIPT) $(DEFAULT_FLAGS) $< -o $@ $(TESTS_OBJ) $(CPPFLAGS) $(DEFAULT_LDLIBS) $(foreach l,$(LDLIBS),-l$(l)) $(LDFLAGS)

$(ELF): $(OBJ) $(LINKER_SCRIPT) | $(BIN_DIR)
	@echo "Building ELF file as '$@' ($(BUILD_MOD))"
	$(CPPC) $(LINKER_SCRIPT) -o $@ $(OBJ) $(DEFAULT_LDLIBS) $(foreach l,$(LDLIBS),-l$(l)) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(DEFAULT_FLAGS) -c $< -o $@ $(CFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) 
	@mkdir -p $(dir $@)
	$(CPPC) $(DEFAULT_FLAGS) -c $< -o $@ $(CPPFLAGS)

$(BUILD_ROOT):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(TESTS_BUILD):
	mkdir -p $@

$(LINKER_SCRIPT): $(LD_DIR)/$(LINKER_SCRIPT_TEMPLATE) | $(BUILD_ROOT)
	$(CPREPROC) -xc -I$(INC_DIR) $(LD_DIR)/$(LINKER_SCRIPT_TEMPLATE) -o $(LINKER_SCRIPT)

.phony: release
release:
	make "BUILD=release"

.phony: debug
debug:
	make "BUILD=debug"

.phony: help
help:
	@echo "Project manager helper Makefile for C/C++ dev"
	@echo "You can configure variables in the Makefile. DO NOT TOUCH the second part of the Makefile if you don't know what you are doing."
	@echo "Commands :"
	@echo "- NO COMMANDS : same as 'debug' command, see below (I mean, do not write : 'make NO COMMANDS', just 'make' ^^)"
	@echo "- new_c_project [PROJECTNAME=...] : create a new fresh C project with a 'main.c' file with 'Hello World' code. \n\tDoxygen documentation is initialized with project name PROJECTNAME.\n\t"
	@echo "	Note : if you already have project folders and files, it will just replace 'main.c'"
	@echo "- new_cpp_project [PROJECTNAME=...] : create a new frseh C++ project with a 'main.cpp' file with 'Hello World' code. \n\tDoxygen documentation is initialized with project name PROJECTNAME.\n\t"
	@echo "	Note : if you already have project folders and files, it will just replace 'main.cpp'"
	@echo "- release : build a release version of your code with all compiler optimizations enabled, with NDEBUG symbol"
	@echo "- debug : build a debug version of your code"
	@echo "- clean : clean (no shit !?) your compiled code to get only the sources"
	@echo "- genstaticlib : generate a library from your project code as '$(BIN_DIR)/$(LIB_NAME).a'"
	@echo "- help : display this (hopefully) helping message"
	@echo "- count : count the number of lines of your project, just to show off in front of 'friends' because you have none."
.phony: clean
clean:
	@echo "Cleaning build artifacts of the '$(BUILD_ROOT)' directory"
	@$(RM) -rf $(BUILD_ROOT)

.phony: new_c_project
new_c_project:
	@mkdir $(LIB_DIR) $(SRC_DIR) $(INC_DIR) $(LD_DIR) $(DOC_DIR) $(TEST_DIR)
	@printf "#include <stdio.h>\n#include <stdlib.h>\n\nint main(int argc, char*argv[]) {\n\tprintf(\"Hello World !\\\n\");\n\treturn EXIT_SUCCESS;\n}\n" > $(SRC_DIR)/main.c
	@printf "SECTIONS {\n}\n" > $(LD_DIR)/$(LINKER_SCRIPT_TEMPLATE)
	@printf "PROJECT_NAME = $(PROJECTNAME)\nOUTPUT_DIRECTORY = '$(DOC_DIR)'\nOPTIMIZE_OUTPUT_FOR_C = YES\nGENERATE_LATEX = NO\nEXTRACT_ALL = YES\nINPUT = .\nRECURSIVE = YES\nFILE_PATTERNS = *.h *.c\n"  > Doxyfile
	@printf "Project '$(PROJECTNAME)' is initialized with following directories :\n- $(LIB_DIR)\n- $(SRC_DIR) (contains an 'hello world' main.c)\n- $(INC_DIR)\n- $(LD_DIR)\n- $(TEST_DIR)\n- $(DOC_DIR) (contains Doxygen generated documentation)\n"

.phony: delete_project
delete_project:
	@rm -rf $(BUILD_ROOT) $(LIB_DIR) $(SRC_DIR) $(INC_DIR) $(LD_DIR) $(TEMPLATE_DIR) $(DOC_DIR) $(TEST_DIR)
	@rm -rf Doxyfile

.phony: new_cpp_project
new_cpp_project:
	@mkdir $(LIB_DIR) $(SRC_DIR) $(INC_DIR) $(LD_DIR) $(TEMPLATE_DIR) $(DOC_DIR) $(TEST_DIR)
	@touch $(SRC_DIR)/main.cpp
	@printf "#include <iostream>\n\nint main(int argc, char*argv[]) {\n\tstd::cout << \"Hello World !\" << std::endl;\n\treturn EXIT_SUCCESS;\n}\n" > $(SRC_DIR)/main.cpp
	@printf "SECTIONS {\n}\n" > $(LD_DIR)/$(LINKER_SCRIPT_TEMPLATE)
	@printf "PROJECT_NAME = $(PROJECTNAME)\nOUTPUT_DIRECTORY = '$(DOC_DIR)'\nGENERATE_LATEX = NO\nEXTRACT_ALL = YES\nINPUT = .\nRECURSIVE = YES\nFILE_PATTERNS = *.hpp *.cpp\n"  > Doxyfile
	@printf "Project '$(PROJECTNAME)' is initialized with following directories :\n- $(LIB_DIR)\n- $(SRC_DIR) (contains an 'hello world' main.cpp)\n- $(INC_DIR)\n- $(LD_DIR)\n- $(TEMPLATE_DIR)\n- $(TEST_DIR)\n- $(DOC_DIR) (contains Doxygen generated documentation)\n"

.phony: count
count: # Pour le fun
	@echo "Considered files extensions are : $(EXT_TO_COMPILE) $(EXT_OTHERS)"
	@find . -name "" $(foreach e,$(EXT_TO_COMPILE) $(EXT_OTHERS), -or -name "*.$(e)") | xargs wc -l | awk 'BEGIN {total = 0;}{if (NF == 2 && $$2 == "total") total += $$1;} END {print "Total lines count : " total}'

.phony: genstaticlib
genstaticlib: $(OBJ) | $(BIN_DIR)
	@ar rcs $(BIN_DIR)/$(LIB_NAME).a $^
	@echo "Generating static library $(LIB_NAME).a"

.phony: gendynlib
gendynlib: $(OBJ) | $(BIN_DIR)
	$(CC) -shared -o $(BIN_DIR)/$(LIB_NAME).so $^
	@echo "Generating dynamic library $(LIB_NAME).so"
.phony: doc
doc:
	@doxygen
	xdg-open ./$(DOC_DIR)/html/index.html&

-include $(OBJ:.o=.d)
