#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы
Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()),
sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
	std::unique_ptr<Impl> impl;
	if (text.empty()) {
		impl = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
		impl = std::make_unique<FormulaImpl>(text);
		
	}
	else {
		impl = std::make_unique<TextImpl>(text);	
	}

	if (HasCyclicDependencies(impl->GetReferencedCells(), this)) {
		throw CircularDependencyException("Cyclic dependency with cell: " + text);
	}

	for (auto cell : referenced_cells_) {
		cell->dependent_cells_.erase(this);
	}
	referenced_cells_.clear();

	for (auto pos : impl->GetReferencedCells()) {
		if (sheet_.GetCell(pos) == nullptr) {
			sheet_.SetCell(pos, "");
		}
		auto cell = dynamic_cast<Cell*>(sheet_.GetCell(pos));
		referenced_cells_.insert(cell);
		cell->dependent_cells_.insert(this);
	}

	ClearCache();

	impl_ = std::move(impl);
}

void Cell::Clear() {
	impl_ = nullptr;
}

Cell::Value Cell::GetValue() const {
	if (!cache_) {
		cache_ = impl_->GetValue(sheet_);
	}
	return *cache_;
}
std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

void Cell::ClearCache() {
	if (!cache_) {
		return;
	}
	cache_.reset();
	for (auto cell : dependent_cells_) {
		cell->ClearCache();
	}
}

bool Cell::HasCyclicDependencies(const std::vector<Position>& ref_cells, Cell* search_val) {
	for (auto ref_cell : ref_cells) {
		if (sheet_.GetCell(ref_cell) == nullptr) {
			sheet_.SetCell(ref_cell, "");
		}
		auto check_cell = dynamic_cast<Cell*>(sheet_.GetCell(ref_cell));
		if (check_cell == search_val) { return true; }
		if (check_cell->HasCyclicDependencies(check_cell->GetReferencedCells(), search_val)) {
			return true;
		}
	}
	return false;
}

//===============EmptyImpl================
Cell::Value Cell::EmptyImpl::GetValue(const SheetInterface& sheet) const {
	return "";
}

std::string Cell::EmptyImpl::GetText() const {
	return "";
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
	return {};
}

//===============TextImpl================
Cell::TextImpl::TextImpl(std::string text) : text_(text) {}

Cell::Value Cell::TextImpl::GetValue(const SheetInterface& sheet) const {
	if (text_[0] == ESCAPE_SIGN) {
		return text_.substr(1);
	}
	return text_;
}
std::string Cell::TextImpl::GetText() const {
	return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
	return {};
}

//===============FormulaImpl================
Cell::FormulaImpl::FormulaImpl(std::string text) {
	formula_ = ParseFormula(std::move(text.substr(1)));
}

Cell::Value Cell::FormulaImpl::GetValue(const SheetInterface& sheet) const {
	auto value = formula_->Evaluate(sheet);
	if (std::holds_alternative<double>(value)) {
		return std::get<double>(value);
	}
	else {
		return std::get<FormulaError>(value);
	}
}
std::string Cell::FormulaImpl::GetText() const {
	return FORMULA_SIGN + formula_->GetExpression();
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
	return formula_->GetReferencedCells();
}