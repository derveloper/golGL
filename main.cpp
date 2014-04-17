#include <iostream>

#include <boost/range/irange.hpp>

#include <Gosu/Gosu.hpp>

#include "world.hpp"

using namespace std;

class GameWindow : public Gosu::Window
{
public:
  world w;
  Gosu::Font font;
  bool evolution = false;
  bool paint_cell = false;

public:
  GameWindow(int width, int height, int scale)
    : Window(width*scale, height*scale, false),
      w(width, height),
      font(graphics(), Gosu::defaultFontName(), 20)
  {
    setCaption(L"Game of Life");
    w.ratio_w = (graphics().width()/w.width);
    w.ratio_h = (graphics().height()/w.height);
    needsCursor();
  }

  Gosu::Color const get_cell_color(const cell &c, const cell &cl) {
    if(c.alive /*&& cl.alive*/)
      return Gosu::Color::WHITE;
    if(c.alive && !cl.alive)
      return Gosu::Color::BLUE;
    if(!c.alive && cl.alive)
      return Gosu::Color::GREEN;

    return Gosu::Color::BLACK;
  }

  void draw_cell(const int &x, const int &y, const Gosu::Color cell_color) {
    graphics().drawQuad(
          x*w.ratio_w  ,y*w.ratio_h  ,cell_color,
          (x*w.ratio_w)+w.ratio_w,y*w.ratio_h  ,cell_color,
          (x*w.ratio_w)+w.ratio_w,(y*w.ratio_h)+w.ratio_h,cell_color,
          x*w.ratio_w  ,(y*w.ratio_h)+w.ratio_h,cell_color,
          0);
  }

  void render_cells() {
    for(auto x : boost::irange(0, w.width)) {
      cell_vector &row = w.cells[x];
      cell_vector &row_last = w.last_gen[x];
      for(auto y : boost::irange(0, w.height)) {
        Gosu::Color cell_color = get_cell_color(row[y], row_last[y]);
        draw_cell(x,y,cell_color);
      }
    }
  }

  void toggle_cell() {
    int x = input().mouseX()/w.ratio_w;
    int y = input().mouseY()/w.ratio_h;
    w.cells[x][y].alive = !w.cells[x][y].alive;
  }

  void update()
  {
    if(evolution)
      w.next_generation();
  }

  void draw()
  {
    render_cells();
    draw_cell(input().mouseX()/w.ratio_w, input().mouseY()/w.ratio_h, Gosu::Color::GRAY);
    font.draw(L"gen: "+std::to_wstring(w.generation), 10, 10, 10);
  }

  void buttonDown(Gosu::Button btn)
  {
    if (btn == Gosu::kbEscape)
      close();
    else if(btn == Gosu::kbSpace) {
      w.seed_life();
      w.generation = 0;
    }
    else if(btn == Gosu::kbE) {
      evolution = !evolution;
    }
    else if(btn == Gosu::kbC) {
      evolution = false;
      w.seed_life(false);
      w.generation = 0;
    }
    else if(btn == Gosu::kbS) {
      evolution = false;
      w.next_generation();
    }
    else if(btn == Gosu::kbD) {
      w.dump_generation();
    }
    else if(btn == Gosu::kbL) {
      w.load_generation("dump_"+w.last_dump_str+".gol");
    }
    else if(btn == Gosu::msLeft) {
      toggle_cell();
      paint_cell = true;
    }
  }

  void buttonUp(Gosu::Button btn) {
    paint_cell = false;
  }
};

int main(int argc, char **argv)
{

  if(argc < 4) {
    std::cout << "usage: " << argv[0] << " <width> <height> <scale> [<filename>]" << std::endl;
    return -1;
  }

  GameWindow window(std::stoi(argv[1]),std::stoi(argv[2]),std::stoi(argv[3]));

  if(argc == 5) {
    window.w.load_generation(argv[4]);
  }

  window.show();

  return 0;
}

