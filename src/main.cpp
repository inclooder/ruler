#include <gtkmm/application.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/button.h>
#include <gdkmm/device.h>
#include <gdkmm/seat.h>
#include <iostream>
#include <string>

class RulerWindow : public Gtk::ApplicationWindow {
  public:
    RulerWindow() : rotation_degrees(0) {
      set_events(get_events() | Gdk::POINTER_MOTION_MASK);
      set_name(""); //Picom shadow hack
      set_modal(true);
      set_decorated(false);
      set_default_size(500, 500);
      set_app_paintable(true);

      on_screen_changed(get_screen());

      show_all();
    }
  protected:
    virtual bool on_draw(const ::Cairo::RefPtr<::Cairo::Context> & cr) override {
      cr->save();
      cr->set_operator(Cairo::OPERATOR_SOURCE);

      int width = get_width();
      int height = get_height();
      int half_width = width / 2;
      int half_height = height / 2;

      cr->translate(half_width, half_height);
      cr->rotate_degrees(rotation_degrees);


      if(is_active()) cr->set_source_rgba(0.2, 0.5, 0.5, 1.0);
      else cr->set_source_rgba(0.5, 0.5, 0.5, 1.0);

      cr->rectangle(-half_width, 0, width, 40);
      cr->fill();


      //Paint markers
      cr->set_line_width(0.5);
      for(int x = -half_width; x < half_width; x += 5) {
        if (x == 0) continue; //Skip first mark
        bool isTenth = x % 10 == 0;

        //Draw marker
        if (isTenth) {
          cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
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
          cr->set_source_rgba(0.8, 0.8, 0.8, 1.0);
          cr->move_to(x, 0);
          cr->line_to(x, 10);
          cr->stroke();
        }
      }
      ::Cairo::RefPtr<Cairo::Surface> surface = cr->get_target();

      cairo_region_t * region = gdk_cairo_region_create_from_surface(surface->cobj());

      Cairo::RefPtr<Cairo::Region> reg = Cairo::RefPtr<Cairo::Region>(new Cairo::Region(region));

      GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(gobj()));
      /* gdk_window_shape_combine_region(gdk_window, NULL, 0, 0); */
      gdk_window_input_shape_combine_region(gdk_window, NULL, 0, 0);

      input_shape_combine_region(reg);
      /* shape_combine_region(reg); */
      /* cairo_region_destroy(region); */
      cr->restore();

      return true;
    }

    virtual bool on_button_press_event(GdkEventButton* button_event) override {
      this->moveWithCursor = true;
      this->clickX = button_event->x;
      this->clickY = button_event->y;
      return true;
    }

    virtual bool on_button_release_event(GdkEventButton* button_event) override {
      this->moveWithCursor = false;
      return true;
    }

    virtual bool on_motion_notify_event (GdkEventMotion* motion_event) override {
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

    virtual bool on_key_press_event(GdkEventKey* key_event) override {
      switch(key_event->keyval) {
        case GDK_KEY_q:
          {
            close();
            return true;
          }
        case GDK_KEY_r:
          {
            rotation_degrees = (rotation_degrees + 5) % 360;
            queue_draw();
            return true;
          }
      }

      return false;
    }

    virtual bool on_event(GdkEvent* gdk_event) override {
      if (gdk_event->type == GDK_WINDOW_STATE) {
        GdkWindowState changed_mask = gdk_event->window_state.changed_mask;

        bool focusChanged = (changed_mask & GDK_WINDOW_STATE_FOCUSED) == GDK_WINDOW_STATE_FOCUSED;

        if (focusChanged) queue_draw();
      }

      return false;
    }

    virtual void on_screen_changed (const Glib::RefPtr<Gdk::Screen>& previous_screen) override {
      std::cout << "Screen changed" << std::endl;

      auto screen = get_screen();
      auto visual = screen->get_rgba_visual();
      if (!visual) {
        std::cout << "transparent window not supported" << std::endl;

        visual = screen->get_system_visual();
      }

      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual->gobj());
    }
  private:
    bool moveWithCursor = false;
    double clickX, clickY;
    int rotation_degrees;
};

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create(argc, argv, "pl.inclooder.ruler");
  RulerWindow hw;
  return app->run(hw);
}
