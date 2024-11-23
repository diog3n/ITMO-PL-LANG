grammar Blaise;

program : (stmt)* EOF;

stmt    : function_def
        | function_call ';'
        | block
        | expr ';'
        | return_stmt ';'
        | if_stmt
        | loop_stmt
        | assignment ';'
        | writeln_stmt ';'
        ;

function_def    : FUNCTION IDENTIFIER '(' (param_list)? ')' ';'                             # FunctionDeclaration
                | FUNCTION IDENTIFIER '(' (param_list)? ')' stmt                            # FunctionDefinition
                ;

function_call   : IDENTIFIER '(' (arg_list)? ')'                                            # FunctionCall
                ;

param_list      : IDENTIFIER ',' param_list                                                 # ParamListComma
                | IDENTIFIER                                                                # ParamListEnd
                ;

arg_list        : (IDENTIFIER | expr) ',' arg_list                                          # ArgListComma
                | (IDENTIFIER | expr)                                                       # ArgListEnd
                ;

block           : 'begin' (stmt)* 'end'                                                     # CodeBlock
                ;

return_stmt     : 'return' (IDENTIFIER | expr)                                              # ReturnStmt
                ;

writeln_stmt    : 'writeln' '(' expr ')'                                                    # WritelnStmt
                ;

if_stmt         : 'if' '(' expr ')' 'then' stmt (else_stmt)?                                # IfStmtBlock
                ;

else_stmt       : 'else' stmt                                                               # ElseStmtBlock
                | 'else' if_stmt                                                            # ElseIfStmt
                ;

loop_stmt       : 'loop' 'if' '(' expr ')' (stmt | ';')                                     # LoopStmt
                ;

assignment      : IDENTIFIER ASSIGN expr                                                    # AssignStmt
                ;

expr            : operand operator expr                                                     # ExprOperation
                | MINUS operand                                                             # ExprUnaryMinusOperation
                | PLUS operand                                                              # ExprUnaryPlusOperation
                | operand                                                                   # ExprOperand
                ;

operand         : BOOLEAN                                                                   # OperandBoolean
                | INT                                                                       # OperandInt
                | DOUBLE                                                                    # OperandDouble
                | CHAR                                                                      # OperandChar
                | STRING                                                                    # OperandString
                | IDENTIFIER                                                                # OperandId
                | function_call                                                             # OperandFunctionCall
                | '(' expr ')'                                                              # OperandExpr
                ;

operator        : PLUS                                                                      # OperatorPlus
                | MINUS                                                                     # OperatorMinus
                | ASTERISK                                                                  # OperatorAster
                | SLASH                                                                     # OperatorSlash
                | EQUAL                                                                     # OperatorEqual
                | NOT_EQUAL                                                                 # OperatorNEqual
                | LESS                                                                      # OperatorLess
                | LESS_OR_EQUAL                                                             # OperatorLEqual
                | GREATER                                                                   # OperatorGreater
                | GREATER_OR_EQUAL                                                          # OperatorGEqual
                ;

FUNCTION            : 'function' ;
RETURNS             : 'returns' ;
RETURN              : 'return' ;
BOOLEAN             : 'true'
                    | 'false'
                    ;
IDENTIFIER          : [a-zA-Z_] [a-zA-Z_0-9]*;
INT                 : [0-9]+;
DOUBLE              : [0-9]+ '.' [0-9]+
                    | [0-9]+ 'e' [+|-][0-9]+
                    ;
CHAR                : '\'' . '\''
                    ;
STRING              : '"' .*? '"'
                    ;
ASTERISK            : '*' ;
SLASH               : '/' ;
PLUS                : '+' ;
MINUS               : '-' ;
ASSIGN              : '=' ;
EQUAL               : '==' ;
NOT_EQUAL           : '!=' ;
LESS                : '<' ;
LESS_OR_EQUAL       : '<=' ;
GREATER             : '>' ;
GREATER_OR_EQUAL    : '>=' ;

SPACE               : [ \r\n\t]+ -> skip;
LINE_COMMENT        : '//' ~[\n\r]* -> skip;
BLOCK_COMMENT       : '/*' .*? '*/' -> skip;