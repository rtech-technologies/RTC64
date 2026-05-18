#include <rsl.h>

void shell_main() {
    print("\n--- RTECH OSx2 (Limemade Core) Shell ---\n");
    for(;;) {
        char* cmd = input("unice64> ");
        if (cmd[0] == '\0') continue;

        if (cmd[0] == 'l' && cmd[1] == 's') {
            print("BOOT:/  INITRD:/  SATA0:/\n");
        } else if (cmd[0] == 'p' && cmd[1] == 'a' && cmd[2] == 'n' && cmd[3] == 'i' && cmd[4] == 'c') {
            extern void quartermaster_panic(const char* msg);
            quartermaster_panic("User requested kernel lockdown.");
        } else {
            print("Unknown command. Try: ls, panic\n");
        }
    }
}
