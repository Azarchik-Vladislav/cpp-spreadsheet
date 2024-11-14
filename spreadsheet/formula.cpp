#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <functional>
#include <iostream>
#include <sstream>

using namespace std::literals;

namespace {

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(expression)) {
    } catch(const std::exception& e) {
         std::throw_with_nested(FormulaException(e.what()));
    }
    Value Evaluate(const SheetInterface& sheet) const override  {
        ASTImpl::ArgCell functor(sheet);

        try {
            return ast_.Execute(functor);
        } catch(const FormulaError& err) {
            return err;
        }
    }
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const {
        std::vector<Position> cells;
        for (auto cell : ast_.GetReferencedCells()) {
                cells.push_back(cell);
        }
        
        std::sort(cells.begin(), cells.end());
        cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());

        return cells;
    }
private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}