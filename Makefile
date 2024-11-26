CPP_SRC		:= $(wildcard src/*.cpp)
CPP_DIR		:= $(wildcard src/)
ANTLR_DIR	:= $(wildcard src/antlr)
ANTLR_GEN	:= $(wildcard src/antlr/*.cpp)

ANTLR_RUNTIME_INCLUDE	:= $(wildcard /path/to/antlr4-runtime)
ANTLR_RUNTIME_LIB		:= $(wildcard /path/to/runtime/lib)
ANTLR_CMD				:= java -Xmx500M -cp "$(HOME)/path/to/antlr-4.13.2-complete.jar:$(CLASSPATH)" org.antlr.v4.Tool

cpp:
	clang++ -g -std=c++17 -o blaise $(CPP_SRC) $(ANTLR_GEN) -I$(ANTLR_RUNTIME_INCLUDE) -L$(ANTLR_RUNTIME_LIB) -lantlr4-runtime
antlr-gen:
	$(ANTLR_CMD) -Dlanguage=Cpp $(ANTLR_DIR)/Blaise.g4 -visitor
antlr-clean:
	cd $(ANTLR_DIR) && ./clean.sh
