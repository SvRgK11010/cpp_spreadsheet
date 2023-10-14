#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {}
    catch (...) {
        throw FormulaException("incorrect formula");
    }
    
    Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute([&sheet](Position pos) {
                auto cell = sheet.GetCell(pos);
                if (cell == nullptr) {
                    return 0.0;
                }
                auto value = cell->GetValue();
                if (std::holds_alternative<double>(value)) {
                    return std::get<double>(value);
                } 
                else if (std::holds_alternative<std::string>(value)) {
                    std::string text_val = std::get<std::string>(value);
                    if (text_val.empty()) {
                        return 0.0;
                    }
                    std::istringstream input(text_val);
                    double val;
                    input >> val;
                    if (input.eof()) {
                        return val;
                    }
                    throw FormulaError(FormulaError::Category::Value);
                }
                throw FormulaError(std::get<FormulaError>(value));
                });
        }
        catch (FormulaError& error) {
            return error;
        }
    }
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> GetReferencedCells() const {
        std::forward_list<Position> cells = ast_.GetCells();
        std::vector<Position> referenced_cells = { cells.begin(), cells.end() };
        referenced_cells.erase(std::unique(
            referenced_cells.begin(), 
            referenced_cells.end()),
            referenced_cells.end()
        );
        return referenced_cells;
    }

private:
    FormulaAST ast_;

};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}