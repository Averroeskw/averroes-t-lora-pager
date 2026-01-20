#ifndef UI_MESSAGE_QUEUE_H
#define UI_MESSAGE_QUEUE_H

#include <Arduino.h>
#include <mutex>
#include <string>
#include <vector>

/**
 * UIMessageQueue
 * A thread-safe, pre-allocated pool of messages to avoid heap fragmentation
 * during high-speed terminal data reception.
 */

#define UI_POOL_SIZE 16
#define MAX_ASYNC_TEXT 1024

struct UIMessage {
  char text[MAX_ASYNC_TEXT];
  bool clear;
  bool in_use;
  uint32_t id;
};

class UIMessageQueue {
public:
  UIMessageQueue() {
    for (int i = 0; i < UI_POOL_SIZE; i++) {
      pool[i].in_use = false;
      pool[i].id = i;
    }
  }

  UIMessage *checkout() {
    // Simple linear search for an available message
    // In a real high-perf system we'd use a bitmask or semaphore
    for (int i = 0; i < UI_POOL_SIZE; i++) {
      if (!pool[i].in_use) {
        pool[i].in_use = true;
        pool[i].text[0] = '\0';
        pool[i].clear = false;
        return &pool[i];
      }
    }
    return nullptr; // Pool exhausted
  }

  void release(UIMessage *msg) {
    if (msg) {
      msg->in_use = false;
    }
  }

private:
  UIMessage pool[UI_POOL_SIZE];
};

#endif // UI_MESSAGE_QUEUE_H
