#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace std;
using sv = string_view;

// ---------------- Types ----------------
enum class TokenType {
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    COMMA,
    DOT,
    SEMICOLON,
    COLON,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    CARET,
    HASH,
    DOT_DOT,
    DOT_DOT_DOT,
    EQUAL,
    EQUAL_EQUAL,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    IDENTIFIER,
    NUMBER,
    STRING,
    AND,
    BREAK,
    DO,
    ELSE,
    ELSEIF,
    END,
    FALSE_,
    FOR,
    FUNCTION,
    GOTO,
    IF,
    IN,
    LOCAL,
    NIL,
    NOT,
    OR,
    REPEAT,
    RETURN,
    THEN,
    TRUE_,
    UNTIL,
    WHILE,
    END_OF_FILE
};

struct Token {
    TokenType type;
    sv text;  // points into original source
    int line;
};

enum class ASTType {
    // statements / top-level
    Chunk,
    Block,
    LocalStatement,
    AssignmentStatement,
    FunctionDeclaration,
    FunctionExpression,
    IfStatement,
    IfClause,
    ElseifClause,
    ElseClause,
    WhileStatement,
    RepeatStatement,
    ForNumericStatement,
    ForGenericStatement,
    ReturnStatement,
    BreakStatement,
    DoStatement,
    GotoStatement,
    LabelStatement,
    CallStatement,

    // expressions
    BinaryExpression,
    UnaryExpression,
    LogicalExpression,
    CallExpression,
    IndexExpression,
    MemberExpression,
    TableConstructorExpression,
    TableValue,
    TableKey,
    TableKeyString,

    // leaves
    Identifier,
    NumericLiteral,
    StringLiteral,
    BooleanLiteral,
    NilLiteral,
    VarargLiteral,

    // misc
    VariableAttribute
};

struct AST {
    ASTType type;
    std::string text;
    int line;

    // named slots -> lists of child nodes
    std::unordered_map<std::string, std::vector<AST>> children;
};

// ---------------- Helpers ----------------
static inline char hexDigit(unsigned v) noexcept {
    static const char* H = "0123456789ABCDEF";
    return H[v & 0xF];
}

