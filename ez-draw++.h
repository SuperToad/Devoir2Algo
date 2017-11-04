#include <string>
#include <map>

extern "C" {
 #include "ez-draw.h"
}

class EZWindow {
 static std::map<Window,EZWindow*> windows;
 static void dispatch(Ez_event *ev);
 Window win;
 Ez_event* current_event;
 bool _isVisible;
public:
                         EZWindow     (int w=320,int h=200,const char *name="");
 virtual                 ~EZWindow    ();
        void             doubleBuffer (bool state=true);
        void             setWidth     (unsigned int w);
        void             setHeight    (unsigned int h);
        unsigned int     getWidth     () const;
        unsigned int     getHeight    () const;
        bool             isVisible    () const { return _isVisible; }
        void             setVisible   (bool visible=true);
        void             clear        () const;
 inline void             setColor     (unsigned long color)      { ez_set_color(color); }
 inline void             setThick     (int thick=1)              { ez_set_thick(thick); }     
 inline unsigned long    getRGB       (unsigned char r, unsigned char g, unsigned char b) const { return ez_get_RGB(r,g,b); }
 inline unsigned long    getGrey      (unsigned long g) const    { return ez_get_grey(g); }
 inline int              loadFont     (int num,const char *name) { return ez_font_load(num,name); }
 inline void             setFont      (int num)                  { ez_set_nfont(num); }
        void             drawPoint    (int x1, int y1) const;
        void             drawLine     ( int x1, int y1, int x2, int y2) const;
        void             drawRectangle(int x1, int y1, int x2, int y2) const;
        void             fillRectangle(int x1, int y1, int x2, int y2) const;
        void             drawCircle   (int x1, int y1, int x2, int y2) const;
        void             fillCircle   (int x1, int y1, int x2, int y2) const;
        void             drawTriangle (int x1, int y1, int x2, int y2, int x3, int y3) const;
        void             fillTriangle (int x1, int y1, int x2, int y2, int x3, int y3) const;
        void             drawText     (Ez_Align align,int x1,int y1,const std::string &str) const;
        void             drawText     (Ez_Align align,int x1,int y1,const char *str) const;
 inline void             sendExpose   () const { ez_send_expose(win); } // Force le rafraichissement du contenu de la fenetre
 inline void             startTimer   (unsigned int delay) const { ez_start_timer(win,(int) delay); }
 inline void             stopTimer    () const                   { ez_start_timer(win,-1);}

 // Evenements
 inline  const Ez_event* currentEvent () const { return current_event; }
 virtual void            expose       ();
 virtual void            close        ();
 virtual void            buttonPress  (int mouse_x,int mouse_y,int button);
 virtual void            buttonRelease(int mouse_x,int mouse_y,int button);
 virtual void            motionNotify (int mouse_x,int mouse_y,int button);
 virtual void            keyPress     (KeySym keysym);
 virtual void            keyRelease   (KeySym keysym);
 virtual void            configureNotify(int with,int height);
 virtual void            timerNotify  ();
};

class EZDraw {
public:
                    EZDraw();
 virtual            ~EZDraw();
 inline static void quit()                  { ez_quit(); }
 inline        void setAutoQuit(bool value) { ez_auto_quit(int(value)); }
 inline        void mainLoop()              { ez_main_loop(); }
 inline static int  random(int n)           { return ez_random(n); }
};

