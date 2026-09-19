#include "benchmark/benchmark.h"
#include "sync/semaphore.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>
#include <stdlib.h>

/* --- Mutex Benchmark Setup/Step/Teardown --- */

typedef struct {
  syscore_mutex_t mutex;
  syscore_sem_t sem_start;
  syscore_sem_t sem_done;
  syscore_thread_t threads[8];
  size_t threads_count;
  size_t threads_created;
  size_t ops_per_thread;
  volatile int stop_flag;
  volatile size_t counter;
} mutex_exp_ctx_t;

typedef struct {
  mutex_exp_ctx_t *ctx;
} mutex_worker_arg_t;

static void *mutex_exp_worker(void *arg) {
  mutex_worker_arg_t *warg = (mutex_worker_arg_t *)arg;
  mutex_exp_ctx_t *ctx = warg->ctx;
  free(warg);

  while (1) {
    syscore_error_t err = syscore_sem_wait(&ctx->sem_start);
    if (err != SYSCORE_SUCCESS || ctx->stop_flag) {
      break;
    }

    for (size_t i = 0; i < ctx->ops_per_thread; i++) {
      syscore_mutex_lock(&ctx->mutex);
      ctx->counter++;
      syscore_mutex_unlock(&ctx->mutex);
    }

    syscore_sem_post(&ctx->sem_done);
  }

  return NULL;
}

static syscore_error_t mutex_exp_setup(void **user_data) {
  mutex_exp_ctx_t *ctx = (mutex_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) return err;

  ctx->counter = 0;
  ctx->threads_created = 0;
  ctx->stop_flag = 0;

  if (ctx->threads_count > 1) {
    err = syscore_sem_init(&ctx->sem_start, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_mutex_destroy(&ctx->mutex);
      return err;
    }

    err = syscore_sem_init(&ctx->sem_done, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_sem_destroy(&ctx->sem_start);
      syscore_mutex_destroy(&ctx->mutex);
      return err;
    }

    ctx->ops_per_thread = 10;
    for (size_t i = 0; i < ctx->threads_count; i++) {
      mutex_worker_arg_t *arg = (mutex_worker_arg_t *)malloc(sizeof(mutex_worker_arg_t));
      if (!arg) {
        err = SYSCORE_ERROR_OUT_OF_MEMORY;
        break;
      }
      arg->ctx = ctx;

      err = syscore_thread_create(&ctx->threads[i], NULL, mutex_exp_worker, arg);
      if (err != SYSCORE_SUCCESS) {
        free(arg);
        break;
      }
      ctx->threads_created++;
    }

    if (err != SYSCORE_SUCCESS) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
      syscore_mutex_destroy(&ctx->mutex);
      return err;
    }
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t mutex_exp_step(void *user_data) {
  mutex_exp_ctx_t *ctx = (mutex_exp_ctx_t *)user_data;

  if (ctx->threads_count <= 1) {
    syscore_mutex_lock(&ctx->mutex);
    ctx->counter++;
    syscore_mutex_unlock(&ctx->mutex);
  } else {
    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_post(&ctx->sem_start);
    }

    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_wait(&ctx->sem_done);
    }
  }

  return SYSCORE_SUCCESS;
}

static void mutex_exp_teardown(void *user_data) {
  mutex_exp_ctx_t *ctx = (mutex_exp_ctx_t *)user_data;
  if (ctx) {
    if (ctx->threads_created > 0) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
    }
    syscore_mutex_destroy(&ctx->mutex);
  }
}

/* --- Semaphore Benchmark Setup/Step/Teardown --- */

typedef struct {
  syscore_sem_t sem;
  syscore_sem_t sem_start;
  syscore_sem_t sem_done;
  syscore_thread_t threads[8];
  size_t threads_count;
  size_t threads_created;
  size_t ops_per_thread;
  volatile int stop_flag;
  volatile size_t counter;
} sem_exp_ctx_t;

typedef struct {
  sem_exp_ctx_t *ctx;
} sem_worker_arg_t;

static void *sem_exp_worker(void *arg) {
  sem_worker_arg_t *warg = (sem_worker_arg_t *)arg;
  sem_exp_ctx_t *ctx = warg->ctx;
  free(warg);

  while (1) {
    syscore_error_t err = syscore_sem_wait(&ctx->sem_start);
    if (err != SYSCORE_SUCCESS || ctx->stop_flag) {
      break;
    }

    for (size_t i = 0; i < ctx->ops_per_thread; i++) {
      syscore_sem_wait(&ctx->sem);
      ctx->counter++;
      syscore_sem_post(&ctx->sem);
    }

    syscore_sem_post(&ctx->sem_done);
  }

  return NULL;
}

