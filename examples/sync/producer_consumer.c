#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>

#define BUFFER_SIZE 5
#define ITEMS_TO_PRODUCE 15

static int g_buffer[BUFFER_SIZE];
static int g_count = 0;
static int g_in = 0;
static int g_out = 0;

static syscore_mutex_t g_mutex;
static syscore_cond_t g_cond_not_full;
static syscore_cond_t g_cond_not_empty;

static void *producer_func(void *arg) {
  SYSCORE_UNUSED(arg);
  for (int i = 0; i < ITEMS_TO_PRODUCE; i++) {
    syscore_mutex_lock(&g_mutex);

    while (g_count == BUFFER_SIZE) {
      SYSCORE_LOG_DEBUG("[Producer] Buffer full, waiting...");
      syscore_cond_wait(&g_cond_not_full, &g_mutex);
    }

    g_buffer[g_in] = i + 1;
    g_in = (g_in + 1) % BUFFER_SIZE;
    g_count++;
    SYSCORE_LOG_INFO("[Producer] Produced item: %d (buffer count: %d)", i + 1,
                     g_count);

    syscore_cond_signal(&g_cond_not_empty);
    syscore_mutex_unlock(&g_mutex);

    syscore_thread_sleep_ms(50);
  }
  return NULL;
}

static void *consumer_func(void *arg) {
  SYSCORE_UNUSED(arg);
  for (int i = 0; i < ITEMS_TO_PRODUCE; i++) {
    syscore_mutex_lock(&g_mutex);

    while (g_count == 0) {
      SYSCORE_LOG_DEBUG("[Consumer] Buffer empty, waiting...");
      syscore_cond_wait(&g_cond_not_empty, &g_mutex);
    }

    int item = g_buffer[g_out];
    g_out = (g_out + 1) % BUFFER_SIZE;
    g_count--;
    SYSCORE_LOG_INFO("[Consumer] Consumed item: %d (buffer count: %d)", item,
                     g_count);

    syscore_cond_signal(&g_cond_not_full);
    syscore_mutex_unlock(&g_mutex);

    syscore_thread_sleep_ms(80);
  }
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Producer-Consumer Demo");

  syscore_mutex_init(&g_mutex);
  syscore_cond_init(&g_cond_not_full);
  syscore_cond_init(&g_cond_not_empty);

  syscore_thread_t prod_thread, cons_thread;

  syscore_thread_create(&prod_thread, NULL, producer_func, NULL);
  syscore_thread_create(&cons_thread, NULL, consumer_func, NULL);

  syscore_thread_join(prod_thread, NULL);
  syscore_thread_join(cons_thread, NULL);

  syscore_mutex_destroy(&g_mutex);
  syscore_cond_destroy(&g_cond_not_full);
  syscore_cond_destroy(&g_cond_not_empty);

  SYSCORE_LOG_INFO("Producer-Consumer Demo completed successfully.");
  return 0;
}
