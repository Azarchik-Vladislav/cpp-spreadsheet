#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <iostream>
#include <sstream>

#include "formula.h"
#include "FormulaAST.h"

using namespace std::literals;

namespace {

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression);

    Value Evaluate(const SheetInterface& sheet) const override;
    std::string GetExpression() const override;
    std::vector<Position> GetReferencedCells() const;
private:
    FormulaAST ast_;
};

//_______Formula_______
Formula::Formula(std::string expression) try
    : ast_(ParseFormulaAST(expression)) {
} catch(const std::exception& e) {
 std::throw_with_nested(FormulaException(e.what()));
}

FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const  {
    ASTImpl::ArgCell functor(sheet);

    try {
        return ast_.Execute(functor);
    } catch(const FormulaError& err) {
        return err;
    }
}

std::string Formula::GetExpression() const {
    std::ostringstream out;
    ast_.PrintFormula(out);

    return out.str();
}

std::vector<Position> Formula::GetReferencedCells() const {
    std::vector<Position> cells;
    for (auto cell : ast_.GetReferencedCells()) {
            cells.push_back(cell);
    }
    
    std::sort(cells.begin(), cells.end());
    cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());

    return cells;
}
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}