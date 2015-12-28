#include <iostream>
#include <memory>
#include <thread>
#include <future>

#include <boost/range/irange.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "ThreadPool.h"

#include "gif.h"
#include "world.hpp"
#include "random.hpp"

using namespace std;
namespace po = boost::program_options;

namespace Color {
    static const SDL_Color RED = SDL_Color{255, 0, 0, 0};
    static const SDL_Color GREEN = SDL_Color{0, 255, 0, 0};
    static const SDL_Color BLUE = SDL_Color{0, 0, 255, 0};
    static const SDL_Color BLACK = SDL_Color{0, 0, 0, 0};
    static const SDL_Color YELLOW = SDL_Color{255, 255, 0, 0};
    static const SDL_Color WHITE = SDL_Color{255, 255, 255, 0};
    static const SDL_Color TRANSPARENT = SDL_Color{0, 0, 0, 255};
};

struct SDL_Deleter {
    void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
    void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
    void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
    void operator()(SDL_Surface* t) const { SDL_FreeSurface(t); }
    void operator()(TTF_Font* t) const { TTF_CloseFont(t); }
};

namespace sdl2
{
    typedef std::unique_ptr<SDL_Window, SDL_Deleter> window_ptr_t;
    typedef std::unique_ptr<SDL_Renderer, SDL_Deleter> renderer_ptr_t;
    typedef std::unique_ptr<SDL_Texture, SDL_Deleter> texture_ptr_t;
    typedef std::unique_ptr<SDL_Surface, SDL_Deleter> surface_ptr_t;
    typedef std::unique_ptr<TTF_Font, SDL_Deleter> font_ptr_t;
}


