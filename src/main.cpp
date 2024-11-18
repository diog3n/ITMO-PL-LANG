#include <any>
#include <iostream>

#include <antlr4-runtime.h>
#include "antlr/BlaiseParser.h"
#include "antlr/BlaiseLexer.h"
#include "InterpreterVisitor.h"

int main(int argc, const char** argv) {

    // Create an input file stream
    std::ifstream infile("src/antlr/inputfile.lang");

    if (!infile.is_open())
        return -1;

    // Create an ANTLR stream from the file stream
    antlr4::ANTLRInputStream input(infile);

    BlaiseLexer lexer(&input);

    // Create a token stream from the lexer
    antlr4::CommonTokenStream tokens(&lexer);

    // Create a parser from the token stream
    BlaiseParser parser(&tokens);

    BlaiseParser::ProgramContext* parse_result = parser.program();

    std::cout << parse_result->toStringTree(true) << std::endl;

    // Associate a visitor with the Suite context
    InterpreterVisitor visitor;

    std::any result = visitor.visitProgram(parse_result);

    std::cout << result.has_value() << std::endl;

    // std::string str = std::any_cast<std::string>(visitor.visitProgram(parser.program()));
    // std::cout << "Visitor output: " << str << std::endl;

    return 0;
}