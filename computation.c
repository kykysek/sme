#include <string.h>
#include "computation.h"
#include "xwin_sdl.h"
#include "utils.h"


//s timhle pracujem jen v ramci modulu
static struct {
    double c_re;//pocatecni souradnice
    double c_im;
    int n; //pocet iteraci 

    double range_re_min; //meze intervalu
    double range_re_max;
    double range_im_min;
    double range_im_max;

    int grid_w;//vel gridu
    int grid_h;

    int cur_x; //kurzor
    int cur_y;

    double d_re; //sample step - navzorkovani intervalu
    double d_im;

    int nbr_chunks; //rozdeleni obrazku, pocet chunku na 1 obrazek
    int cid;//dany vyrez
    double chunk_re; //zacatek vyrezu
    double chunk_im;//zacatek vyrezu

    uint8_t chunk_n_re; //pocet vzorku 1 chunku v realne a v im ose
    uint8_t chunk_n_im;

    uint8_t *grid; //vypocetni grid
    bool computing; //indikator ze pocitam (nemuzu pocitat vse najednou, zpravy budu pocitat nekolikrat)
    bool abort;
    bool done;//hotovo
    bool delete;
    bool blank;

//pocita fraktal
} comp = {//inicializacni hodnoty

    // TODO change the fractol structor 
    .c_re = -0.4*1,//zacatek ze zadani
    .c_im = 0.6*1,
    // TODO change the color of the fractol
    .n = 60*1, //pocet iteraci ze zadani

    // TODO zooming to center replace the 1 with a zoom factor >1 smaller <1 bigger
    .range_re_min = -1.6*1,//rozsah ze zadani
    .range_re_max = 1.6*1,    
    .range_im_min = -1.1*1,
    .range_im_max = 1.1*1,

    .grid_w = 640, //velikost okna ze zadani
    .grid_h = 480,

    .cur_x = 0, //poc poloha kurzoru
    .cur_y = 0,

    .d_re = 0.0,
    .d_im = 0.0,

    .nbr_chunks = 0,
    .cid = 0,
    .chunk_re = 0.0,
    .chunk_im = 0.0,

    .chunk_n_re = 64, //velikos 1 bloku 64x48
    .chunk_n_im = 48,

    .grid = NULL,
    .computing = false,
    .abort = false,
    .done = false,
    .delete = false,
    .blank = true
};
//zvetsi obrazek
void zoom_init(double zoom) {
    comp.range_re_min = -1.6*zoom;//rozsah ze zadani
    comp.range_re_max = 1.6*zoom;
    comp.range_im_min = -1.1*zoom;
    comp.range_im_max = 1.1*zoom;
}
//posune stred
void startpoint_init(double x, double y) {
    comp.range_re_min += x ;
    comp.range_re_max += x  ;
    comp.range_im_min += y;
    comp.range_im_max += y;
}

//inicializuje grid podle zadani
void computation_init(void)
{
    comp.grid = my_alloc(comp.grid_w * comp.grid_h * sizeof(uint8_t));// allokuje pamet pro grid
    comp.d_re = (comp.range_re_max - comp.range_re_min) / (1. * comp.grid_w);//vypocet re step size
    comp.d_im = -(comp.range_im_max - comp.range_im_min) / (1. * comp.grid_h); // vypocet im step size
    comp.nbr_chunks = (comp.grid_h / comp.chunk_n_im) * (comp.grid_w / comp.chunk_n_re); //vypocet poctu chunku
}
// smaze data
void computation_cleanup(void)
{
    if (comp.grid) {
        free(comp.grid);
    }
    comp.grid = NULL;

}
//zjisti jestli se prave ted pocita
bool is_computing(void) 
{ 
    return comp.computing; 
}

bool is_done(void) 
{ 
    return comp.done; 
}
//indikuje jestli computing je false
bool is_abort(void) 
{ 
    return comp.abort;
}

void get_grid_size(int *w, int *h)
{
    *w = comp.grid_w;
    *h = comp.grid_h;
}
//zastavi computation
void abort_comp(void) 
{ 
    comp.abort = true; 
}
// znovu spusti computation (po abortu)
void enable_comp(void) 
{ 
    comp.abort = false;
    comp.done = false; 
}



