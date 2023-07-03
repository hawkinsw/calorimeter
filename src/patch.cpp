#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory.h>
#include <patch/patch.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <vector>

const uint8_t CALL_BYTE_CODE{0xE8};
const uint8_t CALL_OP_SIZE{0x5};
const uint8_t NOPL_BYTES[] = {0x0f, 0x1f, 0x44, 0x00, 0x00};
const uint8_t NOPL_OP_SIZE{0x5};

extern uint8_t __patch_eligible;

extern "C" void entry_target(void);
extern "C" void exit_target(void);

template <typename SymbolNameType, typename SymbolTargetType = uintptr_t>
class SymbolTable {
public:
  SymbolTable() { load(); };

  bool get(const SymbolNameType &name, SymbolTargetType &address) const {
    auto found{symbol_table.find(name)};
    if (found == symbol_table.end()) {
      return false;
    }
    address = std::get<1>(*found);
    return true;
  }

  bool set(const SymbolNameType &name, SymbolTargetType &address) {
    symbol_table.insert(std::make_pair(name, address));
    return true;
  }

private:
  std::map<SymbolNameType, SymbolTargetType> symbol_table{};

  void load() {
    size_t offset{0};

    uint8_t *patch_eligible{&__patch_eligible};
    while (patch_eligible[offset] != 0xff) {
      char symbol_buffer[256]{};
      uint64_t address{};
      size_t symbol_offset_start{offset};

      while (patch_eligible[offset] != 0x0) {
        symbol_buffer[offset - symbol_offset_start] = patch_eligible[offset];
        offset++;
      }
      offset++;
      memcpy(&address, (uint64_t *)&patch_eligible[offset], sizeof(uint64_t));
      offset += sizeof(uint64_t);

      printf("%s -> 0x%x\n", symbol_buffer, address);

      set(symbol_buffer, address);
    }
  }
};
SymbolTable<std::string, uintptr_t> symbol_table{};

template <typename PatchSourceType = uintptr_t,
          typename PatchTargetType = uintptr_t>
class PatchTable {
public:
  PatchTable(){};

  bool get_target(const PatchSourceType &source,
                  PatchTargetType &target) const {
    if (auto found{symbol_table.find(source)}; found != symbol_table.end()) {
      target = std::get<1>(*found);
      return true;
    }
    return false;
  }

  bool set_target(const PatchSourceType &source, PatchTargetType target) {
    symbol_table.insert(std::make_pair(source, target));
    return true;
  }

  bool unset_target(const PatchSourceType &source) {
    if (auto to_erase{symbol_table.find(source)};
        to_erase != symbol_table.end()) {
      symbol_table.erase(to_erase);
      return true;
    }
    return false;
  }

private:
  std::map<PatchSourceType, PatchTargetType> symbol_table{};
};
PatchTable<uintptr_t, uintptr_t> patch_table{};

template <typename PatchSourceType = uintptr_t> class PatchStack {
public:
  PatchStack(){};
  void push(const PatchSourceType source) { m_patches.push_back(source); }
  PatchSourceType pop() {
    auto back{m_patches.back()};
    m_patches.pop_back();
    return back;
  }

private:
  std::vector<PatchSourceType> m_patches{};
};
PatchStack<uintptr_t> patch_stack{};

static long get_page_size(long &page_size) {
  page_size = sysconf(_SC_PAGESIZE);
  return page_size != -1;
}

static bool page_align_address(std::uintptr_t &address) {
  long alignment_page_size{0};
  if (!get_page_size(alignment_page_size)) {
    std::cout << "Unable to get the page size!\n";
    return false;
  }
  uintptr_t ualignment_page_size{static_cast<uintptr_t>(alignment_page_size)};
  uintptr_t alignment_page_mask{~(ualignment_page_size - 1)};
  address &= alignment_page_mask;
  return true;
}

static bool make_address_writable(std::uintptr_t address) {
  if (!page_align_address(address)) {
    return false;
  }
  long page_size{0};
  if (!get_page_size(page_size)) {
    return false;
  }
  return !mprotect((void *)address, page_size,
                   PROT_READ | PROT_WRITE | PROT_EXEC);
}

static bool make_address_unwritable(std::uintptr_t address) {
  if (!page_align_address(address)) {
    return false;
  }
  long page_size{0};
  if (!get_page_size(page_size)) {
    return false;
  }
  return !mprotect((void *)address, page_size,
                   PROT_READ | PROT_WRITE | PROT_EXEC);
}

extern "C" void trampoline_out(uintptr_t old_ra, uintptr_t trampoline_src) {
  std::cout << "Being asked to remember: 0x" << std::hex << old_ra
            << " (source: " << trampoline_src << ")\n";
  patch_stack.push(old_ra);

  uintptr_t user_target{};
  // Our trampoline src is the place we are going to return once we are done
  // bouncing to here. In other words, it will be sizeof(call operation) bytes
  // *past* where the user actually inserted the patch. So, adjust by that size
  // before we do the lookup into the patch table.
  trampoline_src -= CALL_OP_SIZE;
  if (patch_table.get_target(trampoline_src, user_target)) {
    void (*fn)(void){(void (*)(void))user_target};
    fn();
  }
}

extern "C" uintptr_t trampoline_in() {
  auto prior_ra{patch_stack.pop()};
  std::cout << "Being asked to recall: 0x" << std::hex << prior_ra << "\n";
  return prior_ra;
}

void make_patch_to_call(uint8_t insn_buffer[], uintptr_t source,
                        uintptr_t destination, size_t &insn_size) {
  int32_t rip_rel_call_target{
      static_cast<int32_t>(destination - (source + CALL_OP_SIZE))};
  insn_buffer[0] = CALL_BYTE_CODE;
  memcpy(insn_buffer + 1, &rip_rel_call_target, sizeof(int32_t));
  insn_size = CALL_OP_SIZE;
}

bool hook(const std::string &source, void *to) {
  uintptr_t source_address{};

  // Find the address of the function that the user wants to hook
  // If we cannot find a matching symbol, then we'll jump away.
  if (!symbol_table.get(source, source_address)) {
    return false;
  }

  patch_table.set_target(source_address, (uintptr_t)to);

  // We always patch to entry_target because that handles all the setup. *That*
  // will ultimately take us to where the user wants to go! So, make the call
  // that fulfills that duty!
  uint8_t call_insn[CALL_OP_SIZE]{};
  size_t call_insn_size{0};
  make_patch_to_call(call_insn, source_address, (uintptr_t)entry_target,
                     call_insn_size);

  // Now, actually write that call into the proper place by
  // 1. Making the memory there writable
  // 2. Doing the write
  // 3. Making the memory there unwritable
  if (!make_address_writable(source_address)) {
    return false;
  }
  memcpy((void *)source_address, call_insn, call_insn_size);
  if (!make_address_unwritable(source_address)) {
    return false;
  }

  return true;
}

bool unhook(const std::string &source) {
  uintptr_t source_address{};

  // Find the address of the function that the user wants to hook
  // If we cannot find a matching symbol, then we'll jump away.
  if (!symbol_table.get(source, source_address)) {
    return false;
  }

  patch_table.unset_target(source_address);

  // Now, actually write that nop into place by
  // 1. Making the memory there writable
  // 2. Doing the write
  // 3. Making the memory there unwritable
  if (!make_address_writable(source_address)) {
    return false;
  }
  memcpy((void *)source_address, NOPL_BYTES, NOPL_OP_SIZE);
  if (!make_address_unwritable(source_address)) {
    return false;
  }

  return true;
}