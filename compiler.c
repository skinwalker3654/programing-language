#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {
    /*Keywords*/
    TOKEN_LET,
    TOKEN_STRING,
    TOKEN_FLOAT,
    TOKEN_BOOL,
    TOKEN_INTEGER,
    TOKEN_TO,
    TOKEN_LOOP,

    /*Symbols*/
    TOKEN_COLON,
    TOKEN_EQUAL, 
    TOKEN_LPAR,
    TOKEN_RPAR,
    TOKEN_SEMICOLON,

    /*Values type*/
    TOKEN_NUMLITERAL,
    TOKEN_STRLITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,

    /*Functions*/
    TOKEN_WRITE,
    TOKEN_GETINPUT,

    /*Other*/
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_UNKNOWN,
} TokenType;

typedef struct Lexer {
    const char *source;
    int length;
    int pos;
    int line;
    int column;
} Lexer;

/*Util functions*/
Lexer lexer_init(const char *source) {
    Lexer new_lx = {source,strlen(source),0,1,1};
    return new_lx;
}

char peek(Lexer *ptr) {
    if(ptr->pos >= ptr->length) return '\0';
    return ptr->source[ptr->pos];
}

char advance(Lexer *ptr) {
    char current_char;
    if(ptr->pos >= ptr->length) {
        current_char = '\0';
        return current_char;
    }

    current_char = ptr->source[ptr->pos++];
    if(current_char == '\n') {
        ptr->line++;
        ptr->column=1;
        return current_char;
    } else {
        ptr->column++;
        return current_char;
    }
}

void skip_white_spaces(Lexer *ptr) {
    while(isspace(peek(ptr))) {
        advance(ptr);
    }
}

/******TOKENS******/
typedef struct Token {
    TokenType type;
    char value[256];
    int line;
    int column;
} Token;

Token make_token(TokenType type,char *value,int line,int column) {
    Token newToken;
    newToken.type = type;
    strcpy(newToken.value,value);
    newToken.line = line;
    newToken.column = column;
    return newToken;
}

Token getNextToken(Lexer *ptr) {
    skip_white_spaces(ptr);
    char current_char = peek(ptr);

    if(peek(ptr) == '\0') 
        return make_token(TOKEN_EOF,"",ptr->line,ptr->column);

    if(peek(ptr) == '#') {
        while(peek(ptr) != '\0') {
            advance(ptr);
        }
    }

    if(isalpha(current_char)) {
        char buffer[256];
        int counter = 0;

        while(isalnum(peek(ptr)) || peek(ptr) == '_') {
            buffer[counter] = advance(ptr);
            counter++;
        }

        buffer[counter] = '\0';
        if(strcmp(buffer,"let")==0) {
            return make_token(TOKEN_LET,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"write")==0) {
            return make_token(TOKEN_WRITE,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"getInput")==0) {
            return make_token(TOKEN_GETINPUT,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"integer")==0) {
            return make_token(TOKEN_INTEGER,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"string")==0) {
            return make_token(TOKEN_STRING,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"float")==0) {
            return make_token(TOKEN_FLOAT,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"boolean")==0) {
            return make_token(TOKEN_BOOL,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"true")==0) {
            return make_token(TOKEN_TRUE,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"false")==0) {
            return make_token(TOKEN_FALSE,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"loop")==0) {
            return make_token(TOKEN_LOOP,buffer,ptr->line,ptr->column);
        } else if(strcmp(buffer,"to")==0) {
            return make_token(TOKEN_TO,buffer,ptr->line,ptr->column);
        } else {
            return make_token(TOKEN_IDENTIFIER,buffer,ptr->line,ptr->column);
        }
    }

    if(isdigit(current_char) || (current_char== '-' && isdigit(ptr->source[ptr->pos+1]))) {
        char buffer[256];
        int counter = 0;

        if(peek(ptr) == '-') {
            buffer[counter] = advance(ptr);
            counter++;
        }

        while(isdigit(peek(ptr)) || peek(ptr) == '.') {
            buffer[counter] = advance(ptr);
            counter++;
        }

        buffer[counter] = '\0';
        return make_token(TOKEN_NUMLITERAL,buffer,ptr->line,ptr->column);
    }

    if(current_char == '"') {
        char buffer[256];
        int counter = 0;

        advance(ptr);
        while(peek(ptr) != '"' && peek(ptr) != '\0') {
            buffer[counter] = advance(ptr);
            counter++;
        }

        advance(ptr);
        buffer[counter] = '\0';

        return make_token(TOKEN_STRLITERAL,buffer,ptr->line,ptr->column);
    }

    char operation[2] = {current_char,'\0'};
    advance(ptr);

    switch(operation[0]) {
        case ':': return make_token(TOKEN_COLON,operation,ptr->line,ptr->column);
        case '=': return make_token(TOKEN_EQUAL,operation,ptr->line,ptr->column);
        case ';': return make_token(TOKEN_SEMICOLON,operation,ptr->line,ptr->column);
        case '(': return make_token(TOKEN_LPAR,operation,ptr->line,ptr->column);
        case ')': return make_token(TOKEN_RPAR,operation,ptr->line,ptr->column);
    }

    return make_token(TOKEN_UNKNOWN,"",ptr->line,ptr->column);
}

