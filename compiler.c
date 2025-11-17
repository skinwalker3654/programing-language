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
    LOOP,
    TO,
    EOF_T,
} TokenType;

typedef struct Token {
    TokenType type;
    char name[100];
} Token;

Token getNextToken(char **input) {
    while(isspace(**input)) (*input)++;
    if(**input == '\0') return (Token){EOF_T,""};

    if(**input == '#') while(**input != '\0') (*input)++;
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
        } else if(strcmp(token.name,"loop")==0) {
            token.type = LOOP;
            return token;
        } else if(strcmp(token.name,"to")==0) {
            token.type = TO;
            return token;
        } else {
            token.type = VAR_NAME;
            return token;
        }
    }

    if(isdigit(**input)||(**input == '-' && isdigit((*input)[1]))) {
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
        Write write; 
    } Loop;
    int counter;
} Values;

int parseWriteFuncVar(Values *ptr,Token token,char **input,int writeOrloop) {
    if(writeOrloop == 0) {
        int foundIdx = -1;
        int index = -1;
        for(int i=0; i<=ptr->counter; i++) {
            if(strcmp(ptr->var_name[i],token.name)==0) {
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
    
            strcpy(ptr->Write.write.var_name[ptr->counter],token.name);
            ptr->Write.write.TextOrVar[ptr->counter] = 1;
    
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
    } else if(writeOrloop == 1) {
        int foundIdx = -1;
        int index = -1;
        for(int i=0; i<=ptr->counter; i++) {
            if(strcmp(ptr->var_name[i],token.name)==0) {
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
    
            strcpy(ptr->Loop.write.var_name[ptr->counter],token.name);
            ptr->Loop.write.TextOrVar[ptr->counter] = 1;

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

    return -1;
}

int parseWriteFunc(Values *ptr,char **input,int writeOrloop) {
    if(writeOrloop == 0) {
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
            int check = parseWriteFuncVar(ptr,token,input,0);
            if(check == -1) { return -1; }
            return 0;
        }

        ptr->Write.write.TextOrVar[ptr->counter] = 0;
        strcpy(ptr->Write.write.write_func_content[ptr->counter],token.name);
    
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
    } else if(writeOrloop == 1) {
        ptr->type[ptr->counter] = LOOP;
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
            int check = parseWriteFuncVar(ptr,token,input,1);
            if(check == -1) { return -1; }
            return 0;
        }

        ptr->Loop.write.TextOrVar[ptr->counter] = 0;
        strcpy(ptr->Loop.write.write_func_content[ptr->counter],token.name);
    
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

    return -1;
}

int parseAssignGetInput(Values *val,Token token,char **input) {
    int indexIdx = -1;
    int foundIdx = 0;
    for(int i=0; i<val->counter; i++) {
        if(strcmp(token.name,val->var_name[i])==0) {
            foundIdx = 1;
            indexIdx = i;
            break;
        }
    }

    if(!foundIdx) {
        printf("Error: Values '%s' not found\n",token.name);
        return -1;
    }
        
    if(val->type[indexIdx] == BOOL) {
        printf("Error: Cannot take input on booleans\n");
        return -1;
    }

    val->GetInput.variableType[val->counter] = val->type[indexIdx];
    strcpy(val->GetInput.var_name_to_get_input[val->counter],token.name);

    token = getNextToken(input);
    if(token.type != EQUAL) {
        printf("Error: Forgot to assing the value -> '='\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != GETINPUT) {
        printf("Error: Invalid valiable\n");
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

    strcpy(val->GetInput.content[val->counter],token.name);

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

    val->type[val->counter] = GETINPUT;
    val->counter++;

    return 0;
}

int parseGetInput(Values *val,Token token,char **input,char *tempVarName,int tempType) {
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
    if(tempType == BOOL) {
        printf("Error: Cannot take input on booleans\n");
        return -1;
    }

    val->GetInput.variableType[val->counter] = tempType;
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
        
    strcpy(val->GetInput.content[val->counter],token.name);
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

    val->type[val->counter] = GETINPUT;
    val->counter++;

    return 0;
}

int parseTokens(Values *val,char **input) {
    Token token = getNextToken(input);
    if(token.type == EOF_T) return 0;

    if(token.type == LOOP) {
        token = getNextToken(input);
        if(token.type != VAR_NAME) {
            printf("Error: Invalid value '%s'\n",token.name);
            return -1;
        }

        strcpy(val->var_name[val->counter],token.name);
        token = getNextToken(input);
        if(token.type != EQUAL) {
            printf("Error: Forgot to assign the '='\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != VALUE) {
            printf("Error: Invalid value '%s'\n",token.name);
            return -1;
        }

        char *endPtr;
        int number = strtol(token.name,&endPtr,10);
        if(*endPtr != '\0') {
            printf("Error: Invalid value '%s'\n",token.name);
            return -1;
        }
        
        val->Values.int_value[val->counter] = number;
        token = getNextToken(input);
        if(token.type != TO) {
            printf("Error: Forgot to assign 'to'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != VALUE) {
            printf("Error: Invalid value '%s'\n",token.name);
            return -1;
        }

        char *endPtr2;
        int number2 = strtol(token.name,&endPtr2,10);
        if(*endPtr2 != '\0') {
            printf("Error: Invalid value '%s'\n",token.name);
            return -1;
        }
        
        val->Loop.end[val->counter] = number2;
        token = getNextToken(input);
        if(token.type != COLON_CHAR) {
            printf("Error: Forgot to assign ':'\n");
            return -1;
        }

        token = getNextToken(input);
        if(token.type != WRITE) {
            printf("Error: Only write() is allowed for loops\n");
            return -1;
        }

        int check = parseWriteFunc(val,input,1);
        if(check == -1) { return -1;}
        return 0;
    }

    if(token.type == VAR_NAME) {
        int check = parseAssignGetInput(val,token,input);
        if(check == -1) { return -1; }
        return 0;
    }

    if(token.type == WRITE) {
        if(parseWriteFunc(val,input,0)==-1) return -1;
        return 0;
    }

    if(token.type != LET_KEYWORD && token.type != WRITE) {
        printf("Error: Forgot to assing the valiable using 'let'\n");
        return -1;
    } 

    token = getNextToken(input);
    if(token.type != VAR_NAME) {
        printf("Error: Invalid valiable name '%s'\n",token.name);
        return -1;
    } 

    char tempVarName[40];
    strcpy(tempVarName,token.name);
    strcpy(val->var_name[val->counter],token.name);

    token = getNextToken(input);
    if(token.type != COLON_CHAR) {
        printf("Error: Forgot to assing ':' for valiable values\n");
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
    val->type[val->counter] = token.type;

    token = getNextToken(input);
    if(token.type != EQUAL) {
        printf("Error: Forgot to assing the valiable with '='\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type == GETINPUT) {
        int check = parseGetInput(val,token,input,tempVarName,tempType);
        if(check == -1) { return -1; }
        return 0;
    }
    
    if(token.type != VALUE && token.type != STRINGVALUE && token.type != FALSE_ && token.type != TRUE_) {
        printf("Error: Invalid value '%s'\n",token.name);
        return -1;
    }

    if(val->type[val->counter] == INTEGER) {
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

        val->Values.int_value[val->counter] = number;
    } else if(val->type[val->counter] == TEXT) {
        if(token.type != STRINGVALUE) {
            printf("Error: Strings must be inside \"\"\n");
            return -1;
        }

        strcpy(val->Values.stringValue[val->counter],token.name);
    } else if(val->type[val->counter] == FLOAT) {
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

        val->Values.float_value[val->counter] = number;
    } else if(val->type[val->counter] == BOOL) {
        if(token.type == TRUE_) strcpy(val->Values.boolean[val->counter],"true");
        else if(token.type == FALSE_) strcpy(val->Values.boolean[val->counter],"false");
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

    val->GetInput.AssignOrNot[val->counter] = 0;
    val->counter++;

    return 0;
}

void codeGen(FILE *file,Values *val) {
    for(int i=0; i<=val->counter; i++) {
        if(val->type[i] == INTEGER) {
            fprintf(file,"    int %s = %d;\n",val->var_name[i],val->Values.int_value[i]);
        } else if(val->type[i] == TEXT) {
            fprintf(file,"    char %s[256] = \"%s\";\n",val->var_name[i],val->Values.stringValue[i]);
        } else if(val->type[i] == FLOAT) {
            fprintf(file,"    float %s = %f;\n",val->var_name[i],val->Values.float_value[i]);
        } else if(val->type[i] == BOOL) {
            fprintf(file,"    bool %s = %s;\n",val->var_name[i],val->Values.boolean[i]);
        } else if(val->type[i] == WRITE) {
            if(val->Write.write.TextOrVar[i] == 0) {
                fprintf(file,"    printf(\"%s\");\n",val->Write.write.write_func_content[i]);
            } else if(val->Write.write.TextOrVar[i] == 1){
                if(val->Write.write.type[i] == INTEGER) {
                    fprintf(file,"    printf(\"%%d\\n\",%s);\n",val->Write.write.var_name[i]);
                } else if(val->Write.write.type[i] == FLOAT) {
                    fprintf(file,"    printf(\"%%f\\n\",%s);\n",val->Write.write.var_name[i]);
                } else if(val->Write.write.type[i] == TEXT) {
                    fprintf(file,"    printf(\"%%s\\n\",%s);\n",val->Write.write.var_name[i]);
                } else if(val->Write.write.type[i] == BOOL) {
                    fprintf(file,"    printf(\"%%s\\n\", %s ? \"true\" : \"false\");\n", val->Write.write.var_name[i]);
                }
            }
        } else if(val->type[i] == GETINPUT) {
            if(val->GetInput.AssignOrNot[i] == 0) {
                fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                if(val->GetInput.variableType[i] == INTEGER) {
                    fprintf(file,"    scanf(\"%%d\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == TEXT) {
                    fprintf(file,"    scanf(\"%%s\",%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == FLOAT) {
                    fprintf(file,"    scanf(\"%%f\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                }  
            } else {
                if(val->GetInput.variableType[i] == INTEGER) {
                    fprintf(file,"    int %s;\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%d\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == TEXT) {
                    fprintf(file,"    char %s[256];\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%s\",%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\\n'&&ch%d!=EOF);\n",i,i);
                } else if(val->GetInput.variableType[i] == FLOAT) {
                    fprintf(file,"    float %s;\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    printf(\"%s\");\n",val->GetInput.content[i]);
                    fprintf(file,"    scanf(\"%%f\",&%s);\n",val->GetInput.var_name_to_get_input[i]);
                    fprintf(file,"    int ch%d;\n",i);
                    fprintf(file,"    while((ch%d=getchar())!='\n'&&ch%d!=EOF);\n",i,i);

                } 
            }
        } else if(val->type[i] == LOOP) {
            if(val->Loop.write.TextOrVar[i]==0) {
                fprintf(file,"    for(int %s=%d; %s<=%d; %s++) printf(\"%s\");\n"
                        ,val->var_name[i]
                        ,val->Values.int_value[i]
                        ,val->var_name[i]
                        ,val->Loop.end[i]
                        ,val->var_name[i]
                        ,val->Loop.write.write_func_content[i]);
            } else if(val->Loop.write.TextOrVar[i] == 1) {
                if(val->Loop.write.type[i] == INTEGER) {
                    fprintf(file,"    for(int %s=%d; %s<=%d; %s++) printf(\"%%d\\n\",%s);\n"
                            ,val->var_name[i]
                            ,val->Values.int_value[i]
                            ,val->var_name[i]
                            ,val->Loop.end[i]
                            ,val->var_name[i]
                            ,val->Loop.write.var_name[i]);
                } else if(val->Loop.write.type[i] == FLOAT) {
                    fprintf(file,"    for(int %s=%d; %s<=%d; %s++) printf(\"%%f\\n\",%s);\n"
                            ,val->var_name[i]
                            ,val->Values.int_value[i]
                            ,val->var_name[i]
                            ,val->Loop.end[i]
                            ,val->var_name[i]
                            ,val->Loop.write.var_name[i]);
                } else if(val->Loop.write.type[i] == TEXT) {
                    fprintf(file,"    for(int %s=%d; %s<=%d; %s++) printf(\"%%s\\n\",%s);\n"
                            ,val->var_name[i]
                            ,val->Values.int_value[i]
                            ,val->var_name[i]
                            ,val->Loop.end[i]
                            ,val->var_name[i]
                            ,val->Loop.write.var_name[i]);
                } else if(val->Loop.write.type[i] == BOOL) {
                    fprintf(file,"    for(int %s=%d; %s<=%d; %s++) printf(\"%%s\\n\", %s ? \"true\" : \"false\");\n"
                            ,val->var_name[i]
                            ,val->Values.int_value[i]
                            ,val->var_name[i]
                            ,val->Loop.end[i]
                            ,val->var_name[i]
                            ,val->Loop.write.var_name[i]);
                } else {
                    fprintf(file,"    for(int %s=%d; %s<=%d; %s++) printf(\"%%d\\n\",%s);\n"
                            ,val->var_name[i]
                            ,val->Values.int_value[i]
                            ,val->var_name[i]
                            ,val->Loop.end[i]
                            ,val->var_name[i]
                            ,val->Loop.write.var_name[i]);
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

    char line[256];
    while(fgets(line,sizeof(line),input)) {
        char *ptr = line;
        int check = parseTokens(&val,&ptr);
        if(check == -1) { remove("output.c"); return -1; }
    }

    fclose(input);
    codeGen(file,&val);

    fprintf(file,"    return 0;\n}\n");
    fclose(file);

    system("gcc output.c -o main");
    system("rm output.c");

    return 0;
}