static inline bool is_alpha(char c) noexcept {
    return (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
static inline bool is_digit(char c) noexcept { return (c >= '0' && c <= '9'); }
static inline bool is_alnum(char c) noexcept {
    return is_alpha(c) || is_digit(c);
}

static inline TokenType keywordTypeFast(const sv& w) noexcept {
    if (w.empty()) return TokenType::IDENTIFIER;
    char c = w[0];
    switch (c) {
        case 'a':
            if (w == "and") return TokenType::AND;
            break;
        case 'b':
            if (w == "break") return TokenType::BREAK;
            break;
        case 'd':
            if (w == "do") return TokenType::DO;
            break;
        case 'e':
            if (w == "else") return TokenType::ELSE;
            if (w == "elseif") return TokenType::ELSEIF;
            if (w == "end") return TokenType::END;
            break;
        case 'f':
            if (w == "false") return TokenType::FALSE_;
            if (w == "for") return TokenType::FOR;
            if (w == "function") return TokenType::FUNCTION;
            break;
        case 'g':
            if (w == "goto") return TokenType::GOTO;
            break;
        case 'i':
            if (w == "if") return TokenType::IF;
            if (w == "in") return TokenType::IN;
            break;
        case 'l':
            if (w == "local") return TokenType::LOCAL;
            break;
        case 'n':
            if (w == "nil") return TokenType::NIL;
            if (w == "not") return TokenType::NOT;
            break;
        case 'o':
            if (w == "or") return TokenType::OR;
            break;
        case 'r':
            if (w == "repeat") return TokenType::REPEAT;
            if (w == "return") return TokenType::RETURN;
            break;
        case 't':
            if (w == "then") return TokenType::THEN;
            if (w == "true") return TokenType::TRUE_;
            break;
        case 'u':
            if (w == "until") return TokenType::UNTIL;
            break;
        case 'w':
            if (w == "while") return TokenType::WHILE;
            break;
        default:
            break;
    }
    return TokenType::IDENTIFIER;
}

// ---------------- Lexer ----------------
vector<Token> Lexer(const string& Code) {
    const char* data = Code.data();
    size_t Len = Code.size();
    size_t idx = 0;
    int line = 1;

    vector<Token> tokens;
    tokens.reserve(min<size_t>(512, max<size_t>(16, Len / 8)));

    auto peek = [&](size_t off = 0) -> char {
        size_t p = idx + off;
        return (p < Len) ? data[p] : '\0';
    };
    auto pushTok = [&](TokenType ttype, size_t start, size_t length) {
        tokens.push_back(Token{ttype, sv(data + start, length), line});
    };

    while (idx < Len) {
        char c = data[idx];

        if (c == '\n') {
            ++line;
            ++idx;
            continue;
        }
        if ((unsigned char)c <= 0x20) {
            ++idx;
            continue;
        }  // skip other controls & whitespace quickly

        switch (c) {
            case '(':
                pushTok(TokenType::LEFT_PAREN, idx, 1);
                ++idx;
                continue;
            case ')':
                pushTok(TokenType::RIGHT_PAREN, idx, 1);
                ++idx;
                continue;
            case '{':
                pushTok(TokenType::LEFT_BRACE, idx, 1);
                ++idx;
                continue;
            case '}':
                pushTok(TokenType::RIGHT_BRACE, idx, 1);
                ++idx;
                continue;
            case '[': {
                if (peek(1) == '[' || peek(1) == '=') {
                    size_t check = idx + 1;
                    int eqs = 0;
                    while (check < Len && data[check] == '=') {
                        ++eqs;
                        ++check;
                    }
                    if (check < Len && data[check] == '[') {
                        bool isComment = (idx >= 2 && data[idx - 2] == '-' &&
                                          data[idx - 1] == '-');
                        idx = check + 1;
                        size_t start = idx;
                        bool closed = false;
                        while (idx < Len) {
                            if (data[idx] == ']') {
                                size_t closing = idx + 1;
                                int eqCount = 0;
                                while (closing < Len && data[closing] == '=') {
                                    ++eqCount;
                                    ++closing;
                                }
                                if (closing < Len && data[closing] == ']' &&
                                    eqCount == eqs) {
                                    if (!isComment)
                                        pushTok(TokenType::STRING, start,
                                                idx - start);
                                    idx = closing + 1;
                                    closed = true;
                                    break;
                                }
                            }
                            if (data[idx] == '\n') ++line;
                            ++idx;
                        }
                        if (!closed) {
                            if (!isComment)
                                pushTok(TokenType::STRING, start, idx - start);
                        }
                        continue;
                    }
                }
                pushTok(TokenType::LEFT_BRACKET, idx, 1);
                ++idx;
                continue;
            }
            case ']':
                pushTok(TokenType::RIGHT_BRACKET, idx, 1);
                ++idx;
                continue;
            case ',':
                pushTok(TokenType::COMMA, idx, 1);
                ++idx;
                continue;
            case ';':
                pushTok(TokenType::SEMICOLON, idx, 1);
                ++idx;
                continue;
            case '+':
                pushTok(TokenType::PLUS, idx, 1);
                ++idx;
                continue;
            case '*':
                pushTok(TokenType::STAR, idx, 1);
                ++idx;
                continue;
            case '/':
                pushTok(TokenType::SLASH, idx, 1);
                ++idx;
                continue;
            case ':':
                pushTok(TokenType::COLON, idx, 1);
                ++idx;
                continue;
            case '%':
                pushTok(TokenType::PERCENT, idx, 1);
                ++idx;
                continue;
            case '^':
                pushTok(TokenType::CARET, idx, 1);
                ++idx;
                continue;
            case '#':
                pushTok(TokenType::HASH, idx, 1);
                ++idx;
                continue;
            case '.': {
                if (peek(1) == '.' && peek(2) == '.') {
                    pushTok(TokenType::DOT_DOT_DOT, idx, 3);
                    idx += 3;
                } else if (peek(1) == '.') {
                    pushTok(TokenType::DOT_DOT, idx, 2);
                    idx += 2;
                } else {
                    pushTok(TokenType::DOT, idx, 1);
                    ++idx;
                }
                continue;
            }
            case '-': {
                if (peek(1) == '-') {
                    idx += 2;
                    if (peek() == '[') continue;
                    while (idx < Len && data[idx] != '\n') ++idx;
                    continue;
                }
                pushTok(TokenType::MINUS, idx, 1);
                ++idx;
                continue;
            }
            case '=': {
                if (peek(1) == '=') {
                    pushTok(TokenType::EQUAL_EQUAL, idx, 2);
                    idx += 2;
                } else {
                    pushTok(TokenType::EQUAL, idx, 1);
                    ++idx;
                }
                continue;
            }
            case '~': {
                if (peek(1) == '=') {
                    pushTok(TokenType::BANG_EQUAL, idx, 2);
                    idx += 2;
                } else {
                    ++idx;
                    continue;
                }
            }
            case '<': {
                if (peek(1) == '=') {
                    pushTok(TokenType::LESS_EQUAL, idx, 2);
                    idx += 2;
                } else {
                    pushTok(TokenType::LESS, idx, 1);
                    ++idx;
                }
                continue;
            }
            case '>': {
                if (peek(1) == '=') {
                    pushTok(TokenType::GREATER_EQUAL, idx, 2);
                    idx += 2;
                } else {
                    pushTok(TokenType::GREATER, idx, 1);
                    ++idx;
                }
                continue;
            }
            case '"':
            case '\'': {
                char q = c;
                size_t start = idx + 1;
                ++idx;
                while (idx < Len && data[idx] != q) {
                    if (data[idx] == '\n') ++line;
                    if (data[idx] == '\\' && idx + 1 < Len)
                        idx += 2;
                    else
                        ++idx;
                }
                pushTok(TokenType::STRING, start,
                        (idx < Len ? idx - start : idx - start));
                if (idx < Len && data[idx] == q) ++idx;
                continue;
            }
            default:
                break;
        }

        // numbers
        if (is_digit(c) || (c == '.' && is_digit(peek(1)))) {
            size_t start = idx;
            if (c == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
                idx += 2;
                while (idx < Len &&
                       (isxdigit((unsigned char)data[idx]) || data[idx] == '.'))
                    ++idx;
                if (idx < Len && (data[idx] == 'p' || data[idx] == 'P')) {
                    ++idx;
                    if (peek() == '+' || peek() == '-') ++idx;
                    while (idx < Len && is_digit(data[idx])) ++idx;
                }
            } else {
                while (idx < Len && is_digit(data[idx])) ++idx;
                if (peek() == '.') {
                    ++idx;
                    while (idx < Len && is_digit(data[idx])) ++idx;
                }
                if (peek() == 'e' || peek() == 'E') {
                    ++idx;
                    if (peek() == '+' || peek() == '-') ++idx;
                    while (idx < Len && is_digit(data[idx])) ++idx;
                }
            }
            pushTok(TokenType::NUMBER, start, idx - start);
            continue;
        }

        // identifiers / keywords
        if (is_alpha(c)) {
            size_t start = idx;
            ++idx;
            while (idx < Len && is_alnum(data[idx])) ++idx;
            sv word(data + start, idx - start);
            TokenType k = keywordTypeFast(word);
            pushTok(k, start, idx - start);
            continue;
        }

        // fallback: skip unknown
        ++idx;
    }

    tokens.push_back(Token{TokenType::END_OF_FILE, sv(), line});
    return tokens;
}

// ---------------- Parser ----------------

static inline int precedenceOf(const Token& t) noexcept {
    switch (t.type) {
        case TokenType::OR:
            return 1;
        case TokenType::AND:
            return 2;
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::EQUAL_EQUAL:
        case TokenType::BANG_EQUAL:
            return 3;
        case TokenType::DOT_DOT:
            return 4;
        case TokenType::PLUS:
        case TokenType::MINUS:
            return 5;
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
            return 6;
        case TokenType::CARET:
            return 8;
        default:
            return 0;
    }
}
static inline bool isRightAssociative(const Token& t) noexcept {
    return t.type == TokenType::CARET || t.type == TokenType::DOT_DOT;
}

// forward
AST parseExpressionForwardDecl(const vector<Token>& Tokens, int& Index);

inline AST makeLeaf(ASTType t, const sv& txt, int line) {
    AST a;
    a.type = t;
    a.text.assign(txt.begin(),
                  txt.end());  // convert string_view to string once
    a.line = line;
    a.children.clear();
    return a;
}

AST parsePrimary(const vector<Token>& Tokens, int& Index) {
    if ((size_t)Index >= Tokens.size())
        return makeLeaf(ASTType::Identifier, "<?>", 0);
    const Token& tk = Tokens[Index];
    switch (tk.type) {
        case TokenType::NUMBER:
            ++Index;
            return makeLeaf(ASTType::NumericLiteral, tk.text, tk.line);
        case TokenType::STRING:
            ++Index;
            return makeLeaf(ASTType::StringLiteral, tk.text, tk.line);
        case TokenType::TRUE_:
        case TokenType::FALSE_:
            ++Index;
            return makeLeaf(ASTType::BooleanLiteral, tk.text, tk.line);
        case TokenType::NIL:
            ++Index;
            return makeLeaf(ASTType::NilLiteral, "nil", tk.line);
        case TokenType::IDENTIFIER:
            ++Index;
            return makeLeaf(ASTType::Identifier, tk.text, tk.line);
        case TokenType::DOT_DOT_DOT:
            ++Index;
            return makeLeaf(ASTType::VarargLiteral, "...", tk.line);
        case TokenType::LEFT_PAREN: {
            ++Index;
            AST inner = parseExpressionForwardDecl(Tokens, Index);
            if (Index < (int)Tokens.size() &&
                Tokens[Index].type == TokenType::RIGHT_PAREN)
                ++Index;
            return inner;
        }
        case TokenType::LEFT_BRACE: {
            int line = tk.line;
            ++Index;
            vector<AST> elements;
            elements.reserve(4);
            while (Index < (int)Tokens.size() &&
                   Tokens[Index].type != TokenType::RIGHT_BRACE) {
                AST val = parseExpressionForwardDecl(Tokens, Index);
                AST tv = makeLeaf(ASTType::TableValue, sv(), val.line);
                // named slot for value
                tv.children["value"].push_back(std::move(val));
                elements.emplace_back(std::move(tv));
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::COMMA)
                    ++Index;
                else
                    break;
            }
            if (Index < (int)Tokens.size() &&
                Tokens[Index].type == TokenType::RIGHT_BRACE)
                ++Index;
            AST table =
                makeLeaf(ASTType::TableConstructorExpression, sv(), line);
            table.children["fields"] = std::move(elements);
            return table;
        }
        case TokenType::FUNCTION: {
            int line = tk.line;
            ++Index;
            if (Index < (int)Tokens.size() &&
                Tokens[Index].type == TokenType::LEFT_PAREN) {
                ++Index;
                vector<AST> params;
                params.reserve(4);
                while (Index < (int)Tokens.size() &&
                       Tokens[Index].type != TokenType::RIGHT_PAREN) {
                    if (Tokens[Index].type == TokenType::IDENTIFIER) {
                        params.push_back(makeLeaf(ASTType::Identifier,
                                                  Tokens[Index].text,
                                                  Tokens[Index].line));
                        ++Index;
                        if (Index < (int)Tokens.size() &&
                            Tokens[Index].type == TokenType::COMMA)
                            ++Index;
                    } else
                        ++Index;
                }
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::RIGHT_PAREN)
                    ++Index;
                vector<AST> body;
                body.reserve(8);
                while (Index < (int)Tokens.size() &&
                       Tokens[Index].type != TokenType::END) {
                    if (Tokens[Index].type == TokenType::RETURN) {
                        int rline = Tokens[Index].line;
                        ++Index;
                        AST ev = parseExpressionForwardDecl(Tokens, Index);
                        AST ret = makeLeaf(ASTType::ReturnStatement,
                                           sv("return"), rline);
                        ret.children["values"].push_back(std::move(ev));
                        body.push_back(std::move(ret));
                        if (Index < (int)Tokens.size() &&
                            Tokens[Index].type == TokenType::SEMICOLON)
                            ++Index;
                        continue;
                    }
                    AST ev = parseExpressionForwardDecl(Tokens, Index);
                    body.push_back(std::move(ev));
                    if (Index < (int)Tokens.size() &&
                        Tokens[Index].type == TokenType::SEMICOLON)
                        ++Index;
                }
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::END)
                    ++Index;
                AST fn = makeLeaf(ASTType::FunctionExpression, sv(), line);

                AST blk = makeLeaf(ASTType::Block, sv("body"), line);
                blk.children["statements"] = std::move(body);

                fn.children["body"].push_back(std::move(blk));
                if (!params.empty()) fn.children["params"] = std::move(params);
                return fn;
            }
            return makeLeaf(ASTType::FunctionExpression, sv(), tk.line);
        }
        default:
            ++Index;
            return makeLeaf(ASTType::Identifier, sv("?"), tk.line);
    }
}

