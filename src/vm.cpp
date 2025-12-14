#include "vm.h"

// ============================================================================
// Keyboard Check
// ============================================================================

[[nodiscard]] bool VirtualMachine::check_key() {
  // klavye okumasına bakılıyor [[nodiscard]] ile bunun kontrol edilip
  // edilmediğini kontrol ediyoruz.
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  struct timeval timeout{.tv_sec = 0, .tv_usec = 0};

  // POLLING-> herhangi bir bekleme yapılmıyor anlık olarak kontrol ediliyor.
  // LC-3'te bu kullanılıyor.
  /*
  Interrupt-Driven vs Polling
  Continuing with the above keyboard example, the question is how does the
  microprocessor know when the ready bit has been set? One way is by polling
  where the microprocessor is continuously checking to see if the ready bit has
  been set or not. If it is set then it will go and read in the key. This method
  does not require any extra hardware support but waste a lot of CPU time for
  the microprocessor to continually check the ready bit. A more efficient
  method, but requires extra hardware support, is to use an interrupt. The
  microprocessor is doing its own thing until it is interrupted by the keyboard,
  at which time it will then go and read in the key. The LC3 uses the polling
  method.
  https://hwang.lasierra.edu/~enoch/CPTG%20245/LC-3/LC-3%20InputOutput.pdf
  */

  /*
  extern int select (int __nfds, fd_set *__restrict __readfds,
  fd_set *__restrict __writefds,
  fd_set *__restrict __exceptfds,
  struct timeval *__restrict __timeout);

  Dosya ID'leri:  0   1   2   3   4   5  ...  1023   FD'lerin gösterimi
  Bits:         [ 1 | 0 | 0 | 0 | 0 | 0 | ... | 0 ]
                  ^
             (Bizim Klavye - STDIN)
  */

  return select(1, // kaç tane fd'ye bakmak istiyorsak gibi düşünebiliriz.
                &readfds, // klavyeden veri gelip gelmediğine bakılıyor.
                nullptr, // yazma listesi ile ilgili bir şeyi kontrol etmiyoruz.
                nullptr, // hata listesini de kontrol etmiyoruz.
                &timeout // bekleme süresini ayarladığımız gibi veriyoruz.
                ) > 0;   // 0 dan büyükse veri var demektir.
}

// ============================================================================
// Memory Operations
// ============================================================================

void VirtualMachine::mem_write(uint16_t address, uint16_t val) {
  memory.at(address) = val;
}
[[nodiscard]] uint16_t VirtualMachine::mem_read(uint16_t address) {
  
  //<utily>'de bulunan fonksiyon içi yazılmış template, static cast alternatifi(detayına bakınız) fonksiyon
  if (address == to_underlying(MemoryMappedRegister::KBSR)) {
    /*
    enum MMR
    {
        //     MR_KBSR = 0xFE00, keyboard status
        //     MR_KBDR = 0xFE02  keyboard data
        }
        burada klavyeden gelen datayı kontrol etmek için adreslerimiz var eğer
    klavyede bir tuşa basılırsa obj dosyası içinde tuşa atanan işlev
    gerçekleştiririz
    */

    if (check_key()) // klavyeden giriş yaptıysak bu fonksiyon sayesinde kontrol
                     // yapıyoruz
    {
      memory.at(to_underlying(MemoryMappedRegister::KBSR)) = (1 << 15); 
      
      // KBSR'in 15. biti (ready bit) 1 olursa karakterin geldiği anlaşılıyor -.obj dosyası içinde-
      memory.at(to_underlying(MemoryMappedRegister::KBDR)) = static_cast<uint16_t>(std::cin.get());
    }

    /*
    BEKLEME_DONGUSU
    LDI R0, KBSR      ; 1. Adım: KBSR (0xFE00) adresindeki değeri R0'a yükle.
    ;    (burada  C++'taki mem_read fonksiyonun çağrılır)

    BRzp BEKLEME_DONGUSU ; 2. Adım: Eğer R0 >= 0 ise (Pozitif veya Sıfırsa),
    ; BEKLEME_DONGUSU'na geri dön (Zıpla).
    
                                                                                              -burayı assembly olarak bakmak için gemini'a yorumlattım-
    ; --- BURAYA SADECE SAYI NEGATİF OLURSA DÜŞER --- 15 SOLA KAYDIRARAK SAYIYI
    NEGATİF YAPIYORUZ !!!!

    LDI R0, KBDR      ; 3. Adım: KBDR (0xFE02) adresindeki harfi R0'a yükle.
    memory.at(MR_KBDR) = static_cast<uint16_t>(std::cin.get());
    */

    else {
      memory.at(to_underlying(MemoryMappedRegister::KBSR)) = 0;
    }
  }
  return memory.at(address);
}

