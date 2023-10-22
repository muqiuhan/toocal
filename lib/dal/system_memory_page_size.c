#include <unistd.h>

#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>

value
__ocaml_system_memory_page_size ()
{
  CAMLparam0 ();
  CAMLlocal1 (page_size);

  page_size = Val_int (getpagesize ());

  return page_size;
}