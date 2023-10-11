#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell();
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:

    std::unique_ptr<Impl> impl_;
    SheetInterface& sheet_;
    mutable std::optional<Value> cache_;
    std::unordered_set<Cell*> referenced_cells_; //св€занные €чейки
    std::unordered_set<Cell*> dependent_cells_; //зависимые €чейки

    bool HasCyclicDependencies(const std::vector<Position>& ref_cells, const Cell& search_val);

    void ClearCache();

    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;
        virtual ~Impl() = default;
    };

    class EmptyImpl : public Impl {
    public:
        EmptyImpl() = default;
        ~EmptyImpl() = default;
        Value GetValue() const override;
        std::string GetText() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text);
        Value GetValue() const override;
        std::string GetText() const override;

    private:
        std::string text_;
    };
    
    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string formula);
        Value GetValue() const override;
        std::string GetText() const override;

        std::vector<Position> GetReferencedCells() const override;

    private:
        std::unique_ptr<FormulaInterface> formula_;
    };
   
};