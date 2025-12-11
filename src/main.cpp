#include "vm.h"
#include "terminal.h"



int main(int argc, const char* argv[])
{
    TerminalManager terminal_manager;

    global_terminal_manager = &terminal_manager;
    std::signal(SIGINT, handle_interrupt);

    VirtualMachine vm;
    vm.run(argc, argv);


    return 0;
}