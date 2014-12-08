#define _GNU_SOURCE
#include <libdis.h>
#include <dlfcn.h>
#include <link.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define LINE_SIZE 1024
#define PTR_SIZE ((sizeof(void *) << 1) + 2)

void main_end();
int main(int, char **);

typedef struct disasm_state {
  void *base, *func, *file_base;
  char *filename, *func_name;
  unsigned long offset, size, func_offset;
  x86_insn_t instruction;
  unsigned short instruction_size;
} disasm_state;

void disasm_reporter(enum x86_report_codes code, void *data, void *arg)
{
  disasm_state *state = (disasm_state *) arg;

  fprintf(stderr, "Error at %p: ", data);

  switch (code)
  {
    case report_disasm_bounds:
      fprintf(stderr, "report_disasm_bounds (Attempt to disassemble RVA beyond end of buffer)\n");
      break;

    case report_insn_bounds:
      fprintf(stderr, "report_insn_bounds (Instruction at RVA extends beyond end of buffer)\n");
      break;

    case report_invalid_insn:
      fprintf(stderr, "report_invalid_insn (Invalid opcode)\n");
      break;

    case report_unknown:
      fprintf(stderr, "report_unknown (Unknown error)\n");
      break;
  }
}

void disassemble(void *address, size_t size)
{
  disasm_state state;
  unsigned short insn_size;
  x86_insn_t insn;
  Dl_info dli;
  char *fname = NULL;
  char line[256];

  state->base = address;
  state->offset = 0;
  state->size = size;
  state->filename = NULL;
  state->func = NULL;
  state->func_name = NULL;
  state->func_offset = 0;
  state->instruction_size = 0;

  x86_init(opt_none, disasm_reporter, (unsigned char *) state->base);

  for (state->offset = 0; state->offset < state->size; state->offset += state->instruction_size)
  {
    if (dladdr(state->base + state->offset, &dli)
    {
      if (dli.dli_sname && (!state->func_name || strcmp(state->func_name, dli.dli_sname)))
      {
        if (state->func_name)
          free(state->func_name);
        state->func = strdup(dli.dli_sname);
      }

      if (dli.dli_saddr)
      {
        state->func = dli.dli_saddr;
        state->func_offset = (state->base + state->offset) - dli.dli_saddr;
      }

      if (dli.dli_fname)
      {
        if ((state->filename && strcmp(state->filename, dli.dli_fname)) || !state->filename)
        {
          state->filename = strdup(dli.dli_fname);
          state->file_base = dli.dli_fbase;
          fprintf(stderr, "%*p | .file \"%s\"\n", PTR_SIZE, state->file_base, state->filename);
        }
      }

      state->size = x86_disasm((unsigned char *) state->base, state->size
    }

    state->instruction_size = x86_disasm((unsigned char *) state->base, state->size, 0, state->offset, &state->instruction);
    
    x86_format_insn(&state->instruction, line, sizeof(line), att_syntax);
    fprintf(stderr, "%*p | %s", PTR_SIZE, line);
    
  }

  for (offset = 0; offset < size; offset += insn_size)
  {
    if (dladdr(base + offset, &dli))
    {
      if (dli.dli_fname && (!fname || strcmp(fname, dli.dli_fname)))
      {
        if (fname)
          free(fname);
        fname = strdup(dli.dli_fname);
        printf("%*p | .file \"%s\"\n", PTR_SIZE, fname);
      }
      if (dli.dli_sname && (dli.dli_saddr == (base + offset)))
      {
        printf("%*p | .global %s\n%*p | %s:\n", PTR_SIZE, dli.dli_saddr, dli.dli_sname, PTR_SIZE, dli.dli_saddr, dli.dli_sname);
      }
    }

  }

  x86_cleanup();
}

int main(int argc, char *argv[])
{
  size_t main_size = (void *) main_end - (void *) main;
  char line[LINE_SIZE];
  int pos = 0;
  int size;                /* size of instruction */
  x86_insn_t insn;         /* instruction */
  Dl_info info;
  unsigned char *disasm_addr;
  char *last_file = NULL;

  disasm_addr = (unsigned char *) main;
  x86_init(opt_none, NULL, NULL);
  printf("Disassembling %d bytes @ %p\n", main_size, main);

  while ( pos < main_size ) {
    if (dladdr(disasm_addr + pos, &info))
    {
      if (info.dli_fname && (!last_file || !strcmp(last_file, info.dli_fname)))
      {
        if (last_file)
          free(last_file);
        last_file = strdup(info.dli_fname);
        printf("%*p\t.file \"%s\"\n", PTR_SIZE, disasm_addr + pos, last_file);
      }

      if (info.dli_sname && (info.dli_saddr == disasm_addr + pos))
        printf("%*p\t.global %s\n%*p\t%s:\n", PTR_SIZE, disasm_addr + pos, info.dli_sname, PTR_SIZE, disasm_addr + pos, info.dli_sname);
    }
    else
    {
      info.dli_sname = NULL;
    }

    /* disassemble address */
    size = x86_disasm(disasm_addr, main_size, 0, pos, &insn);

    if ( size ) {

      /* print instruction */
      x86_format_insn(&insn, line, main_size, att_syntax);
      printf("%*p\t%-32s", PTR_SIZE, disasm_addr + pos, line);

      if (info.dli_sname)
        printf("# <%s> + %lu", info.dli_sname, (unsigned long) ((disasm_addr + pos) - (unsigned char *) info.dli_saddr));

      pos += size;

    } else {
      printf("Invalid instruction\n");
      pos++;
    }
  }

  x86_cleanup();

  return 0;
}

void __attribute__((used)) main_end() {}
