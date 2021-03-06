/*
 * ez-draw.c : boite a outil EZ-Draw pour faire du graphisme en C
 *
 * Edouard.Thiel@lif.univ-mrs.fr - 21/09/2009 - version 0.9
 *
 * This program is free software under the terms of the
 * GNU Lesser General Public License (LGPL) version 2.1.
*/

/* Decommenter pour afficher les traces */
//#define DEBUG

#include "ez-draw.h"

/* Variable globale dans laquelle on memorise tous les parametres internes */
Ez_X ezx;

/* Couleurs */
unsigned long ez_black, ez_white, ez_grey, ez_red, ez_green, ez_blue,
              ez_yellow, ez_cyan, ez_magenta;


/*------------------- I N T E R F A C E   P U B L I Q U E -------------------*/

/*
 * Initialisation generale.
 * Renvoie 0 succes, -1 echec.
*/

int ez_init ()
{
    /* On met a zero tous les champs de ezx */
    memset (&ezx, 0, sizeof(Ez_X));

#ifndef _WIN32

    /* On se connecte au display de la variable d'environnement $DISPLAY
       par defaut c'est le display local. */
    ezx.display = XOpenDisplay ("");
    if (ezx.display == NULL) {
        fprintf (stderr, "ez_init: XOpenDisplay failed for \"%s\"\n",
            XDisplayName (""));
        return -1;
    }

    /* On memorise les parametres habituels du display */
    ezx.screen_num = DefaultScreen (ezx.display);
    ezx.display_width = DisplayWidth (ezx.display, ezx.screen_num);
    ezx.display_height = DisplayHeight (ezx.display, ezx.screen_num);
    ezx.root_win = RootWindow (ezx.display, ezx.screen_num);
    ezx.depth = DefaultDepth (ezx.display, ezx.screen_num);
    ezx.black = BlackPixel (ezx.display, ezx.screen_num);
    ezx.white = WhitePixel (ezx.display, ezx.screen_num);

    /* Contexte graphique gc, supprime events NoExpose et GraphicsExpose */
    ezx.gc = DefaultGC (ezx.display, ezx.screen_num);
    XSetGraphicsExposures(ezx.display, ezx.gc, False);

    /* Cree un xid pour memoriser la callback et le double-buffer */
    ezx.func_xid = XUniqueContext ();
    ezx.dbuf_xid = XUniqueContext ();

    /* Timers */
    ezx.timer_nb = 0;

    /* Atomes pour le WM */
    ezx.atom_protoc = XInternAtom (ezx.display, "WM_PROTOCOLS", False);
    ezx.atom_delwin = XInternAtom (ezx.display, "WM_DELETE_WINDOW", False);

#else /* _WIN32 */

    /* Recupere le handle sur le programme */
    ezx.hand_prog = GetModuleHandle(NULL);

    /* On prepare une classe etendue de fenetres */
    ezx.wnd_class.cbSize        = sizeof(WNDCLASSEX);
    ezx.wnd_class.style         = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
    ezx.wnd_class.lpfnWndProc   = ez_win_proc;
    ezx.wnd_class.cbClsExtra    = 0;
    ezx.wnd_class.cbWndExtra    = 0;
    ezx.wnd_class.hInstance     = ezx.hand_prog;
    ezx.wnd_class.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
    ezx.wnd_class.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);
    ezx.wnd_class.hCursor       = LoadCursor (NULL, IDC_ARROW);
    ezx.wnd_class.hbrBackground = (HBRUSH) (COLOR_WINDOW+1);
    ezx.wnd_class.lpszMenuName  = NULL;
    ezx.wnd_class.lpszClassName = "EZ_WND_CLASS";

    /* On enregistre la classe de fenetres */
    if (!RegisterClassEx (&ezx.wnd_class)) {
        fprintf (stderr, "ez_init: RegisterClassEx failed for class \"%s\"\n",
            ezx.wnd_class.lpszClassName);
        return -1;
    }

    /* On memorise les parametres habituels de l'ecran */
    ezx.screen_num = 0;
    ezx.display_width  = GetSystemMetrics(SM_CXSCREEN);
    ezx.display_height = GetSystemMetrics(SM_CYSCREEN);
    ezx.root_win = NULL;

    /* Proprietes pour memoriser la callback et le double-buffer */
    ezx.func_prop = "EZ_PROP_FUNC"; 
    ezx.dbuf_prop = "EZ_PROP_DBUF"; 

    /* Evenements */
    ezx.mv_win = None;                 /* Pour filtrer WM_MOUSEMOVE */
    ezx.key_sym = 0;                   /* Pour renseigner les Keyrelease */
    ezx.key_name = ezx.key_string = "";

    /* Pour le dessin */
    ezx.dc_win = None;                 /* fenetre courante associe'e au dc */
    ezx.color = 0L;                    /* couleur courante */
    ezx.hpen = NULL;                   /* Pen pour dessin en couleur */
    ezx.hbrush = NULL;                 /* Brush pour remplissage en couleur */

#endif /* _WIN32 */

    /* Init double-buffer pour l'affichage des fenetres */
    ez_dbuf_init ();

    /* Init fontes, fonte par defaut */
    ez_font_init ();
    ez_set_nfont (0);

    /* Init des couleurs, couleur et epaisseur par defaut */
    ez_color_init ();
    ez_set_color (ez_black);
    ez_set_thick (1);

    /* Configuration de la boucle d'evenements */
    ezx.main_loop = 1;    /* mettre a 0 pour quitter la boucle d'evenements */
    ezx.last_expose = 1;  /* mettre a 0 pour desactiver attente dernier Expose */
    ezx.auto_quit = 1;    /* bouton "Fermer" barre titre Fen ==> quitte prog */
    ezx.mouse_b = 0;      /* utilise' pour MotionNotify */
    ezx.win_nb = 0;       /* fait aussi sortir boucle d'evenements */

    /* Init le ge'ne'rateur de nombres ale'atoires */
    ez_random_init ();

    /* Pour terminer proprement */
    atexit (ez_close_disp);

    return 0;
}


/*
 * Cree une fenetre principale, de largeur w et hauteur h,
 * avec un titre name, et une callback func appele pour chaque evenement.
 * Renvoie le Window.
*/

Window ez_window_create (int w, int h, const char *name, Ez_func func)
{
    Window win;

#ifndef _WIN32

    /* On cree une fenetre principale = fille de la fenetre racine */
    win = XCreateSimpleWindow (ezx.display, ezx.root_win,
        0, 0, w, h, 0, ezx.black, ezx.white);

    /* Rend la fenetre visible et la place devant toute les autres */
    XMapRaised (ezx.display, win);

    /* Place un masque d'evenements */
    XSelectInput (ezx.display, win,
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
        StructureNotifyMask);

    /* On donne le titre de la fenetre */
    XStoreName (ezx.display, win, name);

    /* Protege les fenetres de la destruction */
    XSetWMProtocols (ezx.display, win, &ezx.atom_delwin, 1);

#else /* _WIN32 */

    /* On cree une fenetre principale = fille de NULL sous Windows */
    win = CreateWindowEx (
        WS_EX_CLIENTEDGE,
        ezx.wnd_class.lpszClassName,
        name,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        w + 4 + GetSystemMetrics (SM_CXSIZEFRAME)*2,
        h + 4 + GetSystemMetrics (SM_CYSIZEFRAME)*2
              + GetSystemMetrics (SM_CYCAPTION),
        NULL, NULL, ezx.hand_prog, NULL);

    if (win == NULL) {
        fprintf (stderr, "ez_window_create: failed for \"%s\"\n", name);
        return win;
    }

    /* Rend la fenetre visible */
    ShowWindow (win, SW_SHOWNORMAL);

    /* Si on fait UpdateWindow, il envoie le premier WM_PAINT dans la winproc,
       qui est appele' avant le debut de ez_main_loop ! La solution est
       d'envoyer un WM_PAINT dans la queue. */
    PostMessage (win, WM_PAINT, 0, 0);

#endif /* _WIN32 */

    /* Memorise la callback func et le double-buffer a None */
    ez_func_set (win, func);
    ez_dbuf_save (win, None);

    /* Compte nb de fenetres */
    ezx.win_nb++;

    return win;
}


/*
 * Detruit un window win, ainsi que tout ce qui est associe'.
*/

void ez_window_destroy (Window win)
{
    ez_func_destroy (win);

    ez_window_dbuf (win, 0);
    ez_dbuf_delete (win);
    ez_timer_remove (win);

#ifndef _WIN32

    XDestroyWindow (ezx.display, win);
    
#else /* _WIN32 */

    if (win == ezx.dc_win) ez_cur_win (None);
    if (win == ezx.mv_win) ezx.mv_win = None;
    DestroyWindow (win);
    
#endif /* _WIN32 */

    ezx.win_nb--;
}


/*
 * Rend une fenetre visible (val = 1) ou invisible (val = 0).
*/

void ez_window_show (Window win, int val)
{
#ifndef _WIN32
    if (val) {
        XMapRaised (ezx.display, win);
    } else {
        XUnmapWindow (ezx.display, win);
    }
#else /* _WIN32 */
    if (val) {
        ShowWindow (win, SW_SHOWNORMAL);
    } else {
        ShowWindow (win, SW_HIDE);
    }
#endif /* _WIN32 */
}


/*
 * Change ou recupere la taille d'un window.
*/

void ez_window_set_size (Window win, int w, int h)
{
#ifndef _WIN32

    XWindowChanges wc;

    wc.width = w > 1 ? w : 1;
    wc.height = h > 1 ? h : 1;
    XConfigureWindow (ezx.display, win, CWWidth|CWHeight, &wc);

#else /* _WIN32 */

    if (w < 1) w = 1;
    if (h < 1) h = 1;
    SetWindowPos (win, HWND_TOP, 0, 0,
        w + 4 + GetSystemMetrics (SM_CXSIZEFRAME)*2,
        h + 4 + GetSystemMetrics (SM_CYSIZEFRAME)*2
              + GetSystemMetrics (SM_CYCAPTION),
        SWP_NOMOVE | SWP_NOZORDER );

#endif /* _WIN32 */
}

void ez_window_get_size (Window win, int *w, int *h)
{
#ifndef _WIN32

    Window root_ret;
    unsigned int w_ret, h_ret, b_ret, d_ret;
    int x_ret, y_ret;

    XGetGeometry (ezx.display, win, &root_ret, &x_ret, &y_ret,
        &w_ret, &h_ret, &b_ret, &d_ret);

#else /* _WIN32 */

    int w_ret, h_ret;
    RECT rect;

    GetWindowRect (win, &rect);

    w_ret = rect.right - rect.left - 4
            - GetSystemMetrics (SM_CXSIZEFRAME)*2;
    if (w_ret < 0) w_ret = 0;
    h_ret = rect.bottom - rect.top - 4
            - GetSystemMetrics (SM_CYSIZEFRAME)*2
            - GetSystemMetrics (SM_CYCAPTION);
    if (h_ret < 0) h_ret = 0;

#endif /* _WIN32 */

    if (w) *w = w_ret;
    if (h) *h = h_ret;
}


/*
 * Vide une fenetre et reinitialise les parametres de dessin.
*/

void ez_window_clear (Window win)
{
    int w, h;
    unsigned long oldcolor = ezx.color;
    
    ez_window_get_size (win, &w, &h);
    ez_set_color (ez_white);
    ez_fill_rectangle (win, 0, 0, w, h);
    ez_set_color (oldcolor);
    ez_set_thick (1);
    ez_set_nfont (0);
}


/*
 * Active ou inactive l'affichage double-buffer pour une fenetre.
*/

void ez_window_dbuf (Window win, int val)
{
    XdbeBackBuffer dbuf;

    if (ez_dbuf_get (win, &dbuf) < 0) return;

    if (val) {

        if (dbuf != None) return;
#ifndef _WIN32
        dbuf = XdbeAllocateBackBufferName (ezx.display, win, XdbeUndefined);
#else /* _WIN32 */
        ez_cur_win (win);
        dbuf = CreateCompatibleDC (ezx.hdc);
#endif /* _WIN32 */
        ez_dbuf_save (win, dbuf);

    } else {

        if (dbuf == None) return;
#ifndef _WIN32
        XdbeDeallocateBackBufferName (ezx.display, dbuf);
#else /* _WIN32 */
        ez_cur_win (None);
        DeleteDC (dbuf);
#endif /* _WIN32 */
        ez_dbuf_save (win, None);
    }
}


/*
 * Fait sortir de ez_main_loop().
*/

void ez_quit ()
{
#ifdef _WIN32
    PostQuitMessage (0);
#endif /* _WIN32 */

    ezx.main_loop = 0;
}


