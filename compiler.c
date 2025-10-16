#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    LET_KEYWORD,
    VAR_NAME,
    COLON_CHAR,
    INTEGER,
    TEXT,
    FLOAT,
    BOOL,
    EQUAL,
    VALUE,
    STRINGVALUE,
    TRUE_,
    FALSE_,
    SEMICOLON,
    WRITE,
    GETINPUT,
    LPAREN,
    RPAREN,
    EOF_T,
} TokenType;

typedef struct Token {
    TokenType type;
    char name[100];
} Token;

Token getNextToken(char **input) {
    while(isspace(**input)) (*input)++;
    if(**input == '\0') return (Token){EOF_T,""};

    if(isalpha(**input)) {
        Token token;
        int counter = 0;

        while(isalnum(**input) || **input == '_') {
            token.name[counter] = **input;
            (*input)++;
            counter++;
        }

        token.name[counter] = '\0';
        if(strcmp(token.name,"let")==0) {
            token.type = LET_KEYWORD;
            return token;
        } else if(strcmp(token.name,"write")==0) {
            token.type = WRITE;
            return token;
        } else if(strcmp(token.name,"getInput")==0) {
            token.type = GETINPUT;
            return token;
        } else if(strcmp(token.name,"integer")==0) {
            token.type = INTEGER;
            return token;
        } else if(strcmp(token.name,"string")==0) {
            token.type = TEXT;
            return token;
        } else if(strcmp(token.name,"float")==0) {
            token.type = FLOAT;
            return token;
        } else if(strcmp(token.name,"boolean")==0) {
            token.type = BOOL;
            return token;
        } else if(strcmp(token.name,"true")==0) {
            token.type = TRUE_;
            return token;
        } else if(strcmp(token.name,"false")==0) {
            token.type = FALSE_;
            return token;
        } else {
            token.type = VAR_NAME;
            return token;
        }
    }

    if(isdigit(**input)||**input=='-') {
        Token token = {VALUE};
        int counter = 0;
        if(**input == '-') {
            token.name[counter++] = **input;
            (*input)++;
        }

        while(isdigit(**input) || **input == '.') {
            token.name[counter] = **input;
            (*input)++;
            counter++;
        }

        token.name[counter] = '\0';
        return token;
    }

    if(**input == '"') {
        Token token = {STRINGVALUE};
        int counter = 0;

        (*input)++;
        while(**input != '"' && **input != '\0') {
            token.name[counter] = **input;
            (*input)++;
            counter++;
        }

        token.name[counter] = '\0';
        if(**input == '"') (*input)++; 

        return token;
    }

    char operation = *(*input)++;
    switch(operation) {
        case ':': return (Token){COLON_CHAR,":"}; 
        case '=': return (Token){EQUAL,"="};
        case ';': return (Token){SEMICOLON,";"};
        case '(': return (Token){LPAREN,"("};
        case ')': return (Token){RPAREN,")"};
        default: return (Token){EOF_T,""};
    }
}

typedef struct Variable {
    TokenType type[200];
    char var_name[200][40];
    struct {
        int int_value[200];
        float float_value[200];
        char stringValue[200][40];
        char boolean[200][20];
    } Values;
    struct {
        int TextOrVar[200]; //0 = CONTENT | 1 = VARIABLE
        char write_func_content[200][100];
        char var_name[200][40];
        TokenType type[200];
    } Write;
    struct {
        int AssignOrNot[200]; //0 = ASSIGN | 1 = NOT ASSIGN
        char content[200][100];
        char var_name_to_get_input[200][40];
        TokenType variableType[200];
    } GetInput;
    int counter;
} Variable;

