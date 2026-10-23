#include "bench.h"
#include "uci.h"
#include "misc.h"
#include "types.h"
#include "perft.h"
#include "search.h"
#include "time_manager.h"
#include "ttable.h"
#include "init.h"
#include "io.h"
#include "threads.h"
#include "position.h"
#include "movegen.h"
#include <atomic>
#include <iostream>
#include "tune.h"
#include "eval.h"

// Parse a move from algebraic notation to the engine's internal encoding
Move ParseMove(const std::string& moveString, Position* pos) {
    // create move list instance
    MoveList moveList;

    // generate moves
    GenerateMoves(&moveList, pos, MOVEGEN_ALL);

    // parse source square
    const unsigned int sourceSquare = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;

    // parse target square
    const unsigned int targetSquare = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;

    // loop over the moves within a move list
    for (int move_count = 0; move_count < moveList.count; move_count++) {
        // init move
        const Move move = moveList.moves[move_count].move;
        // make sure source & target squares are available within the generated move
        const bool isChess960Castle = pos->isChess960() && isCastle(move)
            && targetSquare == pos->getCastlingRookSquare(GetMovetype(move) == static_cast<uint>(Movetype::KSCastle)
                ? pos->side == WHITE ? WKCA : BKCA
                : pos->side == WHITE ? WQCA : BQCA);
        if (sourceSquare == From(move) &&
            (targetSquare == To(move) || isChess960Castle)) {
            // promoted piece is available
            if (isPromo(move)) {
                switch (getPromotedPiecetype(move)) {
                case QUEEN:
                    if (moveString[4] == 'q')
                        return move;
                    break;
                case ROOK:
                    if (moveString[4] == 'r')
                    return move;
                    break;
                case BISHOP:
                    if (moveString[4] == 'b')
                    return move;
                    break;
                case KNIGHT:
                    if (moveString[4] == 'n')
                    return move;
                    break;
                }
                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }
            // return legal move
            return move;
        }
    }
    std::cout << "Illegal move parsed: " << moveString;
    // return illegal move
    return NOMOVE;
}

// parse UCI "position" command
void ParsePosition(const std::string& command, Position* pos, std::vector<ZobristKey>& keyHistory, const bool chess960) {
    // parse UCI "startpos" command
    if (command.find("startpos") != std::string::npos) {
        // init chess board with start position
        ParseFen(start_position, pos, chess960);
        keyHistory.clear();
    }

    // parse UCI "fen" command
    else {
        // if a "fen" command is available within command string
        if (command.find("fen") != std::string::npos) {
            // Substring from after "fen" up to "moves"
            std::string position = getPosition(command);
            ParseFen(position, pos, chess960);
            keyHistory.clear();
        }
        else {
            // init chess board with start position
            ParseFen(start_position, pos, chess960);
            keyHistory.clear();
        }
    }

    // if there are moves to be played in the fen play them
    if (command.find("moves") != std::string::npos) {
        auto string_start = command.find("moves") + 6;
        // Avoid looking for a moves that doesn't exist in the case of "position startpos moves <blank>" (Needed for Scid support)
        if (command.length() >= string_start) {
            std::string moves_substr = command.substr(string_start, std::string::npos);
            parse_moves(moves_substr, pos, keyHistory);
        }
    }
}

// parse UCI "go" command, returns true if we have to search afterwards and false otherwise
bool ParseGo(const std::string& line, SearchInfo* info, Position* pos, std::vector<ZobristKey>& keyHistory) {
    info->Reset();
    int depth = -1, movetime = -1;
    int movestogo;
    int time = -1, inc = 0;

    std::vector<std::string> tokens = split_command(line);

    if (tokens.size() == 3 && tokens[1] == "perft") {
        PerftTest(std::stoi(tokens[2]), pos, keyHistory);
        return false;
    }

    // loop over all the tokens and parse the commands
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens.at(1) == "infinite") {
            ;
        }

        if (tokens.at(i) == "binc" && pos->side == BLACK) {
            inc = std::stoi(tokens[i + 1]);
        }

        if (tokens.at(i) == "winc" && pos->side == WHITE) {
            inc = std::stoi(tokens[i + 1]);
        }

        if (tokens.at(i) == "wtime" && pos->side == WHITE) {
            time = std::stoi(tokens[i + 1]);
            info->timeset = true;
        }
        if (tokens.at(i) == "btime" && pos->side == BLACK) {
            time = std::stoi(tokens[i + 1]);
            info->timeset = true;
        }

        if (tokens.at(i) == "movestogo") {
            movestogo = std::stoi(tokens[i + 1]);
            if (movestogo > 0)
                info->movestogo = movestogo;
        }

        if (tokens.at(i) == "movetime") {
            movetime = std::stoi(tokens[i + 1]);
            time = movetime;
            info->movetimeset = true;
        }

        if (tokens.at(i) == "depth") {
            depth = std::stoi(tokens[i + 1]);
        }

        if (tokens.at(i) == "nodes") {
            std::cout << "Nodes limit is not supported, ignoring it" << std::endl;
        }
    }

    info->starttime = GetTimeMs();
    info->depth = depth;
    // calculate time allocation for the move
    Optimum(info, time, inc);

    if (depth == -1) {
        info->depth = MAXDEPTH;
    }
    return true;
}

