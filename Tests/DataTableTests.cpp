#include "Core/Data/DataTable.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class Operation> void rejects(Operation operation, const char* message) {
    bool rejected = false;
    try { operation(); } catch (const std::exception&) { rejected = true; }
    check(rejected, message);
}
} // namespace

int main(int argc, char** argv) {
    try {
        const auto table = md::DataTable::parse("\xef\xbb\xbf// ignored\r\n Key \tAmount\tSpeed\r\n A \t+42 \t0.16\r\n B\t-\t-1.25\n// trailing comment\n");
        check(table.rowCount() == 2 && table.columns().size() == 3, "comments/CRLF/BOM");
        check(table.findRow("A") == 0 && !table.findRow("absent"), "key lookup");
        check(table.integer(0, "Amount") == 42 && !table.integer(1, "Amount"), "integer or explicit missing number");
        check(std::abs(table.number(0, "Speed").value() - 0.16f) < 0.00001f, "decimal parsing");
        check(table.number(1, "Speed") == -1.25f, "signed decimal parsing");
        rejects([&] { (void)table.value(10, "Key"); }, "row bounds");
        rejects([&] { (void)table.value(0, "unknown"); }, "column bounds");
        rejects([] { md::DataTable::parse("A\tA\nx\ty"); }, "duplicate column rejected");
        rejects([] { md::DataTable::parse("A\tB\nx\ty\tz"); }, "populated surplus field rejected");
        rejects([] { md::DataTable::parse("A\tB\nx"); }, "missing field rejected");
        check(md::DataTable::parse("A\tB\nx\ty\t\n").value(0, "B") == "y", "original trailing tab accepted");
        const auto numeric = md::DataTable::parse("Key\tValue\noverflow\t9999999999999999999999999999\ntrailing\t12x\nfloat\t1e999\n");
        check(!numeric.integer(0, "Value") && !numeric.integer(1, "Value") && !numeric.number(2, "Value"), "invalid numeric values rejected");
        if (argc != 2) throw std::runtime_error("usage: DataTableTests Resources/Tables");
        const std::filesystem::path root(argv[1]);
        std::size_t count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(root)) {
            if (entry.path().extension() == ".tsv") { md::DataTable::load(entry.path()); ++count; }
        }
        check(count == 94, "all 94 unique tables parsed (95 source files; identical MachBuild_Parts alias)");
        const auto maps = md::DataTable::load(root / "MapList.tsv");
        check(maps.rowCount() == 80 && maps.value(0, "ImageFile") == "BG_01" && maps.value(79, "ImageFile") == "BG_04", "80 original missions");
        const auto weapons = md::DataTable::load(root / "WeaponInfo.tsv");
        check(weapons.rowCount() == 357, "211 original weapons plus 56 level extension rows and 90 aircraft rows");
        const auto upgrades = md::DataTable::load(root / "BaseUpgrade.tsv");
        check(upgrades.rowCount() == 60, "54 original base upgrade rows plus 6 level extension rows");
        const auto cannon = upgrades.findRow("BaseCannon_01").value();
        check(upgrades.integer(cannon, "UpgCost") == 600, "original base cannon cost");
        const auto capacity = upgrades.findRow("CellStorage_02").value();
        check(upgrades.integer(capacity, "Param") == 700, "level-two energy capacity matches port observation");
        const auto builds = md::DataTable::load(root / "MachBuild_Parts.tsv");
        const auto first = builds.findRow("MachineGun_01").value();
        check(builds.integer(first, "Cost") == 60 && builds.integer(first, "Armor") == 11001, "original build/parts binding");
        const auto english = md::DataTable::load(root / "Description_us.tsv");
        check(english.value(english.findRow("NotEnoughCr").value(), "Value") == "Not enough Cr.", "English original description");
        std::cout << "PASS DataTable: parser boundaries, 94 unique tables (95 sources), 80 missions, weapon/upgrade/build/English values\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL DataTable: " << error.what() << '\n';
        return 1;
    }
}