static syscore_error_t sem_exp_setup(void **user_data) {
  sem_exp_ctx_t *ctx = (sem_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_sem_init(&ctx->sem, 0, 1);
  if (err != SYSCORE_SUCCESS) return err;

  ctx->counter = 0;
  ctx->threads_created = 0;
  ctx->stop_flag = 0;

  if (ctx->threads_count > 1) {
    err = syscore_sem_init(&ctx->sem_start, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_sem_destroy(&ctx->sem);
      return err;
    }

    err = syscore_sem_init(&ctx->sem_done, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem);
      return err;
    }

    ctx->ops_per_thread = 10;
    for (size_t i = 0; i < ctx->threads_count; i++) {
      sem_worker_arg_t *arg = (sem_worker_arg_t *)malloc(sizeof(sem_worker_arg_t));
      if (!arg) {
        err = SYSCORE_ERROR_OUT_OF_MEMORY;
        break;
      }
      arg->ctx = ctx;

      err = syscore_thread_create(&ctx->threads[i], NULL, sem_exp_worker, arg);
      if (err != SYSCORE_SUCCESS) {
        free(arg);
        break;
      }
      ctx->threads_created++;
    }

    if (err != SYSCORE_SUCCESS) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
      syscore_sem_destroy(&ctx->sem);
      return err;
    }
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t sem_exp_step(void *user_data) {
  sem_exp_ctx_t *ctx = (sem_exp_ctx_t *)user_data;

  if (ctx->threads_count <= 1) {
    syscore_sem_wait(&ctx->sem);
    ctx->counter++;
    syscore_sem_post(&ctx->sem);
  } else {
    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_post(&ctx->sem_start);
    }

    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_wait(&ctx->sem_done);
    }
  }

  return SYSCORE_SUCCESS;
}

static void sem_exp_teardown(void *user_data) {
  sem_exp_ctx_t *ctx = (sem_exp_ctx_t *)user_data;
  if (ctx) {
    if (ctx->threads_created > 0) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
    }
    syscore_sem_destroy(&ctx->sem);
  }
}

/* --- RWLock Benchmark Setup/Step/Teardown --- */

typedef struct {
  syscore_rwlock_t rwlock;
  syscore_sem_t sem_start;
  syscore_sem_t sem_done;
  syscore_thread_t threads[8];
  size_t threads_count;
  size_t threads_created;
  size_t ops_per_thread;
  volatile int stop_flag;
  volatile size_t counter;
} rwlock_exp_ctx_t;

typedef struct {
  rwlock_exp_ctx_t *ctx;
  int is_writer;
} rwlock_worker_arg_t;

static void *rwlock_exp_worker(void *arg) {
  rwlock_worker_arg_t *warg = (rwlock_worker_arg_t *)arg;
  rwlock_exp_ctx_t *ctx = warg->ctx;
  int is_writer = warg->is_writer;
  free(warg);

  while (1) {
    syscore_error_t err = syscore_sem_wait(&ctx->sem_start);
    if (err != SYSCORE_SUCCESS || ctx->stop_flag) {
      break;
    }

    for (size_t i = 0; i < ctx->ops_per_thread; i++) {
      if (is_writer) {
        syscore_rwlock_wrlock(&ctx->rwlock);
        ctx->counter++;
        syscore_rwlock_unlock(&ctx->rwlock);
      } else {
        syscore_rwlock_rdlock(&ctx->rwlock);
        (void)ctx->counter;
        syscore_rwlock_unlock(&ctx->rwlock);
      }
    }

    syscore_sem_post(&ctx->sem_done);
  }

  return NULL;
}

