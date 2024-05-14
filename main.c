#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "utils.h"
#include "main.h"
#include "event_queue.h"
#include "messages.h"
#include "computation.h"
#include "gui.h"
//umi pracovat s ruznymi zpravami, ktere se objevuji mezi nasema vlaknama
//generuje zpravy pro nas vypocetni modul

static void process_pipe_message(event * const ev);

void* main_thread(void* d)
{
    my_assert(d != NULL, __func__, __LINE__, __FILE__);
    int pipe_out = *(int*)d;//nainicializuje pajpu
    message msg;//nainicializuje message
    uint8_t msg_buf[sizeof(message)];//buffer na max delkzu message
    int msg_len;
    bool quit = false;

    // initialize computation, visualize
    computation_init();
    gui_init();

    do {
        event ev = queue_pop();//precteni eventu
        msg.type = MSG_NBR;//inicializace na neprijatelny typ
        switch(ev.type) {
            case EV_QUIT:
                set_quit();
                debug("Quit received");
                break;
            case EV_GET_VERSION:
                msg.type = MSG_GET_VERSION;
                break;
            case EV_SET_COMPUTE:
                info ( set_compute(&msg) ? "Set compute" : "Set compute failed");
                break;
            case EV_COMPUTE:   
                enable_comp();         
                info (compute(&msg) ? "Compute" : "Compute failed");
                break;
            case EV_ABORT:
                msg.type = MSG_ABORT;
                break;
            case EV_PIPE_IN_MESSAGE:
                process_pipe_message(&ev);
                break;
            default:
                break;
        } // switch end
        if (msg.type != MSG_NBR) {
            my_assert(fill_message_buf(&msg, msg_buf, sizeof(msg_buf), &msg_len), __func__, __LINE__, __FILE__);
            if (write(pipe_out, msg_buf, msg_len) == msg_len) {
                debug("sent data to pipe_out\n");
            } else {
                error("send messange fail\n");
            }
        }
       quit = is_quit();
    } while (!quit);    

    // cleanup computation, visualize
    computation_cleanup();
    gui_cleanup();
    return NULL;
}

void process_pipe_message(event * const ev)
{
    my_assert(ev != NULL && ev->type == EV_PIPE_IN_MESSAGE && ev->data.msg, __func__, __LINE__, __FILE__);
    ev->type = EV_TYPE_NUM; //inicializace na nevhodny typ
    const message *msg = ev->data.msg;
    switch (msg->type) {
    case MSG_OK:
        info("OK");
        break;
    case MSG_COMPUTE_DATA:
        if (!is_abort()){ 
            update_data(&(msg->data.compute_data));
        }
        break;
    case MSG_DONE://potrebujem pocitat novej chunk
        gui_refresh();
        if (is_done()){
            info("computation_done");
        } else {//zavola case MSG_COMPUTE
            event ev = { .type = EV_COMPUTE };
            queue_push(ev);
        }
        break;
    case MSG_VERSION:
        fprintf(stderr, "INFO: Module version %d.%d-p%d\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
        break;
    case MSG_ABORT:
        info("Computation aborted");
        abort_comp();
        break;
    //TODO '1', 
    case MSG_COMPUTE:
        //TODO 
        info("Compute");
        break;
    default:
        fprintf(stderr, "Unhandled pipe message type %d\n", msg->type);
        break;
    }
    free(ev->data.msg);
    ev->data.msg = NULL;
}

/* end of b3b36prg-sem/main.c */