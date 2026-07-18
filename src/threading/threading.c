#include "threading/threading.h"
#include "common/logging.h"
#include "common/errors.h"
#include <errno.h>
#include <string.h>
#include <time.h>

static syscore_error_t map_pthread_error(int err_num) {
  switch (err_num) {
  case 0:
    return SYSCORE_SUCCESS;
  case EAGAIN:
    return SYSCORE_ERROR_BUSY;
  case EINVAL:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case EPERM:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  case ESRCH:
    return SYSCORE_ERROR_NOT_FOUND;
  case ENOMEM:
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  default:
    return SYSCORE_ERROR_GENERIC;
  }
}

syscore_error_t syscore_thread_attr_init(syscore_thread_attr_t *attr) {
  if (attr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  attr->stack_size = 0;   // system default
  attr->detach_state = 0; // joinable by default
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_thread_create(syscore_thread_t *out_thread,
                                      const syscore_thread_attr_t *attr,
                                      syscore_thread_func_t func, void *arg) {
  if (out_thread == NULL || func == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Creating thread...");
  pthread_attr_t pattr;
  int rc = pthread_attr_init(&pattr);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_attr_init failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }

  if (attr != NULL) {
    if (attr->stack_size > 0) {
      rc = pthread_attr_setstacksize(&pattr, attr->stack_size);
      if (rc != 0) {
        SYSCORE_LOG_ERROR("pthread_attr_setstacksize failed: %s", strerror(rc));
        pthread_attr_destroy(&pattr);
        return map_pthread_error(rc);
      }
    }
    int p_detach = (attr->detach_state == 1) ? PTHREAD_CREATE_DETACHED
                                             : PTHREAD_CREATE_JOINABLE;
    rc = pthread_attr_setdetachstate(&pattr, p_detach);
    if (rc != 0) {
      SYSCORE_LOG_ERROR("pthread_attr_setdetachstate failed: %s", strerror(rc));
      pthread_attr_destroy(&pattr);
      return map_pthread_error(rc);
    }
  }

  rc = pthread_create(out_thread, &pattr, func, arg);
  pthread_attr_destroy(&pattr);

  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_create failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }

  SYSCORE_LOG_DEBUG("Thread created successfully.");
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_thread_join(syscore_thread_t thread,
                                    void **out_retval) {
  SYSCORE_LOG_DEBUG("Joining thread...");
  int rc = pthread_join(thread, out_retval);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_join failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  SYSCORE_LOG_DEBUG("Thread joined successfully.");
  return SYSCORE_SUCCESS;
}

syscore_thread_t syscore_thread_self(void) { return pthread_self(); }

void syscore_thread_sleep_ms(unsigned int ms) {
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}