AST parseSuffixed(const vector<Token>& Tokens, int& Index) {
    AST expr = parsePrimary(Tokens, Index);
    while (Index < (int)Tokens.size()) {
        const Token& t = Tokens[Index];
        if (t.type == TokenType::DOT) {
            ++Index;
            if (Index < (int)Tokens.size() &&
                Tokens[Index].type == TokenType::IDENTIFIER) {
                AST member = makeLeaf(ASTType::Identifier, Tokens[Index].text,
                                      Tokens[Index].line);
                ++Index;
                AST node = makeLeaf(ASTType::MemberExpression, sv("."), t.line);
                // named slots
                node.children["object"].push_back(std::move(expr));
                node.children["property"].push_back(std::move(member));
                expr = std::move(node);
                continue;
            }
            break;
        } else if (t.type == TokenType::LEFT_BRACKET) {
            ++Index;
            AST key = parseExpressionForwardDecl(Tokens, Index);
            if (Index < (int)Tokens.size() &&
                Tokens[Index].type == TokenType::RIGHT_BRACKET)
                ++Index;
            AST node = makeLeaf(ASTType::IndexExpression, sv("[]"), t.line);
            node.children["object"].push_back(std::move(expr));
            node.children["index"].push_back(std::move(key));
            expr = std::move(node);
            continue;
        } else if (t.type == TokenType::LEFT_PAREN) {
            ++Index;
            vector<AST> args;
            args.reserve(4);
            while (Index < (int)Tokens.size() &&
                   Tokens[Index].type != TokenType::RIGHT_PAREN) {
                AST a = parseExpressionForwardDecl(Tokens, Index);
                args.push_back(std::move(a));
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::COMMA)
                    ++Index;
                else
                    break;
            }
            if (Index < (int)Tokens.size() &&
                Tokens[Index].type == TokenType::RIGHT_PAREN)
                ++Index;
            AST node = makeLeaf(ASTType::CallExpression, sv("call"), t.line);
            node.children["callee"].push_back(std::move(expr));
            if (!args.empty()) node.children["arguments"] = std::move(args);
            expr = std::move(node);
            continue;
        } else
            break;
    }
    return expr;
}

