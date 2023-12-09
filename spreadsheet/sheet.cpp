#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

const Position Position::NONE = {-1, -1};
const Position MAX = {Position::MAX_COLS, Position::MAX_ROWS};

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid position");
    }
    std::string key = pos.ToString();
    if (sheet_.count(key) == 0){
        sheet_[key] = std::make_unique<Cell>(*this);
    }
    sheet_.at(key)->Set(text);
    ReCalculatePrintableArea(pos);

}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid position");
    }
    std::string key = pos.ToString();
    if (sheet_.count(key) == 0){
        return nullptr;
    }
    else {
        return sheet_.at(key).get();
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid position");
    }
    std::string key = pos.ToString();
    if (sheet_.count(key) == 0){
        return nullptr;
    }
    else {
        return sheet_.at(key).get();
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()){
        throw InvalidPositionException("Invalid position");
    }
    std::string key = pos.ToString();
    if (sheet_.count(key) != 0){
        sheet_.erase(pos.ToString());
    }
    if (pos.row == bottom_right_.row || pos.col == bottom_right_.col){
        bottom_right_ = Position::NONE;
        for (const auto& [key, value] : sheet_){
            ReCalculatePrintableArea(Position::FromString(key));
        }
    }
}

Size Sheet::GetPrintableSize() const {
    if (bottom_right_ == Position::NONE){
        return {0, 0};
    }
    else {
        return {bottom_right_.row + 1, bottom_right_.col + 1};
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i <= bottom_right_.row; ++i){
        for (int j = 0; j <= bottom_right_.col; ++j){
            auto ptr = GetCell({i, j});
                if (ptr != nullptr){
                    auto value = ptr->GetValue();
                    if (std::holds_alternative<std::string>(value)){
                        output << std::get<std::string>(value);
                    }
                    else if (std::holds_alternative<double>(value)){
                        output << std::get<double>(value);
                    }
                    else {
                        output << std::get<FormulaError>(value);
                    }
                }
            output << (j < bottom_right_.col ? "\t" : "\n");
        } 
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i <= bottom_right_.row; ++i){
        for (int j = 0; j <= bottom_right_.col; ++j){
            auto ptr = GetCell({i, j});
            if (j < bottom_right_.col){
                output << ((ptr != nullptr) ? ptr->GetText() : "") << "\t";
            }
            if (j == bottom_right_.col){
                output << ((ptr != nullptr) ? ptr->GetText() :  "") << "\n";
            }
        }
    }
}

    void Sheet::ReCalculatePrintableArea(Position pos){
        if (pos.row > bottom_right_.row){
            bottom_right_.row = pos.row;
        }
        if (pos.col > bottom_right_.col){
            bottom_right_.col = pos.col;
        }
    }

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}