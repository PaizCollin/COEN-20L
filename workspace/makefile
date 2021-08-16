PREFIX	=	

# -------- Programs ---------------------------------------------------
ifeq ($(OS),Windows_NT)
CC	=	arm-none-eabi-gcc
AS	=	arm-none-eabi-as
OBJCOPY	=	arm-none-eabi-objcopy
else ifeq ($(PREFIX),)
$(error **** PREFIX has not been defined in makefile! ****)
else
CC	=	$(PREFIX)arm-none-eabi-gcc
AS	=	$(PREFIX)arm-none-eabi-as
OBJCOPY	=	$(PREFIX)arm-none-eabi-objcopy
endif
GREP	=	grep
WHERE	=	where
DEL	=	$(ComSpec) /C del
RM	=	rm

# -------- Command line options ----------------------------------------
AFLAGS	=	-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS	=	-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -std=c99 -O3 -Wall -ffunction-sections -fdata-sections -fno-strict-aliasing
LFLAGS	=	-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 $(LIB) -lm -lc -lnosys -lc -u _printf_float -Wl,-gc-sections -T linker.ld -Wl,-Map,$(MAP)

# -------- Files -------------------------------------------------------
CFILES	=	$(wildcard src/*.c)
SFILES	=	$(wildcard src/*.s)
DFILES	=	$(wildcard obj/*.o)
OFILES	=	$(patsubst src/%.c,obj/%-c.o,$(CFILES)) $(patsubst src/%.s,obj/%-s.o,$(SFILES))
OUTPUT	=	$(wildcard output.*)
LIB	=	library.a
ELF	=	output.elf
BIN	=	output.bin
MAP	=	output.map

all:		$(BIN)

$(BIN):	$(OFILES) $(LIB)
		$(info Linking:    $(OFILES) $(LIB))
		@$(CC) -o $(ELF) $(OFILES) $(LIB) $(LFLAGS)
		@$(OBJCOPY) -O binary $(ELF) $(BIN)
ifeq ($(OS),Windows_NT)
		@$(DEL) /Q $(ELF)
else
		@$(RM) $(ELF)
endif
		$(info Executable: $(BIN))

obj/%-c.o:	src/%.c
		$(info Compiling:  $<)
		@$(CC) $(CFLAGS) -Iinc -c -o $@ $<

obj/%-s.o:	src/%.s
		$(info Assembling: $<)
		@$(AS) $(AFLAGS) -o $@ $<

clean:
ifeq ($(DFILES)$(OUTPUT),)
		$(info **** There are no .o or output.* files to delete ****)
else ifeq ($(OS),Windows_NT)
		@$(DEL) /Q $(subst /,\,$(DFILES) $(OUTPUT))
		$(info Deleted:    $(subst /,\,$(DFILES) $(OUTPUT)))
else
		@$(RM) $(DFILES) $(OUTPUT)
		$(info Deleted:    $(DFILES) $(OUTPUT))
endif

cleanall:	clean
ifeq ($(CFILES)$(SFILES),)
		$(info **** There are no .c or .s files to delete ****)
else ifeq ($(OS),Windows_NT)
		@$(DEL) /P $(subst /,\,$(CFILES) $(SFILES))
else
		@$(RM) -i $(CFILES) $(SFILES)
endif

check:
ifeq ($(wildcard *.map),)
		$(info **** There is no $(MAP) file to check ****)
else ifeq ($(OS),Windows_NT)
		@$(WHERE) /Q $(GREP) || echo '**** GNU grep for Windows has not been installed! ***'
		@$(GREP) -H -T -F "__attribute__((weak))" src/*.c 2>NUL || echo '**** There are no weak functions in the *.c files! ****'
		@$(GREP) -A 12 -G "[.]text[^\r\n]*obj/.*-s[.]o$$" $(MAP) 2>NUL || echo '**** Assembly language replacements not found! ***'
else
		@$(GREP) -H -F "__attribute__((weak))" src/*.c 2>/dev/null || echo '**** There are no weak functions in the *.c files! ****'
		@$(GREP) -A 12 -G "[.]text[^\r\n]*obj/.*-s[.]o$$" $(MAP) 2>/dev/null || echo '**** Assembly language replacements not found! ***'
endif
