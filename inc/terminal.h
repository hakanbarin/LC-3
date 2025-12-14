#ifndef TERMINAL_H
#define TERMINAL_H

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <termios.h>
#include <unistd.h>

class TerminalManager {
private:
  // terminalin başlangıç ayarlarını burada tutuyoruz bu çok önemli yoksa bilgisayarı resetlememiz gerekir. :D
  inline static struct termios original_tio{};
  inline static bool is_raw_mode = false;

public:
  TerminalManager() {
    // Terminal Control Get Attributes - terminal ayarlarını kopyalıyoruz.
    if (tcgetattr(STDIN_FILENO, &original_tio) == -1) {
      throw std::runtime_error(
          "Terminal ayarlari alinamadi (tcgetattr hatasi)");
    }

    // ayarları değiştirmek için kopya başlangıç ayarlarından kopya
    // oluşturuyoruz.
    struct termios new_tio = original_tio;
    // ~ICANON satır tamponlamayı devre dışı bırak. Entera basmadan terminal
    // üzerinden veri alıyoruz. ~ECHO   tuşların ekrana geri basılmasını
    // engeller(echoyu engeller)
    new_tio.c_lflag &= static_cast<tcflag_t>(~ICANON & ~ECHO);

    // Terminal Control Set Attributes - yeni ayarları kopyamıza yüklüyoruz.
    // TCSANOW hemen yapılmasını sağlar.
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_tio) == -1) {
      throw std::runtime_error(
          "Terminal ayarlari yapilamadi (tcsetattr hatasi)");
    }
    is_raw_mode = true;
  }

  ~TerminalManager() { restore(); }

  // Copy/Move operations disabled - RAII
  TerminalManager(const TerminalManager &) = delete;
  TerminalManager &operator=(const TerminalManager &) = delete;
  TerminalManager(TerminalManager &&) = delete;
  TerminalManager &operator=(TerminalManager &&) = delete;

  static void restore() noexcept {
    // Terminal Control Set Attributes - eski ayarları geri yüklüyoruz. TCSANOW
    // hemen yapılmasını sağlar.
    if (is_raw_mode) {
      tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
      is_raw_mode = false;
    }
  }
};


// Signal handler (defined in terminal.cpp)
void handle_interrupt(int signal);

#endif // TERMINAL_H