/*
 * Determine le comportement lorsqu'on clique sur le bouton "fermer" de la
 * barre de titre d'une fenetre :
 *   val = 1  ==> le programme est quitte' (comportement par defaut).
 *   val = 0  ==> le prog recoit l'evenement WindowClose ; il peut alors
 *                decider si il quitte avec ez_quit, si il detruit la fenetre
 *                avec ez_window_destroy, si il ouvre une fenetre de
 *                dialogue, si il ignore l'evenement, etc.
*/

void ez_auto_quit (int val)
{
    ezx.auto_quit = val;
}


/*
 * Envoie un evenement Expose a la fenetre, pour la forcer a se redessiner.
*/

void ez_send_expose (Window win)
{
#ifndef _WIN32

    XEvent ev;

    ev.type = Expose;
    ev.xexpose.window = win;
    ev.xexpose.count = 0;

    XSendEvent (ezx.display, win, False, 0L, &ev);

#else /* _WIN32 */

    PostMessage (win, EZ_MSG_PAINT, 0, 0);

#endif /* _WIN32 */
}


/*
 * Provoque l'envoi d'un evenement TimerNotify a la fenetre win dans delay 
 * millisecondes. Un nouvel appel annule et remplace le precedent.
 * Pour annuler le timer en cours, mettre delay = -1.
*/

void ez_start_timer (Window win, int delay)
{
    if (delay < 0) {
        ez_timer_remove (win);
    } else {
        if (ez_timer_add (win, delay) < 0)
            fprintf (stderr, "ez_start_timer: could not set timer delay = %d ms for win 0x%x\n",
                delay, (int) win);
    }
}


/*
 * Boucle principale. Pour l'interrompre, appeler ez_quit().
 * Cette fonction affiche les fenetres creees, puis attend les evenements et
 * appelle au fur et a mesure les callbacks correspondantes.
 * Une fois revenu de cette fonction, il ne faut plus faire de graphisme.
*/

void ez_main_loop ()
{
#ifndef _WIN32

    Ez_event ev;

    while (ezx.main_loop != 0 && ezx.win_nb > 0) {

        /* On attend le prochain evenement */
        ez_event_next (&ev);

        /* On decode l'evenement et on appelle la callback */
        ez_event_dispatch (&ev);
    }

#else /* _WIN32 */

    MSG msg;

    while (ezx.main_loop != 0 && ezx.win_nb > 0) {

        /* On attend le prochain message */
        if (GetMessage (&msg, NULL, 0, 0) <= 0) break;

        /* Rajoute message WM_CHAR apres un WM_KEYDOWN */
        TranslateMessage (&msg);

        /* Appelle la winproc, qui appelle la callback.
           RQ : le plus souvent, la winproc est appele'e
           directement par le systeme sans passer par la`. */
        DispatchMessage (&msg);
    }
#endif /* _WIN32 */
}


/*
 * Renvoie un entier ale'atoire entre 0 et n-1 
*/

int ez_random (int n)
{
    return rand() % n;
}


/*
 * Calcule une couleur a partir des niveaux R,G,B entre 0 et 255.
*/

unsigned long ez_get_RGB (unsigned char R, unsigned char G, unsigned char B)
{
#ifndef _WIN32

    if (ezx.visual->class == PseudoColor)
         return ezx.pseudoColor.palette [R / 51][G / 51][B / 51];
    else return R >> (8 - ezx.trueColor.red  .length) << ezx.trueColor.red  .shift |
                G >> (8 - ezx.trueColor.green.length) << ezx.trueColor.green.shift |
                B >> (8 - ezx.trueColor.blue .length) << ezx.trueColor.blue .shift ;

#else /* _WIN32 */

    return RGB (R, G, B);

#endif /* _WIN32 */
}


/*
 * Calcule une couleur grise a partir de son niveau g entre 0 et 255.
*/

unsigned long ez_get_grey (unsigned long g)
{
    return ez_get_RGB (g, g, g);
}


/*
 * Affecte la couleur des prochains dessins, y compris affichage de texte.
*/

void ez_set_color (unsigned long color)
{
    ezx.color = color;

#ifndef _WIN32

    XSetForeground (ezx.display, ezx.gc, ezx.color);

#else /* _WIN32 */

    /* Pen pour dessin */
    ez_update_pen ();

    /* Brush pour remplissage couleur */
    if (ezx.hbrush != NULL) DeleteObject (ezx.hbrush);
    ezx.hbrush = CreateSolidBrush (ezx.color);
    if (ezx.dc_win != None) SelectObject (ezx.hdc, ezx.hbrush);

    /* Pour le dessin de texte */
    if (ezx.dc_win != None) SetTextColor (ezx.hdc, ezx.color);

#endif /* _WIN32 */
}


/*
 * Affecte l'epaisseur des lignes dans les prochains dessins.
 * Concerne : ez_draw_point, ez_draw_line, ez_draw_rectangle, ez_draw_triangle, 
 *            ez_draw_circle.
*/

void ez_set_thick (int thick)
{
    ezx.thick = (thick <= 0) ? 1 : thick;

#ifndef _WIN32
    XSetLineAttributes (ezx.display, ezx.gc, (ezx.thick == 1) ? 0 : ezx.thick, 
        LineSolid, CapRound, JoinRound);
#else /* _WIN32 */
    ez_update_pen ();
#endif /* _WIN32 */
}


/*
 * Dessins de base. x1,y1 et y2,y2 sont les coordonnees en haut a gauche et
 * en bas a droite de la boite englobante.
*/

#define EZ_MIN(x,y) ((x)<(y)?(x):(y))
#define EZ_MAX(x,y) ((x)>(y)?(x):(y))

void ez_draw_point (Window win, int x1, int y1)
{
#ifndef _WIN32
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    if (ezx.thick == 1)
         XDrawPoint (ezx.display, win, ezx.gc, x1, y1);
    else XFillArc (ezx.display, win, ezx.gc, x1-ezx.thick/2, y1-ezx.thick/2, 
                   ezx.thick+1, ezx.thick+1, 0, 360*64);
#else /* _WIN32 */
    ez_cur_win (win);
    MoveToEx (ezx.hdc, x1, y1, NULL);
    LineTo (ezx.hdc, x1+1, y1);  /* point final exclus */
#endif /* _WIN32 */
}

void ez_draw_line (Window win, int x1, int y1, int x2, int y2)
{
#ifndef _WIN32
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    XDrawLine (ezx.display, win, ezx.gc, x1, y1, x2, y2);
#else /* _WIN32 */
    ez_cur_win (win);
    MoveToEx (ezx.hdc, x1, y1, NULL);
    LineTo (ezx.hdc, x2, y2);
    if (ezx.thick == 1) LineTo (ezx.hdc, x2+1, y2);   /* point final exclus */
#endif /* _WIN32 */
}

void ez_draw_rectangle (Window win, int x1, int y1, int x2, int y2)
{
#ifndef _WIN32
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    XDrawRectangle (ezx.display, win, ezx.gc,
        EZ_MIN(x1,x2), EZ_MIN(y1,y2), abs(x2-x1), abs(y2-y1));
#else /* _WIN32 */
    ez_cur_win (win);
    MoveToEx (ezx.hdc, x1, y1, NULL);
    LineTo (ezx.hdc, x2, y1);
    LineTo (ezx.hdc, x2, y2);
    LineTo (ezx.hdc, x1, y2);
    LineTo (ezx.hdc, x1, y1);
#endif /* _WIN32 */
}

void ez_fill_rectangle (Window win, int x1, int y1, int x2, int y2)
{
#ifndef _WIN32
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    XFillRectangle (ezx.display, win, ezx.gc,
        EZ_MIN(x1,x2), EZ_MIN(y1,y2), abs(x2-x1)+1, abs(y2-y1)+1);
#else /* _WIN32 */
    int old_thick = ezx.thick;
    ez_cur_win (win);
    if (ezx.thick != 1) ez_set_thick (1);
    Rectangle (ezx.hdc, EZ_MIN(x1,x2)  , EZ_MIN(y1,y2)   ,
                        EZ_MAX(x1,x2)+1, EZ_MAX(y1,y2)+1 );
    if (ezx.thick != old_thick) ez_set_thick (old_thick);
#endif /* _WIN32 */
}

void ez_draw_triangle (Window win, int x1, int y1, int x2, int y2, int x3, int y3)
{
#ifndef _WIN32
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    XDrawLine (ezx.display, win, ezx.gc, x1, y1, x2, y2);
    XDrawLine (ezx.display, win, ezx.gc, x2, y2, x3, y3);
    XDrawLine (ezx.display, win, ezx.gc, x3, y3, x1, y1);
#else /* _WIN32 */
    ez_cur_win (win);
    MoveToEx (ezx.hdc, x1, y1, NULL);
    LineTo (ezx.hdc, x2, y2);
    LineTo (ezx.hdc, x3, y3);
    LineTo (ezx.hdc, x1, y1);
#endif /* _WIN32 */
}

void ez_fill_triangle (Window win, int x1, int y1, int x2, int y2, int x3, int y3)
{
#ifndef _WIN32
    XPoint points[] = { {x1,y1}, {x2,y2}, {x3,y3} };
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    XFillPolygon (ezx.display, win, ezx.gc, points, 3, Convex, CoordModeOrigin);
#else /* _WIN32 */
    POINT points[] = { {x1,y1}, {x2,y2}, {x3,y3} };
    int old_thick = ezx.thick;
    ez_cur_win (win);
    if (ezx.thick != 1) ez_set_thick (1);
    Polygon (ezx.hdc, points, 3 );
    if (ezx.thick != old_thick) ez_set_thick (old_thick);
#endif /* _WIN32 */
}

void ez_draw_circle (Window win, int x1, int y1, int x2, int y2)
{
#ifndef _WIN32
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    XDrawArc (ezx.display, win, ezx.gc,
        EZ_MIN(x1,x2), EZ_MIN(y1,y2), abs(x2-x1), abs(y2-y1), 0, 360*64);
#else /* _WIN32 */
    int xa = EZ_MIN(x1,x2), ya = EZ_MIN(y1,y2),
        xb = EZ_MAX(x1,x2), yb = EZ_MAX(y1,y2),
        xc = (xa+xb)/2;
    ez_cur_win (win);
    Arc (ezx.hdc, xa, ya, xb, yb, xc, ya, xc, ya);
#endif /* _WIN32 */
}

void ez_fill_circle (Window win, int x1, int y1, int x2, int y2)
{
#ifndef _WIN32
    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    XFillArc (ezx.display, win, ezx.gc,
        EZ_MIN(x1,x2), EZ_MIN(y1,y2), abs(x2-x1)+1, abs(y2-y1)+1, 0, 360*64);
#else /* _WIN32 */
    int old_thick = ezx.thick;
    ez_cur_win (win);
    if (ezx.thick != 1) ez_set_thick (1);
    Ellipse (ezx.hdc, EZ_MIN(x1,x2)  , EZ_MIN(y1,y2)   ,
                      EZ_MAX(x1,x2)+1, EZ_MAX(y1,y2)+1 );
    if (ezx.thick != old_thick) ez_set_thick (old_thick);
#endif /* _WIN32 */
}


/*
 * Charge une fonte a partir de son nom (exemple "6x13") et la stocke
 * dans ezx.font[num]. Renvoie 0 succes, -1 erreur.
 *
 * Sous X11, le nom peut avoir une forme quelconque mais doit correspondre
 * a une fonte existante.
 * Sous windows, le nom doit etre sous la forme "<width>x<height>", et
 * une fonte approchante est obtenue.
*/

int ez_font_load (int num, const char *name)
{
    if (num < 0 || num >= EZ_FONT_MAX) {
        fprintf (stderr, "ez_font_load: bad num\n");
        return -1;
    }

#ifndef _WIN32

    ezx.font[num] = XLoadQueryFont (ezx.display, name);

#else /* _WIN32 */

    int w, h;

    if (sscanf (name, "%dx%d", &w, &h) != 2) {
        fprintf (stderr, "ez_font_load: could not get wxh in \"%s\"\n", name);
        return -1;
    }

    ezx.font[num] = CreateFont (
        -h, -w, 0, 0,               /* nHeight,nWidth,nEscapement,nOrientation */
        h <= 18 ? 500 : 600,        /* fnWeight */
        FALSE, FALSE, FALSE,        /* fdwItalic, fdwUnderline, fdwStrikeOut */
        ANSI_CHARSET,               /* fdwCharset. DEFAULT_CHARSET ? */
        OUT_DEFAULT_PRECIS,         /* fdwOutputPrecision */
        CLIP_DEFAULT_PRECIS,        /* fdwClipPrecision */
        DEFAULT_QUALITY,            /* fdwQuality */
        FIXED_PITCH | FF_MODERN,    /* fdwPitchAndFamily */
        ""                          /* lpszFace */
    );

#endif /* _WIN32 */

    if (ezx.font[num] == NULL)  {
        fprintf (stderr, "ez_font_load: could not load font \"%s\"\n", name);
        return -1;
    }
    return 0;
}


/*
 * Affecte le numero de fonte pour les prochains affichages de texte.
*/

