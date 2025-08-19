# Lua Parser (written in C++)

This is a **Lua parser and lexer** I’ve been working on in C++.
It takes a Lua source file, breaks it down into tokens, builds an **AST (Abstract Syntax Tree)**, and prints everything out as **JSON**.

---

## What it Does

* **Lexer** → splits Lua code into tokens (like numbers, strings, keywords, etc.)
* **Parser** → builds an AST from those tokens
* **AST with named slots** → instead of just “children,” nodes have meaningful keys like `"variables"`, `"values"`, `"body"`
* **JSON output** → makes it easy to visualize or use in other tools
* **File input** → drag + drop a file onto the exe, or run it from terminal and type the path

---

## How to Run

### Compile:

```bash
g++ -std=c++17 -O2 -o lua_parser "Lua Parser.cpp"
```

### Use:

```bash
./lua_parser script.lua
```

It will print the AST in JSON to your terminal.

---

## Example

If your Lua file has:

```lua
local x = 42
```

The parser prints:

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

## About the Code

I want to be straight up: I used **AI (ChatGPT)** to help rewrite parts of this.

* The AI helped me **speed things up** (like reducing string copies, pre-allocating vectors, simplifying loops).
* It also helped **refactor the AST** to use named slots instead of just "children."
* The core idea and logic are mine, but AI suggestions made it **faster and cleaner**.
* I went through and **checked everything manually** so it actually works.

---

## Limitations

* It doesn't handle 100% of Lua yet (some grammar stuff is missing).
* Error handling is very basic.

---

## Future Plans

* Add more Lua grammar coverage
* Better error messages
* Options for different JSON formats (pretty/compact)
* Benchmarking with larger Lua projects
