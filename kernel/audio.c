#define MINIAUDIO_IMPLEMENTATION
#include "pro_os.h"
#include "external/miniaudio.h"

void hal_audio_init(void) {
    serial_write("[AUDIO] Initializing Sovereign Audio Engine (Miniaudio)...\n");
}
