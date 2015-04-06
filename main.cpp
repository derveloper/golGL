#include <iostream>
#include <memory>
#include <thread>
#include <future>

#include <boost/range/irange.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/program_options.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "gif.h"
#include "world.hpp"
#include "random.hpp"

using namespace std;
namespace po = boost::program_options;


template<typename Creator, typename Destructor, typename... Arguments>
auto make_resource(Creator c, Destructor d, Arguments&&... args);

namespace Color {
    static const SDL_Color RED = SDL_Color{255, 0, 0, 0};
    static const SDL_Color GREEN = SDL_Color{0, 255, 0, 0};
    static const SDL_Color BLUE = SDL_Color{0, 0, 255, 0};
    static const SDL_Color BLACK = SDL_Color{0, 0, 0, 0};
    static const SDL_Color WHITE = SDL_Color{255, 255, 255, 0};
    static const SDL_Color TRANSPARENT = SDL_Color{0, 0, 0, 255};
};

struct SDL_Deleter {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
    void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
    void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
};

namespace sdl2
{
    typedef std::unique_ptr<SDL_Window, SDL_Deleter> window_ptr_t;
    typedef std::unique_ptr<SDL_Renderer, SDL_Deleter> renderer_ptr_t;
    typedef std::unique_ptr<SDL_Texture, SDL_Deleter> texture_ptr_t;
}

