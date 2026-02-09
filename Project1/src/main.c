#include "ipc.h"
#include "cli.h"

int main(void) {
    printf("[main] Starting interactive process, pid=%d\n", getpid());

    ipc_init();
    cli_loop();      
    ipc_cleanup();
      
    printf("[main] exiting pid=%d\n", getpid());
    return 0;
}
