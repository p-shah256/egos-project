# [lab3-ex2]: TODO: change "NAIVE" to "MLFQ"
SCHEDULER=NAIVE
# [lab4]: TODO: change "SOFTTIMER" to "ECALL"
SYSCALLFUNC=SOFTTIMER
# [lab5]: TODO: change  "VMOFF" to "VMON"
IFVM=VMOFF
# [lab6]: TODO: change  "FLASH" to "SDCARD"
DISK=FLASH
# [lab7]: TODO: change  "RWFSOFF" to "RWFSON"
RWFS=RWFSOFF

CPU_TYPE=SIFIVE_U

RISCV_QEMU = qemu-system-riscv32
RISCV_CC = riscv64-unknown-elf-gcc
OBJDUMP = riscv64-unknown-elf-objdump
OBJCOPY = riscv64-unknown-elf-objcopy

LIB_HEADERS = Makefile library/*.h library/*/*.h
EARTH_SRCS = earth/earth.S earth/*.c library/elf/*.c library/libc/*.c
EARTH_HEADERS = earth/earth.lds $(LIB_HEADERS)
GRASS_SRCS = grass/grass.S grass/context.S grass/*.c library/elf/*.c library/libc/*.c
GRASS_HEADERS = grass/grass.lds grass/*.h $(LIB_HEADERS)
APPS_SRCS = apps/app.S library/*/*.c grass/context.S
APP_HEADERS = apps/app.lds apps/*.h $(LIB_HEADERS)
USRAPP_HEADERS = $(wildcard apps/user/*.h)


# CFLAGS = -march=rv32i -mabi=ilp32 -mcmodel=medlow -ffunction-sections -fdata-sections
CFLAGS = -march=rv32i -mabi=ilp32 -mcmodel=medlow -ffunction-sections -fdata-sections -fno-common -ggdb -g
LDFLAGS = -Wl,--gc-sections -nostartfiles -nostdlib
INCLUDE = -Ilibrary -Ilibrary/elf -Ilibrary/libc -Ilibrary/file -Ilibrary/servers
# QEMU_FLAGS = -bios none -readconfig $(QEMU)/sifive-e31.cfg -kernel $(QEMU)/qemu.elf -nographic
QEMU_FLAGS = -bios none -readconfig $(QEMU)/sifive-u540.cfg -nographic
VERBOSE_LINKER = -Xlinker --verbose

COMMON = $(CFLAGS) $(LDFLAGS) $(INCLUDE) -D$(CPU_TYPE) -D CPU_CLOCK_RATE=65000000 -D$(SCHEDULER) -D$(SYSCALLFUNC) -D$(IFVM) -D$(DISK) -D$(RWFS)

APPS_LD = -Tapps/app.lds -lc -lgcc
GRASS_LD = -Tgrass/grass.lds -lc -lgcc
EARTH_LD = -Tearth/earth.lds -lc -lgcc

TOOLS = tools
QEMU = tools/qemu
DEBUG = build/debug
RELEASE = build/release
SYSAPP = apps/system
USRAPP = apps/user
SYSAPPELFS = $(patsubst $(SYSAPP)/%.c,$(RELEASE)/%.elf,$(wildcard $(SYSAPP)/*.c))
USRAPPELFS = $(patsubst $(USRAPP)/%.c,$(RELEASE)/%.elf,$(wildcard $(USRAPP)/*.c))
OBJDUMP_FLAGS =  --source --all-headers --demangle --line-numbers --wide

QEMUGDB = -gdb tcp::6640

GREEN = \033[1;32m
YELLOW = \033[1;33m
CYAN = \033[1;36m
END = \033[0m


all:  earth grass apps install

earth: $(RELEASE)/earth.elf

grass: $(RELEASE)/grass.elf

apps: appprint $(SYSAPPELFS) $(USRAPPELFS)

.PHONY: apps earth grass

$(RELEASE)/earth.elf: $(EARTH_SRCS) $(EARTH_HEADERS)
	@mkdir -p $(DEBUG) $(RELEASE)
	@echo "$(YELLOW)-------- Compile the Earth Layer --------$(END)"
	$(RISCV_CC) $(COMMON) $(EARTH_SRCS) $(EARTH_LD) -o $(RELEASE)/earth.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/earth.elf > $(DEBUG)/earth.lst

$(RELEASE)/grass.elf: $(GRASS_SRCS) $(GRASS_HEADERS)
	@mkdir -p $(DEBUG) $(RELEASE)
	@echo "$(GREEN)-------- Compile the Grass Layer --------$(END)"
	$(RISCV_CC) $(COMMON) $(GRASS_SRCS) $(GRASS_LD) -o $(RELEASE)/grass.elf
	$(OBJDUMP) $(OBJDUMP_FLAGS) $(RELEASE)/grass.elf > $(DEBUG)/grass.lst

appprint:
	@echo "$(CYAN)-------- Compile the Apps Layer --------$(END)"

$(SYSAPPELFS): $(RELEASE)/%.elf : $(SYSAPP)/%.c $(APPS_SRCS) $(APP_HEADERS)
	@mkdir -p $(DEBUG) $(RELEASE)
	@echo "Compile" $< "=>" $@
	@$(RISCV_CC) $(COMMON) $(APPS_SRCS) $< $(APPS_LD) -Iapps -o $@ || exit 1
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $(patsubst $(RELEASE)/%.elf,$(DEBUG)/%.lst,$@) || exit 1

$(USRAPPELFS): $(RELEASE)/%.elf : $(USRAPP)/%.c $(APPS_SRCS) $(APP_HEADERS) $(USRAPP_HEADERS)
	@mkdir -p $(DEBUG) $(RELEASE)
	@echo "Compile" $< "=>" $@
	@$(RISCV_CC) $(COMMON) $(APPS_SRCS) $< $(APPS_LD) -Iapps -o $@ || exit 1
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $(patsubst $(RELEASE)/%.elf,$(DEBUG)/%.lst,$@) || exit 1


install:
	@echo "$(YELLOW)-------- Create the Disk Image --------$(END)"
	$(CC) $(TOOLS)/mkfs.c library/file/file.c -DMKFS $(INCLUDE) -o $(TOOLS)/mkfs
	cd $(TOOLS); ./mkfs

qemu:
	@echo "$(YELLOW)-------- Simulate on QEMU-RISCV --------$(END)"
	cp $(RELEASE)/earth.elf $(QEMU)/egos.bin
	$(OBJCOPY) --update-section .image=$(TOOLS)/disk.img $(QEMU)/egos.bin
	$(RISCV_QEMU) $(QEMU_FLAGS)

qemu-gdb:
	@echo "$(YELLOW)-------- Simulate on QEMU-RISCV (with GDB) --------$(END)"
	@echo "$(YELLOW)-------- run 'gdb' in another window --------$(END)"
	cp $(RELEASE)/earth.elf $(QEMU)/egos.bin
	$(OBJCOPY) --update-section .image=$(TOOLS)/disk.img $(QEMU)/egos.bin
	$(RISCV_QEMU) $(QEMU_FLAGS) -S $(QEMUGDB)

clean:
	rm -rf build
	rm -rf $(TOOLS)/qemu/qemu.elf
	rm -rf $(TOOLS)/mkfs
	rm -rf $(TOOLS)/disk.img

