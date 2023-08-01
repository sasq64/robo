#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <cwchar>

#include "utf8.h"

#include "ansi_protocol.h"
#include "terminal.h"

namespace bbs {

inline bool is_wide(char32_t c)
{
    static std::unordered_set<char32_t> const wide{
        0x1fa78, 0x1f463, 0x1f311, 0x1f3f9, 0x1f4b0, 0x1f480, 0x274c};
    return wide.contains(c);
    // return c > 0xffff;
}

template <typename Protocol = AnsiProtocol>
class Console
{
public:
    enum AnsiColors
    {
        WHITE,
        RED,
        GREEN,
        BLUE,
        ORANGE,
        BLACK,
        BROWN,
        PINK,
        DARK_GREY,
        GREY,
        LIGHT_GREEN,
        LIGHT_BLUE,
        LIGHT_GREY,
        PURPLE,
        YELLOW,
        CYAN,
        CURRENT_COLOR = -2, // Use the currently set fg or bg color
        NO_COLOR = -1
    };

    std::unique_ptr<Terminal> terminal;

    explicit Console(std::unique_ptr<Terminal> terminal_) :
          terminal(std::move(terminal_)),
          cur_bg(0x00000000),
          cur_fg(0xc0c0c007)
    {
        put_fg = cur_fg;
        put_bg = cur_bg;
        write(Protocol::init());
        write(Protocol::set_color(cur_fg, cur_bg));
        write(Protocol::goto_xy(0, 0));
        write(Protocol::clear());
        write(Protocol::show_cursor(false));
        resize(terminal->width(), terminal->height());
    }

    ~Console()
    {
        if (terminal != nullptr) {
            write(Protocol::exit());
        }
    }

    using ColorIndex = uint16_t;
    using Char = char32_t;

    std::vector<uint32_t> palette;

    struct Tile
    {
        Char c = 0x20;
        uint32_t fg = 0;
        uint32_t bg = 0;
        uint16_t flags = 0;

        bool operator==(Tile const& other) const
        {
            return (other.c == c && other.fg == fg && other.bg == bg &&
                    other.flags == flags);
        }

        bool operator!=(Tile const& other) const { return !operator==(other); }
    };

    std::vector<Tile> grid;
    std::vector<Tile> old_grid;

    int32_t width = 0;
    int32_t height = 0;

    int32_t put_x = 0;
    int32_t put_y = 0;
    uint32_t put_fg = 0;
    uint32_t put_bg = 0;

    int32_t cur_x = 0;
    int32_t cur_y = 0;
    uint32_t cur_fg = 0;
    uint32_t cur_bg = 0;

    void resize(int32_t w, int32_t h)
    {
        width = w;
        height = h;
        grid.resize(w * h);
        old_grid.resize(w * h);
        std::fill(grid.begin(), grid.end(), Tile{' ', 0, 0, 0});
        std::fill(old_grid.begin(), old_grid.end(), Tile{' ', 0, 0, 0});
    }

    void blit(
        int32_t x, int32_t y, int32_t stride, std::vector<Tile> const& from)
    {
        int32_t i = 0;
        auto xx = x;
        for (auto const& c : from) {
            if (xx < width && y < height) {
                grid[xx + width * y] = c;
            }
            xx++;
            if (++i == stride) {
                xx = x;
                y++;
                i = 0;
            }
        }
    }

    void fill(uint32_t fg, uint32_t bg)
    {
        std::fill(grid.begin(), grid.end(), Tile{' ', fg, bg, 0});
        //utils::fill(grid, Tile{' ', fg, bg, 0});
    }

    void set_xy(int32_t x, int32_t y)
    {
        put_x = x;
        put_y = y;
    }

    void set_color(uint32_t fg, uint32_t bg)
    {
        put_fg = fg;
        put_bg = bg;
    }

    void put(std::string const& text)
    {
        auto ut = utils::utf8_decode(text);
        for (auto c : ut) {
            grid[put_x + width * put_y] = {
                static_cast<Char>(c), put_fg, put_bg, 0};
            put_x++;
        }
    }

    void put_char(int x, int y, Char c) { grid[x + width * y].c = c; }

    void put_char(int x, int y, Char c, uint16_t flg)
    {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        grid[x + width * y].c = c;
        grid[x + width * y].flags = flg;
    }

    void put_color(int x, int y, uint32_t fg, uint32_t bg)
    {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        grid[x + width * y].fg = fg;
        grid[x + width * y].bg = bg;
    }

    void put_color(int x, int y, uint32_t fg, uint32_t bg, uint16_t flg)
    {
        if (x < 0 || y < 0 || x >= width || y >= height) return;
        grid[x + width * y].fg = fg;
        grid[x + width * y].bg = bg;
        grid[x + width * y].flags = flg;
    }

    Tile& at(int x, int y) { return grid[x + width * y]; }

    Char get_char(int x, int y) { return grid[x + width * y].c; }

    auto get_width() const { return width; }
    auto get_height() const { return height; }

    void write(std::string_view text) const
    {
        if (terminal != nullptr) {
            terminal->write(text);
        } else {
            std::cout << text;
            //fmt::print(text);
        }
    }

    int32_t read_key() const
    {
        std::string target;
        if (terminal->read(target)) {
            std::string_view const s = target;
            return Protocol::translate_key(s);
        }
        return 0;
    }

    void flush()
    {
        using namespace std::string_literals;
        int chars = 0;
        int xy = 0;
        bool skip_next = false;
        cur_x = cur_y = -1;
        for (int32_t y = 0; y < height; y++) {
            write(Protocol::goto_xy(0, y));
            skip_next = false;
            for (int32_t x = 0; x < width; x++) {
                auto& t0 = old_grid[x + y * width];
                auto const& t1 = grid[x + y * width];
                if(skip_next) {
                    t0 = t1;
                    skip_next = false;
                }
                if (t0 != t1) {
                    if (cur_y != y || cur_x != x) {
                        write(Protocol::goto_xy(x, y));
                        xy++;
                        cur_x = x;
                        cur_y = y;
                    }
                    write((t1.flags & 1) != 1
                              ? Protocol::set_color(t1.fg, t1.bg)
                              : Protocol::set_color(t1.bg, t1.fg));
                    terminal->write(utils::utf8_encode({t1.c}));
                    bool wide = is_wide(t1.c);
                    cur_x++;
                    if (wide) {
                        cur_x++;
                        skip_next = true;
                    }
                    chars++;
                    t0 = t1;
                }
            }
        }
        /* if (chars != 0) { */
        /*     static int counter = 0; */
        /*     counter++; */
        /*     write(Protocol::goto_xy(60, 0)); */
        /*     write(Protocol::set_color(0xffffff00, 0xff000000)); */
        /*     write("  ["s + std::to_string(counter) + ":"s +
         * std::to_string(chars) + "]  "s); */
        /* } */

        fflush(stdout);
    }

    void printAll()
    {
        int chars = 0;
        cur_x = cur_y = -1;
        for (int32_t y = 0; y < height; y++) {
            for (int32_t x = 0; x < width; x++) {
                auto const& t1 = grid[x + y * width];
                write((t1.flags & 1) != 1 ? Protocol::set_color(t1.fg, t1.bg)
                                          : Protocol::set_color(t1.bg, t1.fg));
                write(utils::utf8_encode({t1.c}));
                chars++;
            }
            putchar(10);
        }
        printf("\e[0m");
    }
};

} // namespace bbs
