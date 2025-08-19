# Lua Parser (C++)

This project is a **Lua lexer and parser** implemented in modern C++.
It reads a `.lua` source file, tokenizes it, parses it into an **Abstract Syntax Tree (AST)**, and prints the AST in **JSON format**.

---

## ✨ Features

* **Lexer**: Converts Lua source code into tokens (identifiers, keywords, numbers, strings, operators, etc.).
* **Parser**: Builds a structured AST from tokens.
* **Named Slots**: AST nodes organize children in semantically meaningful groups (e.g., `"variables"`, `"values"`, `"body"`).
* **JSON Output**: AST can be exported to JSON for further tooling, debugging, or visualization.
* **File Input**:

  * Drag & drop a Lua file onto the executable, **or**
  * Run the program and type the file path.

---

## 🚀 Usage

### Compile

```bash
g++ -std=c++17 -O2 -o lua_parser "Lua Parser.cpp"
```

### Run

```bash
./lua_parser myscript.lua
```

You’ll see the AST printed as JSON to stdout.

---

## 🔧 Example

If `myscript.lua` contains:

```lua
local x = 42
```

The output will be something like:

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

## ⚡ Performance Notes

This parser was **refactored and optimized with the assistance of AI** (ChatGPT).
The AI was specifically used to:

* Suggest **speed improvements** (e.g., memory reservations, avoiding redundant string copies, simplifying parsing loops).
* Refactor the AST structure for **named children slots** instead of generic lists.
* Help reorganize parsing logic for clarity and efficiency.

### Transparency

* The **original design and logic** were human-authored.
* **AI rewrites** were applied mainly to improve **speed, maintainability, and readability**.
* Manual review and testing were performed to validate changes.

---

## 📚 Limitations

* This is not a full production-grade Lua parser (e.g., some complex grammar cases may not be covered).
* Error handling is minimal.
* AST output is focused on structure, not full fidelity with Lua semantics.

---

## 🛠 Future Improvements

* More complete grammar support (loops, function scopes, metatables).
* Better error recovery in the parser.
* CLI options for output format (pretty JSON, compact JSON).
* Benchmarks against larger Lua codebases.
