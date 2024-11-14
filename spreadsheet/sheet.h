#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <map>
#include <unordered_map>

class CellHasher {
public:
    size_t operator()(const Position p) const {
        return std::hash<std::string>()(p.ToString());
    }
};

class Sheet : public SheetInterface {
public:
    using Sheet_ = std::unordered_map<Position, std::unique_ptr<Cell>, CellHasher>;

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
private:
    class MinPrintArea {
    public:
        void AddCountPositions(Position pos);
        void SubCountPositions(Position pos);

        Size GetMinPrintArea() const;
    private:
        std::map<int, int> cols_with_data_per_index;
        std::map<int, int> rows_with_data_per_index;
        
        void DeleteNullRowPosition(int index);
        void DeleteNullColPosition(int index);
    };

    Sheet_ sheet_;
    MinPrintArea min_print_area_;
        
    //Возвращает true, когда позиция задана, не является nullptr и не пустая ячейка
    bool CheckCurrentPosition(Position pos) const;
};