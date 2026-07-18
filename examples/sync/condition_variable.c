#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>

static int g_ready = 0;
static syscore_mutex_t g_mutex;
static syscore_cond_t g_cond;

static void *worker_func(void *arg) {
  SYSCORE_UNUSED(arg);
  syscore_mutex_lock(&g_mutex);

  SYSCORE_LOG_INFO("[Worker] Waiting for signal...");
  while (g_ready == 0) {
    syscore_cond_wait(&g_cond, &g_mutex);
  }

  SYSCORE_LOG_INFO("[Worker] Signal received! Worker proceeding...");
  syscore_mutex_unlock(&g_mutex);
  return NULL;
}

static void *supervisor_func(void *arg) {
  SYSCORE_UNUSED(arg);
  SYSCORE_LOG_INFO("[Supervisor] Preparing work...");
  syscore_thread_sleep_ms(500);

  syscore_mutex_lock(&g_mutex);
  g_ready = 1;
  SYSCORE_LOG_INFO("[Supervisor] Signaling worker...");
  syscore_cond_signal(&g_cond);
  syscore_mutex_unlock(&g_mutex);
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Condition Variable Demo");

  syscore_mutex_init(&g_mutex);
  syscore_cond_init(&g_cond);

  syscore_thread_t worker_thread, supervisor_thread;

  syscore_thread_create(&worker_thread, NULL, worker_func, NULL);
  syscore_thread_create(&supervisor_thread, NULL, supervisor_func, NULL);

  syscore_thread_join(worker_thread, NULL);
  syscore_thread_join(supervisor_thread, NULL);

  syscore_mutex_destroy(&g_mutex);
  syscore_cond_destroy(&g_cond);

  SYSCORE_LOG_INFO("Condition Variable Demo completed successfully.");
  return 0;
}