void ez_set_nfont (int num)
{
    if (num < 0 || num >= EZ_FONT_MAX || ezx.font[num] == NULL) {
        fprintf (stderr, "ez_set_nfont: bad num\n");
        return;
    }
    ezx.nfont = num;

#ifndef _WIN32
    XSetFont (ezx.display, ezx.gc, ezx.font[ezx.nfont]->fid);
#else /* _WIN32 */
    if (ezx.dc_win != None) SelectObject (ezx.hdc, ezx.font[ezx.nfont]);
#endif /* _WIN32 */
}


/*
 * Affiche du texte dans la fenetre, s'utilise comme printf.
 *
 * Exemple : ez_draw_text (win, EZ_TL, x, y,
 *                         "Largeur = %d\nHauteur = %d", w, h);
 *
 * Les coordonne'es sont par rapport a` align, cf definition Ez_Align.
 * align pre'cise aussi si le fond est efface' ou non.
 *
 * Par defaut :
 *  - le texte est affiche' avec la fonte nume'ro 0 ("6x13") ;
 *    se change avec ez_set_nfont().
 *  - le texte est affiche' en noir ; se change avec ez_set_color().
*/

void ez_draw_text (Window win, Ez_Align align, int x1, int y1,
    const char *format, ...)
{
    int valign, halign, fillbg;
    va_list (ap);
    char buf[16384];
    int i, j, k, n, x, y, a, b, c;
    
#ifndef _WIN32
    XFontStruct *font = ezx.font[ezx.nfont];
#else /* _WIN32 */
    TEXTMETRIC text_metric;
#endif /* _WIN32 */

    if (align <= EZ_AA || align == EZ_BB || align >= EZ_CC)
      { fprintf (stderr, "ez_draw_text: bad align\n"); return; }

    /* Decode align */
    fillbg = 0;
    if (align > EZ_BB) { fillbg = 1; align -= 10; }
    align -= EZ_AA + 1;
    halign = align % 3;
    valign = align / 3;

    /* Ecrit la chaine formatee dans buf */
    va_start (ap, format);
    vsnprintf (buf, sizeof(buf)-1, format, ap);
    va_end (ap);
    buf[sizeof(buf)-1] = 0;
    if (buf[0] == 0) return;

    /* Compte le nombre n de lignes */
    for (i = j = k = 0; ; i++)
    if (buf[i] == '\n' || buf[i] == 0) {
        k++; j = i+1;
        if (buf[i] == 0) break;
    }
    n = k;

#ifndef _WIN32

    if (win == ezx.dbuf_win) win = ezx.dbuf_pix;
    a = font->ascent; b = font->descent; c = a+b+b;

    /* Affiche ligne a ligne */
    for (i = j = k = 0; ; i++)
    if (buf[i] == '\n' || buf[i] == 0) {
        x = x1 - XTextWidth (font, buf+j, i-j) * halign/2;
        y = y1 + a + c*k - (c*n-b) * valign/2;
        if (fillbg == 0)
             XDrawString      (ezx.display, win, ezx.gc, x, y, buf+j, i-j);
        else XDrawImageString (ezx.display, win, ezx.gc, x, y, buf+j, i-j);
        k++; j = i+1;
        if (buf[i] == 0) break;
    }
    
#else /* _WIN32 */

    ez_cur_win (win);
    GetTextMetrics (ezx.hdc, &text_metric);
    a = text_metric.tmAscent; b = text_metric.tmDescent; c = a+b;
    
    if (fillbg == 0) SetBkMode (ezx.hdc, TRANSPARENT);

    /* Affiche ligne a ligne */
    for (i = j = k = 0; ; i++)
    if (buf[i] == '\n' || buf[i] == 0) {
        x = x1 - (i-j)*text_metric.tmAveCharWidth * halign/2;
        y = y1 + c*k - (c*n-b) * valign/2 -2;
        TextOut (ezx.hdc, x, y, buf+j, i-j);
        k++; j = i+1;
        if (buf[i] == 0) break;
    }
    
    /* On retablit le mode de dessin du fond */
    if (fillbg == 0) SetBkMode (ezx.hdc, OPAQUE);
    
#endif /* _WIN32 */
}


/*-------------------- F O N C T I O N S   P R I V E E S --------------------*/


/*
 * Appelee par at_exit()
*/

void ez_close_disp ()
{
    ez_font_delete ();

#ifndef _WIN32
    /* Fermeture du display ; a partir de maintenant on ne peut plus appeler
        aucune fonction qui l'utilise. */
    XCloseDisplay (ezx.display);
#endif /* _WIN32 */
}


/*
 * Insere un timer dans la liste. Renvoie 0 succes, -1 erreur.
*/

int ez_timer_add (Window win, int delay)
{
#ifndef _WIN32
    int m1, m2, mid;
    struct timeval t, *mt;

    ez_timer_remove (win);

    if (ezx.timer_nb >= EZ_TIMER_MAX) {
        fprintf (stderr, "ez_timer_add: too much timers\n");
        return -1;
    }

    /* Re'cupe`re date courante */
    gettimeofday (&t, NULL);

    /* Calcule date d'expiration */
    t.tv_usec += delay * 1000;   /* delay en millisecondes */
    if (t.tv_usec > 1000000) {
        t.tv_sec += t.tv_usec / 1000000;
        t.tv_usec %= 1000000;
    }

    /* Recherche dichotomique */
    m1 = 0; m2 = ezx.timer_nb;
    while (m2-m1 > 0) {
        mid = (m1+m2) / 2;    /* Milieu tq m1 <= mid < m2 */
        mt = &ezx.timer_l[mid].expiration;
        if ( mt->tv_sec < t.tv_sec ||
            (mt->tv_sec == t.tv_sec && mt->tv_usec < t.tv_usec) )
             m1 = mid+1;   /* t doit aller dans [mid+1 .. m2[ */
        else m2 = mid;     /* t doit aller dans [m1 .. mid[   */
    }

    /* Insere en position m1 */
    if (m1 < ezx.timer_nb)
        memmove ( ezx.timer_l+m1+1, ezx.timer_l+m1,
                 (ezx.timer_nb-m1)*sizeof(Ez_timer) );
    ezx.timer_nb++;
    ezx.timer_l[m1].win = win;
    ezx.timer_l[m1].expiration = t;

    return 0;
#else /* _WIN32 */
    int k = SetTimer(win, EZ_TIMER1, (UINT) delay, (TIMERPROC) NULL);
    return k == 0 ? -1 : 0;
#endif /* _WIN32 */
}


/*
 * Supprime un timer de la liste. Renvoie 0 succes, -1 erreur.
*/

int ez_timer_remove (Window win)
{
#ifndef _WIN32
    int i;
    if (win == None) return 0;
    for (i = 0; i < ezx.timer_nb; i++)
        if (ezx.timer_l[i].win == win) {
            memmove ( ezx.timer_l+i, ezx.timer_l+i+1,
                     (ezx.timer_nb-i-1)*sizeof(Ez_timer) );
            ezx.timer_nb--;
            return 0;
        }
    return -1;
#else /* _WIN32 */
    int k = KillTimer (win, EZ_TIMER1);
    return k == 0 ? -1 : 0;
#endif /* _WIN32 */
}


#ifndef _WIN32

/* 
 * Renvoie le delai entre la date courante et le prochain timer,
 * a passer directement a select().
*/

struct timeval *ez_timer_delay ()
{
    static struct timeval t;

    /* Aucun timer */
    if (ezx.timer_nb == 0) return NULL;

    /* Recupere date courante */
    gettimeofday (&t, NULL);

    /* Le prochain timeout est ezx.timer_list[0].expiration ;
       on calcule la difference avec la date courante */
    t.tv_sec  = ezx.timer_l[0].expiration.tv_sec  - t.tv_sec;
    t.tv_usec = ezx.timer_l[0].expiration.tv_usec - t.tv_usec;
    if (t.tv_usec < 0) { t.tv_usec += 1000000; t.tv_sec -= 1;}
    if (t.tv_sec  < 0) t.tv_sec = t.tv_usec = 0;

    /* printf ("Timeout dans %d s  %6d us\n", (int)t.tv_sec, (int)t.tv_usec); */

    /* Renvoie adresse statique du struct */
    return &t;
}


/*
 * Attente bloquante d'un evenement.
 * Remplace XNextEvent en ajoutant des evenements (TimerNotify).
*/

void ez_event_next (Ez_event *ev)
{
    int n, res, fdx = ConnectionNumber(ezx.display);
    fd_set set1;

    /* Init ev */
    memset (ev, 0, sizeof(Ez_event));
    ev->type = EzLastEvent;
    ev->win = None;

    /* Label permettant d'ignorer un evenement et de recommencer l'attente */
    debut_attente:

    /* Fait un XFlush et renvoie le nombre d'evenements dans la queue,
     * sans faire de lecture et sans bloquer.
    */
    n = XEventsQueued (ezx.display, QueuedAfterFlush);

    /* S'il y a au moins un evenement dans la queue, on peut le lire sans
     * bloquer avec XNextEvent(). Il faut le faire ici car une attente avec
     * select() serait bloquante si le serveur avait deja tout envoye'.
    */
    if (n > 0) {
        XNextEvent (ezx.display, &ev->xev);
        if ( (ev->xev.type == Expose) &&
             ezx.last_expose && ! ez_is_last_expose (&ev->xev))
            goto debut_attente;
        return;
    }

    /* La queue cote' client est vide, on se met en attente ge'ne'rale
    */
    FD_ZERO (&set1);
    FD_SET (fdx, &set1);

    res = select (fdx+1, &set1, NULL, NULL, ez_timer_delay ());

    if (res > 0) {
        if (FD_ISSET (fdx, &set1)) {
            XNextEvent (ezx.display, &ev->xev);
            if ( (ev->xev.type == Expose) &&
                 ezx.last_expose && ! ez_is_last_expose (&ev->xev) )
                goto debut_attente;
            return;
        }

    } else if (res == 0) {

        if (ezx.timer_nb == 0) {
            fprintf (stderr, "ez_event_next: select() should not return 0\n");
            goto debut_attente;
        }
        ev->type = TimerNotify;
        ev->win = ezx.timer_l[0].win;
        ez_timer_remove (ev->win);

    } else {
        perror ("ez_event_next: select()");
    }
}


/* Type pour XCheckIfEvent */
typedef struct { int later; Window win; } Ez_predicat_Expose;

/* Prototype impose par XCheckIfEvent */
Bool ez_predicat_expose (Display *display, XEvent *xev, XPointer arg)
{
    (void) display;  /* variable inutilise'e, annule le warning. */
    Ez_predicat_Expose *p = (Ez_predicat_Expose *) arg;
    if (xev->type == Expose && xev->xexpose.window == p->win)
        p->later = 1;
    return 0;
}

/*
 * Teste si c'est le dernier Expose present dans la queue, sans la modifier
 * pour preserver l'ordre temporel. Renvoie 1 vrai, 0 faux.
*/

int ez_is_last_expose (XEvent *xev)
{
    Ez_predicat_Expose p;
    XEvent ev_return;

    if (xev->xexpose.count > 0) return 0;

    p.later = 0;
    p.win = xev->xexpose.window;

    /* Examine toute la queue sans l'alterer, car la fonction
       de predicat ez_predicat_expose renvoie toujours FALSE */
    XCheckIfEvent (ezx.display, &ev_return, ez_predicat_expose, (XPointer) &p);

    return ! p.later;
}


/*
 * Analyse un evenement puis appelle la callback.
*/

