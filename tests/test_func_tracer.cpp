#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include <string>
#include <set>


bool isBlacklisted(const std::string &func_name)
{
    if (func_name.length() >= 4 && func_name.compare(func_name.length() - 4, 4, "@plt") == 0) return true; // Skip PLT entries
    if (func_name.length() >= 2 && func_name.compare(0, 2, "__") == 0) return true; // Skip internal functions starting with "__"
    // Check if the function name is in the blacklist
    static const std::set<std::string> blacklist = {
        "main", "_init", "_start"
    };
    return blacklist.find(func_name) != blacklist.end();
}


TEST_CASE("isBlacklisted works as expected") {
    SECTION("PLT functions are blacklisted") {
        REQUIRE(isBlacklisted("foo@plt"));
        REQUIRE(isBlacklisted("bar@plt"));
    }
    SECTION("Functions starting with __ are blacklisted") {
        REQUIRE(isBlacklisted("__internal"));
        REQUIRE(isBlacklisted("__something"));
    }
    SECTION("Explicit blacklist") {
        REQUIRE(isBlacklisted("main"));
        REQUIRE(isBlacklisted("_init"));
        REQUIRE(isBlacklisted("_start"));
    }
    SECTION("Normal functions are not blacklisted") {
        REQUIRE_FALSE(isBlacklisted("foo"));
        REQUIRE_FALSE(isBlacklisted("bar"));
        REQUIRE_FALSE(isBlacklisted("baz"));
    }
    SECTION("Short names are not blacklisted") {
        REQUIRE_FALSE(isBlacklisted("a"));
        REQUIRE_FALSE(isBlacklisted("b@p"));
        REQUIRE_FALSE(isBlacklisted("_m"));
    }
}