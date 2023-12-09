#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>


// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : sheet_(sheet), content_(std::move(std::make_unique<EmptyImpl>()))
{   
}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    referenced_cells_.clear();
    if (text[0] == '=' &&  text.size() != 1){
        auto tmp_ptr = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        const std::vector<Position>& referenced_cells = tmp_ptr->GetReferencedCells();
        std::unordered_set<const CellInterface*> set_of_checked_cells;
        ChechForCircularDependencies(referenced_cells, set_of_checked_cells);

        if (!referenced_cells.empty()){
            SetRelations(referenced_cells);
        }
        content_ = std::move(tmp_ptr);
    }
    else if (text.size() == 1){
        content_ = std::move(std::make_unique<TextImpl>(text));
    }
    else {
        content_ = std::move(std::make_unique<TextImpl>(text));
    }
    std::unordered_set<Cell*> set_of_checked_cells;
    ResetCache(set_of_checked_cells);
}

void Cell::Clear() {
    content_ = std::move(std::make_unique<EmptyImpl>());
}

Cell::Value Cell::GetValue() const {
    if (cache_.has_value()){
        return cache_.value();
    }
    else {
        cache_ = content_->GetValue();
        return cache_.value();
    }
    
}
std::string Cell::GetText() const {
    return content_->GetText();
}



Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}

std::string Cell::TextImpl::GetText() const {
    return content_;
}

Cell::Value Cell::TextImpl::GetValue() const {
    if (content_[0] == '\''){
        return content_.substr(1);
    }
    else {
        return content_;
    }
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return {};
}

std::string Cell::FormulaImpl::GetText() const {
    return "=" + content_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return content_->GetReferencedCells();
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    auto result = content_->Evaluate(sheet_);
    if (std::holds_alternative<double>(result)){
        return std::get<double>(result);
    } else {
        return std::get<FormulaError>(result);
    }
}

void Cell::ChechForCircularDependencies(const std::vector<Position>& ref_cells,
                                              std::unordered_set<const CellInterface*>& container) const {
    for (const auto position : ref_cells){
        const CellInterface* cell_ptr = sheet_.GetCell(position);
        if (cell_ptr == this){
            throw CircularDependencyException("Circular dependency exists");
        }
        if (cell_ptr != nullptr && container.count(cell_ptr) == 0){
            const auto referenced_cells_inner = cell_ptr->GetReferencedCells();
            ChechForCircularDependencies(referenced_cells_inner, container);
        }
        
        else {
            container.insert(cell_ptr);
        }
    }
}

void Cell::SetRelations(const std::vector<Position>& container){
    for (auto& position : container){
        auto cell_ptr = dynamic_cast<Cell*>(sheet_.GetCell(position));
        if (cell_ptr == nullptr){
            sheet_.SetCell(position, "");
        }
        cell_ptr = dynamic_cast<Cell*>(sheet_.GetCell(position));
        cell_ptr->referring_cells_.insert(this);
        referenced_cells_.insert(cell_ptr);
    }
}

void Cell::ResetCache(std::unordered_set<Cell*> container){
    cache_ = std::nullopt;
    for (auto cell : referring_cells_){
        if (container.count(cell) == 0){
            cell->ResetCache(container);
            container.insert(cell);
        }
    }
}

std::vector<Position> Cell::GetReferencedCells() const {
    return content_->GetReferencedCells();
}
