#pragma once

#include <unordered_set>
#include <optional>

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    void ClearCache();

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    mutable std::optional<Value> cache_;
    std::unordered_set<Cell*> referenced_cells_; //св€занные €чейки
    std::unordered_set<Cell*> dependent_cells_; //зависимые €чейки

    bool HasCyclicDependencies(const std::vector<Position>& ref_cells, Cell* search_val);

    class Impl {
    public:
        virtual Value GetValue(const SheetInterface& sheet) const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
        virtual ~Impl() = default;
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        ~EmptyImpl() = default;
        Value GetValue(const SheetInterface& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);
        Value GetValue(const SheetInterface& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string formula);
        Value GetValue(const SheetInterface& sheet) const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

};