CPP_SRC		:= $(wildcard src/*.cpp)
CPP_DIR		:= $(wildcard src/)
ANTLR_DIR	:= $(wildcard src/antlr)
ANTLR_GEN	:= $(wildcard src/antlr/*.cpp)

ANTLR_RUNTIME_INCLUDE	:= $(wildcard /home/ilia/bin/antlr4/runtime/Cpp/run/usr/local/include/antlr4-runtime)
ANTLR_RUNTIME_LIB		:= $(wildcard /home/ilia/bin/antlr4/runtime/Cpp/run/usr/local/lib)
ANTLR_CMD				:= java -Xmx500M -cp "$(HOME)/bin/antlr4/antlr-4.13.2-complete.jar:$(CLASSPATH)" org.antlr.v4.Tool

cpp:
	clang++ -g -std=c++17 -o parser $(CPP_SRC) $(ANTLR_GEN) -I$(ANTLR_RUNTIME_INCLUDE) -L$(ANTLR_RUNTIME_LIB) -lantlr4-runtime
antlr-gen:
	$(ANTLR_CMD) -Dlanguage=Cpp $(ANTLR_DIR)/Blaise.g4 -visitor
antlr-clean:
	cd $(ANTLR_DIR) && ./clean.sh
