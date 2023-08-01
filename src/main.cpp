#include <memory>

#include <ansi/console.h>
#include <ansi/terminal.h>

//#include <full_console.hpp>
//#include <tile_set.hpp>

#include <mrb/class.hpp>
#include <mrb/conv.hpp>
#include <mrb/get_args.hpp>

std::shared_ptr<bbs::Console<>> con;

struct Game
{
    Game() {
        con->put("Game created");
        con->flush();
    }
};

struct DummyTerminal : bbs::Terminal
{
    DummyTerminal() = default;

    size_t write(std::string_view source) override
    {
        return source.length();
    }

    bool read(std::string& target) override
    {
        return false;
    }

    int width() const override { return 80; }
    int height() const override { return 25; }

    ~DummyTerminal() override = default;
};

#if 0

std::shared_ptr<System> sys;

void init()
{
#ifdef RASPBERRY_PI
    sys = create_pi_system();
#else
    sys = create_glfw_system();
#endif

    Screen::Settings const settings
    {
        .display_width = 1200,
        .display_height = 800,
    };
    auto screen = sys->init_screen(settings);
    sys->init_input();
    bool quit = false;
    auto context = std::make_shared<pix::Context>(settings.display_width, settings.display_height);

    auto font = std::make_shared<TileSet>(FreetypeFont::unscii);

    auto con2 = std::make_shared<FullConsole>(
        std::make_shared<PixConsole>(80, 50, font), sys);

}
#endif

int main()
{
    auto* ruby = mrb_open();
    mrb::make_class<Game>(ruby, "Game");

//#ifdef __CLION_IDE__
//    std::unique_ptr<bbs::Terminal> term = std::make_unique<DummyTerminal>();
//#else
    auto term = bbs::create_local_terminal();
//#endif

    con = std::make_shared<bbs::Console<>>(std::move(term));
    con->put("Hello");
    con->flush();

    mrb_load_string(ruby, "g = Game.new");

    uint32_t key = 0;
    while (key == 0) {
        key = con->read_key();
    }

    return 0;
}
