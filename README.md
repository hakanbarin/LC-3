# LC-3 Sanal Makine (Modern C++)

**Little Computer 3 (LC-3)** mimarisinin, **Modern C++ (C++20/23)** ile yazılmış eksiksiz bir uygulaması. Bu sanal makine, **2048** ve **Rogue** gibi oyunlar dahil olmak üzere LC-3 assembly programlarını doğrudan terminalinizde çalıştırabilir.

## 🚀 Özellikler

- **Modern C++ Standartları:** `std::byteswap`, `std::format`, `std::filesystem`.
- **Bellek Eşlemeli G/Ç (Memory Mapped I/O):** Gerçek zamanlı klavye etkileşimi için donanım yazmaçlarını (`KBSR`, `KBDR`) simüle eder.
- **Ham (Raw) Terminal Modu:** Oyun deneyimi için gerekli olan standart dışı girişi (Enter tuşuna basmadan algılama) işlemek ve yankıyı (echo) devre dışı bırakmak için özel bir `TerminalManager` sınıfı uygular.
- **Komut Seti:** Tüm LC-3 işlem kodları (`ADD`, `AND`, `NOT`, `BR`, `JMP`, `JSR`, `LD`, `LDI`, `LDR`, `LEA`, `ST`, `STI`, `STR`, `TRAP`) için tam destek sağlar.
- **Trap Rutinleri:** `GETC`, `OUT`, `PUTS`, `IN`, `PUTSP` ve `HALT` işlemleri Yüksek Seviyeli Emülasyon (HLE) ile işlenir.

## 🛠️ Mimari

VM aşağıdaki donanım bileşenlerini simüle eder:
- **Bellek:** 65,536 konum (16-bit adreslenebilir).
- **Yazmaçlar (Registers):** 8 Genel Amaçlı Yazmaç (R0-R7), PC (Program Sayacı) ve COND (Durum Bayrakları).
- **Giriş/Çıkış:** UNIX `select()` sistem çağrısını kullanarak asenkron klavye yoklaması (polling).

## 📦 Kurulum ve Derleme

C++20/23 destekleyen bir C++ derleyicisinin (GCC 12+ veya Clang 15+) ve CMake'in sisteminizde kurulu olduğundan emin olun.

```bash
# Depoyu klonlayın
git clone https://github.com/hakanbarin/LC-3.git
cd LC-3

# Derleme (build) klasörünü oluşturun
mkdir build
cd build

# Yapılandırın ve Derleyin
cmake ..
make

#.obj dosyasını çalıştırmak için
./lc3 2048.obj 
./lc3 rogue.obj