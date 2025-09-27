#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    VARIABLE_VALUE,
    VARIABLE_NAME,
    INTEGER_KEYWORD,
    STRING_KEYWORD,
    LET_KEYWORD,
    COLON_CHARACTER,
    SEMICOLON_CHARACTER,
    ASSIGN_CHARACTER,
} TokenType;

#define NAME_SIZE 100
typedef struct Token {
    TokenType type;
    char name[NAME_SIZE];
} Token;

#define VALUE_SIZE 50
typedef struct Variable {
    TokenType type;
    char name[NAME_SIZE];
    char value[VALUE_SIZE];
} Variable;

Token getToken(char **input) {
    while(**input == ' ') (*input)++;

    if(isalpha(**input)) {
        Token token;
        int counter = 0;

        while(isalnum(**input)) {
            token.name[counter] = **input;
            (*input)++;
            counter++;
        }

        token.name[counter] = '\0';
        if(strcmp(token.name,"let")==0) {
            token.type = LET_KEYWORD;
            return token;
        } else if(strcmp(token.name,"integer")==0) {
            token.type = INTEGER_KEYWORD;
            return token;
        } else if(strcmp(token.name,"string")==0) {
            token.type = STRING_KEYWORD;
            return token;
        } else {
            token.type = VARIABLE_NAME;
            return token;
        }
    }

    if(isdigit(**input)) {
        Token token = {VARIABLE_VALUE,""};
        int counter = 0;
        while(isdigit(**input)) {
            token.name[counter] = **input;
            (*input)++;
            counter++;
        }

        token.name[counter] = '\0';
        return token;
    }

    if(**input == '"') {
        Token token = {VARIABLE_VALUE,""};
        int counter = 0;

        (*input)++;
        while(**input != '"' && **input != '\0') {
            token.name[counter] = **input;
            (*input)++;
            counter++;
        }

        (*input)++;
        token.name[counter] = '\0';

        return token;
    }

    char operation = *(*input)++;
    switch(operation) {
        case '=': return (Token){ASSIGN_CHARACTER,"="};
        case ':': return (Token){COLON_CHARACTER,":"};
    }

    return (Token){SEMICOLON_CHARACTER,""};
}

Variable parseTokens(char **input) {
    Variable var = {0,"",""};

    Token token = getToken(input);
    if(token.type != LET_KEYWORD) {
        printf("Error: Invalid keyword -> '%s'\n",token.name);
        return var;
    }

    token = getToken(input);
    if(token.type != VARIABLE_NAME) {
        printf("Error: Invalid variable name -> '%s'\n",token.name);
        return var;
    }

    strcpy(var.name,token.name);
    token = getToken(input);
    if(token.type != COLON_CHARACTER) {
        printf("Error: ':' <- is missing\n");
        return var;
    }

    token = getToken(input);
    if(token.type != INTEGER_KEYWORD && token.type != STRING_KEYWORD) {
        printf("Error: Invalid variable type -> '%s'\n",token.name);
        return var;
    } else {
        if(token.type == INTEGER_KEYWORD) {
            var.type = INTEGER_KEYWORD;
        } else {
            var.type = STRING_KEYWORD;
        }
    }

    token = getToken(input);
    if(token.type != ASSIGN_CHARACTER) {
        printf("Error: '=' <- is missign\n");
        return var;
    }

    token = getToken(input);
    if(token.type != VARIABLE_VALUE) {
        printf("Error: Invalid value -> '%s'\n",token.name);
        return var;
    }

    strcpy(var.value,token.name);
    token = getToken(input);
    if(token.type != SEMICOLON_CHARACTER) {
        printf("Error: Forgot the '%s' at the end\n",token.name);
        return var;
    }

    return var;
}

int main(int argc,char **argv) {
    if(argc != 2) {
        printf("Error: Invalid usage\n");
        printf("Correct usage: ./main <file_name>\n");
        return 1;
    }

    char line[NAME_SIZE];
    FILE *file1 = fopen(argv[1],"r");
    if(file1 == NULL) {
        printf("Error: Failed to open the file\n");
        return 1;
    }

    
    FILE *file2 = fopen("out.c","w");
    if(!file2) {
        printf("Error: Failed to open the file\n");
        fclose(file1);
        return 1;
    }

    fprintf(file2,"#include <stdio.h>\n");
    fprintf(file2,"\nint main(void) {\n");

    while(fgets(line,sizeof(line),file1)) {
        char *ptr = line;
        Variable var = parseTokens(&ptr);

        if(var.type == INTEGER_KEYWORD) {
            fprintf(file2,"    int %s = %s;\n",var.name,var.value);
        } else if(var.type == STRING_KEYWORD) {
            fprintf(file2,"    char *%s = \"%s\";\n",var.name,var.value);
        }
    }

    fprintf(file2,"    return 0;\n");
    fprintf(file2,"}\n");

    fclose(file1);
    fclose(file2);

    return 0;
}