namespace {
class RootSearchWorker {
public:
    RootSearchWorker() : worker([this] { Run(); }) {}

    void Start(const int depth, ThreadData* td, UciOptions* options) {
        searchDepth = depth;
        threadData = td;
        uciOptions = options;
        taskGeneration.fetch_add(1, std::memory_order_release);
        taskGeneration.notify_one();
    }

    void Wait() {
        const uint64_t targetGeneration = taskGeneration.load(std::memory_order_acquire);
        uint64_t completed = completedGeneration.load(std::memory_order_acquire);
        while (completed < targetGeneration) {
            completedGeneration.wait(completed, std::memory_order_acquire);
            completed = completedGeneration.load(std::memory_order_acquire);
        }
    }

    void Stop(ThreadData* td) {
        td->stopped = true;
        StopHelperThreads();
        Wait();
    }

    void Shutdown() {
        Wait();
        shuttingDown.store(true, std::memory_order_release);
        taskGeneration.fetch_add(1, std::memory_order_release);
        taskGeneration.notify_one();
        if (worker.joinable())
            worker.join();
    }

private:
    void Run() {
        uint64_t workerGeneration = 0;
        while (true) {
            taskGeneration.wait(workerGeneration, std::memory_order_acquire);
            workerGeneration = taskGeneration.load(std::memory_order_acquire);
            if (shuttingDown.load(std::memory_order_acquire))
                return;

            const int depth = searchDepth;
            ThreadData* td = threadData;
            UciOptions* options = uciOptions;

            RootSearch(depth, td, options);

            completedGeneration.store(workerGeneration, std::memory_order_release);
            completedGeneration.notify_all();
        }
    }

    std::thread worker;
    std::atomic<uint64_t> taskGeneration = 0;
    std::atomic<uint64_t> completedGeneration = 0;
    std::atomic<bool> shuttingDown = false;
    ThreadData* threadData = nullptr;
    UciOptions* uciOptions = nullptr;
    int searchDepth = 0;
};
}

