$(VERBOSE).SILENT:
SRC_DIR=src
INC_DIR=include
TEST_DIR=test
LD_DIR=ldscripts
LIB_DIR=lib
ELF_NAME=cpf_c
TEMPLATE_DIR=templates
DOCS_DIR=docs
LIB_NAME=libcpf_c
# Compilateurs C et C++
CPREPROC = gcc -E
CC = gcc
CPPC = g++
LINKER = ld

# Répertoires des builds (a priori inutile d'y toucher)
BUILD_ROOT = build
OBJ_DIR := $(BUILD_ROOT)/obj
BIN_DIR := $(BUILD_ROOT)/bin
TESTS_BUILD = $(BUILD_ROOT)/tests

# Répertoire racine lors de l'exécution de tests : TODO : système en isolation
TESTS_EXECUTION_DIR=$(BUILD_ROOT)/tests_exec

GLOBAL_INCLUDES = # a priori toujours vide

LINKER_SCRIPT_TEMPLATE=link.template.ld
LINKER_SCRIPT = $(BUILD_ROOT)/link.ld

# Extensions
EXT_TO_COMPILE = cpp c
EXT_INCLUDES = h hpp tpp
EXT_OTHERS = # par exemple, extensions de shaders : frag vert geom tesc tese comp glsl

# Liens vers bibliothèques à ajouter au projet
LDLIBS = # -lm : pour la bibliothèque <math.h>

LDFLAGS = # évite que des bibliothèques dynamiques inutilisées soient chargés par défaut

CFLAGS = # -pipe -MMD -MP
CPPFLAGS = # -pipe -MMD -MP

###############################################
# LE CODE CI-DESSOUS NE DOIT PAS ETRE MODIFIE #
###############################################

PRECOMPILED_MACROS = -D__PROJECT_DIRECTORY__=\"$(shell pwd)\"

DEFAULT_FLAGS = -Wall -Wextra -Wconversion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -Wdouble-promotion
DEFAULT_FLAGS += -I$(INC_DIR) -I$(TEMPLATE_DIR) $(foreach i, $(GLOBAL_INCLUDES), -include $(i))
DEFAULT_FLAGS += $(PRECOMPILED_MACROS)
DEFAULT_LDLIBS = # Rien pour l'instant...

# Choix des drapeaux selon le mode de compilation :
ifeq ($(BUILD), release)
BUILD_MOD = RELEASE
DEFAULT_FLAGS += -DNDEBUG
DEFAULT_FLAGS += -O3
DEFAULT_FLAGS += -march=native # fast permet les optis mathématiques même si moins bien dans le cas général
else
BUILD_MOD = DEBUG
DEFAULT_FLAGS += -O0 -fsanitize=address,leak -g3
LDFLAGS += -fsanitize=address,leak
endif

# Récupération des données nécessaires à la construction des binaires
ELF = $(BIN_DIR)/$(ELF_NAME)
SRC = $(foreach e,$(EXT_TO_COMPILE),$(shell find ./$(SRC_DIR)/ -name "*.$(e)"))
SRC := $(SRC:./%=%)
INC = $(foreach e,$(EXT_INCLUDES),$(shell find ./$(INC_DIR)/ -name "*.$(e)"))
INC := $(INC:./%=%)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJ := $(OBJ:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
LIB = $(wildcard $(LIB_DIR)/*.a)
ifdef TFILES
TESTS = $(foreach t,$(TFILES),$(shell find ./$(TEST_DIR)/ -name "$(t)"))
else
TESTS = $(foreach e,$(EXT_TO_COMPILE),$(shell find ./$(TEST_DIR)/ -name "*.test.$(e)"))
endif
TESTS := $(TESTS:./%=%)
TESTS := $(TESTS:$(TEST_DIR)/%.test.c=$(TESTS_BUILD)/%.c.exe)
TESTS := $(TESTS:$(TEST_DIR)/%.test.cpp=$(TESTS_BUILD)/%.cpp.exe)
TESTS_OBJ = $(filter-out %/main.o, $(OBJ))

all: $(ELF)

test: $(LINKER_SCRIPT) $(TESTS) | $(TESTS_BUILD)
	@echo "Compiling all tests ($(BUILD_MOD))"
	$(foreach t, $(TESTS), ./$(t);)

run: $(ELF)
	$(ELF)

$(TESTS_BUILD)/%.c.exe: $(TEST_DIR)/%.test.c $(TESTS_OBJ) | $(TESTS_BUILD) $(TESTS_EXECUTION_DIR)
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

$(TESTS_EXECUTION_DIR):
	mkdir -p $@

$(LINKER_SCRIPT): $(LD_DIR)/$(LINKER_SCRIPT_TEMPLATE) | $(BUILD_ROOT)
	$(CPREPROC) -xc -I$(INC_DIR) $(LD_DIR)/$(LINKER_SCRIPT_TEMPLATE) -o $(LINKER_SCRIPT)

.phony: format
format:
	clang-format $(SRC) $(INC) -i


.phony: help
help:
	@echo "OPTIONS :"
	@echo "- BUILD=(debug|release) : specify if compiling must be in debug of release mode, default is 'debug'"
	@echo "Commands :"
	@echo "- run : compile and run the executable"
	@echo "- clean : clean (no shit !?) your compiled code to get only the sources"
	@echo "- genstaticlib : generate a library from your project code as '$(BIN_DIR)/$(LIB_NAME).a'"
	@echo "- help : display this (hopefully) helping message"
	@echo "- count : count the number of lines of your project, just to show off in front of 'friends' because you have none."
	@echo "EXAMPLES :"
	@echo "- make test"
	@echo "- make run BUILD=release"
	@echo "- make genstaticlib BUILD=release"
	@echo "- make clean"
.phony: clean
clean:
	@echo "Cleaning build artifacts of the '$(BUILD_ROOT)' directory"
	@$(RM) -rf $(BUILD_ROOT)

.phony: count
count: # Pour le fun
	@echo "Considered files extensions are : $(EXT_TO_COMPILE) $(EXT_OTHERS) $(EXT_INCLUDES)"
	@find . -name "" $(foreach e,$(EXT_TO_COMPILE) $(EXT_OTHERS) $(EXT_INCLUDES), -or -name "*.$(e)") | xargs wc -l | awk 'BEGIN {total = 0;}{if (NF == 2 && $$2 == "total") total += $$1;} END {print "Total lines count : " total}'

.phony: genstaticlib
genstaticlib: $(OBJ) | $(BIN_DIR)
	@ar rcs $(BIN_DIR)/$(LIB_NAME).a $^
	@echo "Generating static library $(LIB_NAME).a ($(BUILD_MOD))"

.phony: gendynlib
gendynlib: $(OBJ) | $(BIN_DIR)
	$(CC) -shared -o $(BIN_DIR)/$(LIB_NAME).so $^
	@echo "Generating dynamic library $(LIB_NAME).so ($(BUILD_MOD))"

-include $(OBJ:.o=.d)
