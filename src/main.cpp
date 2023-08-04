#include <memory>

#include <ansi/console.h>
#include <ansi/terminal.h>

#include <full_console.hpp>
#include <tile_set.hpp>

#include <mrb/class.hpp>
#include <mrb/conv.hpp>
#include <mrb/get_args.hpp>


struct Game
{
    std::shared_ptr<System> sys;
    std::shared_ptr<Screen> screen;
    std::shared_ptr<FullConsole> con;
    std::shared_ptr<pix::Context> context;


    void init()
    {
#ifdef RASPBERRY_PI
        sys = create_pi_system();
#else
        sys = create_glfw_system();
#endif

        Screen::Settings const settings{
            .display_width = 1200,
            .display_height = 800,
        };
        screen = sys->init_screen(settings);
        sys->init_input();
        context = std::make_shared<pix::Context>(settings.display_width,
                                                      settings.display_height);

        auto font = std::make_shared<TileSet>(FreetypeFont::unscii);

        con = std::make_shared<FullConsole>(
            std::make_shared<PixConsole>(80, 50, font), sys);
    }

    Game()
    {
        init();
        con->write("Welcome");
    }

    bool quit = false;

    void run()
    {
        while(!quit) {
            sys->handle_events(
                Overload{[this](QuitEvent) { quit = true; },
                         [&](TextEvent const& te) { },
                         [&](auto) {}});

            con->render(context.get(), {0,0}, {-1,-1});
            screen->swap();
        }
    }
};

int main()
{
    auto* ruby = mrb_open();
    mrb::make_class<Game>(ruby, "Game");
    mrb::add_method<&Game::run>(ruby, "run");
    mrb_load_string(ruby, "g = Game.new\ng.run()\n");
    return 0;
}

void run_terminal()
{
    auto term = bbs::create_local_terminal();
    auto con = std::make_shared<bbs::Console<>>(std::move(term));
    con->put("Hello");
    con->flush();

    uint32_t key = 0;
    while (key == 0) {
        key = con->read_key();
    }
}
