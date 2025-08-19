# Lua Parser (written in C++)

[![Build](https://img.shields.io/badge/build-passing-brightgreen)]()  
[![Language](https://img.shields.io/badge/language-C++17-blue)]()  
[![License](https://img.shields.io/badge/license-MIT-lightgrey)]()  

A **Lua parser and lexer** written in modern C++.  
It takes a Lua source file, breaks it into tokens, builds an **AST (Abstract Syntax Tree)**, and prints everything as **JSON**.

---

## 📖 Table of Contents
- [Features](#-features)
- [How to Run](#-how-to-run)
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

---

## 🚀 How to Run

### Compile
```bash
g++ -std=c++17 -O2 -o lua_parser "Lua Parser.cpp"
````

### Run

```bash
./lua_parser script.lua
```

It will print the AST in JSON to your terminal.

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
* Benchmarking with larger Lua projects

---

## 🤝 Contributing

Contributions are welcome!

* Open an issue for bugs/feature requests
* Submit pull requests with improvements
* Add tests for new features where possible

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).



---

👉 If you want, I can also help you set up:
- A `LICENSE` file (MIT by default)  
- A starter `CMakeLists.txt` so people can build without typing long g++ commands  
- A GitHub Actions workflow for auto-build  