static syscore_error_t rwlock_exp_setup(void **user_data) {
  rwlock_exp_ctx_t *ctx = (rwlock_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_rwlock_init(&ctx->rwlock);
  if (err != SYSCORE_SUCCESS) return err;

  ctx->counter = 0;
  ctx->threads_created = 0;
  ctx->stop_flag = 0;

  if (ctx->threads_count > 1) {
    err = syscore_sem_init(&ctx->sem_start, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_rwlock_destroy(&ctx->rwlock);
      return err;
    }

    err = syscore_sem_init(&ctx->sem_done, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_sem_destroy(&ctx->sem_start);
      syscore_rwlock_destroy(&ctx->rwlock);
      return err;
    }

    ctx->ops_per_thread = 10;
    for (size_t i = 0; i < ctx->threads_count; i++) {
      rwlock_worker_arg_t *arg = (rwlock_worker_arg_t *)malloc(sizeof(rwlock_worker_arg_t));
      if (!arg) {
        err = SYSCORE_ERROR_OUT_OF_MEMORY;
        break;
      }
      arg->ctx = ctx;
      arg->is_writer = (i % 2 == 0) ? 1 : 0;

      err = syscore_thread_create(&ctx->threads[i], NULL, rwlock_exp_worker, arg);
      if (err != SYSCORE_SUCCESS) {
        free(arg);
        break;
      }
      ctx->threads_created++;
    }

    if (err != SYSCORE_SUCCESS) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
      syscore_rwlock_destroy(&ctx->rwlock);
      return err;
    }
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t rwlock_exp_step(void *user_data) {
  rwlock_exp_ctx_t *ctx = (rwlock_exp_ctx_t *)user_data;

  if (ctx->threads_count <= 1) {
    syscore_rwlock_wrlock(&ctx->rwlock);
    ctx->counter++;
    syscore_rwlock_unlock(&ctx->rwlock);
  } else {
    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_post(&ctx->sem_start);
    }

    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_wait(&ctx->sem_done);
    }
  }

  return SYSCORE_SUCCESS;
}

static void rwlock_exp_teardown(void *user_data) {
  rwlock_exp_ctx_t *ctx = (rwlock_exp_ctx_t *)user_data;
  if (ctx) {
    if (ctx->threads_created > 0) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
    }
    syscore_rwlock_destroy(&ctx->rwlock);
  }
}

/* --- Condition Variable Benchmark Setup/Step/Teardown --- */

typedef struct {
  syscore_mutex_t mutex;
  syscore_cond_t cond;
  syscore_sem_t sem_start;
  syscore_sem_t sem_done;
  syscore_thread_t threads[8];
  size_t threads_count;
  size_t threads_created;
  volatile int stop_flag;
  volatile int ready;
} condvar_exp_ctx_t;

static void *condvar_exp_worker(void *arg) {
  condvar_exp_ctx_t *ctx = (condvar_exp_ctx_t *)arg;

  while (1) {
    syscore_error_t err = syscore_sem_wait(&ctx->sem_start);
    if (err != SYSCORE_SUCCESS || ctx->stop_flag) {
      break;
    }

    syscore_mutex_lock(&ctx->mutex);
    ctx->ready = 1;
    syscore_cond_signal(&ctx->cond);
    syscore_mutex_unlock(&ctx->mutex);

    syscore_sem_post(&ctx->sem_done);
  }

  return NULL;
}

static syscore_error_t condvar_exp_setup(void **user_data) {
  condvar_exp_ctx_t *ctx = (condvar_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) return err;

  err = syscore_cond_init(&ctx->cond);
  if (err != SYSCORE_SUCCESS) {
    syscore_mutex_destroy(&ctx->mutex);
    return err;
  }

  ctx->ready = 0;
  ctx->threads_created = 0;
  ctx->stop_flag = 0;

  if (ctx->threads_count > 1) {
    err = syscore_sem_init(&ctx->sem_start, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_cond_destroy(&ctx->cond);
      syscore_mutex_destroy(&ctx->mutex);
      return err;
    }

    err = syscore_sem_init(&ctx->sem_done, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_sem_destroy(&ctx->sem_start);
      syscore_cond_destroy(&ctx->cond);
      syscore_mutex_destroy(&ctx->mutex);
      return err;
    }

    for (size_t i = 0; i < ctx->threads_count; i++) {
      err = syscore_thread_create(&ctx->threads[i], NULL, condvar_exp_worker, ctx);
      if (err != SYSCORE_SUCCESS) break;
      ctx->threads_created++;
    }

    if (err != SYSCORE_SUCCESS) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
      syscore_cond_destroy(&ctx->cond);
      syscore_mutex_destroy(&ctx->mutex);
      return err;
    }
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t condvar_exp_step(void *user_data) {
  condvar_exp_ctx_t *ctx = (condvar_exp_ctx_t *)user_data;

  ctx->ready = 0;

  if (ctx->threads_count <= 1) {
    syscore_mutex_lock(&ctx->mutex);
    ctx->ready = 1;
    syscore_cond_signal(&ctx->cond);
    ctx->ready = 0;
    syscore_mutex_unlock(&ctx->mutex);
  } else {
    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_post(&ctx->sem_start);
    }

    syscore_mutex_lock(&ctx->mutex);
    while (!ctx->ready) {
      syscore_cond_wait(&ctx->cond, &ctx->mutex);
    }
    ctx->ready = 0;
    syscore_mutex_unlock(&ctx->mutex);

    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_wait(&ctx->sem_done);
    }
  }

  return SYSCORE_SUCCESS;
}

