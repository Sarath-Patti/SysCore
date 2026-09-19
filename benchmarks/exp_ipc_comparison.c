#include "benchmark/benchmark.h"
#include "ipc/ipc.h"
#include "ipc/message_queue.h"
#include "memory/shared_memory.h"
#include "sync/sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHM_EXP_NAME "/sc_exp_ipc_shm"
#define SHM_EXP_BUF_SIZE 8192

#define MQ_EXP_NAME "/sc_exp_ipc_mq"

/* --- Pipe Benchmark Setup/Step/Teardown --- */

typedef struct {
  syscore_ipc_handle_t rd;
  syscore_ipc_handle_t wr;
  size_t payload_size;
  char *write_buf;
  char *read_buf;
} pipe_exp_ctx_t;

static syscore_error_t pipe_exp_setup(void **user_data) {
  pipe_exp_ctx_t *ctx = (pipe_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  ctx->write_buf = (char *)malloc(ctx->payload_size);
  ctx->read_buf = (char *)malloc(ctx->payload_size);
  if (!ctx->write_buf || !ctx->read_buf) {
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }
  memset(ctx->write_buf, 'P', ctx->payload_size);

  syscore_error_t err = syscore_ipc_pipe_create(&ctx->rd, &ctx->wr);
  if (err != SYSCORE_SUCCESS) {
    free(ctx->write_buf);
    free(ctx->read_buf);
    return err;
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t pipe_exp_step(void *user_data) {
  pipe_exp_ctx_t *ctx = (pipe_exp_ctx_t *)user_data;
  size_t written = 0;
  size_t read_bytes = 0;

  syscore_error_t err =
      syscore_ipc_write(ctx->wr, ctx->write_buf, ctx->payload_size, &written);
  if (err != SYSCORE_SUCCESS) return err;

  return syscore_ipc_read(ctx->rd, ctx->read_buf, ctx->payload_size, &read_bytes);
}

static void pipe_exp_teardown(void *user_data) {
  pipe_exp_ctx_t *ctx = (pipe_exp_ctx_t *)user_data;
  if (ctx) {
    syscore_ipc_close(ctx->rd);
    syscore_ipc_close(ctx->wr);
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
  }
}

/* --- Shared Memory Benchmark Setup/Step/Teardown --- */

typedef struct {
  syscore_shm_handle_t handle;
  void *addr;
  syscore_mutex_t mutex;
  size_t payload_size;
  char *write_buf;
  char *read_buf;
} shm_exp_ctx_t;

static syscore_error_t shm_exp_setup(void **user_data) {
  shm_exp_ctx_t *ctx = (shm_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  ctx->write_buf = (char *)malloc(ctx->payload_size);
  ctx->read_buf = (char *)malloc(ctx->payload_size);
  if (!ctx->write_buf || !ctx->read_buf) {
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }
  memset(ctx->write_buf, 'S', ctx->payload_size);

  syscore_shm_destroy(SHM_EXP_NAME);

  syscore_error_t err =
      syscore_shm_create(SHM_EXP_NAME, SHM_EXP_BUF_SIZE, 0666, &ctx->handle);
  if (err != SYSCORE_SUCCESS) {
    free(ctx->write_buf);
    free(ctx->read_buf);
    return err;
  }

  err = syscore_shm_map(ctx->handle, SHM_EXP_BUF_SIZE, 1, &ctx->addr);
  if (err != SYSCORE_SUCCESS) {
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(SHM_EXP_NAME);
    free(ctx->write_buf);
    free(ctx->read_buf);
    return err;
  }

  err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) {
    syscore_shm_unmap(ctx->addr, SHM_EXP_BUF_SIZE);
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(SHM_EXP_NAME);
    free(ctx->write_buf);
    free(ctx->read_buf);
    return err;
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t shm_exp_step(void *user_data) {
  shm_exp_ctx_t *ctx = (shm_exp_ctx_t *)user_data;

  syscore_mutex_lock(&ctx->mutex);
  memcpy(ctx->addr, ctx->write_buf, ctx->payload_size);
  syscore_mutex_unlock(&ctx->mutex);

  syscore_mutex_lock(&ctx->mutex);
  memcpy(ctx->read_buf, ctx->addr, ctx->payload_size);
  syscore_mutex_unlock(&ctx->mutex);

  return SYSCORE_SUCCESS;
}

static void shm_exp_teardown(void *user_data) {
  shm_exp_ctx_t *ctx = (shm_exp_ctx_t *)user_data;
  if (ctx) {
    syscore_mutex_destroy(&ctx->mutex);
    syscore_shm_unmap(ctx->addr, SHM_EXP_BUF_SIZE);
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(SHM_EXP_NAME);
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
  }
}

/* --- Message Queue Benchmark Setup/Step/Teardown --- */

typedef struct {
  syscore_mq_handle_t mq;
  size_t payload_size;
  char *write_buf;
  char *read_buf;
} mq_exp_ctx_t;

static syscore_error_t mq_exp_setup(void **user_data) {
  mq_exp_ctx_t *ctx = (mq_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  ctx->write_buf = (char *)malloc(ctx->payload_size);
  ctx->read_buf = (char *)malloc(ctx->payload_size);
  if (!ctx->write_buf || !ctx->read_buf) {
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }
  memset(ctx->write_buf, 'M', ctx->payload_size);

  syscore_mq_unlink(MQ_EXP_NAME);

  syscore_mq_attr_t attr;
  attr.flags = 0;
  attr.max_msgs = 10;
  attr.msg_size = (long)ctx->payload_size;
  attr.cur_msgs = 0;

  syscore_error_t err = syscore_mq_open(MQ_EXP_NAME, 2, 0, 0666, &attr, &ctx->mq);
  if (err != SYSCORE_SUCCESS) {
    free(ctx->write_buf);
    free(ctx->read_buf);
    return err;
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t mq_exp_step(void *user_data) {
  mq_exp_ctx_t *ctx = (mq_exp_ctx_t *)user_data;
  unsigned int prio = 1;
  size_t read_bytes = 0;

  syscore_error_t err =
      syscore_mq_send(ctx->mq, ctx->write_buf, ctx->payload_size, prio);
  if (err != SYSCORE_SUCCESS) return err;

  return syscore_mq_receive(ctx->mq, ctx->read_buf, ctx->payload_size, NULL,
                            &read_bytes);
}

static void mq_exp_teardown(void *user_data) {
  mq_exp_ctx_t *ctx = (mq_exp_ctx_t *)user_data;
  if (ctx) {
    syscore_mq_close(ctx->mq);
    syscore_mq_unlink(MQ_EXP_NAME);
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
  }
}

/* --- Main Entry Point --- */

int main(void) {
  static const size_t payload_sizes[] = {64, 256, 1024};
  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(payload_sizes); i++) {
    size_t sz = payload_sizes[i];

    /* 1. Pipe Experiment */
    {
      pipe_exp_ctx_t ctx;
      ctx.payload_size = sz;

      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf), "IPC Pipe (Payload %zuB)", sz);

      syscore_benchmark_config_t config;
      config.name = name_buf;
      config.warmup_iterations = 200;
      config.measured_iterations = 3000;
      config.setup = pipe_exp_setup;
      config.step = pipe_exp_step;
      config.teardown = pipe_exp_teardown;
      config.user_data = &ctx;

      syscore_error_t err = syscore_benchmark_run(&config, NULL);
      if (err != SYSCORE_SUCCESS) status = err;
    }

    /* 2. Shared Memory Experiment */
    {
      shm_exp_ctx_t ctx;
      ctx.payload_size = sz;

      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf), "IPC Shared Memory (Payload %zuB)", sz);

      syscore_benchmark_config_t config;
      config.name = name_buf;
      config.warmup_iterations = 200;
      config.measured_iterations = 3000;
      config.setup = shm_exp_setup;
      config.step = shm_exp_step;
      config.teardown = shm_exp_teardown;
      config.user_data = &ctx;

      syscore_error_t err = syscore_benchmark_run(&config, NULL);
      if (err != SYSCORE_SUCCESS) status = err;
    }

    /* 3. POSIX Message Queue Experiment */
    {
      mq_exp_ctx_t ctx;
      ctx.payload_size = sz;

      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf), "IPC Message Queue (Payload %zuB)", sz);

      syscore_benchmark_config_t config;
      config.name = name_buf;
      config.warmup_iterations = 100;
      config.measured_iterations = 1500;
      config.setup = mq_exp_setup;
      config.step = mq_exp_step;
      config.teardown = mq_exp_teardown;
      config.user_data = &ctx;

      syscore_error_t err = syscore_benchmark_run(&config, NULL);
      if (err != SYSCORE_SUCCESS) status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
