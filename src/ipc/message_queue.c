#include "ipc/message_queue.h"
#include "common/errors.h"
#include "common/logging.h"

#if defined(SYSCORE_OS_LINUX)
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

static syscore_error_t map_mq_error(int err_num) {
  switch (err_num) {
  case 0:
    return SYSCORE_SUCCESS;
  case EACCES:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  case EEXIST:
    return SYSCORE_ERROR_BUSY;
  case EINTR:
    return SYSCORE_ERROR_BUSY;
  case EINVAL:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case EMSGSIZE:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case ENOENT:
    return SYSCORE_ERROR_NOT_FOUND;
  case ENOMEM:
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  case EAGAIN:
    return SYSCORE_ERROR_BUSY;
  default:
    return SYSCORE_ERROR_GENERIC;
  }
}

syscore_error_t syscore_mq_open(const char *name, int write_mode, int non_block,
                                int mode, const syscore_mq_attr_t *attr,
                                syscore_mq_handle_t *out_handle) {
  if (name == NULL || out_handle == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  int oflag = 0;
  if (write_mode == 0) {
    oflag |= O_RDONLY;
  } else if (write_mode == 1) {
    oflag |= O_WRONLY;
  } else {
    oflag |= O_RDWR;
  }

  oflag |= O_CREAT;

  if (non_block) {
    oflag |= O_NONBLOCK;
  }

  struct mq_attr posix_attr;
  struct mq_attr *attr_ptr = NULL;
  if (attr != NULL) {
    posix_attr.mq_flags = attr->flags;
    posix_attr.mq_maxmsg = attr->max_msgs;
    posix_attr.mq_msgsize = attr->msg_size;
    posix_attr.mq_curmsgs = attr->cur_msgs;
    attr_ptr = &posix_attr;
  }

  SYSCORE_LOG_DEBUG("Opening message queue: %s...", name);
  mqd_t mq = mq_open(name, oflag, (mode_t)mode, attr_ptr);
  if (mq == (mqd_t)-1) {
    int err = errno;
    SYSCORE_LOG_ERROR("mq_open failed for %s: %s (errno %d)", name,
                      strerror(err), err);
    return map_mq_error(err);
  }

  *out_handle = mq;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_close(syscore_mq_handle_t handle) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Closing message queue descriptor...");
  if (mq_close(handle) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("mq_close failed: %s (errno %d)", strerror(err), err);
    return map_mq_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_unlink(const char *name) {
  if (name == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Unlinking message queue: %s...", name);
  if (mq_unlink(name) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("mq_unlink failed for %s: %s (errno %d)", name,
                      strerror(err), err);
    return map_mq_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_send(syscore_mq_handle_t handle, const char *msg_ptr,
                                size_t msg_len, unsigned int msg_prio) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || msg_ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  if (mq_send(handle, msg_ptr, msg_len, msg_prio) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("mq_send failed: %s (errno %d)", strerror(err), err);
    return map_mq_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_receive(syscore_mq_handle_t handle, char *msg_ptr,
                                   size_t msg_len, unsigned int *out_msg_prio,
                                   size_t *out_read_bytes) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || msg_ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  ssize_t res = mq_receive(handle, msg_ptr, msg_len, out_msg_prio);
  if (res < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("mq_receive failed: %s (errno %d)", strerror(err), err);
    return map_mq_error(err);
  }

  if (out_read_bytes != NULL) {
    *out_read_bytes = (size_t)res;
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_get_attr(syscore_mq_handle_t handle,
                                    syscore_mq_attr_t *attr) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || attr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  struct mq_attr posix_attr;
  if (mq_getattr(handle, &posix_attr) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("mq_getattr failed: %s (errno %d)", strerror(err), err);
    return map_mq_error(err);
  }

  attr->flags = posix_attr.mq_flags;
  attr->max_msgs = posix_attr.mq_maxmsg;
  attr->msg_size = posix_attr.mq_msgsize;
  attr->cur_msgs = posix_attr.mq_curmsgs;

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_set_attr(syscore_mq_handle_t handle,
                                    const syscore_mq_attr_t *attr,
                                    syscore_mq_attr_t *old_attr) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || attr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  struct mq_attr new_posix_attr;
  new_posix_attr.mq_flags = attr->flags;
  new_posix_attr.mq_maxmsg = attr->max_msgs;
  new_posix_attr.mq_msgsize = attr->msg_size;
  new_posix_attr.mq_curmsgs = attr->cur_msgs;

  struct mq_attr old_posix_attr;
  if (mq_setattr(handle, &new_posix_attr, &old_posix_attr) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("mq_setattr failed: %s (errno %d)", strerror(err), err);
    return map_mq_error(err);
  }

  if (old_attr != NULL) {
    old_attr->flags = old_posix_attr.mq_flags;
    old_attr->max_msgs = old_posix_attr.mq_maxmsg;
    old_attr->msg_size = old_posix_attr.mq_msgsize;
    old_attr->cur_msgs = old_posix_attr.mq_curmsgs;
  }

  return SYSCORE_SUCCESS;
}

#else /* macOS / Emulated implementation */

#include "memory/shared_memory.h"
#include "sync/semaphore.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// A node inside the shared memory block representing a message
typedef struct {
  int active;
  unsigned int priority;
  size_t len;
  unsigned long long sequence;
  // Followed by raw data of msg_size (in byte offset)
} emulated_msg_node_t;

// The structure mapped in shared memory
typedef struct {
  long max_msgs;
  long msg_size;
  long cur_msgs;
  unsigned long long seq_counter;
  int non_block;
  char sem_mutex_name[64];
  char sem_empty_name[64];
  char sem_full_name[64];
} emulated_queue_shm_t;

// The local handle struct in the heap
typedef struct {
  char name[128];
  syscore_shm_handle_t shm_fd;
  emulated_queue_shm_t *shm_ptr;
  size_t shm_size;
  syscore_sem_t sem_mutex;
  syscore_sem_t sem_empty;
  syscore_sem_t sem_full;
  int write_mode;
  int non_block;
} emulated_mq_t;

static unsigned int djb2_hash(const char *str) {
  unsigned int hash = 5381;
  int c;
  while ((c = (unsigned char)*str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

syscore_error_t syscore_mq_open(const char *name, int write_mode, int non_block,
                                int mode, const syscore_mq_attr_t *attr,
                                syscore_mq_handle_t *out_handle) {
  if (name == NULL || out_handle == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  // Format bounded shared memory and semaphore names using hash
  const char *clean_name = name;
  if (clean_name[0] == '/') {
    clean_name++;
  }
  unsigned int hash = djb2_hash(clean_name);

  char shm_name[64];
  char sem_mutex_name[64];
  char sem_empty_name[64];
  char sem_full_name[64];

  snprintf(shm_name, sizeof(shm_name), "/sc_mq_s_%08x", hash);
  snprintf(sem_mutex_name, sizeof(sem_mutex_name), "/sc_mq_m_%08x", hash);
  snprintf(sem_empty_name, sizeof(sem_empty_name), "/sc_mq_e_%08x", hash);
  snprintf(sem_full_name, sizeof(sem_full_name), "/sc_mq_f_%08x", hash);

  emulated_mq_t *mq = (emulated_mq_t *)malloc(sizeof(emulated_mq_t));
  if (mq == NULL) {
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }
  memset(mq, 0, sizeof(emulated_mq_t));
  snprintf(mq->name, sizeof(mq->name), "%s", name);
  mq->write_mode = write_mode;
  mq->non_block = non_block;

  long max_msgs = attr ? attr->max_msgs : 10;
  long msg_size = attr ? attr->msg_size : 8192;
  size_t shm_size = sizeof(emulated_queue_shm_t) +
                    max_msgs * (sizeof(emulated_msg_node_t) + msg_size);
  mq->shm_size = shm_size;

  // Attempt to open existing first
  syscore_error_t err = syscore_shm_open(shm_name, 1, &mq->shm_fd);
  int is_creator = 0;
  if (err != SYSCORE_SUCCESS) {
    // If not found, create it
    err = syscore_shm_create(shm_name, shm_size, mode, &mq->shm_fd);
    if (err != SYSCORE_SUCCESS) {
      free(mq);
      return err;
    }
    is_creator = 1;
  }

  // Map shared memory
  err = syscore_shm_map(mq->shm_fd, shm_size, 1, (void **)&mq->shm_ptr);
  if (err != SYSCORE_SUCCESS) {
    syscore_shm_close(mq->shm_fd);
    if (is_creator) {
      shm_unlink(shm_name);
    }
    free(mq);
    return err;
  }

  if (is_creator) {
    mq->shm_ptr->max_msgs = max_msgs;
    mq->shm_ptr->msg_size = msg_size;
    mq->shm_ptr->cur_msgs = 0;
    mq->shm_ptr->seq_counter = 0;
    mq->shm_ptr->non_block = non_block;
    snprintf(mq->shm_ptr->sem_mutex_name, sizeof(mq->shm_ptr->sem_mutex_name),
             "%s", sem_mutex_name);
    snprintf(mq->shm_ptr->sem_empty_name, sizeof(mq->shm_ptr->sem_empty_name),
             "%s", sem_empty_name);
    snprintf(mq->shm_ptr->sem_full_name, sizeof(mq->shm_ptr->sem_full_name),
             "%s", sem_full_name);

    // Initialize messages memory block
    char *base = (char *)mq->shm_ptr + sizeof(emulated_queue_shm_t);
    for (int i = 0; i < max_msgs; i++) {
      emulated_msg_node_t *node =
          (emulated_msg_node_t *)(base + i * (sizeof(emulated_msg_node_t) +
                                              msg_size));
      node->active = 0;
    }

    // Clean any stale named semaphores (ignore ENOENT)
    sem_unlink(sem_mutex_name);
    sem_unlink(sem_empty_name);
    sem_unlink(sem_full_name);

    syscore_sem_open(sem_mutex_name, mode, 1, &mq->sem_mutex);
    syscore_sem_open(sem_empty_name, mode, max_msgs, &mq->sem_empty);
    syscore_sem_open(sem_full_name, mode, 0, &mq->sem_full);
  } else {
    // Open existing semaphores
    syscore_sem_open(mq->shm_ptr->sem_mutex_name, mode, 1, &mq->sem_mutex);
    syscore_sem_open(mq->shm_ptr->sem_empty_name, mode, mq->shm_ptr->max_msgs,
                     &mq->sem_empty);
    syscore_sem_open(mq->shm_ptr->sem_full_name, mode, 0, &mq->sem_full);
  }

  *out_handle = (syscore_mq_handle_t)mq;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_close(syscore_mq_handle_t handle) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  emulated_mq_t *mq = (emulated_mq_t *)handle;
  SYSCORE_LOG_DEBUG("Closing emulated message queue handle...");

  syscore_sem_close(&mq->sem_mutex);
  syscore_sem_close(&mq->sem_empty);
  syscore_sem_close(&mq->sem_full);

  syscore_shm_unmap(mq->shm_ptr, mq->shm_size);
  syscore_shm_close(mq->shm_fd);
  free(mq);

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_unlink(const char *name) {
  if (name == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  const char *clean_name = name;
  if (clean_name[0] == '/') {
    clean_name++;
  }
  unsigned int hash = djb2_hash(clean_name);

  char shm_name[64];
  char sem_mutex_name[64];
  char sem_empty_name[64];
  char sem_full_name[64];

  snprintf(shm_name, sizeof(shm_name), "/sc_mq_s_%08x", hash);
  snprintf(sem_mutex_name, sizeof(sem_mutex_name), "/sc_mq_m_%08x", hash);
  snprintf(sem_empty_name, sizeof(sem_empty_name), "/sc_mq_e_%08x", hash);
  snprintf(sem_full_name, sizeof(sem_full_name), "/sc_mq_f_%08x", hash);

  SYSCORE_LOG_DEBUG("Unlinking emulated message queue: %s...", name);

  if (sem_unlink(sem_mutex_name) < 0) {
    int err = errno;
    if (err != ENOENT) {
      SYSCORE_LOG_ERROR("sem_unlink failed for %s: %s (errno %d)",
                        sem_mutex_name, strerror(err), err);
    }
  }
  if (sem_unlink(sem_empty_name) < 0) {
    int err = errno;
    if (err != ENOENT) {
      SYSCORE_LOG_ERROR("sem_unlink failed for %s: %s (errno %d)",
                        sem_empty_name, strerror(err), err);
    }
  }
  if (sem_unlink(sem_full_name) < 0) {
    int err = errno;
    if (err != ENOENT) {
      SYSCORE_LOG_ERROR("sem_unlink failed for %s: %s (errno %d)",
                        sem_full_name, strerror(err), err);
    }
  }

  if (shm_unlink(shm_name) < 0) {
    int err = errno;
    if (err != ENOENT) {
      SYSCORE_LOG_ERROR("shm_unlink failed for %s: %s (errno %d)", shm_name,
                        strerror(err), err);
      if (err == EACCES) {
        return SYSCORE_ERROR_PERMISSION_DENIED;
      }
      return SYSCORE_ERROR_GENERIC;
    }
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_send(syscore_mq_handle_t handle, const char *msg_ptr,
                                size_t msg_len, unsigned int msg_prio) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || msg_ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  emulated_mq_t *mq = (emulated_mq_t *)handle;

  if (msg_len > (size_t)mq->shm_ptr->msg_size) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  if (mq->non_block) {
    syscore_error_t sem_err = syscore_sem_trywait(&mq->sem_empty);
    if (sem_err != SYSCORE_SUCCESS) {
      return SYSCORE_ERROR_BUSY; // Queue full in non-blocking mode
    }
  } else {
    syscore_sem_wait(&mq->sem_empty);
  }

  syscore_sem_wait(&mq->sem_mutex);

  long max_msgs = mq->shm_ptr->max_msgs;
  long msg_size = mq->shm_ptr->msg_size;
  char *base = (char *)mq->shm_ptr + sizeof(emulated_queue_shm_t);
  emulated_msg_node_t *target = NULL;

  for (int i = 0; i < max_msgs; i++) {
    emulated_msg_node_t *node =
        (emulated_msg_node_t *)(base + i * (sizeof(emulated_msg_node_t) +
                                            msg_size));
    if (!node->active) {
      target = node;
      break;
    }
  }

  if (target == NULL) {
    syscore_sem_post(&mq->sem_mutex);
    return SYSCORE_ERROR_GENERIC;
  }

  target->active = 1;
  target->priority = msg_prio;
  target->len = msg_len;
  target->sequence = mq->shm_ptr->seq_counter++;

  char *dest_data = (char *)target + sizeof(emulated_msg_node_t);
  memcpy(dest_data, msg_ptr, msg_len);

  mq->shm_ptr->cur_msgs++;

  syscore_sem_post(&mq->sem_mutex);
  syscore_sem_post(&mq->sem_full);

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_receive(syscore_mq_handle_t handle, char *msg_ptr,
                                   size_t msg_len, unsigned int *out_msg_prio,
                                   size_t *out_read_bytes) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || msg_ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  emulated_mq_t *mq = (emulated_mq_t *)handle;

  if (msg_len < (size_t)mq->shm_ptr->msg_size) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  if (mq->non_block) {
    syscore_error_t sem_err = syscore_sem_trywait(&mq->sem_full);
    if (sem_err != SYSCORE_SUCCESS) {
      return SYSCORE_ERROR_BUSY; // Queue empty in non-blocking mode
    }
  } else {
    syscore_sem_wait(&mq->sem_full);
  }

  syscore_sem_wait(&mq->sem_mutex);

  long max_msgs = mq->shm_ptr->max_msgs;
  long msg_size = mq->shm_ptr->msg_size;
  char *base = (char *)mq->shm_ptr + sizeof(emulated_queue_shm_t);

  emulated_msg_node_t *highest = NULL;

  for (int i = 0; i < max_msgs; i++) {
    emulated_msg_node_t *node =
        (emulated_msg_node_t *)(base + i * (sizeof(emulated_msg_node_t) +
                                            msg_size));
    if (node->active) {
      if (highest == NULL) {
        highest = node;
      } else {
        if (node->priority > highest->priority) {
          highest = node;
        } else if (node->priority == highest->priority) {
          if (node->sequence < highest->sequence) {
            highest = node;
          }
        }
      }
    }
  }

  if (highest == NULL) {
    syscore_sem_post(&mq->sem_mutex);
    return SYSCORE_ERROR_GENERIC;
  }

  char *src_data = (char *)highest + sizeof(emulated_msg_node_t);
  memcpy(msg_ptr, src_data, highest->len);

  if (out_msg_prio != NULL) {
    *out_msg_prio = highest->priority;
  }
  if (out_read_bytes != NULL) {
    *out_read_bytes = highest->len;
  }

  highest->active = 0;
  mq->shm_ptr->cur_msgs--;

  syscore_sem_post(&mq->sem_mutex);
  syscore_sem_post(&mq->sem_empty);

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_get_attr(syscore_mq_handle_t handle,
                                    syscore_mq_attr_t *attr) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || attr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  emulated_mq_t *mq = (emulated_mq_t *)handle;

  syscore_sem_wait(&mq->sem_mutex);
  attr->flags = mq->shm_ptr->non_block ? 0x800 /* O_NONBLOCK on macOS */ : 0;
  attr->max_msgs = mq->shm_ptr->max_msgs;
  attr->msg_size = mq->shm_ptr->msg_size;
  attr->cur_msgs = mq->shm_ptr->cur_msgs;
  syscore_sem_post(&mq->sem_mutex);

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mq_set_attr(syscore_mq_handle_t handle,
                                    const syscore_mq_attr_t *attr,
                                    syscore_mq_attr_t *old_attr) {
  if (handle == SYSCORE_MQ_INVALID_HANDLE || attr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  emulated_mq_t *mq = (emulated_mq_t *)handle;

  syscore_sem_wait(&mq->sem_mutex);
  if (old_attr != NULL) {
    old_attr->flags = mq->shm_ptr->non_block ? 0x800 : 0;
    old_attr->max_msgs = mq->shm_ptr->max_msgs;
    old_attr->msg_size = mq->shm_ptr->msg_size;
    old_attr->cur_msgs = mq->shm_ptr->cur_msgs;
  }

  mq->shm_ptr->non_block = (attr->flags & 0x800) != 0;
  mq->non_block = mq->shm_ptr->non_block;
  syscore_sem_post(&mq->sem_mutex);

  return SYSCORE_SUCCESS;
}

#endif
