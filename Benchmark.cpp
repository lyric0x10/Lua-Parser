volatile std::uint64_t checksum = 0;

void benchLexerOnly(const std::string& code, int iters) {
    // warm up
    for (int i = 0; i < 1000; ++i) {
        auto toks = Lexer(code);
        checksum += toks.size();
    }
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = 0; i < iters; ++i) {
        auto toks = Lexer(code);
        checksum += toks.size();
    }
    auto t1 = chrono::high_resolution_clock::now();
    double secs = chrono::duration<double>(t1 - t0).count();
    cout << "Lexer-only: " << iters << " iterations, " << secs << "s, avg ms/iter=" << (secs * 1000.0 / iters) << "\n";
}

void benchParserOnly(const std::string& code, int iters) {
    auto baseTokens = Lexer(code);
    // warm up
    for (int i = 0; i < 1000; ++i) {
        auto ast = Parse(baseTokens);
        checksum += ast.size();
    }
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = 0; i < iters; ++i) {
        auto ast = Parse(baseTokens);
        checksum += ast.size();
    }
    auto t1 = chrono::high_resolution_clock::now();
    double secs = chrono::duration<double>(t1 - t0).count();
    cout << "Parser-only: " << iters << " iterations, " << secs << "s, avg ms/iter=" << (secs * 1000.0 / iters) << "\n";
}

void benchLexerParser(const std::string& code, int iters) {
    for (int i = 0; i < 1000; ++i) {
        auto toks = Lexer(code);
        auto ast = Parse(toks);
        checksum += toks.size() + ast.size();
    }
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = 0; i < iters; ++i) {
        auto toks = Lexer(code);
        auto ast = Parse(toks);
        checksum += toks.size() + ast.size();
    }
    auto t1 = chrono::high_resolution_clock::now();
    double secs = chrono::duration<double>(t1 - t0).count();
    cout << "Lexer+Parser: " << iters << " iterations, " << secs << "s, avg ms/iter=" << (secs * 1000.0 / iters) << "\n";
}

void test() {
    string code;
    for (int i = 0; i < 50; ++i) {
        code += R"(
function foo(a,b)
  local x = a + b * 123.456
  if x > 100 then
    return "big", x
  else
    return "small", x
  end
end
)";
    }
    constexpr int ITERS = 20000;
    benchLexerOnly(code, ITERS);
    benchParserOnly(code, ITERS);
    benchLexerParser(code, ITERS);
    cout << "checksum: " << checksum << "\n";
}