AST parseBinary(const vector<Token>& Tokens, int& Index, int minPrec) {
    if (Index >= (int)Tokens.size())
        return makeLeaf(ASTType::Identifier, sv("<?>"), 0);
    const Token& t = Tokens[Index];
    if (t.type == TokenType::MINUS || t.type == TokenType::NOT ||
        t.type == TokenType::HASH) {
        int line = t.line;
        ++Index;
        AST right = parseBinary(Tokens, Index, 9);
        AST un = makeLeaf(ASTType::UnaryExpression, t.text, line);
        un.children["argument"].push_back(std::move(right));
        return un;
    }

    AST left = parseSuffixed(Tokens, Index);

    while (Index < (int)Tokens.size()) {
        const Token& op = Tokens[Index];
        int prec = precedenceOf(op);
        if (prec == 0 || prec < minPrec) break;
        ++Index;
        int nextMin = prec + (isRightAssociative(op) ? 0 : 1);
        AST right = parseBinary(Tokens, Index, nextMin);
        AST bin = makeLeaf(ASTType::BinaryExpression, op.text, op.line);
        bin.children["left"].push_back(std::move(left));
        bin.children["right"].push_back(std::move(right));
        left = std::move(bin);
    }
    return left;
}

AST parseExpressionForwardDecl(const vector<Token>& Tokens, int& Index) {
    return parseBinary(Tokens, Index, 1);
}