void ez_event_dispatch (Ez_event *ev)
{
    /* Double-buffer eventuel */
    ezx.dbuf_pix = None; ezx.dbuf_win = None;

    /* On decode l'evenement. Si on decide de l'ignorer, il suffit de faire return */
    if (ev->type == EzLastEvent)
    switch (ev->xev.type) {

        /* Indique qu'il faut entierement redessiner la fenetre. */
        case Expose :
            /* Certains Expose seront ignores */
            if (ezx.last_expose && ! ez_is_last_expose (&ev->xev))
                return;
            ev->type = ev->xev.type;
            ev->win  = ev->xev.xexpose.window;
            ez_dbuf_get (ev->win, &ezx.dbuf_pix);
            if (ezx.dbuf_pix != None) ez_dbuf_preswap (ev->win);
            ez_window_clear (ev->win);
            break;

        /* Indique qu'un bouton de la souris a ete enfonce ou relache. */
        case ButtonPress :
            ev->type = ev->xev.type;
            ev->win  = ev->xev.xbutton.window;
            ev->mx   = ev->xev.xbutton.x;
            ev->my   = ev->xev.xbutton.y;
            ev->mb   = ev->xev.xbutton.button;
            /* Si un autre bouton est enfonce, il est ignore'. */
            if (ezx.mouse_b != 0) return;
            ezx.mouse_b = ev->mb;
            break;

        case ButtonRelease :
            ev->type = ev->xev.type;
            ev->win  = ev->xev.xbutton.window;
            ev->mx   = ev->xev.xbutton.x;
            ev->my   = ev->xev.xbutton.y;
            ev->mb   = ev->xev.xbutton.button;
            /* Si un autre bouton est relache', il est ignore'. */
            if (ezx.mouse_b != ev->mb) return;
            ezx.mouse_b = 0;
            break;

        /* Indique que la souris a ete deplace dans la fenetre. */
        case MotionNotify :
            ev->type = ev->xev.type;
            ev->win  = ev->xev.xmotion.window;
            ev->mx   = ev->xev.xmotion.x;
            ev->my   = ev->xev.xmotion.y;
            ev->mb   = ezx.mouse_b;  /* car pas de xmotion.button ! */
            break;

        /* Indique qu'une touche du clavier a ete enfonce ou relache. */
        case KeyPress :
        case KeyRelease :
            ev->type = ev->xev.type;
            ev->win  = ev->xev.xkey.window;
            ev->mx   = ev->xev.xkey.x;
            ev->my   = ev->xev.xkey.y;

            ev->key_count = XLookupString (&ev->xev.xkey, ev->key_string,
                sizeof(ev->key_string)-1, &ev->key_sym, NULL);
            ev->key_string[ev->key_count] = 0;  /* ajoute zero terminal */
            sprintf (ev->key_name, "XK_%s", XKeysymToString(ev->key_sym));
            break;

        /* Capte nouvelle taille de la fenetre */
        case ConfigureNotify  :
            ev->type   = ev->xev.type;
            ev->win    = ev->xev.xconfigure.window;
            ev->width  = ev->xev.xconfigure.width;
            ev->height = ev->xev.xconfigure.height;
            break;

        /* Intercepte fermeture d'un window : cf ez_auto_quit() */
        case ClientMessage :
            if ((Atom) ev->xev.xclient.message_type == ezx.atom_protoc &&
                (Atom) ev->xev.xclient.data.l[0]    == ezx.atom_delwin)
            {
                if (ezx.auto_quit) {
                    ez_quit ();
                    return;
                } else {
                    ev->type  = WindowClose;
                    ev->win   = ev->xev.xclient.window;
                }
            } else return;
            break;

        default : return;

    } /* Fin decodage evenement */

    /* On appelle la callback du window. */
    ez_func_call (ev);

    /* Swappe le double-buffer eventuel */
    if (ezx.dbuf_pix != None) ez_dbuf_swap (ev->win);
}

#else /* _WIN32 */


/*
 * La winproc appele'e par DispatchMessage ou par le systeme.
 * Renvoie 0L pour dire qu'on a traite' le message.
*/
LRESULT CALLBACK ez_win_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Ez_event ev;

    /* Init ev */
    memset (&ev, 0, sizeof(Ez_event));
    ev.win = None;  /* s'il reste a None, la callback n'est pas appele'e */
    ev.xev.hwnd = hwnd; ev.xev.message = msg;
    ev.xev.wParam = wParam; ev.xev.lParam = lParam;

    /* Double-buffer eventuel */
    ezx.dbuf_dc = None; ezx.dbuf_win = None;

#ifdef DEBUG
    if (msg != WM_SETCURSOR && msg != WM_NCHITTEST)
        printf ("    ez_win_proc : win = 0x%08x  MSG = 0x%04x = %-20s  wP = 0x%08x  lP = 0x%08x\n",
            (int) hwnd, (int) msg, ez_win_msg_name (msg), (int) wParam, (int) lParam);
#endif /* DEBUG */

    /* Decodage message */
    switch (msg) {

        case WM_PAINT :
            ValidateRect (hwnd, NULL);
        case EZ_MSG_PAINT :
            ev.type = Expose;
            ev.win = hwnd;
            ez_dbuf_get (ev.win, &ezx.dbuf_dc);
            if (ezx.dbuf_dc != None) ez_dbuf_preswap (ev.win);
#ifdef DEBUG
            printf ("Expose  win 0x%x  dbuf 0x%x\n", (int) ev.win, (int) ezx.dbuf_dc);
#endif /* DEBUG */
            ez_window_clear (ev.win);
            break;

        /* Indique qu'un bouton de la souris a ete enfonce ou relache. */
        case WM_LBUTTONDOWN :
        case WM_MBUTTONDOWN :
        case WM_RBUTTONDOWN :
            ev.type = ButtonPress;
            ev.win  = hwnd;
            ev.mx   = GET_X_LPARAM (lParam);
            ev.my   = GET_Y_LPARAM (lParam);
            ev.mb   = msg == WM_LBUTTONDOWN ? 1 :
                      msg == WM_MBUTTONDOWN ? 2 :
                      msg == WM_RBUTTONDOWN ? 3 : 4;
                /* Si un autre bouton est enfonce, il est ignore'. */
            if (ezx.mouse_b != 0) return 0L;
            ezx.mouse_b = ev.mb;
            /* On capture la souris pour recevoir les MotionNotify
               quand la souris sort bouton enfonce' */
            SetCapture (hwnd);
            break;

        case WM_LBUTTONUP :
        case WM_MBUTTONUP :
        case WM_RBUTTONUP :
            ev.type = ButtonRelease;
            ev.win  = hwnd;
            ev.mx   = GET_X_LPARAM (lParam);
            ev.my   = GET_Y_LPARAM (lParam);
            ev.mb   = msg == WM_LBUTTONUP ? 1 :
                      msg == WM_MBUTTONUP ? 2 :
                      msg == WM_RBUTTONUP ? 3 : 4;
            /* Si un autre bouton est relache', il est ignore'. */
            if (ezx.mouse_b != ev.mb) return 0L;
            ezx.mouse_b = 0;
            ReleaseCapture ();
            break;

        /* Indique que la souris a ete deplace dans la fenetre. */
        case WM_MOUSEMOVE :
            ev.type = MotionNotify;
            ev.win  = hwnd;
            ev.mx   = GET_X_LPARAM (lParam);
            ev.my   = GET_Y_LPARAM (lParam);
            ev.mb   = ezx.mouse_b;
            /* Si le meme evenement a deja ete envoye', il est ignore' */
            if (ezx.mv_win == ev.win && ezx.mv_x == ev.mx && ezx.mv_y == ev.my)
                    return 0L;
            ezx.mv_win = ev.win; ezx.mv_x = ev.mx; ezx.mv_y = ev.my;
            break;

        /* Indique qu'une touche du clavier a ete enfonce ou relache. */
        case WM_SYSKEYDOWN :
        case WM_KEYDOWN :
        case WM_CHAR :
            /* Certains caracteres on droit a deux evenements, on filtre. */
            if (msg != WM_CHAR) {
                if (ez_keydown_convert (wParam, lParam, &ezx.key_sym,
                     &ezx.key_name, &ezx.key_string) < 0 ) return 0L;
            } else {
                if (ez_keychar_convert (wParam, &ezx.key_sym,
                     &ezx.key_name, &ezx.key_string) < 0 ) return 0L;
            }

            ev.win     = hwnd;
            ev.mx      = ezx.mv_x;
            ev.my      = ezx.mv_y;
            ev.key_sym = ezx.key_sym;
            strcpy (ev.key_name  , ezx.key_name);
            strcpy (ev.key_string, ezx.key_string);
            ev.key_count = strlen (ezx.key_string);

            /* Si repetition on simule KeyRelease */
            if (lParam & (1L << 30)) { ev.type = KeyRelease; ez_func_call (&ev); }
            /* Maintenant on fait KeyPress */
            ev.type = KeyPress;  /* TODO: si ctrl,alt ou shift : ne faire ni Press ni Release */
            break;

        case WM_KEYUP :
        case WM_SYSKEYUP :
            if (ezx.key_sym == 0) return 0L;
            ev.type    = KeyRelease;
            ev.win     = hwnd;
            ev.mx      = ezx.mv_x;
            ev.my      = ezx.mv_y;
            ev.key_sym = ezx.key_sym;
            strcpy (ev.key_name  , ezx.key_name);
            strcpy (ev.key_string, ezx.key_string);
            ev.key_count = strlen (ezx.key_string);
            ezx.key_sym = 0;  /* RAZ */
            break;

        case WM_SIZE :
            ev.type   = ConfigureNotify;
            ev.win    = hwnd;
            ev.width  = LOWORD(lParam);
            ev.height = HIWORD(lParam);
            break;

        case WM_TIMER :
            ev.type   = TimerNotify;
            ev.win    = hwnd;
            /* RQ: en principe, wParam == EZ_TIMER1 */
            break;

        case WM_CLOSE :
            if (ezx.auto_quit) {
                ez_quit ();
                return 0L;
            } else {
                ev.type  = WindowClose;
                ev.win   = hwnd;
            }
            break;

        default:
            return DefWindowProc (hwnd, msg, wParam, lParam);

    } /* Fin decodage message */

    /* On appelle la callback du window. */
    ez_func_call (&ev);

    /* Swappe le double-buffer eventuel */
    if (ezx.dbuf_dc != None) ez_dbuf_swap (ev.win);

    return 0L;
}


/* 
 * Memorise cette fenetre en fenetre courante,
 * libere ou associe les handles de dessin.
*/

void ez_cur_win (Window win)
{
    if (ezx.dc_win == win) return;
    if (ezx.dc_win != None && ezx.dc_win != ezx.dbuf_win) 
        ReleaseDC (ezx.dc_win, ezx.hdc);

    ezx.dc_win = win;
    if (ezx.dc_win == None) return;
    
    if (ezx.dc_win != ezx.dbuf_win) 
         ezx.hdc = GetDC (ezx.dc_win);
    else ezx.hdc = ezx.dbuf_dc;

    if (ezx.hpen   != NULL) SelectObject (ezx.hdc, ezx.hpen);
    if (ezx.hbrush != NULL) SelectObject (ezx.hdc, ezx.hbrush);
    if (ezx.font[ezx.nfont] != NULL)
        SelectObject (ezx.hdc, ezx.font[ezx.nfont]);
    SetTextColor (ezx.hdc, ezx.color);
}


/*
 * Recree un pen pour le dessin avec la couleur et l'epaisseur courante.
*/

void ez_update_pen ()
{
    /* Pen pour dessin */
    if (ezx.hpen != NULL) DeleteObject (ezx.hpen);
    ezx.hpen = CreatePen (PS_SOLID, (ezx.thick == 1) ? 0 : ezx.thick, ezx.color);
    if (ezx.dc_win != None) SelectObject (ezx.hdc, ezx.hpen);
}

#endif /* _WIN32 */


/*
 * Initialise le ge'ne'rateur de nombres ale'atoires
*/

void ez_random_init ()
{
    srand ((int) time (NULL));
}


/*
 * Associe une callback func a un window.
 * Un nouvel appel ecrase l'ancienne callback.
 * Pour desactiver la callback, appeler avec func=NULL.
 * Renvoie 0 succes, -1 erreur.
*/

