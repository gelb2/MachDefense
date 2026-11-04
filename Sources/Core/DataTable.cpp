#include "DataTable.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <iterator>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace md {
namespace {
std::string_view trim(std::string_view text) {
    const auto start = text.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) return {};
    return text.substr(start, text.find_last_not_of(" \t\r\n") - start + 1);
}

std::vector<std::string> fields(std::string_view line) {
    std::vector<std::string> result;
    while (true) {
        const auto end = line.find('\t');
        result.emplace_back(trim(line.substr(0, end)));
        if (end == std::string_view::npos) break;
        line.remove_prefix(end + 1);
    }
    return result;
}
} // namespace

DataTable DataTable::parse(std::string_view text) {
    DataTable table;
    if (text.starts_with("\xef\xbb\xbf")) text.remove_prefix(3);
    if (text.find('\0') != std::string_view::npos)
        throw std::runtime_error("table contains a NUL byte");
    std::size_t lineNumber = 0;
    while (!text.empty()) {
        ++lineNumber;
        const auto end = text.find('\n');
        const auto line = text.substr(0, end);
        if (end == std::string_view::npos) text = {};
        else text.remove_prefix(end + 1);
        if (trim(line).empty() || trim(line).starts_with("//")) continue;
        auto values = fields(line);
        if (table.columns_.empty()) {
            for (const auto& column : values) {
                if (column.empty() || std::find(table.columns_.begin(), table.columns_.end(), column) != table.columns_.end())
                    throw std::runtime_error("empty or duplicate table column");
                table.columns_.push_back(column);
            }
            continue;
        }
        // ESpawn_49 has an extra trailing tab on one line. Ignore only empty
        // surplus fields; never shift a populated field to conceal bad data.
        while (values.size() > table.columns_.size() && values.back().empty()) values.pop_back();
        if (values.size() != table.columns_.size())
            throw std::runtime_error("table field count mismatch at line " + std::to_string(lineNumber));
        table.rows_.push_back(std::move(values));
    }
    if (table.columns_.empty()) throw std::runtime_error("table header is missing");
    return table;
}

DataTable DataTable::load(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw std::runtime_error("cannot read table: " + path.string());
    const std::string text{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
    if (stream.bad()) throw std::runtime_error("failed reading table: " + path.string());
    return parse(text);
}

std::string_view DataTable::value(std::size_t row, std::string_view column) const {
    const auto found = std::find(columns_.begin(), columns_.end(), column);
    if (found == columns_.end()) throw std::out_of_range("unknown table column: " + std::string(column));
    return rows_.at(row).at(static_cast<std::size_t>(found - columns_.begin()));
}

std::optional<std::size_t> DataTable::findRow(std::string_view key) const noexcept {
    for (std::size_t i = 0; i < rows_.size(); ++i)
        if (rows_[i].front() == key) return i;
    return std::nullopt;
}

std::optional<std::int64_t> DataTable::integer(std::size_t row, std::string_view column) const {
    auto text = value(row, column);
    if (text.starts_with('+')) text.remove_prefix(1);
    if (text.empty()) return std::nullopt;
    std::int64_t result{};
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) return std::nullopt;
    return result;
}

std::optional<float> DataTable::number(std::size_t row, std::string_view column) const {
    const auto text = value(row, column);
    if (text.empty()) return std::nullopt;
    std::istringstream stream{std::string(text)};
    stream.imbue(std::locale::classic());
    float result{};
    stream >> std::noskipws >> result;
    if (!stream || !stream.eof() || !std::isfinite(result)) return std::nullopt;
    return result;
}
} // namespace md
