#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <queue>
#include <unordered_set>

#include "cell.h"
#include "sheet.h"

using namespace std::literals;

using std::make_unique;
using std::string;

class Cell::Impl {
public:
    virtual ~Impl() = default;

    virtual Cell::Value GetValue() const;
    virtual std::string GetText() const;
    virtual std::vector<Position> GetReferencedCells() const;

protected:
    Cell::Value value_ = ""s;
    std::string text_ = ""s;
};

class Cell::EmptyImpl final : public Impl {};

class Cell::TextImpl final : public Impl {
public:
    TextImpl(const std::string& text);
};

class Cell::FormulaImpl : public Impl {
public:
    FormulaImpl(const std::string& formula, const SheetInterface& sheet);
    
    bool IsValidCache() const;
    void InvalidateCache() const;

    Cell::Value GetValue() const override;
    std::vector<Position> GetReferencedCells() const override;
private:
    const SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> formula_;
    mutable std::optional<FormulaInterface::Value> cache_; 
};

Cell::~Cell() = default;

Cell::Cell(Sheet& sheet) : sheet_(sheet),
                           impl_(make_unique<EmptyImpl>()) {
}

void Cell::Set(std::string text) { 
    if(text.size() == 0) {
        impl_ = make_unique<EmptyImpl>();
    } else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        std::unique_ptr<Impl> test_impl;
        test_impl = make_unique<FormulaImpl>(std::move(text), sheet_);
        
        if(IsCircularDependency(*test_impl)) {
            throw CircularDependencyException(""s);
        }
        impl_ = std::move(test_impl);
    } else {
        impl_ = make_unique<TextImpl>(std::move(text));
    }
    
    ClearCellInfo();
    UpdateLinkedAndReferencedContainers();
    InvalidateCacheRecursive();
}

void Cell::Clear() {
    Set(""s);
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !linked_cells_.empty();
}

bool Cell::IsCircularDependency(const Impl& new_impl) {
    const auto referenced_cells = new_impl.GetReferencedCells();
    if (referenced_cells.empty()){
        return false;
    }

    std::queue<const Cell*> to_visit;
    for (const auto& pos : referenced_cells) {
        Cell* cell = sheet_.GetConcreteCell(pos);

        if(!cell) {
            sheet_.SetCell(pos, ""s);
        }
        to_visit.push(sheet_.GetConcreteCell(pos));
    }

    std::unordered_set<const Cell*> visited;

    while (!to_visit.empty()) {
        const Cell* current = to_visit.front();
        
        if (this == current){
            return true;
        }

        visited.insert(current);  
        to_visit.pop();

        for (const Cell* reference_cell : current->referenced_cells_) {
            if (visited.find(reference_cell) == visited.end()) {
                to_visit.push(reference_cell);
            }
        }
    }

    return false;
}

void Cell::ClearCellInfo() {
    for(const auto& referenced_cell : referenced_cells_) {
        referenced_cell->linked_cells_.erase(this);
    }
    referenced_cells_.clear();
}

void Cell::UpdateLinkedAndReferencedContainers() {
    const auto pos_ref_cells = GetReferencedCells();

    for(size_t i = 0; i < pos_ref_cells.size(); ++i) {
        Cell* referenced_cell = sheet_.GetConcreteCell(pos_ref_cells[i]);

        referenced_cells_.insert(referenced_cell);
        referenced_cell->linked_cells_.insert(this);
    }
}

void Cell::InvalidateCacheRecursive() {
    if(const FormulaImpl* formula = dynamic_cast<FormulaImpl*>(impl_.get())) {

        if(formula->IsValidCache()) {
            formula->InvalidateCache();
        }  
    }

    for(const auto& linked_cell : linked_cells_) {
        linked_cell->InvalidateCacheRecursive();
    }
}

//_______Cell::Impl_______
Cell::Value Cell::Impl::GetValue() const {
    return value_; 
}

std::string Cell::Impl::GetText() const {
    return text_;
}

std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

//_______Cell::TextImpl_______
Cell::TextImpl::TextImpl(const std::string& text) {
    text_ = text;
    value_ = text[0] == ESCAPE_SIGN ? text.substr(1) : text;
}


//_______Cell::FormulaImpl_______
Cell::FormulaImpl::FormulaImpl(const std::string& formula, const SheetInterface& sheet) 
    : sheet_(sheet) {

    auto expr = formula.substr(1);
    formula_ = ParseFormula(expr);
    text_ = FORMULA_SIGN + formula_->GetExpression();
}

bool Cell::FormulaImpl::IsValidCache() const {
    return cache_.has_value();
}

void Cell::FormulaImpl::InvalidateCache() const {
    cache_.reset();
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if(!cache_) {
        cache_ = formula_->Evaluate(sheet_);
    }

    if(std::holds_alternative<double>(cache_.value())) {
        return std::get<double>(cache_.value());
    }

    return std::get<FormulaError>(cache_.value());
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}