int parseWriteFuncVar(Variable *ptr,Token token,char **input) {
    int foundIdx = -1;
    int index = -1;
    for(int i=0; i<ptr->counter; i++) {
        if(strcmp(ptr->var_name[i],token.name)==0) {
            foundIdx = 1;
            index = i;
            break;
        }
    }

    if(foundIdx != -1) {
        if(ptr->GetInput.AssignOrNot[index] == 0)
            ptr->Write.type[ptr->counter] = ptr->type[index];
        else if(ptr->GetInput.AssignOrNot[index] == 1)
            ptr->Write.type[ptr->counter] = ptr->GetInput.variableType[index]; 
        else 
            ptr->Write.type[ptr->counter] = ptr->type[index];

        strcpy(ptr->Write.var_name[ptr->counter],token.name);
        ptr->Write.TextOrVar[ptr->counter] = 1;

        token = getNextToken(input);
        if(token.type != RPAREN) {
            printf("Error: Forgot to close the parenthesis -> ')'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != SEMICOLON) {
            printf("Error: Forgot the semicolon -> ';'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != EOF_T) {
            printf("Error: Invalid arguments passed\n");
            return -1;
        }

        ptr->counter++;
        return 0;
    } else {
        printf("Error: Variable -> '%s' not found\n",token.name);
        return -1;
    }
}

int parseWriteFunc(Variable *ptr,char **input) {
    ptr->type[ptr->counter] = WRITE;
    Token token = getNextToken(input);
    if(token.type != LPAREN) {
        printf("Error: Forgot parenthesis -> '('\n");
        return -1;
    } 
    
    token = getNextToken(input);
    if(token.type != STRINGVALUE && token.type != VAR_NAME) {
        printf("Error: Invalid content: %s\n",token.name);
        return -1;
    }

    if(token.type == VAR_NAME) {
        int check = parseWriteFuncVar(ptr,token,input);
        if(check == -1) { return -1; }
        return 0;
    }

    ptr->Write.TextOrVar[ptr->counter] = 0;
    strcpy(ptr->Write.write_func_content[ptr->counter],token.name);

    token = getNextToken(input);
    if(token.type != RPAREN) {
        printf("Error: Forgot to close the parenthesis -> ')'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != SEMICOLON) {
        printf("Error: Forgot the semicolon -> ';'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != EOF_T) {
        printf("Error: Invalid arguments passed\n");
        return -1;
    }

    ptr->counter++;
    return 0;
}

int parseAssignGetInput(Variable *var,Token token,char **input) {
    int indexIdx = -1;
    int foundIdx = 0;
    for(int i=0; i<var->counter; i++) {
        if(strcmp(token.name,var->var_name[i])==0) {
            foundIdx = 1;
            indexIdx = i;
            break;
        }
    }

    if(!foundIdx) {
        printf("Error: Variable '%s' not found\n",token.name);
        return -1;
    }
        
    if(var->type[indexIdx] == BOOL) {
        printf("Error: Cannot take input on booleans\n");
        return -1;
    }

    var->GetInput.variableType[var->counter] = var->type[indexIdx];
    strcpy(var->GetInput.var_name_to_get_input[var->counter],token.name);

    token = getNextToken(input);
    if(token.type != EQUAL) {
        printf("Error: Forgot to assing the value -> '='\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != GETINPUT) {
        printf("Error: Invalid variable\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != LPAREN) {
        printf("Error: Forgot to assing the -> '('\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != STRINGVALUE) {
        printf("Error: Text must be inside of \"\"\n");
        return -1;
    }

    strcpy(var->GetInput.content[var->counter],token.name);

    token = getNextToken(input);
    if(token.type != RPAREN) {
        printf("Error: Forgot to assign the -> ')'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != SEMICOLON) {
        printf("Error: Forgot a semicolon -> ';'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != EOF_T) {
        printf("Error: Invalid arguments passed\n");
        return -1;
    }

    var->type[var->counter] = GETINPUT;
    var->counter++;

    return 0;
}

int parseGetInput(Variable *var,Token token,char **input,char *tempVarName,int tempType) {
    int foundIdx = -1;
    for(int i=0; i<var->counter; i++) {
        if(strcmp(var->var_name[i],tempVarName)==0) {
            foundIdx = 1;
            break;
        }
    }
        
    if(foundIdx == 1) {
        printf("Error: Cannot create this variable bc variable name already exists\n");
        return -1;
    }

    var->GetInput.AssignOrNot[var->counter] = 1;
    strcpy(var->GetInput.var_name_to_get_input[var->counter],tempVarName);
    if(tempType == BOOL) {
        printf("Error: Cannot take input on booleans\n");
        return -1;
    }

    var->GetInput.variableType[var->counter] = tempType;
    token = getNextToken(input);
    if(token.type != LPAREN) {
        printf("Error: Forgot to assign the parenthesis -> '('\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != STRINGVALUE) {
        printf("Error: Prompt must be inside of -> \"\"\n");
        return -1;
    }
        
    strcpy(var->GetInput.content[var->counter],token.name);
    token = getNextToken(input);
    if(token.type != RPAREN) {
        printf("Error: Forgot to close the parenthesis -> ')'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != SEMICOLON) {
        printf("Error: Forgot to put a semicolon at the end -> ';'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != EOF_T) {
        printf("Error: Invalid arguments passed\n");
        return -1;
    }

    var->type[var->counter] = GETINPUT;
    var->counter++;

    return 0;
}

int parseTokens(Variable *var,char **input) {
    Token token = getNextToken(input);
    if(token.type == EOF_T) return 0;

    if(token.type == VAR_NAME) {
        int check = parseAssignGetInput(var,token,input);
        if(check == -1) { return -1; }
        return 0;
    }

    if(token.type == WRITE) {
        if(parseWriteFunc(var,input)==-1) return -1;
        return 0;
    }

    if(token.type != LET_KEYWORD && token.type != WRITE) {
        printf("Error: Forgot to assing the variable using 'let'\n");
        return -1;
    } 

    token = getNextToken(input);
    if(token.type != VAR_NAME) {
        printf("Error: Invalid variable name '%s'\n",token.name);
        return -1;
    } 

    char tempVarName[40];
    strcpy(tempVarName,token.name);
    strcpy(var->var_name[var->counter],token.name);

    token = getNextToken(input);
    if(token.type != COLON_CHAR) {
        printf("Error: Forgot to assing ':' for variable values\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != INTEGER &&
      token.type != TEXT &&
      token.type != FLOAT &&
      token.type != BOOL) {
        printf("Error: Invalid type '%s'\n",token.name);
        return -1;
    }

    int tempType = token.type;
    var->type[var->counter] = token.type;

    token = getNextToken(input);
    if(token.type != EQUAL) {
        printf("Error: Forgot to assing the variable with '='\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type == GETINPUT) {
        int check = parseGetInput(var,token,input,tempVarName,tempType);
        if(check == -1) { return -1; }
        return 0;
    }
    
    if(token.type != VALUE && token.type != STRINGVALUE && token.type != FALSE_ && token.type != TRUE_) {
        printf("Error: Invalid value '%s'\n",token.name);
        return -1;
    }

    if(var->type[var->counter] == INTEGER) {
        if(token.type == STRINGVALUE) {
            printf("Error: Integers cannot be inside \"\"\n");
            return -1;
        }

        char *endPtr;
        int number = strtol(token.name,&endPtr,10);
        if(*endPtr != '\0') {
            printf("Error: Invalid integer '%s'\n",token.name);
            return -1;
        }

        var->Values.int_value[var->counter] = number;
    } else if(var->type[var->counter] == TEXT) {
        if(token.type != STRINGVALUE) {
            printf("Error: Strings must be inside \"\"\n");
            return -1;
        }

        strcpy(var->Values.stringValue[var->counter],token.name);
    } else if(var->type[var->counter] == FLOAT) {
        if(token.type == STRINGVALUE) {
            printf("Error: Floats cannot have \"\"\n");
            return -1;
        }

        char *endPtr;
        float number = strtof(token.name,&endPtr);
        if(*endPtr != '\0') {
            printf("Error: Invalid float '%s'\n",token.name);
            return -1;
        }

        var->Values.float_value[var->counter] = number;
    } else if(var->type[var->counter] == BOOL) {
        if(token.type == TRUE_) strcpy(var->Values.boolean[var->counter],"true");
        else if(token.type == FALSE_) strcpy(var->Values.boolean[var->counter],"false");
        else { printf("Error: Booleans cannot be inside of \"\"\n"); return -1; }
    }

    token = getNextToken(input);
    if(token.type != SEMICOLON) {
        printf("Error: Forgot a semicolon ';'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != EOF_T) {
        printf("Error: Invalid arguments passed\n");
        return -1;
    }

    var->GetInput.AssignOrNot[var->counter] = 0;
    var->counter++;

    return 0;
}

void codeGen(FILE *file,Variable *var) {
    for(int i=0; i<var->counter; i++) {
        if(var->type[i] == INTEGER) {
            fprintf(file,"    int %s = %d;\n",var->var_name[i],var->Values.int_value[i]);
        } else if(var->type[i] == TEXT) {
            fprintf(file,"    char %s[256] = \"%s\";\n",var->var_name[i],var->Values.stringValue[i]);
        } else if(var->type[i] == FLOAT) {
            fprintf(file,"    float %s = %f;\n",var->var_name[i],var->Values.float_value[i]);
        } else if(var->type[i] == BOOL) {
            fprintf(file,"    bool %s = %s;\n",var->var_name[i],var->Values.boolean[i]);
        } else if(var->type[i] == WRITE) {
            if(var->Write.TextOrVar[i] == 0) {
                fprintf(file,"    printf(\"%s\");\n",var->Write.write_func_content[i]);
            } else if(var->Write.TextOrVar[i] == 1){
                if(var->Write.type[i] == INTEGER) {
                    fprintf(file,"    printf(\"%%d\\n\",%s);\n",var->Write.var_name[i]);
                } else if(var->Write.type[i] == FLOAT) {
                    fprintf(file,"    printf(\"%%f\\n\",%s);\n",var->Write.var_name[i]);
                } else if(var->Write.type[i] == TEXT) {
                    fprintf(file,"    printf(\"%%s\\n\",%s);\n",var->Write.var_name[i]);
                } else if(var->Write.type[i] == BOOL) {
                    fprintf(file,"    printf(\"%%s\\n\", %s ? \"true\" : \"false\");\n", var->Write.var_name[i]);
                }
            }
        } else if(var->type[i] == GETINPUT) {
            if(var->GetInput.AssignOrNot[i] == 0) {
                fprintf(file,"    printf(\"%s\");\n",var->GetInput.content[i]);
                if(var->GetInput.variableType[i] == INTEGER) {
                    fprintf(file,"    scanf(\"%%d\",&%s);\n",var->GetInput.var_name_to_get_input[i]);
                } else if(var->GetInput.variableType[i] == TEXT) {
                    fprintf(file,"    scanf(\"%%s\",%s);\n",var->GetInput.var_name_to_get_input[i]);
                } else if(var->GetInput.variableType[i] == FLOAT) {
                    fprintf(file,"    scanf(\"%%f\",&%s);\n",var->GetInput.var_name_to_get_input[i]);
                }  
            } else {
                if(var->GetInput.variableType[i] == INTEGER) {
                    fprintf(file,"    int %s;\n",var->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",var->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%d\",&%s);\n",var->GetInput.var_name_to_get_input[i]);
                } else if(var->GetInput.variableType[i] == TEXT) {
                    fprintf(file,"    char %s[256];\n",var->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",var->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%s\",%s);\n",var->GetInput.var_name_to_get_input[i]);
                } else if(var->GetInput.variableType[i] == FLOAT) {
                    fprintf(file,"    float %s;\n",var->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",var->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%f\",&%s);\n",var->GetInput.var_name_to_get_input[i]);
                } 
            }
        } 
    }
}

int main(int argc,char *argv[]) {
    if(argc != 2) {
        printf("Error: Invalid arguments count\n");
        printf("Usage: ./main <file_name>\n");
        return 1;
    }

    Variable var = {.counter=0};
    FILE *input = fopen(argv[1],"r");
    if(!input) {
        printf("Error: Failed to open the file\n");
        return 1;
    }

    FILE *file = fopen("output.c","w");
    fprintf(file,"#include <stdio.h>\n#include <stdbool.h>\nint main(void) {\n");

    char line[256];
    while(fgets(line,sizeof(line),input)) {
        char *ptr = line;
        int check = parseTokens(&var,&ptr);
        if(check == -1) { remove("output.c"); return -1; }
    }

    fclose(input);
    codeGen(file,&var);

    fprintf(file,"    return 0;\n}\n");
    fclose(file);

    system("gcc output.c -o main");
    system("rm output.c");

    return 0;
}
