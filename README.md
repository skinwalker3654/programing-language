I was starting to creating my own programing language
and till now i made the compiler that tokenizes
the file input and does syntax check to generate the C code on an **output.c** file.

---

## Features
it can compile variable creation
in a new language with diffrent syntax

### syntax:
```text
let <var_name>: <var_type> = <value>;
```

---

### example1 = let number: integer = 45;
### example2 = let name: string = "skinwalker";
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
gcc compiler.c -o main
```
2. use the compiler:
```bash
./main filename
```

---

## COMPILE THE PROGRAM
```bash
make  # This will be create an executable file with name 'main'
```

---
I will add more in the future.