// main UCI loop
void UciLoop(int argc, char** argv) {
    if (argv[1] && strncmp(argv[1], "bench", 5) == 0) {
        int benchDepth = 14;
        // If there's an additional input try to parse it as a bench depth
        if (argc == 3) {
            if (std::stoi(argv[2]) > 0) {
                benchDepth = std::stoi(argv[2]);
            }
            else {
                std::cout << "Invalid bench depth";
                return;
            }
        }
        tryhardmode = true;
        StartBench(benchDepth);
        ShutdownHelperThreads();
        return;
    }

    bool parsed_position = false;
    UciOptions uciOptions;
    ThreadData* td(new ThreadData());
    RootSearchWorker searchWorker;
    state threads_state = Idle;
    // print engine info
    std::cout << NAME << "\n";

#ifdef TUNE
    // spsa info dump
    for (const auto &param: tunables()) {
        std::cout << param << std::endl;
    }
#endif

    // main loop
    while (true) {
        // define user / GUI input buffer
        std::string input;

        // get user / GUI input
        if (!std::getline(std::cin, input)) {
            // continue the loop
            break;
        }

        // make sure input is available
        if (input.empty()) {
            // continue the loop
            continue;
        }

        // Split the string into tokens to make it easier to work with
        std::vector<std::string> tokens = split_command(input);

        // parse UCI "position" command
        if (tokens[0] == "position") {
            // call parse position function
            ParsePosition(input, &td->pos, td->keyHistory, uciOptions.chess960);
            parsed_position = true;
        }

        // parse UCI "go" command
        else if (tokens[0] == "go") {
            searchWorker.Stop(td);
            threads_state = Idle;
            if (!parsed_position) { // call parse position function
                ParsePosition("position startpos", &td->pos, td->keyHistory, uciOptions.chess960);
            }
            // call parse go function
            td->stopped = false;
            bool search = ParseGo(input, &searchinfo, &td->pos, td->keyHistory);
            // Start search in a separate thread
            if (search) {
                threads_state = Search;
                searchWorker.Start(searchinfo.depth, td, &uciOptions);
            }
        }

        else if (tokens[0] == "setoption") {
            // check tokens for size to see if we have a value
            if (tokens.size() < 5) {
                std::cout << "Invalid setoption format" << std::endl;
                continue;
            }
            if (tokens.at(2) == "Hash") {
                uciOptions.Hash = std::stoi(tokens.at(4));
                std::cout << "Set Hash to " << uciOptions.Hash << " MB" << std::endl;
                InitTT(uciOptions.Hash);
            }
            else if (tokens.at(2) == "Threads") {
                uciOptions.Threads = std::stoi(tokens.at(4));
                std::cout << "Set Threads to " << uciOptions.Threads << std::endl;;
            }
            else if (tokens.at(2) == "Minimal") {
                auto value = tokens.at(4) == "true";
                uciOptions.shortUci = value;
            }
            else if (tokens.at(2) == "UCI_Chess960") {
                uciOptions.chess960 = tokens.at(4) == "true";
            }

#ifdef TUNE
            else {
                updateTuneVariable(tokens.at(2), std::stoi(tokens.at(4)));
        }
#else
            else std::cout << "Unknown command: " << input << std::endl;
#endif
        }

        // parse UCI "isready" command
        else if (input == "isready") {
            std::cout << "readyok"<< std::endl;
        }

        // parse UCI "ucinewgame" command
        else if (input == "ucinewgame") {
            if (threads_state == Search)
                searchWorker.Stop(td);
            threads_state = Idle;
            InitNewGame(td);
        }
        // parse UCI "stop" command
        else if (input == "stop") {
            if (threads_state == Search) {
                searchWorker.Stop(td);
            }
            threads_state = Idle;
        }

        // parse UCI "position" command
        else if (input == "wait") {
            searchWorker.Wait();
        }

        // parse UCI "quit" command
        else if (input == "quit" || input == "exit") {
            if (threads_state == Search) {
                searchWorker.Stop(td);
            }
            threads_state = Idle;
            // quit from the chess engine program execution
            break;
        }

            // parse UCI "uci" command
        else if (input == "uci") {
            // print engine info
            std::cout << "id name " << NAME << "\n";
            std::cout << "id author Zuppa, CJ and Contributors\n";
            std::cout << "option name Hash type spin default 16 min 1 max 262144 \n";
            std::cout << "option name Threads type spin default 1 min 1 max 256 \n";
            std::cout << "option name Minimal type check default false \n";
            std::cout << "option name UCI_Chess960 type check default false \n";
#ifdef TUNE
            // spsa info dump
            for (const auto &param: tunables()) {
                std::cout << "option name " << param.name << " type spin default " << param.defaultValue <<" min " <<param.minValue <<" max " << param.maxValue << std::endl;
            }
#endif
            std::cout << "uciok" << std::endl;
            // Set uci compatible output mode
            print_uci = true;
        }

            // print board
        else if (input == "d") {
            PrintBoard(&td->pos);
        }

        else if (input == "eval") {// call parse position function
            if (!parsed_position) {
                ParsePosition("position startpos", &td->pos, td->keyHistory, uciOptions.chess960);
            }
            std::cout << "Raw eval: " << EvalPositionRaw(&td->pos, &td->FTable) << std::endl;

            std::cout << "Scaled eval: " << adjustEval(&td->pos, 0, EvalPosition(&td->pos, &td->FTable)) << std::endl;
        }

        else if (input == "bench") {
            tryhardmode = true;
            StartBench();
            tryhardmode = false;
        }

        else if (input == "see") {
            // create move list instance
            MoveList moveList;

            // generate moves
            GenerateMoves(&moveList, &td->pos, MOVEGEN_ALL);
            printf("SEE thresholds\n");
            for (int i = 0; i < moveList.count; i++) {
                Move move = moveList.moves[i].move;
                for (int j = 1200; j > -1200; j--) {
                    if (SEE(&td->pos, move, j)) {
                        printf(" move number %d  %s SEE result: %d \n", i + 1, FormatMove(move), j);
                        break;
                    }
                }
            }
        }
        else std::cout << "Unknown command: " << input << std::endl;
    }

    if (threads_state == Search) {
        searchWorker.Stop(td);
    }
    searchWorker.Shutdown();
    ShutdownHelperThreads();
    delete td;
}
