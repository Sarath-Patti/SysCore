#ifndef SYSCORE_MESSAGE_QUEUE_H
#define SYSCORE_MESSAGE_QUEUE_H

#include "common/errors.h"
#include "common/utils.h"
#include <stddef.h>

#if defined(SYSCORE_OS_LINUX)
#include <mqueue.h>
typedef mqd_t syscore_mq_handle_t;
#define SYSCORE_MQ_INVALID_HANDLE ((syscore_mq_handle_t)-1)
#else
typedef void *syscore_mq_handle_t;
#define SYSCORE_MQ_INVALID_HANDLE ((syscore_mq_handle_t)0)
#endif

/**
 * Message Queue Attributes structure.
 */
typedef struct {
  long flags;    // Flags: 0 or O_NONBLOCK
  long max_msgs; // Max number of messages in queue
  long msg_size; // Max message size in bytes
  long cur_msgs; // Current number of messages in queue
} syscore_mq_attr_t;

/**
 * Opens or creates a message queue.
 *
 * @param name Name of the queue (must start with '/').
 * @param write_mode 0 = read-only, 1 = write-only, 2 = read-write.
 * @param non_block Non-zero for non-blocking mode.
 * @param mode Permissions (e.g., 0666).
 * @param attr Attributes (only used when creating, can be NULL).
 * @param out_handle Pointer to store the handle.
 */
syscore_error_t syscore_mq_open(const char *name, int write_mode, int non_block,
                                int mode, const syscore_mq_attr_t *attr,
                                syscore_mq_handle_t *out_handle);

/**
 * Closes the message queue descriptor.
 *
 * @param handle The message queue descriptor.
 */
syscore_error_t syscore_mq_close(syscore_mq_handle_t handle);

/**
 * Destroys/unlinks the message queue name.
 *
 * @param name The name of the message queue.
 */
syscore_error_t syscore_mq_unlink(const char *name);

/**
 * Sends a message to the queue.
 *
 * @param handle The message queue descriptor.
 * @param msg_ptr Pointer to the message data buffer.
 * @param msg_len Length of the message in bytes.
 * @param msg_prio Priority of the message (lower numbers have lower priority).
 */
syscore_error_t syscore_mq_send(syscore_mq_handle_t handle, const char *msg_ptr,
                                size_t msg_len, unsigned int msg_prio);

/**
 * Receives a message from the queue.
 *
 * @param handle The message queue descriptor.
 * @param msg_ptr Pointer to buffer to write message data.
 * @param msg_len Capacity of the buffer in bytes (must be >= mq_msgsize).
 * @param out_msg_prio Pointer to store the received message priority (can be
 * NULL).
 * @param out_read_bytes Pointer to store the number of bytes read (can be
 * NULL).
 */
syscore_error_t syscore_mq_receive(syscore_mq_handle_t handle, char *msg_ptr,
                                   size_t msg_len, unsigned int *out_msg_prio,
                                   size_t *out_read_bytes);

/**
 * Retrieves the current attributes of the queue.
 *
 * @param handle The message queue descriptor.
 * @param attr Pointer to attributes structure to populate.
 */
syscore_error_t syscore_mq_get_attr(syscore_mq_handle_t handle,
                                    syscore_mq_attr_t *attr);

/**
 * Sets new attributes on the queue (only flags can be modified).
 *
 * @param handle The message queue descriptor.
 * @param attr Pointer to new attributes.
 * @param old_attr Pointer to retrieve old attributes (can be NULL).
 */
syscore_error_t syscore_mq_set_attr(syscore_mq_handle_t handle,
                                    const syscore_mq_attr_t *attr,
                                    syscore_mq_attr_t *old_attr);

#endif // SYSCORE_MESSAGE_QUEUE_H