vector<AST> parseExpressionList(const vector<Token>& Tokens, int& Index) {
    vector<AST> res;
    if (Index >= (int)Tokens.size()) return res;
    res.push_back(parseExpressionForwardDecl(Tokens, Index));
    while (Index < (int)Tokens.size() &&
           Tokens[Index].type == TokenType::COMMA) {
        ++Index;
        res.push_back(parseExpressionForwardDecl(Tokens, Index));
    }
    return res;
}

// Top-level parse
vector<AST> Parse(const vector<Token>& Tokens) {
    bool Running = true;
    int Index = 0;
    vector<AST> Chunk;
    Chunk.reserve(64);

    while (Running && Index < (int)Tokens.size()) {
        const Token& t = Tokens[Index];
        switch (t.type) {
            case TokenType::END_OF_FILE:
                Running = false;
                break;
            case TokenType::SEMICOLON:
                ++Index;
                break;
            case TokenType::LOCAL: {
                int line = t.line;
                ++Index;
                vector<AST> vars;
                vars.reserve(4);
                while (Index < (int)Tokens.size() &&
                       Tokens[Index].type == TokenType::IDENTIFIER) {
                    vars.push_back(makeLeaf(ASTType::Identifier,
                                            Tokens[Index].text,
                                            Tokens[Index].line));
                    ++Index;
                    if (Index < (int)Tokens.size() &&
                        Tokens[Index].type == TokenType::COMMA)
                        ++Index;
                    else
                        break;
                }
                vector<AST> vals;
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::EQUAL) {
                    ++Index;
                    vals = parseExpressionList(Tokens, Index);
                }
                AST node = makeLeaf(ASTType::LocalStatement, sv("local"), line);
                if (!vars.empty()) node.children["variables"] = std::move(vars);
                if (!vals.empty()) node.children["values"] = std::move(vals);
                Chunk.push_back(std::move(node));
                break;
            }
            case TokenType::RETURN: {
                int line = t.line;
                ++Index;
                vector<AST> vals;
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type != TokenType::SEMICOLON) {
                    vals = parseExpressionList(Tokens, Index);
                }
                AST node =
                    makeLeaf(ASTType::ReturnStatement, sv("return"), line);
                if (!vals.empty()) node.children["values"] = std::move(vals);
                Chunk.push_back(std::move(node));
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::SEMICOLON)
                    ++Index;
                break;
            }
            case TokenType::IF: {
                int line = t.line;
                ++Index;
                AST cond = parseExpressionForwardDecl(Tokens, Index);
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::THEN)
                    ++Index;
                vector<AST> clauses;

                // then block
                vector<AST> thenBlock;
                thenBlock.reserve(8);
                while (Index < (int)Tokens.size() &&
                       Tokens[Index].type != TokenType::ELSE &&
                       Tokens[Index].type != TokenType::ELSEIF &&
                       Tokens[Index].type != TokenType::END) {
                    AST node = parseExpressionForwardDecl(Tokens, Index);
                    thenBlock.push_back(std::move(node));
                    if (Index < (int)Tokens.size() &&
                        Tokens[Index].type == TokenType::SEMICOLON)
                        ++Index;
                    if (Index >= (int)Tokens.size()) break;
                }
                AST ifcl = makeLeaf(ASTType::IfClause, sv("if"), line);
                ifcl.children["condition"].push_back(std::move(cond));
                AST thenblk = makeLeaf(ASTType::Block, sv("then"), line);
                if (!thenBlock.empty())
                    thenblk.children["statements"] = std::move(thenBlock);
                ifcl.children["body"].push_back(std::move(thenblk));
                clauses.push_back(std::move(ifcl));

                while (Index < (int)Tokens.size() &&
                       Tokens[Index].type == TokenType::ELSEIF) {
                    int elifLine = Tokens[Index].line;
                    ++Index;
                    AST elifCond = parseExpressionForwardDecl(Tokens, Index);
                    if (Index < (int)Tokens.size() &&
                        Tokens[Index].type == TokenType::THEN)
                        ++Index;
                    vector<AST> elifBlock;
                    while (Index < (int)Tokens.size() &&
                           Tokens[Index].type != TokenType::ELSE &&
                           Tokens[Index].type != TokenType::ELSEIF &&
                           Tokens[Index].type != TokenType::END) {
                        AST node = parseExpressionForwardDecl(Tokens, Index);
                        elifBlock.push_back(std::move(node));
                        if (Index < (int)Tokens.size() &&
                            Tokens[Index].type == TokenType::SEMICOLON)
                            ++Index;
                        if (Index >= (int)Tokens.size()) break;
                    }
                    AST elifcl =
                        makeLeaf(ASTType::ElseifClause, sv("elseif"), elifLine);
                    AST elifblk =
                        makeLeaf(ASTType::Block, sv("elseif"), elifLine);
                    if (!elifBlock.empty())
                        elifblk.children["statements"] = std::move(elifBlock);
                    elifcl.children["condition"].push_back(std::move(elifCond));
                    elifcl.children["body"].push_back(std::move(elifblk));
                    clauses.push_back(std::move(elifcl));
                }

                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::ELSE) {
                    int elseLine = Tokens[Index].line;
                    ++Index;
                    vector<AST> elseBlock;
                    while (Index < (int)Tokens.size() &&
                           Tokens[Index].type != TokenType::END) {
                        AST node = parseExpressionForwardDecl(Tokens, Index);
                        elseBlock.push_back(std::move(node));
                        if (Index < (int)Tokens.size() &&
                            Tokens[Index].type == TokenType::SEMICOLON)
                            ++Index;
                        if (Index >= (int)Tokens.size()) break;
                    }
                    AST ech =
                        makeLeaf(ASTType::ElseClause, sv("else"), elseLine);
                    AST eb = makeLeaf(ASTType::Block, sv("else"), elseLine);
                    if (!elseBlock.empty())
                        eb.children["statements"] = std::move(elseBlock);
                    ech.children["body"].push_back(std::move(eb));
                    clauses.push_back(std::move(ech));
                }

                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::END)
                    ++Index;
                AST ifnode = makeLeaf(ASTType::IfStatement, sv("if"), line);
                if (!clauses.empty())
                    ifnode.children["clauses"] = std::move(clauses);
                Chunk.push_back(std::move(ifnode));
                break;
            }
            case TokenType::WHILE: {
                int line = t.line;
                ++Index;
                AST cond = parseExpressionForwardDecl(Tokens, Index);
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::DO)
                    ++Index;
                vector<AST> body;
                while (Index < (int)Tokens.size() &&
                       Tokens[Index].type != TokenType::END) {
                    AST node = parseExpressionForwardDecl(Tokens, Index);
                    body.push_back(std::move(node));
                    if (Index < (int)Tokens.size() &&
                        Tokens[Index].type == TokenType::SEMICOLON)
                        ++Index;
                }
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::END)
                    ++Index;
                AST w = makeLeaf(ASTType::WhileStatement, sv("while"), line);
                AST wb = makeLeaf(ASTType::Block, sv("while_body"), line);
                if (!body.empty()) wb.children["statements"] = std::move(body);
                w.children["condition"].push_back(std::move(cond));
                w.children["body"].push_back(std::move(wb));
                Chunk.push_back(std::move(w));
                break;
            }
            case TokenType::FUNCTION: {
                int line = t.line;
                ++Index;
                string funcName;
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::IDENTIFIER) {
                    funcName.assign(Tokens[Index].text.begin(),
                                    Tokens[Index].text.end());
                    ++Index;
                } else
                    funcName = "<anon>";
                vector<AST> params;
                params.reserve(6);
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::LEFT_PAREN) {
                    ++Index;
                    while (Index < (int)Tokens.size() &&
                           Tokens[Index].type != TokenType::RIGHT_PAREN) {
                        if (Tokens[Index].type == TokenType::IDENTIFIER) {
                            params.push_back(makeLeaf(ASTType::Identifier,
                                                      Tokens[Index].text,
                                                      Tokens[Index].line));
                            ++Index;
                            if (Index < (int)Tokens.size() &&
                                Tokens[Index].type == TokenType::COMMA)
                                ++Index;
                        } else
                            ++Index;
                    }
                    if (Index < (int)Tokens.size() &&
                        Tokens[Index].type == TokenType::RIGHT_PAREN)
                        ++Index;
                }
                vector<AST> body;
                body.reserve(8);
                while (Index < (int)Tokens.size() &&
                       Tokens[Index].type != TokenType::END) {
                    if (Tokens[Index].type == TokenType::RETURN) {
                        int rline = Tokens[Index].line;
                        ++Index;
                        vector<AST> retvals;
                        if (Index < (int)Tokens.size() &&
                            Tokens[Index].type != TokenType::SEMICOLON)
                            retvals = parseExpressionList(Tokens, Index);
                        AST rt = makeLeaf(ASTType::ReturnStatement,
                                          sv("return"), rline);
                        if (!retvals.empty())
                            rt.children["values"] = std::move(retvals);
                        body.push_back(std::move(rt));
                        if (Index < (int)Tokens.size() &&
                            Tokens[Index].type == TokenType::SEMICOLON)
                            ++Index;
                        continue;
                    }
                    AST node = parseExpressionForwardDecl(Tokens, Index);
                    body.push_back(std::move(node));
                    if (Index < (int)Tokens.size() &&
                        Tokens[Index].type == TokenType::SEMICOLON)
                        ++Index;
                }
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::END)
                    ++Index;
                AST fd = makeLeaf(ASTType::FunctionDeclaration, sv("function"),
                                  line);
                AST id = makeLeaf(ASTType::Identifier, sv(funcName), line);
                AST blk = makeLeaf(ASTType::Block, sv("body"), line);
                if (!body.empty()) blk.children["statements"] = std::move(body);
                fd.children["name"].push_back(std::move(id));
                fd.children["body"].push_back(std::move(blk));
                if (!params.empty()) fd.children["params"] = std::move(params);
                Chunk.push_back(std::move(fd));
                break;
            }
            default: {
                if (t.type == TokenType::IDENTIFIER) {
                    if (Index + 1 < (int)Tokens.size() &&
                        (Tokens[Index + 1].type == TokenType::EQUAL ||
                         Tokens[Index + 1].type == TokenType::COMMA)) {
                        vector<AST> vars;
                        vars.reserve(4);
                        while (Index < (int)Tokens.size() &&
                               Tokens[Index].type == TokenType::IDENTIFIER) {
                            vars.push_back(makeLeaf(ASTType::Identifier,
                                                    Tokens[Index].text,
                                                    Tokens[Index].line));
                            ++Index;
                            if (Index < (int)Tokens.size() &&
                                Tokens[Index].type == TokenType::COMMA)
                                ++Index;
                            else
                                break;
                        }
                        if (Index < (int)Tokens.size() &&
                            Tokens[Index].type == TokenType::EQUAL) {
                            ++Index;
                            vector<AST> vals =
                                parseExpressionList(Tokens, Index);
                            AST asn = makeLeaf(ASTType::AssignmentStatement,
                                               sv("assign"), t.line);
                            if (!vars.empty())
                                asn.children["variables"] = std::move(vars);
                            if (!vals.empty())
                                asn.children["values"] = std::move(vals);
                            Chunk.push_back(std::move(asn));
                        } else {
                            AST expr =
                                parseExpressionForwardDecl(Tokens, Index);
                            AST ch =
                                makeLeaf(ASTType::Chunk, sv("expr"), expr.line);
                            ch.children["statements"].push_back(
                                std::move(expr));
                            Chunk.push_back(std::move(ch));
                        }
                    } else if (Index + 1 < (int)Tokens.size() &&
                               Tokens[Index + 1].type ==
                                   TokenType::LEFT_PAREN) {
                        AST call = parseSuffixed(Tokens, Index);
                        AST cs = makeLeaf(ASTType::CallStatement,
                                          sv("call_stmt"), t.line);
                        cs.children["expression"].push_back(std::move(call));
                        Chunk.push_back(std::move(cs));
                    } else {
                        AST expr = parseExpressionForwardDecl(Tokens, Index);
                        AST ch =
                            makeLeaf(ASTType::Chunk, sv("expr"), expr.line);
                        ch.children["statements"].push_back(std::move(expr));
                        Chunk.push_back(std::move(ch));
                    }
                } else {
                    AST expr = parseExpressionForwardDecl(Tokens, Index);
                    AST ch = makeLeaf(ASTType::Chunk, sv("expr"), expr.line);
                    ch.children["statements"].push_back(std::move(expr));
                    Chunk.push_back(std::move(ch));
                }
                if (Index < (int)Tokens.size() &&
                    Tokens[Index].type == TokenType::SEMICOLON)
                    ++Index;
                break;
            }
        }  // end switch
    }  // end while
    return Chunk;
}

