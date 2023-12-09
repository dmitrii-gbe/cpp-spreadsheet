#pragma once

#include "common.h"
#include "formula.h"
#include <functional>
#include <unordered_set>
#include <optional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    //bool IsReferenced() const;

private:

class Impl {
    public:
        virtual Value GetValue() const = 0;

        virtual std::string GetText() const = 0;

        virtual std::vector<Position> GetReferencedCells() const = 0;
};

class EmptyImpl : public Impl {
    public:
        Value GetValue() const override;

        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;
};

class TextImpl : public Impl {
    public:
        TextImpl(const std::string& str) : content_(str)
        {
        }

        std::string GetText() const override;

        Value GetValue() const override;

        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string content_;
};

class FormulaImpl : public Impl {
    public:
        FormulaImpl(const std::string& str, SheetInterface& sheet) : sheet_(sheet)
                                                                    ,content_(std::move(ParseFormula(str)))
        {
        }
        std::string GetText() const override;

        Value GetValue() const override;

        std::vector<Position> GetReferencedCells() const;

    private:
    SheetInterface& sheet_;
    std::unique_ptr<FormulaInterface> content_;
};

    void ResetCache(std::unordered_set<Cell*>);

    void ChechForCircularDependencies(const std::vector<Position>& ref_cells, std::unordered_set<const CellInterface*>& container) const;
    void SetRelations(const std::vector<Position>& container);
   
    SheetInterface& sheet_;
    std::unique_ptr<Impl> content_;
    mutable std::optional<Value> cache_;
    std::unordered_set<Cell*> referring_cells_;
    std::unordered_set<Cell*> referenced_cells_;

};