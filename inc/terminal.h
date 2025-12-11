#ifndef TERMINAL_H
#define TERMINAL_H

#include <csignal>   
#include <cstdlib>  
#include <iostream>
#include <termios.h> 
#include <unistd.h>  

class TerminalManager
{
private:
// terminalin başlangıç ayarlarını burada tutuyoruz bu çok önemli yoksa bilgisayarı resetlememiz gerekir. :D
    struct termios original_tio; 

public:
    
TerminalManager()
    {
        // Terminal Control Get Attributes - terminal ayarlarını kopyalıyoruz.
        tcgetattr(STDIN_FILENO, &original_tio);

        // ayarları değiştirmek için kopya başlangıç ayarlarından kopya oluşturuyoruz.
        struct termios new_tio = original_tio;

        new_tio.c_lflag &= ~ICANON & ~ECHO;

        // ~ICANON satır tamponlamayı devre dışı bırak. Entera basmadan terminal üzerinden veri alıyoruz.
        // ~ECHO   tuşların ekrana geri basılmasını engeller(echoyu engeller)

        // Terminal Control Set Attributes - yeni ayarları kopyamıza yüklüyoruz. TCSANOW hemen yapılmasını sağlar.
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    }


    ~TerminalManager()
    {
        restore();
    }

    void restore()
    {
        // Terminal Control Set Attributes - eski ayarları geri yüklüyoruz. TCSANOW hemen yapılmasını sağlar.
        tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
    }
};


TerminalManager* global_terminal_manager = nullptr;

// C'de de olduğu gibi sinyalleri istediğimiz fonksiyona yönlendirerek vm'i kapatıyoruz.
void handle_interrupt(int signal)
{
    if (global_terminal_manager)
    {
        global_terminal_manager->restore(); 
    }
    std::cout << "\nprogram ctrl+c ile sonlandirildi.\n";
    std::exit(-2); 
}
#endif // TERMINAL_H