// ---------- Test helpers / main ----------

static inline std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char uc : s) {
        switch (uc) {
            case '\"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (uc < 0x20) {
                    out += "\\u00";
                    out += hexDigit(uc >> 4);
                    out += hexDigit(uc & 0xF);
                } else {
                    out.push_back(static_cast<char>(uc));
                }
        }
    }
    return out;
}

string astTypeToString(ASTType type) {
    switch (type) {
        case ASTType::AssignmentStatement:
            return "AssignmentStatement";
        case ASTType::LocalStatement:
            return "LocalStatement";
        case ASTType::Identifier:
            return "Identifier";
        case ASTType::BooleanLiteral:
            return "BooleanLiteral";
        case ASTType::StringLiteral:
            return "StringLiteral";
        case ASTType::NumericLiteral:
            return "NumericLiteral";
        case ASTType::FunctionDeclaration:
            return "FunctionDeclaration";
        case ASTType::FunctionExpression:
            return "FunctionExpression";
        case ASTType::CallStatement:
            return "CallStatement";
        case ASTType::CallExpression:
            return "CallExpression";
        case ASTType::BinaryExpression:
            return "BinaryExpression";
        case ASTType::ReturnStatement:
            return "ReturnStatement";
        case ASTType::DoStatement:
            return "DoStatement";
        case ASTType::WhileStatement:
            return "WhileStatement";
        case ASTType::TableConstructorExpression:
            return "TableConstructorExpression";
        case ASTType::TableValue:
            return "TableValue";
        case ASTType::TableKey:
            return "TableKey";
        case ASTType::MemberExpression:
            return "MemberExpression";
        case ASTType::UnaryExpression:
            return "UnaryExpression";
        case ASTType::IndexExpression:
            return "IndexExpression";
        case ASTType::ForGenericStatement:
            return "ForGenericStatement";
        case ASTType::ForNumericStatement:
            return "ForNumericStatement";
        case ASTType::IfStatement:
            return "IfStatement";
        case ASTType::IfClause:
            return "IfClause";
        case ASTType::ElseifClause:
            return "ElseifClause";
        case ASTType::ElseClause:
            return "ElseClause";
        case ASTType::BreakStatement:
            return "BreakStatement";
        case ASTType::GotoStatement:
            return "GotoStatement";
        case ASTType::LabelStatement:
            return "LabelStatement";
        case ASTType::RepeatStatement:
            return "RepeatStatement";
        case ASTType::VarargLiteral:
            return "VarargLiteral";
        case ASTType::NilLiteral:
            return "NilLiteral";
        case ASTType::Chunk:
            return "Chunk";
        case ASTType::Block:
            return "Block";
        case ASTType::VariableAttribute:
            return "VariableAttribute";
        case ASTType::LogicalExpression:
            return "LogicalExpression";
        case ASTType::TableKeyString:
            return "TableKeyString";
        default:
            return "Unknown";
    }
}