//zacne pocitat a tisknout obrazek od zacatku
void refresh_comp(void) {
    //abort_comp();
    comp.cid = 0;
    comp.chunk_im = comp.range_im_min;
    comp.chunk_re = comp.range_re_min;
    computation_init();
    comp.computing = false;
    comp.blank = true;
}
//inicializuje message s compute parametry
bool set_compute(message *msg)
{
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);
    bool ret = !is_computing();
    if (ret) {//set_compute volam jen kdyz prave nepocitam
        msg->type = MSG_SET_COMPUTE;
        msg->data.set_compute.c_re = comp.c_re;
        msg->data.set_compute.c_im = comp.c_im;
        msg->data.set_compute.d_re = comp.d_re;
        msg->data.set_compute.d_im = comp.d_im;
        msg->data.set_compute.n = comp.n;
        comp.done = false;//iniciuju novej vypocet takze done je false
    }
    return ret;
}
bool compute(message *msg) //
{      //inicializujeme vypocet
    my_assert(msg != NULL, __func__, __LINE__, __FILE__);//ERROR?
    if (!is_computing()) { // prvni chunk - zaciname od zacatku
        comp.cid = 0; //
        comp.computing = true;
        comp.cur_x = comp.cur_y = 0; // start computation of the whole image kurzor 0
        comp.chunk_re = comp.range_re_min; //left corner 
        comp.chunk_im = comp.range_im_max; //upper left corner (zacatek 0 0 = vlevo  nahore)
        msg->type = MSG_COMPUTE;
    } else { // nepocitame prvni chunk, navazujem na vypocet
        comp.cid += 1;
        if (comp.cid < comp.nbr_chunks) {
            comp.cur_x += comp.chunk_n_re;//presouvam se na dalsi okno
            comp.chunk_re += comp.chunk_n_re * comp.d_re;
            if (comp.cur_x >= comp.grid_w) {
                comp.chunk_re = comp.range_re_min;
                comp.chunk_im += comp.chunk_n_im * comp.d_im;
                comp.cur_x = 0;
                comp.cur_y += comp.chunk_n_im;
            }
            msg->type = MSG_COMPUTE;
        } else {
            msg->type = MSG_DONE;
        }
    }

    if (comp.computing && msg->type == MSG_COMPUTE) {
        msg->data.compute.cid = comp.cid;
        msg->data.compute.re = comp.chunk_re;
        msg->data.compute.im = comp.chunk_im;
        msg->data.compute.n_re = comp.chunk_n_re;
        msg->data.compute.n_im = comp.chunk_n_im;
    }
    return is_computing();
}
//ziska aktualne vypocitana data a vlozi je do obrazku
void update_image(int w, int h, unsigned char *img)//preklopi grid do obrazku
{
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__, __FILE__);
    for (int i = 0; i < w * h; i++) {
        const double t = 1. * comp.grid[i] / (comp.n + 1.0);//ze zadani, ziska zatim vypocitana data, vlozi je do obrazku
        *(img++) = 9 * (1 - t) * t * t * t * 255;//R
        *(img++) = 15 * (1 - t) * (1 - t) * t * t * 255;//G
        *(img++) = 8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255;//B
    }
}

void set_delete(void)
{
    comp.delete = true;
}

bool is_delete(void)
{
    return comp.delete;
}

bool is_blank(void)
{
    return comp.blank;
}

//resetuje chunk, nastavi grid na 0 a tim pak prekresli (vymaze) okno
void delete_image(int w, int h, unsigned char *img)
{   
    if (comp.done) {
        debug("comp done 1");
        set_delete();
    }
    else{
    abort_comp();
    comp.cid = 0;
    comp.cur_x = 0;
    comp.cur_y = 0;
    comp.chunk_im = comp.range_im_min;
    comp.chunk_re = comp.range_re_min;
    comp.computing = false;
    comp.done = true;
    my_assert(img && comp.grid && w == comp.grid_w && h == comp.grid_h, __func__, __LINE__, __FILE__);
    // nastaveni celeho gridu na 0
    if (comp.grid) {
        memset(comp.grid, 0, comp.grid_w * comp.grid_h * sizeof(uint8_t));
    }
    comp.blank = true;
    }
}

void update_data(const msg_compute_data *compute_data)
{
    comp.blank = false;
    my_assert(compute_data != NULL, __func__, __LINE__, __FILE__);
    if (compute_data->cid == comp.cid) { // kontrola ze jsme dostali data k momentalnimu chunku a ne jinemu
        //cur_x x-ova zacatku bloku
        const int idx = comp.cur_x + compute_data->i_re + (comp.cur_y + compute_data->i_im) * comp.grid_w;
        if (idx >= 0 && idx < comp.grid_w * comp.grid_h) {
            comp.grid[idx] = compute_data->iter; // vysledna data do gridu
        }
        if ((comp.cid + 1) >= comp.nbr_chunks && (compute_data->i_re + 1) == comp.chunk_n_re && (compute_data->i_im + 1) == comp.chunk_n_im) {
            comp.done = true; // vyplnen posledni chunk
            comp.blank = false;
            comp.computing = false;
        }
    } else if(!comp.done){
        warn("received chunk with unexpected chunk id (cid)\n");
    }
    
}


/* end of computation.c */