I was starting to creating my own programing language
and till now i made the compiler that tokenizes
the file input and does syntax check to generate the C code on an **output.c** file.

---

## Features
it can compile variable creation
in a new language with diffrent syntax
end has a build in printf function called **write()**.
end one called **getInput()** which is for basic I/O.

### syntax:
```text
let <var_name>: <var_type> = <value>;
```

---
### examples:
```bash
let number: integer = 45;
let name: string = "michael";
let isGood: boolean = true;

write("Hello!!\n");
write(number);
write("How are you!\n");

number = getInput("Enter a number: ");
name = getInput("Enter your name: ");

# OR
let newVar1: integer = getInput("Enter a number: ");
let newVar2: string = getInput("Enter your name: ");
...
```
---

## Types:
```bash
integer
string
float
boolean
```
---

USE the compiler:
1. compile the compiler:
```bash
make
```
2. use the compiler:
```bash
./compile filename
```
---
I will add more in the future.
