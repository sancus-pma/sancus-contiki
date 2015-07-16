#ifndef EVENTLOOP_H
#define EVENTLOOP_H

void event_loop_init(void);
void __attribute__((noreturn)) event_loop_start(void);

#endif