// ---------------- JSON serializer ----------------
void printASTJson(const AST& node, std::ostream& out, int indent = 0) {
    std::string ind(indent, ' ');
    out << ind << "{\n";

    out << ind << "  \"nodeType\": \"" << astTypeToString(node.type) << "\",\n";
    out << ind << "  \"text\": \"" << jsonEscape(node.text) << "\",\n";
    out << ind << "  \"line\": " << node.line << ",\n";

    out << ind << "  \"children\": {";
    if (!node.children.empty()) out << "\n";

    bool firstGroup = true;
    for (auto it = node.children.begin(); it != node.children.end(); ++it) {
        if (!firstGroup) out << ",\n";
        firstGroup = false;

        out << ind << "    \"" << it->first << "\": [\n";
        for (size_t i = 0; i < it->second.size(); i++) {
            printASTJson(it->second[i], out, indent + 6);
            if (i + 1 < it->second.size()) out << ",";
            out << "\n";
        }
        out << ind << "    ]";
    }

    if (!node.children.empty()) out << "\n" << ind << "  ";
    out << "}\n" << ind << "}";
}

int main(int argc, char* argv[]) {
    std::string filePath;

    if (argc >= 2) {
        // Case 1: Drag & drop file
        filePath = argv[1];
    } else {
        // Case 2: Ask user for file path
        std::cout << "Enter path to source file: ";
        std::getline(std::cin, filePath);
    }

    if (!std::filesystem::exists(filePath)) {
        std::cerr << "Error: file not found -> " << filePath << "\n";
        return 1;
    }

    // Read file contents
    std::ifstream in(filePath);
    std::string code((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());

    // Run lexer + parser
    auto tokens = Lexer(code);
    auto chunk = Parse(tokens);

    // Print JSON AST
    std::cout << "[\n";
    for (size_t i = 0; i < chunk.size(); ++i) {
        printASTJson(chunk[i], std::cout, 2);
        if (i + 1 < chunk.size())
            std::cout << ",\n";
        else
            std::cout << "\n";
    }
    std::cout << "]\n";

    return 0;
}