// ============================================================================
// Image Loading
// ============================================================================

[[nodiscard]] bool VirtualMachine::read_image(const std::filesystem::path &path) {
  if (!std::filesystem::exists(path)) {

    std::cerr << "Hata: Dosya bulunamadi: " << path << std::endl;
    return false;
    
  }

  if (!std::filesystem::is_regular_file(path)) {
    std::cerr << "Hata: Gecerli bir dosya degil: " << path << std::endl;
    return false;
  }

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Hata: Dosya acilamadi: " << path << std::endl;
    return false;
  }

  read_image_file(file);



  return true;
}

// burada tüm .obj kodunu okuyup little endian big endian problemini çözüp işlenebilir hale getiriyoruz.
void VirtualMachine::read_image_file(std::ifstream &file) {
  // read(char_type* __s, streamsize __n) read char* alıyor bundan dolayı reinterpret yapıyoruz(static_cast değil dikkat)

  uint16_t origin;
  file.read(reinterpret_cast<char *>(&origin), sizeof(origin));
  if (!file) {
    return;
  }

  origin = std::byteswap(origin);

  const uint16_t max_read = static_cast<uint16_t>(MEMORY_MAX - origin);
  uint16_t *p = memory.data() + origin;

  file.read(reinterpret_cast<char *>(p), max_read * sizeof(uint16_t));

  // static_cast ile tekrardan düzenlenebilir.  gcount long int dönüyor bundan dolayı static_cast gerekiyor
  const auto items_read = static_cast<size_t>(file.gcount()) / sizeof(uint16_t);
  /*
  transform(_InputIterator __first, _InputIterator __last,
  OutputIterator __result, _UnaryOperation __unary_op)
  bu fonksiyon sayesinde tüm file'ı swapliyoruz
  std::execution::par!!!
  */

  std::transform(
      p,              // _InputIterator __first
      p + items_read, // InputIterator __last
      p,              // OutputIterator __result çıkışın yazılacağı yer
      [](uint16_t val) { return std::byteswap(val);}  //  _unary_op fonksiyon yazmak yerine lambda ile hallediyoruz.
  );
}


// update_flag yapmamızın amacı register güncellemeleri sonucunda geri dönüş değerini bilmemiz gerektiği.
//         -örnek olarak bir eşitlik sorgulayacağız dönüş değeri bilmeden bunu yapamayız-

void VirtualMachine::update_flags(uint16_t r) {
  if (reg[r] == 0) {
    reg[to_underlying(Register::COND)] = to_underlying(ConditionFlag::ZRO);
  } else if (reg[r] >> 15) /* en soldaki bitin 1 mi 0 mı olduğuna bakılarak neg
                              pos ya da zero olduğuna karar veriliyor */
  {
    reg[to_underlying(Register::COND)] = to_underlying(ConditionFlag::NEG);
  } else {
    reg[to_underlying(Register::COND)] = to_underlying(ConditionFlag::POS);
  }
}


// ADD (Register Mode) - Toplama (İki Register)
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5 |  4   3 |  2   1   0    3
// ve 4. bit sabit
//  0   0   0   1 |     DR     |    SR1     |  0 |  0   0 |    SR2
// ------------------------------------------------------------------

// ADD (Immediate Mode) - Toplama (Register + Sabit Sayı)
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5 |  4   3   2   1   0
//  0   0   0   1 |     DR     |    SR1     |  1 |       imm5
// ------------------------------------------------------------------

void VirtualMachine::process_ADD(uint16_t instr) {
  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t r1 = (instr >> 6) & 0x7;
  uint16_t imm_flag = (instr >> 5) & 0x1;

  if (imm_flag) {
    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
    reg[r0] = reg[r1] + imm5;
  } else {
    uint16_t r2 = instr & 0x7;
    reg[r0] = reg[r1] + reg[r2];
  }
  update_flags(r0);
}

// AND (Register Mode) - Ve İşlemi (İki Register)
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5 |  4   3 |  2   1   0
//  0   1   0   1 |     DR     |    SR1     |  0 |  0   0 |    SR2
// ------------------------------------------------------------------

// AND (Immediate Mode) - Ve İşlemi (Register + Sabit Sayı)
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5 |  4   3   2   1   0
//  0   1   0   1 |     DR     |    SR1     |  1 |       imm5
// ------------------------------------------------------------------

