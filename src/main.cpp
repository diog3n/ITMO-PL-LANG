#include <any>
#include <iostream>

#include <antlr4-runtime.h>
#include <stdexcept>
#include "TacCompilerVisitor.h"
#include "antlr/BlaiseParser.h"
#include "antlr/BlaiseLexer.h"

#include "InterpreterVisitor.h"
#include "BlaiseErrorListener.h"

enum ARGV_POSITIONS {
    IN_FILE = 1,
};

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cout << "Usage: ./blaise [input_file.bls]" << std::endl;
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

    // Associate a visitor with the Suite context
    // InterpreterVisitor visitor;
    TacCompilerVisitor visitor;

    // try {
        std::any result = visitor.visitProgram(parse_result);
    // } catch (std::invalid_argument& e) {
        // std::cout << e.what() << std::endl;
    // }

    std::cout << std::any_cast<std::string>(result) << std::endl;

    return 0;
}