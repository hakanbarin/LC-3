#include "terminal.h"


// C'de de olduğu gibi sinyalleri istediğimiz fonksiyona yönlendirerek vm'i kapatıyoruz.
// C'de void signal yerine burda maybe_unused kullanılıyor, yani argümanı kullanmıyorum ve bunu biliyorum diyoruz.
void handle_interrupt([[maybe_unused]] int signal) {

//buradaki global değişken tanımlama kısmı değişti
  TerminalManager::restore();

  
  std::cout << "\nProgram Ctrl+C ile sonlandirildi.\n";


  std::exit(130); // 128 + SIGINT(2) = 130 amacımız hatanın ne olduğunu bilmek  --128 direkt sinyallerin hata başlangıç sayısı--
}
