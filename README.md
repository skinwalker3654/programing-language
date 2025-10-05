I was starting to creating my own language
and till now i made the compiler that tokenizes
the file input and does syntax check.

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

## Type:
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

I will add more in the future.
