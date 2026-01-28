# ysh — Minimal Unix Shell (C)

`ysh` is a small Unix-like shell written in C as a systems programming project. It implements core shell fundamentals such as process creation, command parsing, and program execution, focusing on low-level Linux/Unix concepts rather than full POSIX compatibility.

This project is intentionally minimal and educational, designed to demonstrate how shells work internally at the OS level.

---

## Features

- Execution of external programs (`fork()` + `exec`)
- Built-in commands:
  - `cd`
  - `exit`
- Command parsing and tokenization
- Foreground process management
- Basic error handling
- Standard input/output handling

*(Feature set is intentionally minimal and focused on core shell mechanics rather than full POSIX shell compliance.)*

---

## Build & Run

```bash
make
./ysh
```
