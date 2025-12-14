//https://www.jmeiners.com/lc3-vm/

#include "terminal.h"
#include "vm.h"

#include <iostream>
#include <stdexcept>

int main(int argc, const char *argv[]) {
  try {
    std::signal(SIGINT, handle_interrupt);

    VirtualMachine vm;
    TerminalManager terminal_manager;
    return vm.run(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << "Hata: " << e.what() << std::endl;
    return 1;
  }
}