typedef struct Write {
    int TextOrVar[200];
    char write_func_content[200][100];
    char var_name[200][40];
    TokenType type[200];
} Write;

typedef struct Values {
    TokenType type[200];
    char var_name[200][40];
    struct {
        int int_value[200];
        float float_value[200];
        char stringValue[200][40];
        char boolean[200][20];
    } Values;
    struct {
        Write write;
    } Write;
    struct {
        int AssignOrNot[200]; //0 = ASSIGN | 1 = NOT ASSIGN
        char content[200][100];
        char var_name_to_get_input[200][40];
        TokenType variableType[200];
    } GetInput;
    struct {
        int end[200];
        char var_name[200][40];
        char endName[200][40];
        int varNameOrend[200];
        Write write; 
    } Loop;
    int counter;
} Values;

int parseWriteFuncVar(Values *ptr,Token token,Lexer *input,int writeOrloop) {
    if(writeOrloop == 0) {
        int foundIdx = -1;
        int index = -1;
        for(int i=0; i<=ptr->counter; i++) {
            if(strcmp(ptr->var_name[i],token.value)==0) {
                foundIdx = 1;
                index = i;
                break;
            }
        }

        if(foundIdx != -1) {
            if(ptr->GetInput.AssignOrNot[index] == 0)
                ptr->Write.write.type[ptr->counter] = ptr->type[index];
            else if(ptr->GetInput.AssignOrNot[index] == 1)
                ptr->Write.write.type[ptr->counter] = ptr->GetInput.variableType[index]; 
            else 
                ptr->Write.write.type[ptr->counter] = ptr->type[index];
    
            strcpy(ptr->Write.write.var_name[ptr->counter],token.value);
            ptr->Write.write.TextOrVar[ptr->counter] = 1;
    
            token = getNextToken(input);
            if(token.type != TOKEN_RPAR) {
                printf("Error: Forgot to close the parenthesis -> ')'\n");
                return -1;
            }
    
            token = getNextToken(input);
            if(token.type != TOKEN_SEMICOLON) {
                printf("Error: Forgot the semicolon -> ';'\n");
                return -1;
            }
    
            ptr->counter++;
            return 0;
        } else {
            printf("Error: Variable -> '%s' not found\n",token.value);
            return -1;
        }
    } else if(writeOrloop == 1) {
        int foundIdx = -1;
        int index = -1;
        for(int i=0; i<=ptr->counter; i++) {
            if(strcmp(ptr->var_name[i],token.value)==0) {
                foundIdx = 1;
                index = i;
                break;
            }
        }

        if(foundIdx != -1) {
            if(ptr->GetInput.AssignOrNot[index] == 0)
                ptr->Loop.write.type[ptr->counter] = ptr->type[index];
            else if(ptr->GetInput.AssignOrNot[index] == 1)
                ptr->Loop.write.type[ptr->counter] = ptr->GetInput.variableType[index]; 
            else 
                ptr->Loop.write.type[ptr->counter] = ptr->type[index];
    
            strcpy(ptr->Loop.write.var_name[ptr->counter],token.value);
            ptr->Loop.write.TextOrVar[ptr->counter] = 1;

            token = getNextToken(input);
            if(token.type != TOKEN_RPAR) {
                printf("Error: Forgot to close the parenthesis -> ')'\n");
                return -1;
            }

            token = getNextToken(input);
            if(token.type != TOKEN_SEMICOLON) {
                printf("Error: Forgot the semicolon -> ';'\n");
                return -1;
            }

            ptr->counter++;
            return 0;
        } else {
            printf("Error: Variable -> '%s' not found\n",token.value);
            return -1;
        }
    }

    return -1;
}

