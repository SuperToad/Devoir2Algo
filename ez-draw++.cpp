/*
 * ez-draw++.cpp : adaptation de la boite a outil EZ-Draw pour faire du graphisme en C++
 *
 * eric.remy - 28/10/2008 - version 0.1
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

#include "ez-draw++.h"

static EZDraw *ezDraw=NULL;

std::map<Window,EZWindow*> EZWindow::windows;

static void netoyage()
{
 delete ezDraw; // detruit l'objet EZDraw s'il existe encore (aucun effet sinon).
 ezDraw=NULL;
}

EZWindow::EZWindow(int w,int h,const char *name)
 : win(None),current_event(NULL),_isVisible(true)
{
 if(ezDraw==NULL)
  {
   std::cerr <<
    "Vous n'avez pas cree l'instance de la classe EZDraw avant de creer votre premiere EZWindow !"
    " RELISEZ la documentation !" << std::endl;
   ezDraw = new EZDraw();
   atexit(netoyage);
  }
 win = ez_window_create (w, h, name,dispatch);
 windows[win]=this;
}

EZWindow::~EZWindow()
{
 windows.erase(win);
 ez_window_destroy(win);
}

void          EZWindow::doubleBuffer (bool state)                     { ez_window_dbuf(win, state); }
void          EZWindow::setWidth     (unsigned int w)                 { ez_window_set_size(win,w,getHeight()); }
void          EZWindow::setHeight    (unsigned int h)                 { ez_window_set_size(win,getWidth(),h); }
unsigned int  EZWindow::getWidth     () const                         { int w,h; ez_window_get_size(win,&w,&h); return w; }
unsigned int  EZWindow::getHeight    () const                         { int w,h; ez_window_get_size(win,&w,&h); return h; }
void          EZWindow::setVisible   (bool visible)                   { ez_window_show(win,(visible?1:0)); _isVisible = ! _isVisible; }
void          EZWindow::clear        () const                         { ez_window_clear(win); }
void          EZWindow::drawPoint    (int x1, int y1) const           { ez_draw_point(win,x1,y1); }
void          EZWindow::drawLine     (int x1, int y1, int x2, int y2) const { ez_draw_line(win,x1,y1,x2,y2); }
void          EZWindow::drawRectangle(int x1, int y1, int x2, int y2) const { ez_draw_rectangle(win,x1,y1,x2,y2); }
void          EZWindow::fillRectangle(int x1, int y1, int x2, int y2) const { ez_fill_rectangle(win,x1,y1,x2,y2); }
void          EZWindow::drawCircle   (int x1, int y1, int x2, int y2) const { ez_draw_circle(win,x1,y1,x2,y2); }
void          EZWindow::fillCircle   (int x1, int y1, int x2, int y2) const { ez_fill_circle(win,x1,y1,x2,y2); }
void          EZWindow::drawTriangle (int x1, int y1, int x2, int y2, int x3, int y3     ) const { ez_draw_triangle(win,x1,y1,x2,y2,x3,y3); }
void          EZWindow::fillTriangle (int x1, int y1, int x2, int y2, int x3, int y3     ) const { ez_fill_triangle(win,x1,y1,x2,y2,x3,y3); }
void          EZWindow::drawText     (Ez_Align align,int x1,int y1,const std::string &str) const { ez_draw_text(win,align,x1,y1,str.c_str()); }
void          EZWindow::drawText     (Ez_Align align,int x1,int y1,const char *str)        const { ez_draw_text(win,align,x1,y1,str); }
 // Evenements
void          EZWindow::dispatch(Ez_event *ev)
{
 EZWindow *pwin = windows[ev->win];

 if(pwin == NULL)
  { std::cerr <<"Aborting: pwin==NULL in " <<  __func__ << std::endl; return ; }
 pwin->current_event = ev;
 switch(ev->type)
  {
   case Expose:          pwin->expose(); break;
   case WindowClose:     pwin->close();  break;
   case ButtonPress:     pwin->buttonPress(ev->mx,ev->my,ev->mb);   break;
   case ButtonRelease:   pwin->buttonRelease(ev->mx,ev->my,ev->mb); break;
   case MotionNotify:    pwin->motionNotify(ev->mx,ev->my,ev->mb);  break;
   case KeyPress:        pwin->keyPress(ev->key_sym);     break;
   case KeyRelease:      pwin->keyRelease(ev->key_sym);   break;
   case ConfigureNotify: pwin->configureNotify(ev->width,ev->height); break;
   case TimerNotify:     pwin->timerNotify(); break;
   default :
     {
      std::ostringstream oss;
      oss << __FILE__<<":"<<__LINE__<<": Evenement non gere : "<< ev->type;
      throw std::runtime_error(oss.str());
     }
  }
}

void          EZWindow::expose       ()                                   {}
void          EZWindow::close        ()                                   {}
void          EZWindow::buttonPress  (int mouse_x,int mouse_y,int button) {}
void          EZWindow::buttonRelease(int mouse_x,int mouse_y,int button) {}
void          EZWindow::motionNotify (int mouse_x,int mouse_y,int button) {}
void          EZWindow::keyPress     (KeySym keysym)                      {}
void          EZWindow::keyRelease   (KeySym keysym)                      {}
void          EZWindow::configureNotify(int with,int height)              {}
void          EZWindow::timerNotify  ()                                   {}

////////////////////////// class EZDraw

EZDraw::EZDraw()
{
 if(ezDraw != NULL) { std::cerr << "Ne creez pas plus d'une instance de EZDraw !\nRELISEZ la documentation ! " << std::endl; return; }

 if(ez_init()<0)
  throw std::runtime_error("Can't initialize EZ-Draw (error after ez_init()).");
 ezDraw = this;
}

EZDraw::~EZDraw()
{
 if(ezDraw != this) std::cerr << "Comment diable avez-vous pu produire ce message ! Envoyez d'urgence votre programme source a eric.remy@univ-provence.fr ! Merci." << std::endl;
 ezDraw=NULL;
}