class GameWindow {
public:
    bool paint_cell = true;
    sdl2::window_ptr_t window;
    sdl2::renderer_ptr_t renderer;
    sdl2::texture_ptr_t cells_texture;
    sdl2::surface_ptr_t surface;
    SDL_Event event;
    world w;
    sdl2::font_ptr_t font;
    bool evolution = false;
    bool write_gif = false;
    bool write_out = false;
    int scale;
    int generations = -1;
    Uint64 frames = 1;
    Uint32 last_ticks;
    string fps_text = "FPS: 0";
    std::unique_ptr<random_gen> color_random;
    SDL_Color current_color;
    GifWriter gifWriter;
    Uint32 delta = 1;
    ThreadPool pool;
    std::vector< std::future<void> > results;
    SDL_Rect text_pos{16,16,220,32};

public:
    GameWindow(int width, int height, int scale, bool write_gif, bool write_out, const int &cpu_threads, const int &gpu_threads)
            : scale(scale),
              w(width, height, cpu_threads),
              write_gif(write_gif),
              write_out(write_out),
              pool(gpu_threads),
              color_random(new random_gen(0,255)),
              window(SDL_CreateWindow("Game of Life", 0, 0, width*scale, height*scale, 0), SDL_Deleter()),
              surface(SDL_CreateRGBSurfaceFrom(NULL, width, height, 32, 0,
                                               0x00FF0000,
                                               0x0000FF00,
                                               0x000000FF,
                                               0xFF000000), SDL_Deleter()),
              renderer(SDL_CreateRenderer(window.get(), 0, SDL_RENDERER_ACCELERATED), SDL_Deleter()),
              cells_texture(SDL_CreateTexture(
                renderer.get(),
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                width,
                height),
                SDL_Deleter()),
              font(TTF_OpenFont("arial.ttf", 32), SDL_Deleter())
    {
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
            if (c.alive && cl.alive)
                return current_color;
            return Color::BLACK;
        }
        if (c.alive && cl.alive)
            return Color::GREEN;
        if (c.alive && !cl.alive)
            return Color::RED;
        if (!c.alive && cl.alive)
            return Color::BLUE;

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
                &(surface.get())->pixels,
                &(surface.get())->pitch);

        std::vector<std::future<void>> threads;

        auto w_range = boost::irange(0, w.width);
        auto h_range = boost::irange(0, w.height);
        auto workers = boost::irange(0, (int)pool.workers.size());
        auto const worker_load = w.cells.size()/(pool.workers.size());

        auto render_task = [this, &worker_load, w_range, h_range] (auto &worker) {
            auto const start_w = worker*worker_load;
            std::for_each(
                w_range.begin()+start_w, w_range.begin()+start_w+worker_load,
                [this, &worker, &worker_load, h_range, w_range, start_w] (int x) {
                    std::for_each(h_range.begin(), h_range.end(), [&] (int y) {
                        auto &c = w.cells[x][y];
                        auto &c_last = w.last_gen[x][y];
                        auto cell_color = get_cell_color(c, c_last, false);
                        ((Uint32*)(surface.get())->pixels)[(y*w.width+x)] =
                                (0xFF000000|(cell_color.r<<16)|(cell_color.g<<8)|cell_color.b);
                    });
                }
            );
        };

        boost::for_each(workers, [this, &render_task, &threads, w_range, &worker_load, h_range] (int worker) {
            results.emplace_back(pool.enqueue(render_task, worker));
        });

        boost::for_each(results, [] (auto &t) { t.wait(); });

        SDL_UnlockTexture(cells_texture.get());

        if(write_out && evolution) {
            int nullbytes = 0x00000000;
            //fwrite(&nullbytes, 1, 3, stdout);
            Uint32 *pixels = (Uint32*)surface.get()->pixels;
            for(int b = 0; b < w.height*w.width; b++) {
                int pixel = *(pixels+b);
                fwrite(&pixel, 1, 3, stdout);
            }
            //fwrite(&nullbytes, 1, 3, stdout);
            fflush(stdout);
        }
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
            usleep(60000);
        }

        render_cells();

        SDL_RenderClear(renderer.get());
        SDL_RenderCopy(renderer.get(), cells_texture.get(), NULL, NULL);

        auto const fps = calc_fps();
        if(frames > fps) {
            fps_text = "FPS: "+ to_string(1000.f/delta) + " - Generation: " + to_string(w.generation);
            frames = 1;
        }

        sdl2::surface_ptr_t text(
                TTF_RenderText_Shaded(font.get(), fps_text.c_str(), Color::WHITE, Color::TRANSPARENT),
                SDL_Deleter());
        sdl2::texture_ptr_t text_texture(
                SDL_CreateTextureFromSurface(renderer.get(), text.get()),
                SDL_Deleter());

        SDL_RenderCopy(renderer.get(), text_texture.get(), NULL, &text_pos);
        SDL_RenderPresent(renderer.get());

        if (evolution && write_gif) {
            GifWriteFrame(&gifWriter, (uint8_t*)surface.get()->pixels, (uint32_t) w.width, (uint32_t) w.height, 0);
        }

        frames++;

        delta = SDL_GetTicks() - last_ticks;
        last_ticks = SDL_GetTicks();
    }

    inline Uint32 calc_fps() { return (48/(delta +1)); }

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
                evolution = true;
                w.next_generation();
                render_cells();
                if(write_gif) GifWriteFrame(&gifWriter, (uint8_t*)surface->pixels, w.width, w.height, 1);
                evolution = false;
                break;
            case SDL_SCANCODE_P:
                evolution = true;
                render_cells();
                evolution = false;
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
        ("stdout", "write frame bytes to stdout")
        ("cpu-threads,c", po::value<int>()->default_value(1), "cpu threads")
        ("gpu-threads,d", po::value<int>()->default_value(1), "gpu threads")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    GameWindow window(
            vm["width"].as<int>(),
            vm["height"].as<int>(),
            vm["scale"].as<int>(),
            (bool) vm.count("gif"),
            (bool) vm.count("stdout"),
            vm["cpu-threads"].as<int>(),
            vm["gpu-threads"].as<int>()
    );

    if (vm.count("filename")) {
        std::string filename = vm["filename"].as<std::string>();
        if(boost::algorithm::ends_with(filename, ".gol")) {
            window.w.load_generation(filename);
        }
        else {
            window.w.load_generation(filename, false);
        }
    }

    if (vm.count("generations")) {
        window.generations = vm["generations"].as<int>();
        window.evolution = true;
    }

    window.loop();

    SDL_Quit();

    return 0;
}

