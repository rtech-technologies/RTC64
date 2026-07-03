/* Crash notify helper for storing last crash path visible to user UI */
#ifndef CRASH_NOTIFY_H
#define CRASH_NOTIFY_H

void set_last_crash_path(const char* path);
const char* get_last_crash_path(void);

#endif