class GameWindow {
public:
    bool paint_cell = true;
    sdl2::window_ptr_t window;
    sdl2::renderer_ptr_t renderer;
    sdl2::texture_ptr_t cells_texture;
    SDL_Surface *surface;
    SDL_Event event;
    world w;
    TTF_Font *font;
    bool evolution = false;
    bool write_gif = false;
    int scale;
    int generations = -1;
    Uint64 frames = 1;
    Uint32 last_ticks;
    string fps_text = "FPS: 0";
    std::unique_ptr<random_gen> color_random;
    SDL_Color current_color;
    GifWriter gifWriter;
    Uint32 delta = 0;

public:
    GameWindow(int width, int height, int scale, bool write_gif)
            :
              scale(scale),
              w(width, height),
              write_gif(write_gif),
              color_random(std::unique_ptr<random_gen>(new random_gen(0,255))),
              window(std::move(SDL_CreateWindow("Game of Life", 0, 0, width*scale, height*scale, 0))),
              renderer(std::move(SDL_CreateRenderer(window.get(), 0, SDL_RENDERER_ACCELERATED))),
              cells_texture(std::move(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height)))
    {
        //window = std::unique_ptr<SDL_Window, SDL_Deleter>(std::move(SDL_CreateWindow("Game of Life", 0, 0, width*scale, height*scale, 0)));
        //renderer = std::unique_ptr<SDL_Renderer, SDL_Deleter>(std::move(SDL_CreateRenderer(window.get(), 0, SDL_RENDERER_ACCELERATED)));
        //renderer = std::move(sdl2::Renderer());
        //window = make_resource(SDL_CreateWindow, SDL_DestroyWindow, "Game of Life", 0, 0, width*scale, height*scale, 0);
        //renderer = make_resource(SDL_CreateRenderer, SDL_DestroyRenderer, window.get(), 0, SDL_RENDERER_ACCELERATED);
        //cells_texture = SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        TTF_Init();
        font = TTF_OpenFont("arial.ttf", 32);
        surface = SDL_CreateRGBSurfaceFrom(NULL, width, height, 32, 0,
                0x00FF0000,
                0x0000FF00,
                0x000000FF,
                0xFF000000);
        w.ratio_w = (width / w.width);
        w.ratio_h = (height / w.height);
        w.seed_life();
        last_ticks = SDL_GetTicks();
        current_color = get_random_color();
        if(write_gif) {
            GifBegin(&gifWriter, std::string("GoL_"+w.last_dump_str+".gif").c_str(), width, height, 24);
        }
    }

    ~GameWindow() {
        if(write_gif) {
             GifEnd(&gifWriter);
        }
    }

    void loop() {
        while (paint_cell) {
            handleEvents();
            update();
        }
    }

    void handleEvents() {
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYUP:
                    buttonUp();
                    break;
                case SDL_KEYDOWN:
                    buttonDown();
                    break;
                case SDL_MOUSEMOTION:
                    //handleMouse();
                    break;
            }
        }
    }

    SDL_Color const get_cell_color(const cell &c, const cell &cl, bool random = true) {
        if(random) {
            if (c.alive /*&& cl.alive*/)
                return current_color;
            return Color::BLACK;
        }
        if (c.alive /*&& cl.alive*/)
            return Color::GREEN;
        if (c.alive && !cl.alive)
            return Color::BLACK;
        if (!c.alive && cl.alive)
            return Color::BLACK;

        return Color::BLACK;
    }

    SDL_Color get_random_color() {
        Uint8 r = boost::numeric_cast<uint8_t>(color_random->get());
        Uint8 g = boost::numeric_cast<uint8_t>(color_random->get());
        Uint8 b = boost::numeric_cast<uint8_t>(color_random->get());
        Uint8 a = boost::numeric_cast<uint8_t>(color_random->get());
        return SDL_Color{r,g,b,a};
    }

    void render_cells() {
        SDL_LockTexture(cells_texture.get(), NULL,
                &surface->pixels,
                &surface->pitch);
        Uint32 *p = (Uint32*)surface->pixels;

        std::vector<std::future<void>> threads;

        auto w_range = boost::irange(0, w.width);
        auto h_range = boost::irange(0, w.height);
        auto workers = boost::irange(0, 1);
        auto const worker_load = w.cells.size()/(1);

        boost::for_each(workers, [this, &threads, w_range, &worker_load, h_range, &p] (int worker) {
            threads.push_back(std::async(std::launch::async , [this, &worker, &worker_load, w_range, h_range, &p] {
                auto const start_w = worker*worker_load;
                std::for_each(w_range.begin()+start_w, w_range.begin()+start_w+worker_load,
                              [this, &worker, &worker_load, h_range, w_range, &p, start_w] (int x) {
                  std::for_each(h_range.begin(), h_range.end(), [&] (int y) {
                      auto &c = w.cells[x][y];
                      auto &c_last = w.last_gen[x][y];
                      auto cell_color = get_cell_color(c, c_last);
                      p[(y*w.width+x)] = (0xFF000000|(cell_color.r<<16)|(cell_color.g<<8)|cell_color.b);
                  });
              });
            }));
        });

        boost::for_each(threads, [] (auto &t) { t.get(); });

        SDL_UnlockTexture(cells_texture.get());
    }

    void toggle_cell() {
        int x = 1;//input().mouseX() / w.ratio_w;
        int y = 1;//input().mouseY() / w.ratio_h;
        w.cells[x][y].alive = !w.cells[x][y].alive;
    }

    void update() {

        if (generations > -1) {
            if (generations <= w.generation)
                exit(generations);
        }
        if (evolution) {
            w.next_generation();
        }

        render_cells();

        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), cells_texture.get(), NULL, NULL);

        if(delta > 0 && frames > 300/delta) {
            fps_text = "FPS: "+ to_string(1000.f/delta) + " - Generation: " + to_string(w.generation);
            frames = 0;
        }

        SDL_Surface *text = TTF_RenderText_Shaded(font, fps_text.c_str(), Color::WHITE, Color::TRANSPARENT);
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer.get(), text);
        SDL_Rect text_pos{16,16,220,32};

        SDL_RenderCopy(renderer.get(), text_texture, NULL, &text_pos);
        SDL_RenderPresent(renderer.get());

        if (evolution && write_gif) {
            GifWriteFrame(&gifWriter, (uint8_t*)surface->pixels, w.width, w.height, 0);
        }

        frames++;

        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(text);

        delta = SDL_GetTicks() - last_ticks;
        last_ticks = SDL_GetTicks();
    }

    void buttonDown() {
        switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                exit(0);
            case SDL_SCANCODE_SPACE:
                w.seed_life();
                w.generation = 0;
                render_cells();
                break;
            case SDL_SCANCODE_E:
                evolution = !evolution;
                break;
            case SDL_SCANCODE_C:
                evolution = false;
                w.seed_life(false);
                w.generation = 0;
                render_cells();
                break;
            case SDL_SCANCODE_S:
                evolution = false;
                w.next_generation();
                render_cells();
                GifWriteFrame(&gifWriter, (uint8_t*)surface->pixels, w.width, w.height, 1);
                break;
            case SDL_SCANCODE_D:
                w.dump_generation();
                break;
            case SDL_SCANCODE_L:
                w.load_generation("dump_" + w.last_dump_str + ".gol");
                break;
            case SDL_SCANCODE_R:
                current_color = get_random_color();
                break;
            case SDL_SCANCODE_LEFT:
                toggle_cell();
                paint_cell = true;
                break;
        }
    }

    void buttonUp() {
        //paint_cell = true;
    }
};

int main(int argc, char **argv) {

    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("width,w", po::value<int>()->default_value(100), "set the width")
        ("height,h", po::value<int>()->default_value(100), "set the height")
        ("scale,s", po::value<int>()->default_value(1), "set pixel scale")
        ("filename,f", po::value<std::string>(), "opens a gol file")
        ("generations,g", po::value<int>(), "stop after given number of generations")
        ("gif", "create gif")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    GameWindow window(vm["width"].as<int>(), vm["height"].as<int>(), vm["scale"].as<int>(), vm.count("gif"));

    if (vm.count("filename")) {
        window.w.load_generation(vm["filename"].as<std::string>());
    }

    if (vm.count("generations")) {
        window.generations = vm["generations"].as<int>();
        window.evolution = true;
    }

    window.loop();

    return 0;
}

