#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace md {

// Prepared original TSV resources. The first field is a lookup key, but keys
// may repeat (spawn tables). No game defaults or balancing rules live here.
class DataTable {
public:
    static DataTable parse(std::string_view text);
    static DataTable load(const std::filesystem::path& path);

    [[nodiscard]] std::size_t rowCount() const noexcept { return rows_.size(); }
    [[nodiscard]] const std::vector<std::string>& columns() const noexcept { return columns_; }
    [[nodiscard]] const std::vector<std::vector<std::string>>& rows() const noexcept { return rows_; }
    [[nodiscard]] std::string_view value(std::size_t row, std::string_view column) const;
    [[nodiscard]] std::optional<std::size_t> findRow(std::string_view key) const noexcept;
    [[nodiscard]] std::optional<std::int64_t> integer(std::size_t row, std::string_view column) const;
    [[nodiscard]] std::optional<float> number(std::size_t row, std::string_view column) const;

private:
    std::vector<std::string> columns_;
    std::vector<std::vector<std::string>> rows_;
};

} // namespace md
