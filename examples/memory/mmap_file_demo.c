#include "common/errors.h"
#include "common/logging.h"
#include "memory/mmap.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define FILE_NAME "mmap_demo_temp.txt"
#define FILE_SIZE 64

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting mmap File-Backed Mapping Demo");

  // Create temporary file and write some placeholder text
  int fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    SYSCORE_LOG_ERROR("Failed to open file: %s", FILE_NAME);
    return 1;
  }

  // Set file size
  if (ftruncate(fd, FILE_SIZE) < 0) {
    SYSCORE_LOG_ERROR("Failed to set file size");
    close(fd);
    unlink(FILE_NAME);
    return 1;
  }

  const char *msg = "Initial File Data";
  write(fd, msg, strlen(msg));

  // Map file
  void *addr = NULL;
  syscore_error_t err = syscore_mmap(
      NULL, FILE_SIZE, SYSCORE_PROT_READ | SYSCORE_PROT_WRITE,
      SYSCORE_MAP_SHARED, fd, 0, &addr);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("mmap file failed: %s", syscore_error_string(err));
    close(fd);
    unlink(FILE_NAME);
    return 1;
  }

  SYSCORE_LOG_INFO("Mapped file successfully at %p. Content: \"%s\"", addr,
                   (char *)addr);

  // Modify content in memory
  SYSCORE_LOG_INFO("Modifying mapped file memory...");
  strcpy((char *)addr, "Modified File Data via mmap!");

  // Synchronize changes to disk
  SYSCORE_LOG_INFO("Calling msync to flush modifications to disk...");
  err = syscore_msync(addr, FILE_SIZE, SYSCORE_MS_SYNC);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("msync failed: %s", syscore_error_string(err));
  }

  // Unmap and cleanup
  syscore_munmap(addr, FILE_SIZE);
  close(fd);

  // Verify by reading the file
  fd = open(FILE_NAME, O_RDONLY);
  if (fd >= 0) {
    char buffer[64];
    memset(buffer, 0, sizeof(buffer));
    read(fd, buffer, sizeof(buffer) - 1);
    SYSCORE_LOG_INFO("File content read from disk after msync: \"%s\"", buffer);
    close(fd);
  }

  unlink(FILE_NAME);
  SYSCORE_LOG_INFO("File-backed mapping demo finished successfully.");
  return 0;
}
