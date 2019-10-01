#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/button.h>
#include <gdkmm/device.h>
#include <gdkmm/seat.h>
#include <iostream>
#include <string>


class RulerWindow : public Gtk::ApplicationWindow {
  public:
    RulerWindow() {
      set_events(get_events() | Gdk::POINTER_MOTION_MASK);
      set_modal(true);
      set_decorated(false);
      set_default_size(500, 50);
      show_all();
    }
  protected:
    virtual bool on_draw(const ::Cairo::RefPtr<::Cairo::Context> & cr) {
      ::Cairo::RefPtr<::Cairo::Surface> surface = cr->get_target();
      cr->set_source_rgb(0.608, 0.765, 0.031);
      cr->paint();

      //Paint markers
      cr->set_line_width(1.0);
      int width = get_width();
      int height = get_height();

      for(int x = 0; x < width; x += 5) {
        if (x == 0) continue; //Skip first mark
        bool isTenth = x % 10 == 0;

        //Draw marker
        if (isTenth) {
          cr->set_source_rgb(0.0, 0.0, 0.0);
          cr->move_to(x, 0);
          cr->line_to(x, 15);
          cr->stroke();

          //Draw number
          std::ostringstream ss;
          ss << x / 10;
          std::string text = ss.str();
          cr->set_font_size(5);
          ::Cairo::TextExtents extents;
          cr->get_text_extents(text, extents);
          cr->move_to(x - (extents.width / 2), 20);
          cr->show_text(text);
        } else {
          cr->set_source_rgb(0.5, 0.5, 0.5);
          cr->move_to(x, 0);
          cr->line_to(x, 10);
          cr->stroke();
        }
      }
      return true;
    }

    virtual bool on_button_press_event(GdkEventButton* button_event) {
      this->moveWithCursor = true;
      this->clickX = button_event->x;
      this->clickY = button_event->y;
      return true;
    }

    virtual bool on_button_release_event(GdkEventButton* button_event) {
      this->moveWithCursor = false;
      return true;
    }

    virtual bool on_motion_notify_event (GdkEventMotion* motion_event) {
      if (moveWithCursor) {
        //Get current mouse position
        Glib::RefPtr<Gdk::Display> disp = Gdk::Display::get_default();
        Glib::RefPtr<Gdk::Seat> seat = disp->get_default_seat();
        Glib::RefPtr<Gdk::Device> pointer = seat->get_pointer();
        int xpos, ypos;
        pointer->get_position(xpos, ypos);

        //Move window
        move(xpos - clickX, ypos - clickY);
      }
      return true;
    }
  private:
    bool moveWithCursor = false;
    double clickX, clickY;
};

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "pl.codelife.ruler");
  RulerWindow hw;
  return app->run(hw);
}