int ez_func_set (Window win, Ez_func func)
{
#ifndef _WIN32
    if (XSaveContext (ezx.display, win, ezx.func_xid, (XPointer) func) != 0) {
        fprintf (stderr, "ez_func_set: XSaveContext failed\n");
        return -1;
    }
#else /* _WIN32 */
    if (SetProp (win, ezx.func_prop, (HANDLE) func) == 0) {
        fprintf (stderr, "ez_func_set: SetProp failed\n");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}


/*
 * Libere la callback associe'e au window.
 * Renvoie 0 succes, -1 erreur.
*/

int ez_func_destroy (Window win)
{
#ifndef _WIN32
    return XDeleteContext (ezx.display, win, ezx.func_xid) == 0 ? 0 : -1;
#else /* _WIN32 */
    return RemoveProp (win, ezx.func_prop) == NULL ? -1 : 0;
#endif /* _WIN32 */
}


/*
 * Recupere la callback func associe'e a un window.
 * Renvoie 0 succes, -1 erreur
*/

int ez_func_get (Window win, Ez_func *func)
{
#ifndef _WIN32
    *func = NULL;
    if (XFindContext (ezx.display, win, ezx.func_xid, (XPointer*) func) != 0)
            return -1;
#else /* _WIN32 */
    *func = (Ez_func) GetProp (win, ezx.func_prop);
#endif /* _WIN32 */
    return *func == NULL ? -1 : 0;
}


/*
 * Cherche si une callback func a ete definie pour ce window.
 * Si oui, appelle func et renvoie 0 ; sinon renvoie -1.
*/

int ez_func_call (Ez_event *ev)
{
    Ez_func func;

    /* Pas de drawable */
    if (ev->win == None) return -1;

#ifndef _WIN32
    /* NoExpose et GraphicsExpose ne consernent que les pixmaps */
    if (ev->type == NoExpose || ev->type == GraphicsExpose) return -1;
#endif /* _WIN32 */

    /* Y-a-t'il une callback ? */
    if (ez_func_get (ev->win, &func) < 0) return -1;
    if (func == NULL) return -1;

    /* On appelle la callback */
    func (ev);

    return 0;
}


/*
 * Initialise le mode double-buffer, qui permet de rafraichir les fenetres
 * sans clignotement.
*/

void ez_dbuf_init ()
{
#ifndef _WIN32
    int m1, m2;
    /* On charge l'extension DBE */
    if (XdbeQueryExtension (ezx.display, &m1, &m2) == 0)
        fprintf (stderr, "ez_dbuf_init: DBE extension failed\n");
    ezx.dbuf_pix = None;
#else /* _WIN32 */
    ezx.dbuf_dc  = None;
#endif /* _WIN32 */
    ezx.dbuf_win = None;
}


/*
 * Memorise dbuf dans win.
 * Renvoie 0 succes, -1 erreur.
*/

int ez_dbuf_save (Window win, XdbeBackBuffer dbuf)
{
#ifndef _WIN32
    if (XSaveContext (ezx.display, win, ezx.dbuf_xid, (XPointer) dbuf) != 0) {
        fprintf (stderr, "ez_dbuf_set: XSaveContext failed\n");
        return -1;
    }
#else /* _WIN32 */
    if (SetProp (win, ezx.dbuf_prop, (HANDLE) dbuf) == 0) {
        fprintf (stderr, "ez_dbuf_set: SetProp failed\n");
        return -1;
    }
#endif /* _WIN32 */
    return 0;
}


/*
 * Libere le dbuf de win.
 * Renvoie 0 succes, -1 erreur.
*/

int ez_dbuf_delete (Window win)
{
#ifndef _WIN32
    return XDeleteContext (ezx.display, win, ezx.dbuf_xid) == 0 ? 0 : -1;
#else /* _WIN32 */
    return RemoveProp (win, ezx.dbuf_prop) == NULL ? -1 : 0;
#endif /* _WIN32 */
}


/*
 * Recupere le dbuf memorise dans win.
 * Renvoie 0 succes, -1 erreur
*/

int ez_dbuf_get (Window win, XdbeBackBuffer *dbuf)
{
#ifndef _WIN32
    *dbuf = None;
    if (XFindContext (ezx.display, win, ezx.dbuf_xid, (XPointer*) dbuf) != 0)
            return -1;
#else /* _WIN32 */
    *dbuf = (XdbeBackBuffer) GetProp (win, ezx.dbuf_prop);
#endif /* _WIN32 */
    return 0;
}


/*
 * Pre'paration du double buffer pour le swap.
*/

void ez_dbuf_preswap (Window win)
{
#ifndef _WIN32
    ezx.dbuf_win = win;
#else /* _WIN32 */
    ez_cur_win (win); 
    ez_window_get_size (win, &ezx.dbuf_w, &ezx.dbuf_h);
    ezx.hMemBmp = CreateCompatibleBitmap (ezx.hdc, ezx.dbuf_w, ezx.dbuf_h);
    ezx.hOldBmp = (HBITMAP) SelectObject (ezx.dbuf_dc, ezx.hMemBmp);
    ez_cur_win (None);
    ezx.dbuf_win = win;
#endif /* _WIN32 */
}


/*
 * Swappe les buffers d'affichage pour le window.
*/

void ez_dbuf_swap (Window win)
{
#ifndef _WIN32
    XdbeSwapInfo swap_info[] = { { win, XdbeUndefined } };
    XdbeSwapBuffers (ezx.display, swap_info, 1);
#else /* _WIN32 */
    ez_cur_win (None);
    ezx.dbuf_win = None;
    ez_cur_win (win);
    BitBlt (ezx.hdc, 0, 0, ezx.dbuf_w, ezx.dbuf_h, ezx.dbuf_dc, 0, 0, SRCCOPY);
    SelectObject (ezx.dbuf_dc, ezx.hOldBmp);
    DeleteObject (ezx.hMemBmp);
#endif /* _WIN32 */
}


/*
 * Init generale des fontes
*/

void ez_font_init ()
{
    int i;
    for (i = 0; i < EZ_FONT_MAX; i++)
        ezx.font[i] = NULL;
    ez_font_load (0, "6x13");
    ez_font_load (1, "8x16");
    ez_font_load (2, "10x20");
    ez_font_load (3, "12x24");
}


/*
 * Libere les fontes
*/

void ez_font_delete ()
{
#ifndef _WIN32
    int i;
    for (i = 0; i < EZ_FONT_MAX; i++)
    if (ezx.font[i] != NULL)
      { XFreeFont (ezx.display, ezx.font[i]); ezx.font[i] = NULL; }
#else /* _WIN32 */
    int i;
    for (i = 0; i < EZ_FONT_MAX; i++)
    if (ezx.font[i] != 0)
       { DeleteObject (ezx.font[i]); ezx.font[i] = NULL; }
#endif /* _WIN32 */
}


/*
 * Init des couleurs.
 *
 * Auteur : Regis.Barbanchon@lif.univ-mrs.fr
 * code extrait de MyX_Color.c
*/

int ez_color_init () {

#ifndef _WIN32

    ezx.visual = DefaultVisual (ezx.display, ezx.screen_num);

    switch (ezx.visual->class) {
        case PseudoColor : ez_init_PseudoColor (); break;
        case TrueColor   : ez_init_TrueColor   (); break;
        default : fprintf (stderr, "ez_init_color: unsupported Visual\n");
                  return -1;
    }

    ez_black   = ezx.black;
    ez_white   = ezx.white;
    
#else /* _WIN32 */

    ez_black   = ez_get_RGB (0, 0, 0);
    ez_white   = ez_get_RGB (255, 255, 255);
    
#endif /* _WIN32 */

    /* Autres couleurs predefinies */
    ez_grey    = ez_get_RGB (150, 150, 150);
    ez_red     = ez_get_RGB (255, 0, 0);
    ez_green   = ez_get_RGB (0, 255, 0);
    ez_blue    = ez_get_RGB (0, 0, 255);
    ez_yellow  = ez_get_RGB (255, 255, 0);
    ez_cyan    = ez_get_RGB (0, 255, 255);
    ez_magenta = ez_get_RGB (255, 0, 255);
    return 0;
}


#ifndef _WIN32

/*
 * Memorise une couleur dans la palette.
*/

void ez_set_palette (unsigned long pixel, int R, int G, int B,
    int max, int inCube)
{
    XColor c;

    c.pixel = pixel;
    c.red   = R / (double) max * 0xFFFF ;
    c.green = G / (double) max * 0xFFFF ;
    c.blue  = B / (double) max * 0xFFFF ;
    c.flags = DoRed | DoGreen | DoBlue;
    XStoreColor (ezx.display, ezx.pseudoColor.colormap, &c);
    if (inCube) ezx.pseudoColor.palette [R][G][B] = c.pixel;
    ezx.pseudoColor.samples[c.pixel]= c;
}


/*
 * Initialise les couleurs de la palette.
*/

void ez_init_PseudoColor ()
{
    int i, j, k;
    unsigned long pixel;

    ezx.pseudoColor.colormap =
        XCreateColormap (ezx.display, ezx.root_win, ezx.visual, AllocAll);

    for (i = 0, pixel = 0; i < 3; i++)
    for (j = 0; j < 6; j++)
    for (k = 0; k < 6; k++, pixel++)
        ez_set_palette (pixel, i, j, k, 5, 1);

    for (i = 0 ; i < 40; i++, pixel++)
        ez_set_palette (pixel, i, i, i, 39, 0);

    for (i = 3; i < 6; i++)
    for (j = 0; j < 6; j++)
    for (k = 0; k < 6; k++, pixel++)
        ez_set_palette (pixel, i, j, k, 5, 1);
}


/*
 * Initialise le mode vraies-couleurs.
*/

void ez_init_TrueColor ()
{
    ez_init_channel (&ezx.trueColor.blue , ezx.visual-> blue_mask);
    ez_init_channel (&ezx.trueColor.green, ezx.visual->green_mask);
    ez_init_channel (&ezx.trueColor.red  , ezx.visual->  red_mask);
}


/*
 * Initialise un canal de couleurs.
*/

void ez_init_channel (Ez_channel *channel, unsigned long mask)
{
    channel->mask = mask;
    channel->shift = channel->length = channel->max = 0;
    if (! mask) return;
    while ( (mask & 1) == 0 ) { channel->shift  ++; mask >>= 1; }
    while ( (mask & 1) == 1 ) { channel->length ++; mask >>= 1; }
    channel->max = channel->mask >> channel->shift;
}

#else /* _WIN32 */


/*
 * Conversions d'evenements claviers WIN32 --> X11
*/

int ez_keydown_convert (WPARAM wParam, LPARAM lParam, KeySym *k, char **n, char **s)
{
    char *es = "";

    switch (wParam) {
        case VK_PAUSE   : *k = XK_Pause  ; *n = "XK_Pause"  ; *s = es ; break;
        case VK_PRIOR   : *k = XK_Prior  ; *n = "XK_Prior"  ; *s = es ; break;
        case VK_NEXT    : *k = XK_Next   ; *n = "XK_Next"   ; *s = es ; break;
        case VK_END     : *k = XK_End    ; *n = "XK_End"    ; *s = es ; break;
        case VK_HOME    : *k = XK_Home   ; *n = "XK_Home"   ; *s = es ; break;
        case VK_LEFT    : *k = XK_Left   ; *n = "XK_Left"   ; *s = es ; break;
        case VK_UP      : *k = XK_Up     ; *n = "XK_Up"     ; *s = es ; break;
        case VK_RIGHT   : *k = XK_Right  ; *n = "XK_Right"  ; *s = es ; break;
        case VK_DOWN    : *k = XK_Down   ; *n = "XK_Down"   ; *s = es ; break;
        case VK_INSERT  : *k = XK_Insert ; *n = "XK_Insert" ; *s = es ; break;
        case VK_DELETE  : *k = XK_Delete ; *n = "XK_Delete" ; *s = es ; break;
        case VK_F1      : *k = XK_F1     ; *n = "XK_F1"     ; *s = es ; break;
        case VK_F2      : *k = XK_F2     ; *n = "XK_F2"     ; *s = es ; break;
        case VK_F3      : *k = XK_F3     ; *n = "XK_F3"     ; *s = es ; break;
        case VK_F4      : *k = XK_F4     ; *n = "XK_F4"     ; *s = es ; break;
        case VK_F5      : *k = XK_F5     ; *n = "XK_F5"     ; *s = es ; break;
        case VK_F6      : *k = XK_F6     ; *n = "XK_F6"     ; *s = es ; break;
        case VK_F7      : *k = XK_F7     ; *n = "XK_F7"     ; *s = es ; break;
        case VK_F8      : *k = XK_F8     ; *n = "XK_F8"     ; *s = es ; break;
        case VK_F9      : *k = XK_F9     ; *n = "XK_F9"     ; *s = es ; break;
        case VK_F10     : *k = XK_F10    ; *n = "XK_F10"    ; *s = es ; break;
			 
        case VK_SHIFT   : if (lParam & (1L << 20)) 
	                { *k = XK_Shift_R   ; *n = "XK_Shift_R"   ; *s = es; } 
	           else { *k = XK_Shift_L   ; *n = "XK_Shift_L"   ; *s = es; } break;
        case VK_CONTROL : if (lParam & (1L << 24)) 
	                { *k = XK_Control_R ; *n = "XK_Control_R" ; *s = es; } 
	           else { *k = XK_Control_L ; *n = "XK_Control_L" ; *s = es; } break;
        case VK_MENU    : if (lParam & (1L << 24)) 
	                { *k = XK_Alt_R     ; *n = "XK_Alt_R"     ; *s = es; } 
	           else { *k = XK_Alt_L     ; *n = "XK_Alt_L"     ; *s = es; } break;
        /* BUG: AltGr genere Control_L + ALt_R */

        /* Symboles restant a traiter */
     // case VK_CAPITAL  : *k = XK_ ; *n = "XK_"; *s = es; break;
     // case VK_NUMLOCK  : *k = XK_ ; *n = "XK_"; *s = es; break;
     // case VK_SCROLL   : *k = XK_ ; *n = "XK_"; *s = es; break;

        default : *k = 0; *n = es; *s = es; return -1;
    }

    return 0;
}

int ez_keychar_convert (WPARAM wParam, KeySym *k, char **n, char **s)
{
    char *es = "";

    switch (wParam) {
        case VK_BACK   : *k = XK_BackSpace ; *n = "XK_BackSpace" ; *s = es  ; break;
        case VK_TAB    : *k = XK_Tab       ; *n = "XK_Tab"       ; *s = es  ; break;
        case VK_RETURN : *k = XK_Return    ; *n = "XK_Return"    ; *s = es  ; break;
        case VK_ESCAPE : *k = XK_Escape    ; *n = "XK_Escape"    ; *s = es  ; break;
        case VK_SPACE  : *k = XK_space     ; *n = "XK_space"     ; *s = " " ; break;

        case 0x0021    : *k = XK_exclam    ; *n = "XK_exclam"    ; *s = "!" ; break;
        case 0x0022    : *k = XK_quotedbl  ; *n = "XK_quotedbl"  ; *s = "\""; break;
        case 0x0023    : *k = XK_numbersign; *n = "XK_numbersign"; *s = "#" ; break;
        case 0x0024    : *k = XK_dollar    ; *n = "XK_dollar"    ; *s = "$" ; break;
        case 0x0025    : *k = XK_percent   ; *n = "XK_percent"   ; *s = "%" ; break;
        case 0x0026    : *k = XK_ampersand ; *n = "XK_ampersand" ; *s = "&" ; break;
        case 0x0027    : *k = XK_apostrophe; *n = "XK_apostrophe"; *s = "'" ; break;
        case 0x0028    : *k = XK_parenleft ; *n = "XK_parenleft" ; *s = "(" ; break;
        case 0x0029    : *k = XK_parenright; *n = "XK_parenright"; *s = ")" ; break;
        case 0x002a    : *k = XK_asterisk  ; *n = "XK_asterisk"  ; *s = "*" ; break;
        case 0x002b    : *k = XK_plus      ; *n = "XK_plus"      ; *s = "+" ; break;
        case 0x002c    : *k = XK_comma     ; *n = "XK_comma"     ; *s = "," ; break;
        case 0x002d    : *k = XK_minus     ; *n = "XK_minus"     ; *s = "-" ; break;
        case 0x002e    : *k = XK_period    ; *n = "XK_period"    ; *s = "." ; break;
        case 0x002f    : *k = XK_slash     ; *n = "XK_slash"     ; *s = "/" ; break;

        case 0x0030 : *k = XK_0        ; *n = "XK_0"        ; *s = "0" ; break;
        case 0x0031 : *k = XK_1        ; *n = "XK_1"        ; *s = "1" ; break;
        case 0x0032 : *k = XK_2        ; *n = "XK_2"        ; *s = "2" ; break;
        case 0x0033 : *k = XK_3        ; *n = "XK_3"        ; *s = "3" ; break;
        case 0x0034 : *k = XK_4        ; *n = "XK_4"        ; *s = "4" ; break;
        case 0x0035 : *k = XK_5        ; *n = "XK_5"        ; *s = "5" ; break;
        case 0x0036 : *k = XK_6        ; *n = "XK_6"        ; *s = "6" ; break;
        case 0x0037 : *k = XK_7        ; *n = "XK_7"        ; *s = "7" ; break;
        case 0x0038 : *k = XK_8        ; *n = "XK_8"        ; *s = "8" ; break;
        case 0x0039 : *k = XK_9        ; *n = "XK_9"        ; *s = "9" ; break;
        case 0x003a : *k = XK_colon    ; *n = "XK_colon"    ; *s = ":" ; break;
        case 0x003b : *k = XK_semicolon; *n = "XK_semicolon"; *s = ";" ; break;
        case 0x003c : *k = XK_less     ; *n = "XK_less"     ; *s = "<" ; break;
        case 0x003d : *k = XK_equal    ; *n = "XK_equal"    ; *s = "=" ; break;
        case 0x003e : *k = XK_greater  ; *n = "XK_greater"  ; *s = ">" ; break;
        case 0x003f : *k = XK_question ; *n = "XK_question" ; *s = "?" ; break;
        case 0x0040 : *k = XK_at       ; *n = "XK_at"       ; *s = "@" ; break;
        case 0x0041 : *k = XK_A        ; *n = "XK_A"        ; *s = "A" ; break;
        case 0x0042 : *k = XK_B        ; *n = "XK_B"        ; *s = "B" ; break;
        case 0x0043 : *k = XK_C        ; *n = "XK_C"        ; *s = "C" ; break;
        case 0x0044 : *k = XK_D        ; *n = "XK_D"        ; *s = "D" ; break;
        case 0x0045 : *k = XK_E        ; *n = "XK_E"        ; *s = "E" ; break;
        case 0x0046 : *k = XK_F        ; *n = "XK_F"        ; *s = "F" ; break;
        case 0x0047 : *k = XK_G        ; *n = "XK_G"        ; *s = "G" ; break;
        case 0x0048 : *k = XK_H        ; *n = "XK_H"        ; *s = "H" ; break;
        case 0x0049 : *k = XK_I        ; *n = "XK_I"        ; *s = "I" ; break;
        case 0x004a : *k = XK_J        ; *n = "XK_J"        ; *s = "J" ; break;
        case 0x004b : *k = XK_K        ; *n = "XK_K"        ; *s = "K" ; break;
        case 0x004c : *k = XK_L        ; *n = "XK_L"        ; *s = "L" ; break;
        case 0x004d : *k = XK_M        ; *n = "XK_M"        ; *s = "M" ; break;
        case 0x004e : *k = XK_N        ; *n = "XK_N"        ; *s = "N" ; break;
        case 0x004f : *k = XK_O        ; *n = "XK_O"        ; *s = "O" ; break;
        case 0x0050 : *k = XK_P        ; *n = "XK_P"        ; *s = "P" ; break;
        case 0x0051 : *k = XK_Q        ; *n = "XK_Q"        ; *s = "Q" ; break;
        case 0x0052 : *k = XK_R        ; *n = "XK_R"        ; *s = "R" ; break;
        case 0x0053 : *k = XK_S        ; *n = "XK_S"        ; *s = "S" ; break;
        case 0x0054 : *k = XK_T        ; *n = "XK_T"        ; *s = "T" ; break;
        case 0x0055 : *k = XK_U        ; *n = "XK_U"        ; *s = "U" ; break;
        case 0x0056 : *k = XK_V        ; *n = "XK_V"        ; *s = "V" ; break;
        case 0x0057 : *k = XK_W        ; *n = "XK_W"        ; *s = "W" ; break;
        case 0x0058 : *k = XK_X        ; *n = "XK_X"        ; *s = "X" ; break;
        case 0x0059 : *k = XK_Y        ; *n = "XK_Y"        ; *s = "Y" ; break;
        case 0x005a : *k = XK_Z        ; *n = "XK_Z"        ; *s = "Z" ; break;

        case 0x005b : *k = XK_bracketleft  ; *n = "XK_bracketleft"  ; *s = "[" ; break;
        case 0x005c : *k = XK_backslash    ; *n = "XK_backslash"    ; *s = "\\"; break;
        case 0x005d : *k = XK_bracketright ; *n = "XK_bracketright" ; *s = "]" ; break;
        case 0x005e : *k = XK_asciicircum  ; *n = "XK_asciicircum"  ; *s = "^" ; break;
        case 0x005f : *k = XK_underscore   ; *n = "XK_underscore"   ; *s = "_" ; break;
        case 0x0060 : *k = XK_grave        ; *n = "XK_grave"        ; *s = "`" ; break;

        case 0x0061 : *k = XK_a        ; *n = "XK_a"        ; *s = "a" ; break;
        case 0x0062 : *k = XK_b        ; *n = "XK_b"        ; *s = "b" ; break;
        case 0x0063 : *k = XK_c        ; *n = "XK_c"        ; *s = "c" ; break;
        case 0x0064 : *k = XK_d        ; *n = "XK_d"        ; *s = "d" ; break;
        case 0x0065 : *k = XK_e        ; *n = "XK_e"        ; *s = "e" ; break;
        case 0x0066 : *k = XK_f        ; *n = "XK_f"        ; *s = "f" ; break;
        case 0x0067 : *k = XK_g        ; *n = "XK_g"        ; *s = "g" ; break;
        case 0x0068 : *k = XK_h        ; *n = "XK_h"        ; *s = "h" ; break;
        case 0x0069 : *k = XK_i        ; *n = "XK_i"        ; *s = "i" ; break;
        case 0x006a : *k = XK_j        ; *n = "XK_j"        ; *s = "j" ; break;
        case 0x006b : *k = XK_k        ; *n = "XK_k"        ; *s = "k" ; break;
        case 0x006c : *k = XK_l        ; *n = "XK_l"        ; *s = "l" ; break;
        case 0x006d : *k = XK_m        ; *n = "XK_m"        ; *s = "m" ; break;
        case 0x006e : *k = XK_n        ; *n = "XK_n"        ; *s = "n" ; break;
        case 0x006f : *k = XK_o        ; *n = "XK_o"        ; *s = "o" ; break;
        case 0x0070 : *k = XK_p        ; *n = "XK_p"        ; *s = "p" ; break;
        case 0x0071 : *k = XK_q        ; *n = "XK_q"        ; *s = "q" ; break;
        case 0x0072 : *k = XK_r        ; *n = "XK_r"        ; *s = "r" ; break;
        case 0x0073 : *k = XK_s        ; *n = "XK_s"        ; *s = "s" ; break;
        case 0x0074 : *k = XK_t        ; *n = "XK_t"        ; *s = "t" ; break;
        case 0x0075 : *k = XK_u        ; *n = "XK_u"        ; *s = "u" ; break;
        case 0x0076 : *k = XK_v        ; *n = "XK_v"        ; *s = "v" ; break;
        case 0x0077 : *k = XK_w        ; *n = "XK_w"        ; *s = "w" ; break;
        case 0x0078 : *k = XK_x        ; *n = "XK_x"        ; *s = "x" ; break;
        case 0x0079 : *k = XK_y        ; *n = "XK_y"        ; *s = "y" ; break;
        case 0x007a : *k = XK_z        ; *n = "XK_z"        ; *s = "z" ; break;

        case 0x007b : *k = XK_braceleft  ; *n = "XK_braceleft"  ; *s = "{" ; break;
        case 0x007c : *k = XK_bar        ; *n = "XK_bar"        ; *s = "|" ; break;
        case 0x007d : *k = XK_braceright ; *n = "XK_braceright" ; *s = "}" ; break;
        case 0x007e : *k = XK_asciitilde ; *n = "XK_asciitilde" ; *s = "~" ; break;

        case 0x0080 : *k = XK_EuroSign   ; *n = "XK_EuroSign"   ; *s = "\200" ; break;

        case 0x00a0 : *k = XK_nobreakspace   ; *n = "XK_nobreakspace"   ; *s = "\240"; break;
        case 0x00a1 : *k = XK_exclamdown     ; *n = "XK_exclamdown"     ; *s = "\241"; break;
        case 0x00a2 : *k = XK_cent           ; *n = "XK_cent"           ; *s = "\242"; break;
        case 0x00a3 : *k = XK_sterling       ; *n = "XK_sterling"       ; *s = "\243"; break;
        case 0x00a4 : *k = XK_currency       ; *n = "XK_currency"       ; *s = "\244"; break;
        case 0x00a5 : *k = XK_yen            ; *n = "XK_yen"            ; *s = "\245"; break;
        case 0x00a6 : *k = XK_brokenbar      ; *n = "XK_brokenbar"      ; *s = "\246"; break;
        case 0x00a7 : *k = XK_section        ; *n = "XK_section"        ; *s = "\247"; break;
        case 0x00a8 : *k = XK_diaeresis      ; *n = "XK_diaeresis"      ; *s = "\250"; break;
        case 0x00a9 : *k = XK_copyright      ; *n = "XK_copyright"      ; *s = "\251"; break;
        case 0x00aa : *k = XK_ordfeminine    ; *n = "XK_ordfeminine"    ; *s = "\252"; break;
        case 0x00ab : *k = XK_guillemotleft  ; *n = "XK_guillemotleft"  ; *s = "\253"; break;
        case 0x00ac : *k = XK_notsign        ; *n = "XK_notsign"        ; *s = "\254"; break;
        case 0x00ad : *k = XK_hyphen         ; *n = "XK_hyphen"         ; *s = "\255"; break;
        case 0x00ae : *k = XK_registered     ; *n = "XK_registered"     ; *s = "\256"; break;
        case 0x00af : *k = XK_macron         ; *n = "XK_macron"         ; *s = "\257"; break;
        case 0x00b0 : *k = XK_degree         ; *n = "XK_degree"         ; *s = "\260"; break;
        case 0x00b1 : *k = XK_plusminus      ; *n = "XK_plusminus"      ; *s = "\261"; break;
        case 0x00b2 : *k = XK_twosuperior    ; *n = "XK_twosuperior"    ; *s = "\262"; break;
        case 0x00b3 : *k = XK_threesuperior  ; *n = "XK_threesuperior"  ; *s = "\263"; break;
        case 0x00b4 : *k = XK_acute          ; *n = "XK_acute"          ; *s = "\264"; break;
        case 0x00b5 : *k = XK_mu             ; *n = "XK_mu"             ; *s = "\265"; break;
        case 0x00b6 : *k = XK_paragraph      ; *n = "XK_paragraph"      ; *s = "\266"; break;
        case 0x00b7 : *k = XK_periodcentered ; *n = "XK_periodcentered" ; *s = "\267"; break;
        case 0x00b8 : *k = XK_cedilla        ; *n = "XK_cedilla"        ; *s = "\270"; break;
        case 0x00b9 : *k = XK_onesuperior    ; *n = "XK_onesuperior"    ; *s = "\271"; break;
        case 0x00ba : *k = XK_masculine      ; *n = "XK_masculine"      ; *s = "\272"; break;
        case 0x00bb : *k = XK_guillemotright ; *n = "XK_guillemotright" ; *s = "\273"; break;
        case 0x00bc : *k = XK_onequarter     ; *n = "XK_onequarter"     ; *s = "\274"; break;
        case 0x00bd : *k = XK_onehalf        ; *n = "XK_onehalf"        ; *s = "\275"; break;
        case 0x00be : *k = XK_threequarters  ; *n = "XK_threequarters"  ; *s = "\276"; break;
        case 0x00bf : *k = XK_questiondown   ; *n = "XK_questiondown"   ; *s = "\277"; break;
        case 0x00c0 : *k = XK_Agrave         ; *n = "XK_Agrave"         ; *s = "\300"; break;
        case 0x00c1 : *k = XK_Aacute         ; *n = "XK_Aacute"         ; *s = "\301"; break;
        case 0x00c2 : *k = XK_Acircumflex    ; *n = "XK_Acircumflex"    ; *s = "\302"; break;
        case 0x00c3 : *k = XK_Atilde         ; *n = "XK_Atilde"         ; *s = "\303"; break;
        case 0x00c4 : *k = XK_Adiaeresis     ; *n = "XK_Adiaeresis"     ; *s = "\304"; break;
        case 0x00c5 : *k = XK_Aring          ; *n = "XK_Aring"          ; *s = "\305"; break;
        case 0x00c6 : *k = XK_AE             ; *n = "XK_AE"             ; *s = "\306"; break;
        case 0x00c7 : *k = XK_Ccedilla       ; *n = "XK_Ccedilla"       ; *s = "\307"; break;
        case 0x00c8 : *k = XK_Egrave         ; *n = "XK_Egrave"         ; *s = "\310"; break;
        case 0x00c9 : *k = XK_Eacute         ; *n = "XK_Eacute"         ; *s = "\311"; break;
        case 0x00ca : *k = XK_Ecircumflex    ; *n = "XK_Ecircumflex"    ; *s = "\312"; break;
        case 0x00cb : *k = XK_Ediaeresis     ; *n = "XK_Ediaeresis"     ; *s = "\313"; break;
        case 0x00cc : *k = XK_Igrave         ; *n = "XK_Igrave"         ; *s = "\314"; break;
        case 0x00cd : *k = XK_Iacute         ; *n = "XK_Iacute"         ; *s = "\315"; break;
        case 0x00ce : *k = XK_Icircumflex    ; *n = "XK_Icircumflex"    ; *s = "\316"; break;
        case 0x00cf : *k = XK_Idiaeresis     ; *n = "XK_Idiaeresis"     ; *s = "\317"; break;
        case 0x00d0 : *k = XK_ETH            ; *n = "XK_ETH"            ; *s = "\320"; break;
        case 0x00d1 : *k = XK_Ntilde         ; *n = "XK_Ntilde"         ; *s = "\321"; break;
        case 0x00d2 : *k = XK_Ograve         ; *n = "XK_Ograve"         ; *s = "\322"; break;
        case 0x00d3 : *k = XK_Oacute         ; *n = "XK_Oacute"         ; *s = "\323"; break;
        case 0x00d4 : *k = XK_Ocircumflex    ; *n = "XK_Ocircumflex"    ; *s = "\324"; break;
        case 0x00d5 : *k = XK_Otilde         ; *n = "XK_Otilde"         ; *s = "\325"; break;
        case 0x00d6 : *k = XK_Odiaeresis     ; *n = "XK_Odiaeresis"     ; *s = "\326"; break;
        case 0x00d7 : *k = XK_multiply       ; *n = "XK_multiply"       ; *s = "\327"; break;
        case 0x00d8 : *k = XK_Oslash         ; *n = "XK_Oslash"         ; *s = "\330"; break;
        case 0x00d9 : *k = XK_Ugrave         ; *n = "XK_Ugrave"         ; *s = "\331"; break;
        case 0x00da : *k = XK_Uacute         ; *n = "XK_Uacute"         ; *s = "\332"; break;
        case 0x00db : *k = XK_Ucircumflex    ; *n = "XK_Ucircumflex"    ; *s = "\333"; break;
        case 0x00dc : *k = XK_Udiaeresis     ; *n = "XK_Udiaeresis"     ; *s = "\334"; break;
        case 0x00dd : *k = XK_Yacute         ; *n = "XK_Yacute"         ; *s = "\335"; break;
        case 0x00de : *k = XK_THORN          ; *n = "XK_THORN"          ; *s = "\336"; break;
        case 0x00df : *k = XK_ssharp         ; *n = "XK_ssharp"         ; *s = "\337"; break;
        case 0x00e0 : *k = XK_agrave         ; *n = "XK_agrave"         ; *s = "\340"; break;
        case 0x00e1 : *k = XK_aacute         ; *n = "XK_aacute"         ; *s = "\341"; break;
        case 0x00e2 : *k = XK_acircumflex    ; *n = "XK_acircumflex"    ; *s = "\342"; break;
        case 0x00e3 : *k = XK_atilde         ; *n = "XK_atilde"         ; *s = "\343"; break;
        case 0x00e4 : *k = XK_adiaeresis     ; *n = "XK_adiaeresis"     ; *s = "\344"; break;
        case 0x00e5 : *k = XK_aring          ; *n = "XK_aring"          ; *s = "\345"; break;
        case 0x00e6 : *k = XK_ae             ; *n = "XK_ae"             ; *s = "\346"; break;
        case 0x00e7 : *k = XK_ccedilla       ; *n = "XK_ccedilla"       ; *s = "\347"; break;
        case 0x00e8 : *k = XK_egrave         ; *n = "XK_egrave"         ; *s = "\350"; break;
        case 0x00e9 : *k = XK_eacute         ; *n = "XK_eacute"         ; *s = "\351"; break;
        case 0x00ea : *k = XK_ecircumflex    ; *n = "XK_ecircumflex"    ; *s = "\352"; break;
        case 0x00eb : *k = XK_ediaeresis     ; *n = "XK_ediaeresis"     ; *s = "\353"; break;
        case 0x00ec : *k = XK_igrave         ; *n = "XK_igrave"         ; *s = "\354"; break;
        case 0x00ed : *k = XK_iacute         ; *n = "XK_iacute"         ; *s = "\355"; break;
        case 0x00ee : *k = XK_icircumflex    ; *n = "XK_icircumflex"    ; *s = "\356"; break;
        case 0x00ef : *k = XK_idiaeresis     ; *n = "XK_idiaeresis"     ; *s = "\357"; break;
        case 0x00f0 : *k = XK_eth            ; *n = "XK_eth"            ; *s = "\360"; break;
        case 0x00f1 : *k = XK_ntilde         ; *n = "XK_ntilde"         ; *s = "\361"; break;
        case 0x00f2 : *k = XK_ograve         ; *n = "XK_ograve"         ; *s = "\362"; break;
        case 0x00f3 : *k = XK_oacute         ; *n = "XK_oacute"         ; *s = "\363"; break;
        case 0x00f4 : *k = XK_ocircumflex    ; *n = "XK_ocircumflex"    ; *s = "\364"; break;
        case 0x00f5 : *k = XK_otilde         ; *n = "XK_otilde"         ; *s = "\365"; break;
        case 0x00f6 : *k = XK_odiaeresis     ; *n = "XK_odiaeresis"     ; *s = "\366"; break;
        case 0x00f7 : *k = XK_division       ; *n = "XK_division"       ; *s = "\367"; break;
        case 0x00f8 : *k = XK_oslash         ; *n = "XK_oslash"         ; *s = "\370"; break;
        case 0x00f9 : *k = XK_ugrave         ; *n = "XK_ugrave"         ; *s = "\371"; break;
        case 0x00fa : *k = XK_uacute         ; *n = "XK_uacute"         ; *s = "\372"; break;
        case 0x00fb : *k = XK_ucircumflex    ; *n = "XK_ucircumflex"    ; *s = "\373"; break;
        case 0x00fc : *k = XK_udiaeresis     ; *n = "XK_udiaeresis"     ; *s = "\374"; break;
        case 0x00fd : *k = XK_yacute         ; *n = "XK_yacute"         ; *s = "\375"; break;
        case 0x00fe : *k = XK_thorn          ; *n = "XK_thorn"          ; *s = "\376"; break;
        case 0x00ff : *k = XK_ydiaeresis     ; *n = "XK_ydiaeresis"     ; *s = "\377"; break;

        /* Symboles restant a traiter */
     // case ''     : *k = XK_Scroll_Lock    ; *n = "XK_Scroll_Lock"    ; *s = ""; break;
     // case ''     : *k = XK_Menu           ; *n = "XK_Menu"           ; *s = ""; break;
     // case ''     : *k = XK_Num_Lock       ; *n = "XK_Num_Lock"       ; *s = ""; break;
     // case ''     : *k = XK_KP_Enter       ; *n = "XK_KP_Enter"       ; *s = ""; break;
     // case ''     : *k = XK_KP_Home        ; *n = "XK_KP_Home"        ; *s = ""; break;
     // case ''     : *k = XK_KP_Left        ; *n = "XK_KP_Left"        ; *s = ""; break;
     // case ''     : *k = XK_KP_Up          ; *n = "XK_KP_Up"          ; *s = ""; break;
     // case ''     : *k = XK_KP_Right       ; *n = "XK_KP_Right"       ; *s = ""; break;
     // case ''     : *k = XK_KP_Down        ; *n = "XK_KP_Down"        ; *s = ""; break;
     // case ''     : *k = XK_KP_Prior       ; *n = "XK_KP_Prior"       ; *s = ""; break;
     // case ''     : *k = XK_KP_Next        ; *n = "XK_KP_Next"        ; *s = ""; break;
     // case ''     : *k = XK_KP_End         ; *n = "XK_KP_End"         ; *s = ""; break;
     // case ''     : *k = XK_KP_Begin       ; *n = "XK_KP_Begin"       ; *s = ""; break;
     // case ''     : *k = XK_KP_Equal       ; *n = "XK_KP_Equal"       ; *s = "="; break;
     // case ''     : *k = XK_KP_Multiply    ; *n = "XK_KP_Multiply"    ; *s = "*"; break;
     // case ''     : *k = XK_KP_Add         ; *n = "XK_KP_Add"         ; *s = "+"; break;
     // case ''     : *k = XK_KP_Separator   ; *n = "XK_KP_Separator"   ; *s = ""; break;
     // case ''     : *k = XK_KP_Subtract    ; *n = "XK_KP_Subtract"    ; *s = "-"; break;
     // case ''     : *k = XK_KP_Divide      ; *n = "XK_KP_Divide"      ; *s = "/"; break;
     // case ''     : *k = XK_KP_0           ; *n = "XK_KP_0"           ; *s = "0"; break;
     // case ''     : *k = XK_KP_1           ; *n = "XK_KP_1"           ; *s = "1"; break;
     // case ''     : *k = XK_KP_2           ; *n = "XK_KP_2"           ; *s = "2"; break;
     // case ''     : *k = XK_KP_3           ; *n = "XK_KP_3"           ; *s = "3"; break;
     // case ''     : *k = XK_KP_4           ; *n = "XK_KP_4"           ; *s = "4"; break;
     // case ''     : *k = XK_KP_5           ; *n = "XK_KP_5"           ; *s = "5"; break;
     // case ''     : *k = XK_KP_6           ; *n = "XK_KP_6"           ; *s = "6"; break;
     // case ''     : *k = XK_KP_7           ; *n = "XK_KP_7"           ; *s = "7"; break;
     // case ''     : *k = XK_KP_8           ; *n = "XK_KP_8"           ; *s = "8"; break;
     // case ''     : *k = XK_KP_9           ; *n = "XK_KP_9"           ; *s = "9"; break;

     // case ''     : *k = XK_Caps_Lock      ; *n = "XK_Caps_Lock"      ; *s = ""; break;
     // case ''     : *k = XK_Meta_L         ; *n = "XK_Meta_L"         ; *s = ""; break;
     // case ''     : *k = XK_Meta_R         ; *n = "XK_Meta_R"         ; *s = ""; break;
     // case ''     : *k = XK_Alt_L          ; *n = "XK_Alt_L"          ; *s = ""; break;
     // case ''     : *k = XK_Alt_R          ; *n = "XK_Alt_R"          ; *s = ""; break;

        default :
#ifdef DEBUG
            printf ("KEYCODE 0x%x\n", wParam);
#endif /* DEBUG */
            *k = 0; *n = es; *s = es; return -1;
    }

    return 0;
}


#ifdef DEBUG
char *ez_win_msg_name (unsigned int m)
{
    switch (m) {
        case 0x0000 : return "WM_NULL";
        case 0x0001 : return "WM_CREATE";
        case 0x0002 : return "WM_DESTROY";
        case 0x0003 : return "WM_MOVE";
        case 0x0005 : return "WM_SIZE";
        case 0x0006 : return "WM_ACTIVATE";
        case 0x0007 : return "WM_SETFOCUS";
        case 0x0008 : return "WM_KILLFOCUS";
        case 0x000A : return "WM_ENABLE";
        case 0x000B : return "WM_SETREDRAW";
        case 0x000C : return "WM_SETTEXT";
        case 0x000D : return "WM_GETTEXT";
        case 0x000E : return "WM_GETTEXTLENGTH";
        case 0x000F : return "WM_PAINT";
        case 0x0010 : return "WM_CLOSE";
        case 0x0011 : return "WM_QUERYENDSESSION";
        case 0x0013 : return "WM_QUERYOPEN";
        case 0x0016 : return "WM_ENDSESSION";
        case 0x0012 : return "WM_QUIT";
        case 0x0014 : return "WM_ERASEBKGND";
        case 0x0015 : return "WM_SYSCOLORCHANGE";
        case 0x0018 : return "WM_SHOWWINDOW";
        case 0x001A : return "WM_WININICHANGE";
        case 0x001B : return "WM_DEVMODECHANGE";
        case 0x001C : return "WM_ACTIVATEAPP";
        case 0x001D : return "WM_FONTCHANGE";
        case 0x001E : return "WM_TIMECHANGE";
        case 0x001F : return "WM_CANCELMODE";
        case 0x0020 : return "WM_SETCURSOR";
        case 0x0021 : return "WM_MOUSEACTIVATE";
        case 0x0022 : return "WM_CHILDACTIVATE";
        case 0x0023 : return "WM_QUEUESYNC";
        case 0x0024 : return "WM_GETMINMAXINFO";
        case 0x0026 : return "WM_PAINTICON";
        case 0x0027 : return "WM_ICONERASEBKGND";
        case 0x0028 : return "WM_NEXTDLGCTL";
        case 0x002A : return "WM_SPOOLERSTATUS";
        case 0x002B : return "WM_DRAWITEM";
        case 0x002C : return "WM_MEASUREITEM";
        case 0x002D : return "WM_DELETEITEM";
        case 0x002E : return "WM_VKEYTOITEM";
        case 0x002F : return "WM_CHARTOITEM";
        case 0x0030 : return "WM_SETFONT";
        case 0x0031 : return "WM_GETFONT";
        case 0x0032 : return "WM_SETHOTKEY";
        case 0x0033 : return "WM_GETHOTKEY";
        case 0x0037 : return "WM_QUERYDRAGICON";
        case 0x0039 : return "WM_COMPAREITEM";
        case 0x003D : return "WM_GETOBJECT";
        case 0x0041 : return "WM_COMPACTING";
        case 0x0044 : return "WM_COMMNOTIFY";
        case 0x0046 : return "WM_WINDOWPOSCHANGING";
        case 0x0047 : return "WM_WINDOWPOSCHANGED";
        case 0x0048 : return "WM_POWER";
        case 0x004A : return "WM_COPYDATA";
        case 0x004B : return "WM_CANCELJOURNAL";
        case 0x004E : return "WM_NOTIFY";
        case 0x0050 : return "WM_INPUTLANGCHANGEREQUEST";
        case 0x0051 : return "WM_INPUTLANGCHANGE";
        case 0x0052 : return "WM_TCARD";
        case 0x0053 : return "WM_HELP";
        case 0x0054 : return "WM_USERCHANGED";
        case 0x0055 : return "WM_NOTIFYFORMAT";
        case 0x007B : return "WM_CONTEXTMENU";
        case 0x007C : return "WM_STYLECHANGING";
        case 0x007D : return "WM_STYLECHANGED";
        case 0x007E : return "WM_DISPLAYCHANGE";
        case 0x007F : return "WM_GETICON";
        case 0x0080 : return "WM_SETICON";
        case 0x0081 : return "WM_NCCREATE";
        case 0x0082 : return "WM_NCDESTROY";
        case 0x0083 : return "WM_NCCALCSIZE";
        case 0x0084 : return "WM_NCHITTEST";
        case 0x0085 : return "WM_NCPAINT";
        case 0x0086 : return "WM_NCACTIVATE";
        case 0x0087 : return "WM_GETDLGCODE";
        case 0x0088 : return "WM_SYNCPAINT";
        case 0x00A0 : return "WM_NCMOUSEMOVE";
        case 0x00A1 : return "WM_NCLBUTTONDOWN";
        case 0x00A2 : return "WM_NCLBUTTONUP";
        case 0x00A3 : return "WM_NCLBUTTONDBLCLK";
        case 0x00A4 : return "WM_NCRBUTTONDOWN";
        case 0x00A5 : return "WM_NCRBUTTONUP";
        case 0x00A6 : return "WM_NCRBUTTONDBLCLK";
        case 0x00A7 : return "WM_NCMBUTTONDOWN";
        case 0x00A8 : return "WM_NCMBUTTONUP";
        case 0x00A9 : return "WM_NCMBUTTONDBLCLK";
        case 0x00AB : return "WM_NCXBUTTONDOWN";
        case 0x00AC : return "WM_NCXBUTTONUP";
        case 0x00AD : return "WM_NCXBUTTONDBLCLK";
        case 0x00FF : return "WM_INPUT";
        case 0x0100 : return "WM_KEYDOWN";
        case 0x0101 : return "WM_KEYUP";
        case 0x0102 : return "WM_CHAR";
        case 0x0103 : return "WM_DEADCHAR";
        case 0x0104 : return "WM_SYSKEYDOWN";
        case 0x0105 : return "WM_SYSKEYUP";
        case 0x0106 : return "WM_SYSCHAR";
        case 0x0107 : return "WM_SYSDEADCHAR";
        case 0x0109 : return "WM_UNICHAR";
        case 0x010D : return "WM_IME_STARTCOMPOSITION";
        case 0x010E : return "WM_IME_ENDCOMPOSITION";
        case 0x010F : return "WM_IME_COMPOSITION";
        case 0x0110 : return "WM_INITDIALOG";
        case 0x0111 : return "WM_COMMAND";
        case 0x0112 : return "WM_SYSCOMMAND";
        case 0x0113 : return "WM_TIMER";
        case 0x0114 : return "WM_HSCROLL";
        case 0x0115 : return "WM_VSCROLL";
        case 0x0116 : return "WM_INITMENU";
        case 0x0117 : return "WM_INITMENUPOPUP";
        case 0x011F : return "WM_MENUSELECT";
        case 0x0120 : return "WM_MENUCHAR";
        case 0x0121 : return "WM_ENTERIDLE";
        case 0x0122 : return "WM_MENURBUTTONUP";
        case 0x0123 : return "WM_MENUDRAG";
        case 0x0124 : return "WM_MENUGETOBJECT";
        case 0x0125 : return "WM_UNINITMENUPOPUP";
        case 0x0126 : return "WM_MENUCOMMAND";
        case 0x0127 : return "WM_CHANGEUISTATE";
        case 0x0128 : return "WM_UPDATEUISTATE";
        case 0x0129 : return "WM_QUERYUISTATE";
        case 0x0132 : return "WM_CTLCOLORMSGBOX";
        case 0x0133 : return "WM_CTLCOLOREDIT";
        case 0x0134 : return "WM_CTLCOLORLISTBOX";
        case 0x0135 : return "WM_CTLCOLORBTN";
        case 0x0136 : return "WM_CTLCOLORDLG";
        case 0x0137 : return "WM_CTLCOLORSCROLLBAR";
        case 0x0138 : return "WM_CTLCOLORSTATIC";
        case 0x0200 : return "WM_MOUSEMOVE";
        case 0x0201 : return "WM_LBUTTONDOWN";
        case 0x0202 : return "WM_LBUTTONUP";
        case 0x0203 : return "WM_LBUTTONDBLCLK";
        case 0x0204 : return "WM_RBUTTONDOWN";
        case 0x0205 : return "WM_RBUTTONUP";
        case 0x0206 : return "WM_RBUTTONDBLCLK";
        case 0x0207 : return "WM_MBUTTONDOWN";
        case 0x0208 : return "WM_MBUTTONUP";
        case 0x0209 : return "WM_MBUTTONDBLCLK";
        case 0x020A : return "WM_MOUSEWHEEL";
        case 0x020B : return "WM_XBUTTONDOWN";
        case 0x020C : return "WM_XBUTTONUP";
        case 0x020D : return "WM_XBUTTONDBLCLK";
        case 0x0210 : return "WM_PARENTNOTIFY";
        case 0x0211 : return "WM_ENTERMENULOOP";
        case 0x0212 : return "WM_EXITMENULOOP";
        case 0x0213 : return "WM_NEXTMENU";
        case 0x0214 : return "WM_SIZING";
        case 0x0215 : return "WM_CAPTURECHANGED";
        case 0x0216 : return "WM_MOVING";
        case 0x0218 : return "WM_POWERBROADCAST";
        case 0x0220 : return "WM_MDICREATE";
        case 0x0221 : return "WM_MDIDESTROY";
        case 0x0222 : return "WM_MDIACTIVATE";
        case 0x0223 : return "WM_MDIRESTORE";
        case 0x0224 : return "WM_MDINEXT";
        case 0x0225 : return "WM_MDIMAXIMIZE";
        case 0x0226 : return "WM_MDITILE";
        case 0x0227 : return "WM_MDICASCADE";
        case 0x0228 : return "WM_MDIICONARRANGE";
        case 0x0229 : return "WM_MDIGETACTIVE";
        case 0x0230 : return "WM_MDISETMENU";
        case 0x0231 : return "WM_ENTERSIZEMOVE";
        case 0x0232 : return "WM_EXITSIZEMOVE";
        case 0x0233 : return "WM_DROPFILES";
        case 0x0234 : return "WM_MDIREFRESHMENU";
        case 0x0281 : return "WM_IME_SETCONTEXT";
        case 0x0282 : return "WM_IME_NOTIFY";
        case 0x0283 : return "WM_IME_CONTROL";
        case 0x0284 : return "WM_IME_COMPOSITIONFULL";
        case 0x0285 : return "WM_IME_SELECT";
        case 0x0286 : return "WM_IME_CHAR";
        case 0x0288 : return "WM_IME_REQUEST";
        case 0x0290 : return "WM_IME_KEYDOWN";
        case 0x0291 : return "WM_IME_KEYUP";
        case 0x02A1 : return "WM_MOUSEHOVER";
        case 0x02A3 : return "WM_MOUSELEAVE";
        case 0x02A0 : return "WM_NCMOUSEHOVER";
        case 0x02A2 : return "WM_NCMOUSELEAVE";
        case 0x02B1 : return "WM_WTSSESSION_CHANGE";
        case 0x02c0 : return "WM_TABLET_FIRST";
        case 0x02df : return "WM_TABLET_LAST";
        case 0x0300 : return "WM_CUT";
        case 0x0301 : return "WM_COPY";
        case 0x0302 : return "WM_PASTE";
        case 0x0303 : return "WM_CLEAR";
        case 0x0304 : return "WM_UNDO";
        case 0x0305 : return "WM_RENDERFORMAT";
        case 0x0306 : return "WM_RENDERALLFORMATS";
        case 0x0307 : return "WM_DESTROYCLIPBOARD";
        case 0x0308 : return "WM_DRAWCLIPBOARD";
        case 0x0309 : return "WM_PAINTCLIPBOARD";
        case 0x030A : return "WM_VSCROLLCLIPBOARD";
        case 0x030B : return "WM_SIZECLIPBOARD";
        case 0x030C : return "WM_ASKCBFORMATNAME";
        case 0x030D : return "WM_CHANGECBCHAIN";
        case 0x030E : return "WM_HSCROLLCLIPBOARD";
        case 0x030F : return "WM_QUERYNEWPALETTE";
        case 0x0310 : return "WM_PALETTEISCHANGING";
        case 0x0311 : return "WM_PALETTECHANGED";
        case 0x0312 : return "WM_HOTKEY";
        case 0x0317 : return "WM_PRINT";
        case 0x0318 : return "WM_PRINTCLIENT";
        case 0x0319 : return "WM_APPCOMMAND";
        case 0x031A : return "WM_THEMECHANGED";
        case 0x0358 : return "WM_HANDHELDFIRST";
        case 0x035F : return "WM_HANDHELDLAST";
        case 0x0360 : return "WM_AFXFIRST";
        case 0x037F : return "WM_AFXLAST";
        case 0x0380 : return "WM_PENWINFIRST";
        case 0x038F : return "WM_PENWINLAST";
        case 0x8000 : return "WM_APP";
    }
    return "*** UNKNOWN ***";
}
#endif /* DEBUG */

#endif /* _WIN32 */

