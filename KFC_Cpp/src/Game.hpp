#pragma once

#include "Board.hpp"
#include "PieceFactory.hpp"
#include <memory>
#include <vector>
#include <stdexcept>
#include <unordered_set>
#include "img/Img.hpp"
#include "img/ImgFactory.hpp"
#include <fstream>
#include <sstream>
#include "GraphicsFactory.hpp"
#include "Common.hpp"
#include <chrono>
#include <thread>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <cctype>
#include <filesystem>

class InvalidBoard : public std::runtime_error {
public:
    explicit InvalidBoard(const std::string& msg) : std::runtime_error(msg) {}
};

class Game {
public:
    Game(std::vector<PiecePtr> pcs, Board board);

    // --- main public API ---
    int game_time_ms() const;
    // for debug:
    void setTimeFactor(int factor);

    Board clone_board() const;


    // Mirror Python run() behaviour
    void run(int num_iterations = -1, bool is_with_graphics = true);

    std::vector<PiecePtr> pieces;
    Board board;
    // helper for tests to inject commands
    void enqueue_command(const Command& cmd);
private:
    // --- helpers mirroring Python implementation ---
    void start_user_input_thread(); // no-op stub for now
    void run_game_loop(int num_iterations, bool is_with_graphics);
    void update_cell2piece_map();
    void process_input(const Command& cmd);
    void resolve_collisions();
    void announce_win() const;

    void validate();
    bool is_win() const;

    std::unordered_map<std::string, PiecePtr> piece_by_id;
    // Map from board cell to list of occupying pieces
    std::unordered_map<std::pair<int,int>, std::vector<PiecePtr>, PairHash> pos;
    std::vector<Command> user_input_queue;

    std::chrono::steady_clock::time_point start_tp;
    int _timeFactor;
};

// ---------------- Implementation inline --------------------
inline Game::Game(std::vector<PiecePtr> pcs, Board board)
    : pieces(pcs), board(board), _timeFactor(1) {
    validate();
    for(const auto & p : pieces) piece_by_id[p->id] = p;
    start_tp = std::chrono::steady_clock::now();
}

inline int Game::game_time_ms() const {
    return _timeFactor * static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_tp).count());
}

inline void Game::setTimeFactor(int factor) {
    if (factor <= 0)
        throw std::runtime_error("Invalid factor");
    _timeFactor = factor;
}

inline Board Game::clone_board() const {
    return board.clone();
}

inline void Game::run(int num_iterations, bool is_with_graphics) {
    start_user_input_thread();
    int start_ms = game_time_ms();
    for(auto & p : pieces) p->reset(start_ms);

    run_game_loop(num_iterations, is_with_graphics);

    announce_win();
}

inline void Game::start_user_input_thread() {
    // Stub – interactive input not yet implemented in C++ port.
}

inline void Game::run_game_loop(int num_iterations, bool is_with_graphics) {
    int it_counter = 0;
    while(!is_win()) {
        int now = game_time_ms();
        for(auto & p : pieces) p->update(now);

        update_cell2piece_map();

        while(!user_input_queue.empty()) {
            auto cmd = user_input_queue.back();
            user_input_queue.pop_back();
            process_input(cmd);
        }

        if(is_with_graphics) {
            board.show();
        }

        resolve_collisions();

        if(num_iterations >= 0) {
            ++it_counter;
            if(it_counter >= num_iterations) return;
        }
        /* idle to mimic frame pacing */
    }
}

inline void Game::update_cell2piece_map() {
    pos.clear();
    for(const auto & p : pieces) {
        pos[p->current_cell()].push_back(p);
    }
}

inline void Game::process_input(const Command & cmd) {
    auto it = piece_by_id.find(cmd.piece_id);
    if(it == piece_by_id.end()) return;
    it->second->on_command(cmd, pos);
}

inline void Game::resolve_collisions() {
    update_cell2piece_map();
    for(const auto & kv : pos) {
        const auto & plist = kv.second;
        if(plist.size() < 2) continue;
        auto winner = *std::max_element(plist.begin(), plist.end(),
            [](const PiecePtr & a, const PiecePtr & b){
                return a->state->physics->get_start_ms() < b->state->physics->get_start_ms();
            });
        for(const auto & p : plist) {
            if(p == winner) continue;
            if(p->state->can_be_captured()) {
                pieces.erase(std::remove(pieces.begin(), pieces.end(), p), pieces.end());
            }
        }
    }
}

inline void Game::announce_win() const {
    bool black_alive = std::any_of(pieces.begin(), pieces.end(), [](const PiecePtr & p){return p->id.rfind("KB",0)==0;});
}

inline bool Game::is_win() const {
    int kings = 0;
    for(const auto& p : pieces) {
        if(p->id.rfind("KW",0)==0 || p->id.rfind("KB",0)==0) ++kings;
    }
    return kings < 2;
}

inline void Game::validate() {
    bool has_KW=false, has_KB=false;
    std::unordered_set<std::pair<int,int>, PairHash> seen;
    for(const auto& p: pieces) {
        if(p->id.rfind("KW",0) == 0) has_KW=true;
        if(p->id.rfind("KB",0) == 0) has_KB=true;
        auto cell = p->current_cell();
        if(!seen.insert(cell).second) throw InvalidBoard("Duplicate cell");
    }
    if(!has_KW || !has_KB) throw InvalidBoard("Missing kings");
}

inline void Game::enqueue_command(const Command& cmd) {
    user_input_queue.push_back(cmd);
}

// ---------------------------------------------------------------------------
// Helper to read board.csv and create a full game
// ---------------------------------------------------------------------------
inline Game create_game(std::string pieces_root,
                        const ImgFactoryPtr& img_factory) {

    // Trim whitespace that might appear accidentally in test paths
    pieces_root.erase(std::remove_if(pieces_root.begin(), pieces_root.end(), ::isspace), pieces_root.end());
    
    GraphicsFactory gfx_factory(img_factory);

    fs::path root = fs::absolute(fs::path(pieces_root));
    fs::path board_csv = root / "board.csv";
    std::ifstream in(board_csv);
    if(!in) throw std::runtime_error("Cannot open board.csv");

    fs::path board_png = root / "board.png";
    auto board_img = img_factory->load(board_png.string());
    Board board(32,32,8,8, board_img);

    PieceFactory pf(board, root.string(), gfx_factory);
    std::vector<PiecePtr> out;

    std::string line; int row=0;
    while(std::getline(in,line)) {
        std::stringstream ss(line);
        std::string cell; int col=0;
        while(std::getline(ss,cell,',')) {
            if(!cell.empty()) {
                auto piece = pf.create_piece(cell, {row,col});
                out.push_back(piece);
            }
            ++col;
        }
        ++row;
    }
    return Game(out, board);
} 