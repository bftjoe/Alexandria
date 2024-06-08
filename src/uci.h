#pragma once

#include <cstdint>
#include <string>

struct Position;
struct SearchInfo;

// Internal flag to decide if to pretty or ugly print search results
constexpr bool print_uci = true;
// Internal flag to disable the output of search results when we don't want our speed to be limited by the console
constexpr bool tryhardmode = true;
// Parse a move from algebraic notation to the internal value
[[nodiscard]] int ParseMove(const std::string& move_string, Position* pos);
// parse UCI "position" command
void ParsePosition(const std::string& command, Position* pos);

// parse UCI "go" command
[[nodiscard]] bool ParseGo(const std::string& line, SearchInfo* info, Position* pos);

// main UCI loop
void UciLoop(int argc, char** argv);
