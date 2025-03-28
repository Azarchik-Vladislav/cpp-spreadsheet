#pragma once

#include <forward_list>
#include <functional>
#include <stdexcept>
#include <string>

#include "FormulaLexer.h"
#include "common.h"

namespace ASTImpl {
class ArgCell {
public:
    ArgCell(const SheetInterface& sheet);
    double operator()(Position pos) const;
private:
    const SheetInterface& sheet_;
};

class Expr;
}

class ParsingError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FormulaAST {
public:
    explicit FormulaAST(std::unique_ptr<ASTImpl::Expr> root_expr,
                        std::forward_list<Position> cells);
    FormulaAST(FormulaAST&&) = default;
    FormulaAST& operator=(FormulaAST&&) = default;
    ~FormulaAST();

    double Execute(const ASTImpl::ArgCell& args) const;
    void PrintCells(std::ostream& out) const;
    void Print(std::ostream& out) const;
    void PrintFormula(std::ostream& out) const;

    const std::forward_list<Position>& GetReferencedCells() const;

private:
    std::unique_ptr<ASTImpl::Expr> root_expr_;
    std::forward_list<Position> cells_;
};

FormulaAST ParseFormulaAST(std::istream& in);
FormulaAST ParseFormulaAST(const std::string& in_str);