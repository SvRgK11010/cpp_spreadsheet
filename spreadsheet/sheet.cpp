#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }
    if (!sheet_.count(pos)) {
        sheet_.emplace(pos, std::make_unique<Cell>(*this));
    }
    sheet_.at(pos)->Set(std::move(text));
    if (sheet_size_.cols < (pos.col + 1)) {
        sheet_size_.cols = pos.col + 1;
    }
    if (sheet_size_.rows < (pos.row + 1)) {
        sheet_size_.rows = pos.row + 1;
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }
    if (sheet_.count(pos)) {
        return sheet_.at(pos).get();
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }
    if (sheet_.count(pos)) {
        return sheet_.at(pos).get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid cell position");
    }
    if (!sheet_.count(pos))
    {
        return;
    }
    sheet_.erase(pos);
    if (sheet_size_.cols == pos.col + 1 || sheet_size_.rows == pos.row + 1) {
        sheet_size_ = { 0, 0 };
        for (const auto& [pos, cell] : sheet_) {
            sheet_size_.cols = std::max(sheet_size_.cols, pos.col+1);
            sheet_size_.rows = std::max(sheet_size_.rows, pos.row+1);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return sheet_size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < sheet_size_.rows; i++) {
        bool first = true;
        for (int j = 0; j < sheet_size_.cols; j++) {
            if (!first) {
                output << '\t';
            }
            first = false;
            auto pos = Position{ i, j };
            if (sheet_.count(pos)) {
                std::visit([&output](const auto& val) {output << val; }, sheet_.at(pos)->GetValue());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < sheet_size_.rows; i++) {
        bool first = true;
        for (int j = 0; j < sheet_size_.cols; j++) {
            if (!first) {
                output << '\t';
            }
            first = false;
            auto pos = Position{ i, j };
            if (sheet_.count(pos)) {
                output << sheet_.at(pos)->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}