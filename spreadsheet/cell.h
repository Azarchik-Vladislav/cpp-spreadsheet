#pragma once

#include <memory>
#include <set>

#include "common.h"
#include "formula.h"

class Sheet;

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    Cell(Sheet& sheet);

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

private:
    std::set<Cell*> linked_cells_;
    std::set<Cell*> referenced_cells_;
    Sheet& sheet_;

    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;
    
    bool IsCircularDependency(const Impl& new_impl);
    void ClearCellInfo();
    void UpdateLinkedAndReferencedContainers();
    void InvalidateCacheRecursive();
};