void VirtualMachine::process_AND(uint16_t instr) {
  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t r1 = (instr >> 6) & 0x7;
  uint16_t imm_flag = (instr >> 5) & 0x1;

  if (imm_flag) {
    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
    reg[r0] = reg[r1] & imm5;
  } else {
    uint16_t r2 = instr & 0x7;
    reg[r0] = reg[r1] & reg[r2];
  }
  update_flags(r0);
}

// NOT (Bitwise Complement) - Değil İşlemi
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5   4   3   2   1   0
//  1   0   0   1 |     DR     |     SR     |  1   1   1   1   1   1
// ------------------------------------------------------------------

void VirtualMachine::process_NOT(uint16_t instr) {
  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t r1 = (instr >> 6) & 0x7;

  reg[r0] = ~reg[r1];
  update_flags(r0);
}

// BR (Branch) - Koşullu Dallanma
// 15  14  13  12 | 11  10   9 |  8   7   6   5   4   3   2   1   0
//  0   0   0   0 |  n   z   p |             PCoffset9
// ------------------------------------------------------------------

void VirtualMachine::process_BR(uint16_t instr) {
  uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
  uint16_t cond_flag = (instr >> 9) & 0x7;

  if (cond_flag &
      reg[to_underlying(Register::COND)]) // Eğer R_COND ile condflagin herhangi
                                          // bir biti uyuşursa atlıyoruz.
  { 
    //  örnek if(x <=0) dersek condflag 110 ise ve x te 0 ya da 0 dan küçükse zıplarız 
    //  condflags 110 küçük veya 0 demek yani negative = 1 zero = 1 positive = 0 -> küçük veya eşit  
    //      (nzp: negative zero positive)
    reg[to_underlying(Register::PC)] += pc_offset; // sadece 0 göre bakılıyor DİKKAT ET!!!!
  }
}

// JMP (Jump) - Atlama / RET (Return)
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5   4   3   2   1   0
//  1   1   0   0 |  0   0   0 |    BaseR   |  0   0   0   0   0   0
// * Not: Eğer BaseR = 111 (R7) ise bu komut RET (Geri Dön) olur.
// ------------------------------------------------------------------

void VirtualMachine::process_JMP(uint16_t instr) {
  uint16_t r1 = (instr >> 6) & 0x7; // BaseR (opcodes kısmında gösteriliyor)
                                    // değerine göre registera koşulsuz atlar
  reg[to_underlying(Register::PC)] = reg[r1]; // atlamayı sağlar. Koşullu atlama JSR'de yapılıyor. İlk 4 bit ve
               // BaseR bitleri hariç diğer bitler herhangi bir işe yaramıyor.
}

// JSR (Jump to Subroutine) - Alt Programa Atlama (PCoffset Modu)
// 15  14  13  12 | 11 | 10  9    8   7   6   5   4   3   2   1   0
//  0   1   0   0 |  1 |               PCoffset11
// ------------------------------------------------------------------

// JSRR (Jump to Subroutine Register) - Alt Programa Atlama (Register Modu)
// 15  14  13  12 | 11 | 10  9 |  8   7   6 |  5   4   3   2   1   0
//  0   1   0   0 |  0 |  0  0 |    BaseR   |  0   0   0   0   0   0
// ------------------------------------------------------------------

void VirtualMachine::process_JSR(uint16_t instr) {
  reg[to_underlying(Register::R7)] = reg[to_underlying(Register::PC)]; 
  // Geri dönebilmek amacıyla reg[R_R7]'de şu anki adresimizi tutuyoruz.
  uint16_t flag = (instr >> 11) & 0x1;

  if (flag) /* Jump to Subroutine - JSR */
  {
    uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
    reg[to_underlying(Register::PC)] += pc_offset;
  } else /* Jump to Subroutine Register - JSRR */
  {
    uint16_t r1 = (instr >> 6) & 0x7;
    reg[to_underlying(Register::PC)] = reg[r1];
  }
}

// LD (Load) - PC'ye Göre Yükleme
// 15  14  13  12 | 11  10   9 |  8   7   6   5   4   3   2   1   0
//  0   0   1   0 |     DR     |             PCoffset9
// ------------------------------------------------------------------

