#include "formula.h"
#include "common.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <vector>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression))
    {
    }

    Value Evaluate(const SheetInterface& sheet) const override {
        Value result;
        try {
            result = ast_.Execute(sheet);
        }
        catch (const FormulaError& error){
            result = error;
        }
        return result;
    } 
    std::string GetExpression() const override {
        std::ostringstream stream;
        ast_.PrintFormula(stream);
        return stream.str();
    }


    std::vector<Position> GetReferencedCells() const {
        const std::forward_list<Position>& list = ast_.GetCells();
        std::vector<Position> result(list.begin(), list.end());
        std::sort(result.begin(), result.end());
        auto it = std::unique(result.begin(), result.end());
        result.erase(it, result.end());
        return result;
        
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...){
        throw FormulaException("Parsing formula error");
    }

}