int parseWriteFunc(Values *ptr,Lexer *input,int writeOrloop) {
    if(writeOrloop == 0) {
        ptr->type[ptr->counter] = TOKEN_WRITE;
        Token token = getNextToken(input);
        if(token.type != TOKEN_LPAR) {
            printf("Error: Forgot parenthesis -> '('\n");
            return -1;
        } 
    
        token = getNextToken(input);
        if(token.type != TOKEN_STRLITERAL && token.type != TOKEN_IDENTIFIER) {
            printf("Error: Invalid content: %s\n",token.value);
            return -1;
        }

        if(token.type == TOKEN_IDENTIFIER) {
            int check = parseWriteFuncVar(ptr,token,input,0);
            if(check == -1) { return -1; }
            return 0;
        }

        ptr->Write.write.TextOrVar[ptr->counter] = 0;
        strcpy(ptr->Write.write.write_func_content[ptr->counter],token.value);
    
        token = getNextToken(input);
        if(token.type != TOKEN_RPAR) {
            printf("Error: Forgot to close the parenthesis -> ')'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != TOKEN_SEMICOLON) {
            printf("Error: Forgot the semicolon -> ';'\n");
            return -1;
        }

        ptr->counter++;
        return 0;
    } else if(writeOrloop == 1) {
        ptr->type[ptr->counter] = TOKEN_LOOP;
        Token token = getNextToken(input);
        if(token.type != TOKEN_LPAR) {
            printf("Error: Forgot parenthesis -> '('\n");
            return -1;
        } 
    
        token = getNextToken(input);
        if(token.type != TOKEN_STRLITERAL && token.type != TOKEN_IDENTIFIER) {
            printf("Error: Invalid content: %s\n",token.value);
            return -1;
        }

        if(token.type == TOKEN_IDENTIFIER) {
            int check = parseWriteFuncVar(ptr,token,input,1);
            if(check == -1) { return -1; }
            return 0;
        }

        ptr->Loop.write.TextOrVar[ptr->counter] = 0;
        strcpy(ptr->Loop.write.write_func_content[ptr->counter],token.value);
    
        token = getNextToken(input);
        if(token.type != TOKEN_RPAR) {
            printf("Error: Forgot to close the parenthesis -> ')'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != TOKEN_SEMICOLON) {
            printf("Error: Forgot the semicolon -> ';'\n");
            return -1;
        }

        ptr->counter++;
        return 0; 
    }

    return -1;
}

int parseAssignGetInput(Values *val,Token token,Lexer *input) {
    int indexIdx = -1;
    int foundIdx = 0;
    for(int i=0; i<val->counter; i++) {
        if(strcmp(token.value,val->var_name[i])==0) {
            foundIdx = 1;
            indexIdx = i;
            break;
        }
    }

    if(!foundIdx) {
        printf("Error: Values '%s' not found\n",token.value);
        return -1;
    }
        
    if(val->type[indexIdx] == TOKEN_BOOL) {
        printf("Error: Cannot take input on booleans\n");
        return -1;
    }

    val->GetInput.variableType[val->counter] = val->type[indexIdx];
    strcpy(val->GetInput.var_name_to_get_input[val->counter],token.value);

    token = getNextToken(input);
    if(token.type != TOKEN_EQUAL) {
        printf("Error: Forgot to assing the value -> '='\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != TOKEN_GETINPUT) {
        printf("Error: Invalid valiable\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != TOKEN_LPAR) {
        printf("Error: Forgot to assing the -> '('\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != TOKEN_STRLITERAL) {
        printf("Error: Text must be inside of \"\"\n");
        return -1;
    }

    strcpy(val->GetInput.content[val->counter],token.value);

    token = getNextToken(input);
    if(token.type != TOKEN_RPAR) {
        printf("Error: Forgot to assign the -> ')'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != TOKEN_SEMICOLON) {
        printf("Error: Forgot a semicolon -> ';'\n");
        return -1;
    }

    val->type[val->counter] = TOKEN_GETINPUT;
    val->counter++;

    return 0;
}

int parseGetInput(Values *val,Token token,Lexer *input,char *tempVarName,int tempType) {
    int foundIdx = -1;
    for(int i=0; i<val->counter; i++) {
        if(strcmp(val->var_name[i],tempVarName)==0) {
            foundIdx = 1;
            break;
        }
    }
        
    if(foundIdx == 1) {
        printf("Error: Cannot create this valiable bc variable name already exists\n");
        return -1;
    }

    val->GetInput.AssignOrNot[val->counter] = 1;
    strcpy(val->GetInput.var_name_to_get_input[val->counter],tempVarName);
    if(tempType == TOKEN_BOOL) {
        printf("Error: Cannot take input on booleans\n");
        return -1;
    }

    val->GetInput.variableType[val->counter] = tempType;
    token = getNextToken(input);
    if(token.type != TOKEN_LPAR) {
        printf("Error: Forgot to assign the parenthesis -> '('\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != TOKEN_STRLITERAL) {
        printf("Error: Prompt must be inside of -> \"\"\n");
        return -1;
    }
        
    strcpy(val->GetInput.content[val->counter],token.value);
    token = getNextToken(input);
    if(token.type != TOKEN_RPAR) {
        printf("Error: Forgot to close the parenthesis -> ')'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != TOKEN_SEMICOLON) {
        printf("Error: Forgot to put a semicolon at the end -> ';'\n");
        return -1;
    }

    val->type[val->counter] = TOKEN_GETINPUT;
    val->counter++;

    return 0;
}

int parseTokens(Values *val,Lexer *input) {
    Token token = getNextToken(input);
    if(token.type == TOKEN_EOF) return 0;

    if(token.type == TOKEN_LOOP) {
        token = getNextToken(input);
        if(token.type != TOKEN_IDENTIFIER) {
            printf("Error: Invalid value '%s'\n",token.value);

            return -1;
        }

        strcpy(val->var_name[val->counter],token.value);
        token = getNextToken(input);
        if(token.type != TOKEN_EQUAL) {
            printf("Error: Forgot to assign the '='\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != TOKEN_NUMLITERAL) {
            printf("Error: Invalid value '%s'\n",token.value);
            return -1;
        }

        char *endPtr;
        int number = strtol(token.value,&endPtr,10);
        if(*endPtr != '\0') {
            printf("Error: Invalid value '%s'\n",token.value);
            return -1;
        }
        
        val->Values.int_value[val->counter] = number;
        token = getNextToken(input);
        if(token.type != TOKEN_TO) {
            printf("Error: Forgot to assign 'to'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != TOKEN_NUMLITERAL && token.type != TOKEN_IDENTIFIER) {
            printf("Error: Invalid value '%s'\n",token.value);
            return -1;
        }

        if(token.type == TOKEN_NUMLITERAL) {
            char *endPtr2;
            int number2 = strtol(token.value,&endPtr2,10);
            if(*endPtr2 != '\0') {
                printf("Error: Invalid value '%s'\n",token.value);
                return -1;
            }
        
            val->Loop.end[val->counter] = number2;
            val->Loop.varNameOrend[val->counter] = 0;
        } else if(token.type == TOKEN_IDENTIFIER) {
            int found = 0;
            for(int i=0; i<val->counter; i++) {
                if(strcmp(val->var_name[i],token.value)==0) {
                    found = 1;
                    break;
                }
            }

            if(found == 0) {
                printf("Error: Variable '%s' not found\n",token.value);
                return -1;
            }

            strcpy(val->Loop.endName[val->counter],token.value);
            val->Loop.varNameOrend[val->counter] = 1;
        }

        token = getNextToken(input);
        if(token.type != TOKEN_COLON) {
            printf("Error: Forgot to assign ':'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != TOKEN_WRITE) {
            printf("Error: Only write() is allowed for loops\n");
            return -1;
        }

        int check = parseWriteFunc(val,input,1);
        if(check == -1) { return -1;}
        return 0;
    }

    if(token.type == TOKEN_IDENTIFIER) {
        int check = parseAssignGetInput(val,token,input);
        if(check == -1) { return -1; }
        return 0;
    }

    if(token.type == TOKEN_WRITE) {
        if(parseWriteFunc(val,input,0)==-1) return -1;
        return 0;
    }

    if(token.type != TOKEN_LET && token.type != TOKEN_WRITE) {
        printf("Error: Forgot to assing the valiable using 'let'\n");
        return -1;
    } 

    token = getNextToken(input);
    if(token.type != TOKEN_IDENTIFIER) {
        printf("Error: Invalid valiable name '%s'\n",token.value);
        return -1;
    } 

    char tempVarName[40];
    strcpy(tempVarName,token.value);
    strcpy(val->var_name[val->counter],token.value);

    token = getNextToken(input);
    if(token.type != TOKEN_COLON) {
        printf("Error: Forgot to assing ':' for valiable values\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != TOKEN_INTEGER &&
      token.type != TOKEN_STRING &&
      token.type != TOKEN_FLOAT &&
      token.type != TOKEN_BOOL) {
        printf("Error: Invalid type '%s'\n",token.value);
        return -1;
    }

    int tempType = token.type;
    val->type[val->counter] = token.type;

    token = getNextToken(input);
    if(token.type != TOKEN_EQUAL) {
        printf("Error: Forgot to assing the valiable with '='\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type == TOKEN_GETINPUT) {
        int check = parseGetInput(val,token,input,tempVarName,tempType);
        if(check == -1) { return -1; }
        return 0;
    }
    
    if(token.type != TOKEN_NUMLITERAL && token.type != TOKEN_STRLITERAL && token.type != TOKEN_FALSE && token.type != TOKEN_TRUE) {
        printf("Error: Invalid value '%s'\n",token.value);
        return -1;
    }

    if(val->type[val->counter] == TOKEN_INTEGER) {
        if(token.type == TOKEN_STRLITERAL) {
            printf("Error: Integers cannot be inside \"\"\n");
            return -1;
        }

        char *endPtr;
        int number = strtol(token.value,&endPtr,10);
        if(*endPtr != '\0') {
            printf("Error: Invalid integer '%s'\n",token.value);
            return -1;
        }

        val->Values.int_value[val->counter] = number;
    } else if(val->type[val->counter] == TOKEN_STRING) {
        if(token.type != TOKEN_STRLITERAL) {
            printf("Error: Strings must be inside \"\"\n");
            return -1;
        }

        strcpy(val->Values.stringValue[val->counter],token.value);
    } else if(val->type[val->counter] == TOKEN_FLOAT) {
        if(token.type == TOKEN_STRLITERAL) {
            printf("Error: Floats cannot have \"\"\n");
            return -1;
        }

        char *endPtr;
        float number = strtof(token.value,&endPtr);
        if(*endPtr != '\0') {
            printf("Error: Invalid float '%s'\n",token.value);
            return -1;
        }

        val->Values.float_value[val->counter] = number;
    } else if(val->type[val->counter] == TOKEN_BOOL) {
        if(token.type == TOKEN_TRUE) strcpy(val->Values.boolean[val->counter],"true");
        else if(token.type == TOKEN_FALSE) strcpy(val->Values.boolean[val->counter],"false");
        else { printf("Error: Booleans cannot be inside of \"\"\n"); return -1; }
    }

    token = getNextToken(input);
    if(token.type != TOKEN_SEMICOLON) {
        printf("Error: Forgot a semicolon ';'\n");
        return -1;
    }

    val->GetInput.AssignOrNot[val->counter] = 0;
    val->counter++;

    return 0;
}

void codeGen(FILE *file,Values *val) {
    for(int i=0; i<=val->counter; i++) {
        if(val->type[i] == TOKEN_INTEGER) {
            fprintf(file,"    int %s = %d;\n",val->var_name[i],val->Values.int_value[i]);
        } else if(val->type[i] == TOKEN_STRING) {
            fprintf(file,"    char %s[256] = \"%s\";\n",val->var_name[i],val->Values.stringValue[i]);
        } else if(val->type[i] == TOKEN_FLOAT) {
            fprintf(file,"    float %s = %f;\n",val->var_name[i],val->Values.float_value[i]);
        } else if(val->type[i] == TOKEN_BOOL) {
            fprintf(file,"    bool %s = %s;\n",val->var_name[i],val->Values.boolean[i]);
        } else if(val->type[i] == TOKEN_WRITE) {
            if(val->Write.write.TextOrVar[i] == 0) {
                fprintf(file,"    printf(\"%s\");\n",val->Write.write.write_func_content[i]);
            } else if(val->Write.write.TextOrVar[i] == 1){
                if(val->Write.write.type[i] == TOKEN_INTEGER) {
                    fprintf(file,"    printf(\"%%d\\n\",%s);\n",val->Write.write.var_name[i]);
                } else if(val->Write.write.type[i] == TOKEN_FLOAT) {
                    fprintf(file,"    printf(\"%%f\\n\",%s);\n",val->Write.write.var_name[i]);
                } else if(val->Write.write.type[i] == TOKEN_STRING) {
                    fprintf(file,"    printf(\"%%s\\n\",%s);\n",val->Write.write.var_name[i]);
                } else if(val->Write.write.type[i] == TOKEN_BOOL) {
                    fprintf(file,"    printf(\"%%s\\n\", %s ? \"true\" : \"false\");\n", val->Write.write.var_name[i]);
                }
            }
        } else if(val->type[i] == TOKEN_GETINPUT) {
            if(val->GetInput.AssignOrNot[i] == 0) {
                fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                if(val->GetInput.variableType[i] == TOKEN_INTEGER) {
                    fprintf(file,"    scanf(\"%%d\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == TOKEN_STRING) {
                    fprintf(file,"    scanf(\"%%s\",%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == TOKEN_FLOAT) {
                    fprintf(file,"    scanf(\"%%f\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                }  
            } else {
                if(val->GetInput.variableType[i] == TOKEN_INTEGER) {
                    fprintf(file,"    int %s;\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%d\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == TOKEN_STRING) {
                    fprintf(file,"    char %s[256];\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%s\",%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == TOKEN_FLOAT) {
                    fprintf(file,"    float %s;\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%f\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\n'&&ch%d!=EOF);\n",i,i);

                } 
            }
        } else if(val->type[i] == TOKEN_LOOP) {
            if(val->Loop.varNameOrend[i] == 0) {
                if(val->Loop.write.TextOrVar[i]==0) {
                    fprintf(file,"    for(int %s=%d; %s<%d; %s++) printf(\"%s\");\n"
                            ,val->var_name[i]
                            ,val->Values.int_value[i]
                            ,val->var_name[i]
                            ,val->Loop.end[i]
                            ,val->var_name[i]
                            ,val->Loop.write.write_func_content[i]);
                } else if(val->Loop.write.TextOrVar[i] == 1) {
                    if(val->Loop.write.type[i] == TOKEN_INTEGER) {
                        fprintf(file,"    for(int %s=%d; %s<%d; %s++) printf(\"%%d\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.end[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else if(val->Loop.write.type[i] == TOKEN_FLOAT) {
                        fprintf(file,"    for(int %s=%d; %s<%d; %s++) printf(\"%%f\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.end[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else if(val->Loop.write.type[i] == TOKEN_STRING) {
                        fprintf(file,"    for(int %s=%d; %s<%d; %s++) printf(\"%%s\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.end[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else if(val->Loop.write.type[i] == TOKEN_BOOL) {
                        fprintf(file,"    for(int %s=%d; %s<%d; %s++) printf(\"%%s\\n\", %s ? \"true\" : \"false\");\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i] 
                                ,val->Loop.end[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else {
                        fprintf(file,"    for(int %s=%d; %s<%d; %s++) printf(\"%%d\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.end[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]); 
                    }
                } 
            } else if(val->Loop.varNameOrend[i] == 1) {
                if(val->Loop.write.TextOrVar[i]==0) {
                    fprintf(file,"    for(int %s=%d; %s<%s; %s++) printf(\"%s\");\n"
                            ,val->var_name[i]
                            ,val->Values.int_value[i]
                            ,val->var_name[i]
                            ,val->Loop.endName[i]
                            ,val->var_name[i]
                            ,val->Loop.write.write_func_content[i]);
                } else if(val->Loop.write.TextOrVar[i] == 1) {
                    if(val->Loop.write.type[i] == TOKEN_INTEGER) {
                        fprintf(file,"    for(int %s=%d; %s<%s; %s++) printf(\"%%d\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.endName[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else if(val->Loop.write.type[i] == TOKEN_FLOAT) {
                        fprintf(file,"    for(int %s=%d; %s<%s; %s++) printf(\"%%f\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.endName[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else if(val->Loop.write.type[i] == TOKEN_STRING) {
                        fprintf(file,"    for(int %s=%d; %s<%s; %s++) printf(\"%%s\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.endName[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else if(val->Loop.write.type[i] == TOKEN_BOOL) {
                        fprintf(file,"    for(int %s=%d; %s<%s; %s++) printf(\"%%s\\n\", %s ? \"true\" : \"false\");\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i] 
                                ,val->Loop.endName[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]);
                    } else {
                        fprintf(file,"    for(int %s=%d; %s<%s; %s++) printf(\"%%d\\n\",%s);\n"
                                ,val->var_name[i]
                                ,val->Values.int_value[i]
                                ,val->var_name[i]
                                ,val->Loop.endName[i]
                                ,val->var_name[i]
                                ,val->Loop.write.var_name[i]); 
                    }
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

    Values val= {.counter=0};
    FILE *input = fopen(argv[1],"r");
    if(!input) {
        printf("Error: Failed to open the file\n");
        return 1;
    }

    FILE *file = fopen("output.c","w");
    fprintf(file,"#include <stdio.h>\n#include <stdbool.h>\nint main(void) {\n");

    fseek(input,0,SEEK_END);
    int size = ftell(input);
    rewind(input);

    char buffer[size];
    fread(buffer,1,size,input);
    buffer[size] = '\0';

    Lexer lexer = lexer_init(buffer);
    while(peek(&lexer) != '\0') {
        if(parseTokens(&val,&lexer)==-1) {
            remove("output.c");
            return 1;
        }
    }

    fclose(input);
    codeGen(file,&val);

    fprintf(file,"    return 0;\n}\n");
    fclose(file);

    system("gcc output.c -o main");
    remove("output.c");

    return 0;
}
