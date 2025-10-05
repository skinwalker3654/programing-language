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
    EOF_T,
} TokenType;

typedef struct Token {
    TokenType type;
    char name[100];
} Token;

Token getNextToken(char **input) {
    while(isspace(**input)) (*input)++;

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
        (*input)++; 
        return token;
    }

    char operation = *(*input)++;
    switch(operation) {
        case ':': return (Token){COLON_CHAR,":"}; 
        case '=': return (Token){EQUAL,"="};
        case ';': return (Token){SEMICOLON,";"};
    }

    return (Token){EOF_T,""};
}

typedef struct Variable {
    TokenType type[100];
    char var_name[100][40];
    int int_value[100];
    float float_value[100];
    char stringValue[100][40];
    char boolean[100][20];
    int counter;
} Variable;

int parseTokens(Variable *var,char **input) {
    Token token = getNextToken(input);
    if(token.type != LET_KEYWORD) {
        printf("Error: Forgot to assing the variable using 'let'\n");
        return -1;
    }

    token = getNextToken(input);
    if(token.type != VAR_NAME) {
        printf("Error: Invalid variable name '%s'\n",token.name);
        return -1;
    }

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

    var->type[var->counter] = token.type;

    token = getNextToken(input);
    if(token.type != EQUAL) {
        printf("Error: Forgot to assing the variable with '='\n");
        return -1;
    }

    token = getNextToken(input);
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

        var->int_value[var->counter] = number;
    } else if(var->type[var->counter] == TEXT) {
        if(token.type != STRINGVALUE) {
            printf("Error: Strings must be inside \"\"\n");
            return -1;
        }

        strcpy(var->stringValue[var->counter],token.name);
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

        var->float_value[var->counter] = number;
    } else if(token.type == FALSE_) {
        strcpy(var->boolean[var->counter],"false");
    } else if(token.type == TRUE_) {
        strcpy(var->boolean[var->counter],"true");
    } else {
        printf("Error: Booleans cannot be inside \"\"\n");
        return -1;
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

    var->counter++;
    return 0;
}

void codeGen(FILE *file,Variable *var) {
    for(int i=0; i<var->counter; i++) {
        if(var->type[i] == INTEGER) {
            fprintf(file,"    int %s = %d;\n",var->var_name[i],var->int_value[i]);
        } else if(var->type[i] == TEXT) {
            fprintf(file,"    char *%s = \"%s\";\n",var->var_name[i],var->stringValue[i]);
        } else if(var->type[i] == FLOAT) {
            fprintf(file,"    float %s = %f;\n",var->var_name[i],var->float_value[i]);
        } else if(strcmp(var->boolean[i],"true")==0 || strcmp(var->boolean[i],"false")==0) {
            fprintf(file,"    bool %s = %s;\n",var->var_name[i],var->boolean[i]);
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

    char line[100];
    while(fgets(line,sizeof(line),input)) {
        char *ptr = line;
        int check = parseTokens(&var,&ptr);
        if(check == -1) { remove("output.c"); return 1; }
    }

    codeGen(file,&var);

    fprintf(file,"    return 0;\n}\n");
    fclose(file);

    return 0;
}
