#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>

#define NUM_READERS 3

static int g_shared_resource = 0;
static syscore_rwlock_t g_rwlock;

static void *reader_func(void *arg) {
  int id = *(int *)arg;
  for (int i = 0; i < 3; i++) {
    syscore_rwlock_rdlock(&g_rwlock);
    SYSCORE_LOG_INFO("[Reader %d] Read shared resource value: %d", id,
                     g_shared_resource);
    syscore_thread_sleep_ms(50);
    syscore_rwlock_unlock(&g_rwlock);
    syscore_thread_sleep_ms(30);
  }
  return NULL;
}

static void *writer_func(void *arg) {
  SYSCORE_UNUSED(arg);
  for (int i = 0; i < 3; i++) {
    syscore_rwlock_wrlock(&g_rwlock);
    g_shared_resource += 10;
    SYSCORE_LOG_INFO("[Writer] Updated shared resource value to: %d",
                     g_shared_resource);
    syscore_thread_sleep_ms(100);
    syscore_rwlock_unlock(&g_rwlock);
    syscore_thread_sleep_ms(80);
  }
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Read-Write Lock Demo");

  syscore_rwlock_init(&g_rwlock);

  syscore_thread_t readers[NUM_READERS];
  syscore_thread_t writer;
  int reader_ids[NUM_READERS];

  for (int i = 0; i < NUM_READERS; i++) {
    reader_ids[i] = i + 1;
    syscore_thread_create(&readers[i], NULL, reader_func, &reader_ids[i]);
  }

  syscore_thread_create(&writer, NULL, writer_func, NULL);

  for (int i = 0; i < NUM_READERS; i++) {
    syscore_thread_join(readers[i], NULL);
  }
  syscore_thread_join(writer, NULL);

  syscore_rwlock_destroy(&g_rwlock);

  SYSCORE_LOG_INFO("Read-Write Lock Demo completed successfully.");
  return 0;
}