static void condvar_exp_teardown(void *user_data) {
  condvar_exp_ctx_t *ctx = (condvar_exp_ctx_t *)user_data;
  if (ctx) {
    if (ctx->threads_created > 0) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
    }
    syscore_cond_destroy(&ctx->cond);
    syscore_mutex_destroy(&ctx->mutex);
  }
}

/* --- Main Entry Point --- */

int main(void) {
  static const size_t thread_counts[] = {1, 2, 4, 8};
  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(thread_counts); i++) {
    size_t threads = thread_counts[i];

    /* 1. Mutex Experiment */
    {
      mutex_exp_ctx_t ctx;
      ctx.threads_count = threads;

      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf), "Synchronization Mutex (%zu Thread%s)",
               threads, threads > 1 ? "s" : "");

      syscore_benchmark_config_t config;
      config.name = name_buf;
      config.warmup_iterations = (threads > 2) ? 50 : 200;
      config.measured_iterations = (threads > 2) ? 500 : 3000;
      config.setup = mutex_exp_setup;
      config.step = mutex_exp_step;
      config.teardown = mutex_exp_teardown;
      config.user_data = &ctx;

      syscore_error_t err = syscore_benchmark_run(&config, NULL);
      if (err != SYSCORE_SUCCESS) status = err;
    }

    /* 2. Semaphore Experiment */
    {
      sem_exp_ctx_t ctx;
      ctx.threads_count = threads;

      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf),
               "Synchronization Semaphore (%zu Thread%s)", threads,
               threads > 1 ? "s" : "");

      syscore_benchmark_config_t config;
      config.name = name_buf;
      config.warmup_iterations = (threads > 2) ? 50 : 200;
      config.measured_iterations = (threads > 2) ? 500 : 3000;
      config.setup = sem_exp_setup;
      config.step = sem_exp_step;
      config.teardown = sem_exp_teardown;
      config.user_data = &ctx;

      syscore_error_t err = syscore_benchmark_run(&config, NULL);
      if (err != SYSCORE_SUCCESS) status = err;
    }

    /* 3. RWLock Experiment */
    {
      rwlock_exp_ctx_t ctx;
      ctx.threads_count = threads;

      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf), "Synchronization RWLock (%zu Thread%s)",
               threads, threads > 1 ? "s" : "");

      syscore_benchmark_config_t config;
      config.name = name_buf;
      config.warmup_iterations = (threads > 2) ? 50 : 200;
      config.measured_iterations = (threads > 2) ? 500 : 3000;
      config.setup = rwlock_exp_setup;
      config.step = rwlock_exp_step;
      config.teardown = rwlock_exp_teardown;
      config.user_data = &ctx;

      syscore_error_t err = syscore_benchmark_run(&config, NULL);
      if (err != SYSCORE_SUCCESS) status = err;
    }

    /* 4. Condition Variable Experiment */
    {
      condvar_exp_ctx_t ctx;
      ctx.threads_count = threads;

      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf),
               "Synchronization CondVar (%zu Thread%s)", threads,
               threads > 1 ? "s" : "");

      syscore_benchmark_config_t config;
      config.name = name_buf;
      config.warmup_iterations = (threads > 2) ? 50 : 100;
      config.measured_iterations = (threads > 2) ? 300 : 1500;
      config.setup = condvar_exp_setup;
      config.step = condvar_exp_step;
      config.teardown = condvar_exp_teardown;
      config.user_data = &ctx;

      syscore_error_t err = syscore_benchmark_run(&config, NULL);
      if (err != SYSCORE_SUCCESS) status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
