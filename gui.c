
#include <SDL2/SDL.h>
#include "xwin_sdl.h"

#include "utils.h"
#include "computation.h"

#include "event_queue.h"
#include "gui.h"


#ifndef SDL_EVENT_POLL_WAIT_MS
#define SDL_EVENT_POLL_WAIT_MS 10
#endif
//inicializujeme a refreshujem z hlavního vlákna
static struct {
    int w;
    int h;
    unsigned char *img;
} gui = { .img = NULL };
//vezme hodnoty z nainicializovanyho gridu a inicializuje okno pro obrazek?
void gui_init(void)
{
    get_grid_size(&gui.w, &gui.h);
    gui.img = my_alloc(gui.w * gui.h * 3);
    my_assert(xwin_init(gui.w, gui.h) == 0, __func__, __LINE__, __FILE__);
}
void gui_cleanup(void)
{
    if (gui.img) {
        free(gui.img);
        gui.img = NULL;
    }
    xwin_close();
}
//updatuje obrazek
void gui_refresh(void)
{
    if (gui.img) {
        update_image(gui.w, gui.h, gui.img);
        xwin_redraw(gui.w, gui.h, gui.img);
    }
}
//komuunikace s oknem
void *gui_win_thread(void *d)
{
    info("gui_win_thread - start");
    bool quit = false;
    SDL_Event event_sdl;
    event ev;
    while (!quit) {
        ev.type = EV_TYPE_NUM;
        if (SDL_PollEvent(&event_sdl)) { //vrati true jestlize dostane zpravu
            if (event_sdl.type == SDL_KEYDOWN) {
                info("keydown");
                switch (event_sdl.key.keysym.sym) {//stisk klavesy
                    case SDLK_q:
                    info("quit");
                        ev.type = EV_QUIT;
                        //queue_push(ev);
                        break;
                    case SDLK_s:
                        ev.type = EV_SET_COMPUTE;
                        //queue_push(ev);
                        break;
                    case SDLK_a:
                        ev.type = EV_ABORT;
                        //queue_push(ev);
                        break;
                    case SDLK_c:
                        ev.type = EV_COMPUTE;
                        //queue_push(ev);
                        break;
                    case SDLK_g:
                        ev.type = EV_GET_VERSION;
                        //queue_push(ev);
                        break;
                    case SDLK_r:
                        //TODO resetuje cid 
                        ev.type = EV_REFRESH;
                        //queue_push(ev);
                        break;
                    case SDLK_p://pridano
                        // TODO překreslí obsah okna aktuálním stavem výpočtu (bufferem)
                        //queue_push(ev);
                        break;
                    case SDLK_l://pridano
                        // TODO  smaže aktuální obsah výpočtu (bufferu)
                        //queue_push(ev);
                        break;
                    case SDLK_KP_1://pridano
                        // TODO  spustí výpočet
                        ev.type = EV_COMPUTE;
                        break;
                }
            } else if (event_sdl.type == SDL_KEYUP) {
                info("keyup");
            } else if (event_sdl.type == SDL_MOUSEMOTION) {
                info("mousemmotion");
            }
            if (event_sdl.type == SDL_MOUSEBUTTONDOWN) {//pridano
                info("gui_win_thread - mousebuttondown");
                if (event_sdl.button.button == SDL_BUTTON_LEFT) {//pridano
                    int x = event_sdl.button.x;
                    int y = event_sdl.button.y; 
                    fprintf(stderr, "gui_win_thread - mousebuttondown - left - x: %d, y: %d", x, y);
                    //TODO 
                } else if (event_sdl.button.button == SDL_BUTTON_RIGHT) {//pridano
                    info("gui_win_thread - mousebuttondown - right");
                }
            }
        }
        if (ev.type != EV_TYPE_NUM) {
            queue_push(ev);
        }
        SDL_Delay(SDL_EVENT_POLL_WAIT_MS);
        quit = is_quit();
    }
    set_quit();
    info("gui_win_thread - end");
    return NULL;
}

/* end of gui.c */