void VirtualMachine::process_LD(uint16_t instr) 
// Herhangi bir adrese gidip oradaki adrese göre atlamıyor. Direkt PCoffset9'a göre atlıyor.
// Dikkat et LDI'da mem_read içinde mem_read çalıştırıyoruz. Aşamalı atlanıyor. Çekirge 2 zıplıyor.
{
  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
  reg[r0] = mem_read(reg[to_underlying(Register::PC)] + pc_offset);
  update_flags(r0);
}

// LDI (Load Indirect) - Dolaylı Yükleme (Çift Zıplama)
// 15  14  13  12 | 11  10   9 |  8   7   6   5   4   3   2   1   0
//  1   0   1   0 |     DR     |             PCoffset9
// ------------------------------------------------------------------

void VirtualMachine::process_LDI(uint16_t instr) // Load Indirect
{
  // 1010(decisionbits)  DR(3bit) PCoffset(9 bit)

  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

  // Program Counter'a(PC) eklenerek adrese ulaşılıyor.  PC + (Pcoffset 9)

  reg[r0] = mem_read(mem_read(reg[to_underlying(Register::PC)] + pc_offset));
  update_flags(r0);
}

// LDR (Load Base+Offset) - Tabana Göre Yükleme
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5   4   3   2   1   0
//  0   1   1   0 |     DR     |    BaseR   |       offset6
// ------------------------------------------------------------------

void VirtualMachine::process_LDR(uint16_t instr) // Load Register
{
  uint16_t r0 = (instr >> 9) & 0x7; // DR
  uint16_t r1 =
      (instr >> 6) & 0x7; // BaseR yani Register seçmek için bitlerimiz
  uint16_t offset = sign_extend(instr & 0x3F, 6);

  reg[r0] = mem_read(reg[r1] + offset); // Destination Registera(DR) yazıyoruz.
  update_flags(r0);
}

// LEA (Load Effective Address) - Adresi Hesapla ve Yükle
// 15  14  13  12 | 11  10   9 |  8   7   6   5   4   3   2   1   0
//  1   1   1   0 |     DR     |             PCoffset9
// ------------------------------------------------------------------

void VirtualMachine::process_LEA(uint16_t instr) // Load Effective Address
{
  uint16_t r0 = (instr >> 9) & 0x7; // DR
  uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
  reg[r0] = reg[to_underlying(Register::PC)] + pc_offset;
  update_flags(r0);
}

// ST (Store) - PC'ye Göre Kaydetme
// 15  14  13  12 | 11  10   9 |  8   7   6   5   4   3   2   1   0
//  0   0   1   1 |     SR     |             PCoffset9
// ------------------------------------------------------------------
//         BİT ADI SR OLARAK DEĞİŞTİ DİKKAT

void VirtualMachine::process_ST(uint16_t instr) {
  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
  mem_write(reg[to_underlying(Register::PC)] + pc_offset, reg[r0]);
}

// STI (Store Indirect) - Dolaylı Kaydetme (Çift Zıplama)
// 15  14  13  12 | 11  10   9 |  8   7   6   5   4   3   2   1   0
//  1   0   1   1 |     SR     |             PCoffset9
// ------------------------------------------------------------------

void VirtualMachine::process_STI(uint16_t instr) 
// Aynı LDI'daki gibi önce gidip bir adresin gösterdiği
                    // adresi okuyup oraya yazıyoruz. Pointer düşün.
{
  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
  mem_write(mem_read(reg[to_underlying(Register::PC)] + pc_offset), reg[r0]);
}

// STR (Store Base+Offset) - Tabana Göre Kaydetme
// 15  14  13  12 | 11  10   9 |  8   7   6 |  5   4   3   2   1   0
//  0   1   1   1 |     SR     |    BaseR   |       offset6
// ------------------------------------------------------------------

void VirtualMachine::process_STR(uint16_t instr)
// İstediğimiz bir alana erişip orada 6 bitlik offset sınırında (maksimum bu kadar ileri ya da geri gidebiliyoruz) depolama yapıyoruz.
{ 
  uint16_t r0 = (instr >> 9) & 0x7;
  uint16_t r1 = (instr >> 6) & 0x7;
  uint16_t offset = sign_extend(instr & 0x3F, 6);
  mem_write(reg[r1] + offset, reg[r0]);
}

// TRAP (System Call) - İşletim Sistemi Çağrısı
// 15  14  13  12 | 11  10   9   8 |  7   6   5   4   3   2   1   0
//  1   1   1   1 |  0   0   0   0 |          trapvect8
// ------------------------------------------------------------------

