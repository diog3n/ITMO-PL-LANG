#include <any>
#include <iostream>

#include <antlr4-runtime.h>
#include <stdexcept>
#include "ANTLRInputStream.h"
#include "CommonTokenStream.h"
#include "TacCompilerVisitor.h"
#include "antlr/BlaiseParser.h"
#include "antlr/BlaiseLexer.h"

#include "InterpreterVisitor.h"
#include "BlaiseErrorListener.h"

enum ARGV_POSITIONS {
    IN_FILE = 2,
    COMMAND = 1
};

int main(int argc, const char** argv) {
    if (argc < 3) {
        std::cout << "Usage: ./blaise [command] [input_file.bls]" << std::endl;
        return 1;
    }
    std::ifstream infile(argv[IN_FILE]);

    if (!infile.is_open())
        return -1;

    antlr4::ANTLRInputStream input(infile);

    BlaiseLexer lexer(&input);

    antlr4::CommonTokenStream tokens(&lexer);

    BlaiseErrorListener errlistener;

    BlaiseParser parser(&tokens);

    parser.removeErrorListeners();
    parser.addErrorListener(&errlistener);

    BlaiseParser::ProgramContext* parse_result = parser.program();

    if (parser.getNumberOfSyntaxErrors()) {
        std::cout << "Parsing failed with " << parser.getNumberOfSyntaxErrors()
                  << " errors" << std::endl;
        return 1;
    }

    if (strcmp(argv[COMMAND], "comp") == 0) {
        TacCompilerVisitor compiler;
        std::any result = compiler.visitProgram(parse_result);
        std::string compiled_text = std::any_cast<std::string>(result);
        std::cout << compiled_text << std::endl;
    } else if (strcmp(argv[COMMAND], "interp") == 0) {
        InterpreterVisitor interpreter;
        interpreter.visitProgram(parse_result);
    } else {
        std::cout << "Unknown command" << std::endl;
    }

    return 0;
}