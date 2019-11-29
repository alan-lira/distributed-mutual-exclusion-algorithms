#include <unistd.h>
#include <sys/timerfd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <poll.h>

#include "timer.h"

#define MAX_TIMER_COUNT 1000

static pthread_t timerThread;

static struct timer_node *g_head = NULL;

void initialize_timer_thread() {

   pthread_create(&timerThread, NULL, jobTimer, NULL);

}

size_t start_timer(size_t timer_id, unsigned int timerInterval, timeOutHandler handler, s_TT type, void *userData) {

   struct timer_node *timerNode = (struct timer_node*) timer_id;

   if (timerNode == NULL) { // First use of this timer (create it).

      struct timer_node *new_node = NULL;

      struct itimerspec new_value;

      new_node = (struct timer_node*) malloc(sizeof(struct timer_node));

      if (new_node == NULL) {

         return 0;

      }

      new_node->callbackTimeOutHandler = handler;
      new_node->userData = userData;
      new_node->interval = timerInterval;
      new_node->type = type;

      new_node->fd = timerfd_create(CLOCK_REALTIME, 0);

      if (new_node->fd == -1) {

         if (new_node) {

            free(new_node);

         }

         return 0;

      }

      if (type == periodic) {

         new_value.it_value.tv_sec = 0;

         new_value.it_value.tv_nsec = 0;

         new_value.it_interval.tv_sec = timerInterval;

         new_value.it_interval.tv_nsec = timerInterval * 1000000;

      } else {

         new_value.it_value.tv_sec = timerInterval;

         new_value.it_value.tv_nsec = timerInterval * 1000000;

         new_value.it_interval.tv_sec = 0;

         new_value.it_interval.tv_nsec = 0;

      }

      timerfd_settime(new_node->fd, 0, &new_value, NULL);

      // Adding this timer node into the general timer nodes' list (g_head).
      new_node->next = g_head;

      g_head = new_node;

      free(timerNode);

      return (size_t) new_node;

   } else { // This time already exists (overwrite it's timeout value).

      close(timerNode->fd);

      struct itimerspec new_value;

      timerNode->callbackTimeOutHandler = handler;
      timerNode->userData = userData;
      timerNode->interval = timerInterval;
      timerNode->type = type;

      timerNode->fd = timerfd_create(CLOCK_REALTIME, 0);

      if (timerNode->fd == -1) {

         if (timerNode) {

            free(timerNode);

         }

         return 0;

      }

      if (type == periodic) {

         new_value.it_value.tv_sec = 0;

         new_value.it_value.tv_nsec = 0;

         new_value.it_interval.tv_sec = timerInterval;

         new_value.it_interval.tv_nsec = timerInterval * 1000000;

      } else {

         new_value.it_value.tv_sec = timerInterval;

         new_value.it_value.tv_nsec = timerInterval * 1000000;

         new_value.it_interval.tv_sec = 0;

         new_value.it_interval.tv_nsec = 0;

      }

      timerfd_settime(timerNode->fd, 0, &new_value, NULL);

      return timerNode;

   }

}

void *jobTimer() {

   struct pollfd ufds[MAX_TIMER_COUNT] = {{0}};

   int iMaxCount = 0;

   struct timer_node *tmp = NULL;

   int read_fds = 0, i, s;

   uint64_t exp;

   while (1) {

      pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
      pthread_testcancel();
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

      iMaxCount = 0;

      tmp = g_head;

      memset(ufds, 0, (sizeof(struct pollfd) * MAX_TIMER_COUNT));

      while (tmp) {

         ufds[iMaxCount].fd = tmp->fd;

         ufds[iMaxCount].events = POLLIN;

         iMaxCount++;

         tmp = tmp->next;

      }

      read_fds = poll(ufds, iMaxCount, 100);

      if (read_fds <= 0) {

         continue;

      }

      for (i = 0; i < iMaxCount; i++) {

         if (ufds[i].revents & POLLIN) {

            s = read(ufds[i].fd, &exp, sizeof(uint64_t));

            if (s != sizeof(uint64_t)) {

               continue;

            }

            tmp = get_timer_from_fd(ufds[i].fd);

            if (tmp && tmp->callbackTimeOutHandler) {

               tmp->callbackTimeOutHandler((size_t) tmp, tmp->userData);

            }

         }

      }

   }

   return NULL;

}

struct timer_node *get_timer_from_fd(int fd) {

   struct timer_node *tmp = g_head;

   while (tmp) {

      if (tmp->fd == fd) {

         return tmp;

      }

      tmp = tmp->next;

   }

   return NULL;

}

void stop_timer(size_t timer_id) {

   struct timer_node *node = (struct timer_node*) timer_id;

   if (node == NULL) {

      return;

   }

   close(node->fd);

}

void cancel_timer(size_t timer_id) {

   struct timer_node *node = (struct timer_node*) timer_id;

   if (node == NULL) {

      return;

   }

   struct timer_node *previous = NULL;

   struct timer_node *temp = g_head;

   while (temp != NULL && temp->fd != node->fd) {

      previous = temp;

      temp = temp->next;

   }

   if (temp == NULL) {

      return;

   }

   if (previous == NULL) {

      g_head = temp->next;

   } else {

      previous->next = temp->next;

   }

   close(temp->fd);

   free(temp);

}

void finalize_timer_thread() {

   while (g_head) {

      cancel_timer((size_t) g_head);

   }

   pthread_cancel(timerThread);

   pthread_join(timerThread, NULL);

}