// LC-3 programı, klavye okuma veya ekrana yazma gibi donanım işlerini
// kendisi yapamaz. Bunun yerine işletim sisteminden (VM'den) yardım ister.  -bu
// açıklama daha açıklayıcı geldi- Bu fonksiyon, o yardım çağrılarını (Interrupts/Syscalls) simüle eder.
void VirtualMachine::process_TRAP(uint16_t instr) {
  reg[to_underlying(Register::R7)] = reg[to_underlying(Register::PC)];
  uint16_t trapvect = instr & 0xFF; // Hangi TRAP instruction onu çekiyoruz.

  switch (static_cast<Trap>(trapvect)) {
  case Trap::GETC: {
    reg[to_underlying(Register::R0)] = static_cast<uint16_t>(std::cin.get());  // get int döndürüyor bundan dolayı static_cast yapıyoruz.
    update_flags(to_underlying(Register::R0));
    break;
  }

  case Trap::OUT: {
    std::cout.put(static_cast<char>(reg[to_underlying(Register::R0)]));
    std::cout.flush();
    break;
  }

  case Trap::PUTS: {
    uint16_t addr = reg[to_underlying(Register::R0)];
    while (memory.at(addr) != 0x0000) {
      std::cout.put(static_cast<char>(memory.at(addr)));
      addr++;
    }
    std::cout.flush();
    break;
  }

  case Trap::IN: {
    std::cout << "Karakter girin: " << std::flush;
    char c = static_cast<char>(std::cin.get());
    std::cout.put(c);
    std::cout.flush();
    reg[to_underlying(Register::R0)] = static_cast<uint16_t>(c);
    update_flags(to_underlying(Register::R0));
    break;
  }

  case Trap::PUTSP: {
    uint16_t addr = reg[to_underlying(Register::R0)];
    while (memory.at(addr) != 0x0000) {
      uint16_t two_chars = memory.at(addr);
      char char1 = static_cast<char>(two_chars & 0xFF);
      std::cout.put(char1);

      char char2 = static_cast<char>(two_chars >> 8);
      if (char2 != 0) {
        std::cout.put(char2);
      }
      addr++;
    }
    std::cout.flush();
    break;
  }

  case Trap::HALT: {
    std::cout << "\nVM durduruluyor." << std::endl;
    running = false;
    break;
  }

  default:
    std::cerr << "Bilinmeyen TRAP vektoru: 0x" << std::hex << trapvect
              << std::dec << std::endl;
    break;
  }
}

// ============================================================================
// Main Run Loop
// ============================================================================

[[nodiscard]] int VirtualMachine::run(int argc, const char *argv[]) {
  if (argc < 2) {
    std::cerr << "Kullanim: lc3 [image-file1] ...\n";
    return 1;
  }

  bool any_loaded = false;
  for (int j = 1; j < argc; ++j) {
    if (read_image(argv[j])) {
      any_loaded = true;
    } else {
      std::cerr << "Uyari: Dosya yuklenemedi: " << argv[j] << std::endl;
    }
  }

  if (!any_loaded) {
    std::cerr << "Hata: Hicbir image dosyasi yuklenemedi.\n";
    return 1;
  }

  reg[to_underlying(Register::COND)] = to_underlying(ConditionFlag::ZRO);
  reg[to_underlying(Register::PC)] = PC_START;

  while (running) {
    instr = mem_read(reg[to_underlying(Register::PC)]++);
    op = instr >> 12;

    switch (static_cast<Opcode>(op)) {
    case Opcode::ADD:
      process_ADD(instr);
      break;
    case Opcode::AND:
      process_AND(instr);
      break;
    case Opcode::NOT:
      process_NOT(instr);
      break;
    case Opcode::BR:
      process_BR(instr);
      break;
    case Opcode::JMP:
      process_JMP(instr);
      break;
    case Opcode::JSR:
      process_JSR(instr);
      break;
    case Opcode::LD:
      process_LD(instr);
      break;
    case Opcode::LDI:
      process_LDI(instr);
      break;
    case Opcode::LDR:
      process_LDR(instr);
      break;
    case Opcode::LEA:
      process_LEA(instr);
      break;
    case Opcode::ST:
      process_ST(instr);
      break;
    case Opcode::STI:
      process_STI(instr);
      break;
    case Opcode::STR:
      process_STR(instr);
      break;
    case Opcode::TRAP:
      process_TRAP(instr);
      break;

    case Opcode::RES:
    case Opcode::RTI:
    default:
      std::cerr << "Gecersiz opcode: 0x" << std::hex << op << std::dec << std::endl;
      running = false;
      return 1;
    }
  }

  return 0;
}
