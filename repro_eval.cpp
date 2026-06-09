#include <iostream>
#include <string>
#include <map>

struct AssemblyResult {
    std::map<std::string, int> symbols;
};

int evaluate_expression(std::string e, const AssemblyResult& res, bool& ok) {
    if (e.empty()) return 0;
    size_t plus = e.find_last_of('+');
    if (plus != std::string::npos) {
        return evaluate_expression(e.substr(0, plus), res, ok) + evaluate_expression(e.substr(plus+1), res, ok);
    }

    if (res.symbols.count(e)) return res.symbols.at(e);
    try {
        if (e.back() == 'H' && std::isdigit(e[0])) return std::stoi(e.substr(0, e.length()-1), nullptr, 16);
        if (std::isdigit(e[0]) || e[0] == '-') return std::stoi(e, nullptr, 0);
    } catch (...) {}
    ok = false;
    return 0;
}

int main() {
    AssemblyResult res;
    bool ok = true;
    
    std::cout << "Evaluating '0+5': " << evaluate_expression("0+5", res, ok) << " ok=" << ok << std::endl;
    ok = true;
    std::cout << "Evaluating '0BBH': " << evaluate_expression("0BBH", res, ok) << " ok=" << ok << std::endl;
    ok = true;
    std::cout << "Evaluating '0+0100H': " << evaluate_expression("0+0100H", res, ok) << " ok=" << ok << std::endl;
    
    return 0;
}
