#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

#include "cell.h"
#include "common.h"
#include "sheet.h"

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    ThrowIfNotValid(pos);
    
    const auto& cell = sheet_.find(pos);
    if (cell == sheet_.end()) {
        sheet_.emplace(pos, std::make_unique<Cell>(*this));
    }
    
    sheet_.at(pos)->Set(std::move(text));

    if(sheet_.at(pos)->GetText() != ""s) {
        min_print_area_.AddCountPositions(pos);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    ThrowIfNotValid(pos);
    
    const auto cell = sheet_.find(pos);
    if (cell == sheet_.end()) {
        return nullptr;
    }

    return sheet_.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    ThrowIfNotValid(pos);

    const auto cell = sheet_.find(pos);
    if (cell == sheet_.end()) {
        return nullptr;
    }

    return sheet_.at(pos).get();
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
    return reinterpret_cast<const Cell*>(GetCell(pos));
}

Cell *Sheet::GetConcreteCell(Position pos) {
    return reinterpret_cast<Cell*>(GetCell(pos));
}

void Sheet::ClearCell(Position pos) {
    ThrowIfNotValid(pos);

    if(!CheckCurrentPosition(pos)) {
        return;
    }

    sheet_[pos]->Clear();
    min_print_area_.SubCountPositions(pos);
}

Size Sheet::GetPrintableSize() const {
    return min_print_area_.GetMinPrintArea();
}

void Sheet::PrintValues(std::ostream& output) const {
    const Size size = GetPrintableSize();

    for(int row = 0; row < size.rows; ++row) {
        bool it_first = true;

        for(int col = 0; col < size.cols; ++col) {
            if(!it_first){
                output << "\t";
            }
            
            Position pos{row, col};

            if(CheckCurrentPosition(pos)) {
                auto temp_value = sheet_.at(pos)->GetValue();
                std::visit([&output](auto&& arg) {output << arg; }, temp_value);
            }
            it_first = false;
        }
        output << "\n";
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    const Size size = GetPrintableSize();

    for(int row = 0; row < size.rows; ++row) {
        bool it_first = true;

        for(int col = 0; col < size.cols; ++col) {
            if(!it_first){
                output << "\t";
            }
            
            Position pos{row, col};

            if(CheckCurrentPosition(pos)) {
                output << sheet_.at(pos)->GetText();
            }
            it_first = false;
        }
        output << "\n";
    }
}

bool Sheet::CheckCurrentPosition(Position pos) const {
    const auto& cell = sheet_.find(pos);
    return cell != sheet_.end() && cell->second != nullptr && cell->second->GetText() != ""s;
}

void Sheet::ThrowIfNotValid(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Out of MAX or MIN positions");
    }
}

void Sheet::MinPrintArea::AddCountPositions(Position pos) {
    ++rows_with_data_per_index[pos.row];
    ++cols_with_data_per_index[pos.col];
}
void Sheet::MinPrintArea::SubCountPositions(Position pos) {
    --rows_with_data_per_index[pos.row];
    if(rows_with_data_per_index.at(pos.row) == 0) {
        DeleteNullRowPosition(pos.row);
    }

    --cols_with_data_per_index[pos.col];
    if(cols_with_data_per_index.at(pos.col) == 0) {
        DeleteNullColPosition(pos.col);
    }
}

Size Sheet::MinPrintArea::GetMinPrintArea() const {
    if(rows_with_data_per_index.empty() && cols_with_data_per_index.empty()) {
        return Size{0, 0};
    }

    return Size{rows_with_data_per_index.rbegin()->first + 1,
                cols_with_data_per_index.rbegin()->first + 1};
}

void Sheet::MinPrintArea::DeleteNullRowPosition(int index) {
    rows_with_data_per_index.erase(index);
}

void Sheet::MinPrintArea::DeleteNullColPosition(int index) {
    cols_with_data_per_index.erase(index);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}