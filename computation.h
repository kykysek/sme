#ifndef __COMPUTATION_H__
#define __COMPUTATION_H__

#include <stdbool.h>

#include "messages.h"

void computation_init(void);
void computation_cleanup(void);
void startpoint_init(double x, double y);
void zoom_init(double zoom);
void get_grid_size(int *w, int *h);
bool is_computing(void);
bool is_done(void);
bool is_abort(void);
void set_delete(void);
bool is_delete(void);
bool is_blank(void);

void abort_comp(void);
void enable_comp(void);
void delete_comp(void);
void refresh_comp(void); 

bool set_compute(message *msg);
bool compute(message *msg);

void update_image(int w, int h, unsigned char *img);
void delete_image(int w, int h, unsigned char *img);
void update_data(const msg_compute_data *compute_data);

#endif

/* end of computation.h */