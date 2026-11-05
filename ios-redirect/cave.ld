ENTRY(_start)
SECTIONS {
  . = BASE;
  .all : { stub.o(.stub) *(.cavetext) *(.text*) *(.caved) *(.rodata*) *(.data*) }
  /DISCARD/ : { *(.ARM.exidx*) *(.comment) *(.note*) }
}
