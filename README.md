I was starting to creating my own programing language
and till now i made the compiler that tokenizes
the file input and does syntax check to generate the C code on an **output.c** file.

---

## Features
it can compile variable creation
in a new language with diffrent syntax
end has a build in printf function called **write()**.

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

write("Hello!!");
write("How are you!");
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
gcc compiler.c -o main
```
2. use the compiler:
```bash
./main filename
```

---

## COMPILE THE PROGRAM
```bash
make  # This will be create an executable file with name -> 'main'
```

---
I will add more in the future.
