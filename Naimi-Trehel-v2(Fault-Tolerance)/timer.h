#include <stdlib.h>

#include "node.h"

// Enumeração de tipos possíveis de um timer.
typedef enum timerType {
   singleShot,
   periodic
} s_TT;

typedef void (*timeOutHandler) (size_t timerId, void *userData);

struct timer_node {
    int fd;
    timeOutHandler callbackTimeOutHandler;
    void *userData;
    unsigned int interval;
    s_TT type;
    struct timer_node *next;
} timer_node;

void initialize_timer_thread();

size_t start_timer(size_t timer_id, unsigned int timerInterval, timeOutHandler handler, s_TT type, void *userData);

static void *jobTimer();

struct timer_node *get_timer_from_fd(int fd);

void cancel_timer(size_t timerId);

void finalize_timer_thread();
