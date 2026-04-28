#pragma once

// RAII wrappers around the MySQL C client handles.
//
// Why: every callsite that does `MYSQL_RES *r = mysql_read(...); ... mysql_free_result(r);`
// has the same shape, and forgetting to free on an early-return path is a
// common bug in this codebase's history. unique_ptr with a custom deleter ties
// the lifetime to scope and removes that whole bug class.
//
// Example:
//
//     auto res = bc::mysql::wrap_result(mysql_read(sql));
//     if (!res) return false;
//     while (auto *row = mysql_fetch_row(res.get())) { ... }
//     // freed automatically when `res` leaves scope

#include <mysql/mysql.h>

#include <memory>

namespace bc::mysql {

struct mysql_result_deleter {
    void operator()(MYSQL_RES *r) const noexcept {
        if (r) mysql_free_result(r);
    }
};

using result_ptr = std::unique_ptr<MYSQL_RES, mysql_result_deleter>;

inline result_ptr wrap_result(MYSQL_RES *raw) noexcept {
    return result_ptr(raw);
}

}  // namespace bc::mysql
