# Lua Parser (written in C++)

[![Build](https://img.shields.io/badge/build-passing-brightgreen)]()  
[![Language](https://img.shields.io/badge/language-C++17-blue)]()  
[![License](https://img.shields.io/badge/license-MIT-lightgrey)]()  

A **Lua parser and lexer** written in modern C++.  
It takes a Lua source file, breaks it into tokens, builds an **AST (Abstract Syntax Tree)**, and prints everything as **JSON**.  
It also has a built-in **benchmark mode** to test lexer + parser performance.

---

## 📖 Table of Contents
- [Features](#-features)
- [How to Run](#-how-to-run)
- [Benchmark Mode](#-benchmark-mode)
- [Examples](#-examples)
- [About the Code](#-about-the-code)
- [Limitations](#-limitations)
- [Future Plans](#-future-plans)
- [Contributing](#-contributing)
- [License](#-license)

---

## ✨ Features
- **Lexer** → splits Lua code into tokens (numbers, strings, keywords, etc.)
- **Parser** → builds an AST from those tokens
- **AST with named slots** → nodes have meaningful keys like `"variables"`, `"values"`, `"body"`
- **JSON output** → easy to visualize or consume in other tools
- **File input** → drag + drop a file onto the exe, or run it from terminal
- **Benchmark mode** → stress-test lexer + parser on repeated input

---

## 🚀 How to Run

### Compile
```bash
g++ -std=c++17 -O2 -o lua_parser "Lua Parser.cpp"
````

### Run (normal mode)

```bash
./lua_parser script.lua
```

It will print the AST in JSON to your terminal.

---

## ⚡ Benchmark Mode

You can run the parser in **benchmark mode** to measure performance:

```bash
./lua_parser
Enter path to source file: benchmark
[Benchmark] Enter path to source file: myscript.lua
[Benchmark] How many times to repeat the code? (default = 50):
[Benchmark] How many runs to perform? (default = 20000):
```

* The file is read and **repeated N times** (default 50) to simulate a larger program.
* The parser then runs **M iterations** (default 20,000) of `Lexer + Parse`.
* The total time and **average lex+parse time** per iteration are reported.

This is useful for comparing performance against other Lua parsers.

---

## 📝 Examples

### Example 1

Input Lua:

```lua
local x = 42
```

Output JSON:

```json
[
  {
    "nodeType": "LocalStatement",
    "text": "local",
    "line": 1,
    "children": {
      "variables": [
        { "nodeType": "Identifier", "text": "x", "line": 1, "children": {} }
      ],
      "values": [
        { "nodeType": "NumericLiteral", "text": "42", "line": 1, "children": {} }
      ]
    }
  }
]
```

---

### Example 2

Input Lua:

```lua
function greet(name)
  print("Hello " .. name)
end
```

Output JSON (shortened for clarity):

```json
[
  {
    "nodeType": "FunctionDeclaration",
    "text": "function",
    "line": 1,
    "children": {
      "identifier": { "nodeType": "Identifier", "text": "greet", "line": 1, "children": {} },
      "parameters": [
        { "nodeType": "Identifier", "text": "name", "line": 1, "children": {} }
      ],
      "body": [
        {
          "nodeType": "CallStatement",
          "text": "print",
          "line": 2,
          "children": {
            "arguments": [
              {
                "nodeType": "BinaryExpression",
                "text": "..",
                "line": 2,
                "children": {
                  "left": { "nodeType": "StringLiteral", "text": "\"Hello \"", "line": 2, "children": {} },
                  "right": { "nodeType": "Identifier", "text": "name", "line": 2, "children": {} }
                }
              }
            ]
          }
        }
      ]
    }
  }
]
```

---

## 🧑‍💻 About the Code

I used **AI assistance (ChatGPT)** to speed up development (e.g., reducing string copies, simplifying loops).
The **core design and logic are mine**, and I reviewed/refined everything to ensure it works properly.

---

## ⚠️ Limitations

* Not full Lua coverage yet (some grammar not implemented)
* Basic error handling

---

## 🛠 Future Plans

* Expand Lua grammar coverage
* Better error reporting
* Options for JSON formatting (pretty/compact)
* More benchmarking against other Lua parsers

---

## 🤝 Contributing

Contributions are welcome!

* Open an issue for bugs/feature requests
* Submit pull requests with improvements
* Add tests for new features where possible

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).
