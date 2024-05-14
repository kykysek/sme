#include <stdlib.h>
#include <stdio.h>

#include "keyboard.h"
#include "utils.h"
#include "event_queue.h"
//zpracovani 4 klaves
void* keyboard_thread(void* d)
{
    return NULL;
    fprintf(stderr, "Keyboard_thread - start\n");
    call_termios(0);
    int c;
    event ev;
    while (( c = getchar()) != 'q') {
        ev.type = EV_TYPE_NUM;
        switch(c) {
            case 'g':
                //get version
                ev.type = EV_GET_VERSION;
                break;
            case 'a':
                //abort
                ev.type = EV_ABORT;
                break;
            case 's':
                //setcompute
                ev.type = EV_SET_COMPUTE;
                break;
            case 'c':
                //compute
                ev.type = EV_COMPUTE;
                break;
            default:
                break;
        } //end switch
        if (ev.type != EV_TYPE_NUM) {
            queue_push(ev);
        }
    }// end while
    set_quit();
    ev.type = EV_QUIT;
    queue_push(ev);
    call_termios(1);
    fprintf(stderr, "Keyboard_thread - end\n");
    return NULL;
}