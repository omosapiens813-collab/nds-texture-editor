#---------------------------------------------------------------------------------
# NDS Texture Editor - Makefile per devkitARM
# Compatibile con GitHub Actions (devkitpro/devkitarm container)
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
    $(error "Imposta la variabile DEVKITPRO: export DEVKITPRO=/opt/devkitpro")
endif

ifeq ($(strip $(DEVKITARM)),)
    export DEVKITARM := $(DEVKITPRO)/devkitARM
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
TARGET      := nds_texture_editor
BUILD       := build
SOURCES     := source
INCLUDES    := include
DATA        := data
GRAPHICS    := gfx

APP_TITLE   := NDS Texture Editor
APP_AUTHOR  := Homebrew
APP_DESCRIPTION := Texture editor per ROM NDS

#---------------------------------------------------------------------------------
ARCH        := -mthumb -mthumb-interwork

CFLAGS      := -g -Wall \
               -O2 \
               -march=armv5te \
               -mtune=arm946e-s \
               $(ARCH) \
               -fomit-frame-pointer \
               -ffast-math \
               -ffunction-sections \
               -fdata-sections

CFLAGS      += $(INCLUDE) -DARM9

CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS     := -g $(ARCH)

LDFLAGS     := -specs=ds_arm9.specs \
               -g $(ARCH) \
               -Wl,-Map,$(notdir $*.map) \
               -Wl,--gc-sections

LIBS        := -lfat -lnds9

LIBDIRS     := $(LIBNDS) \
               $(DEVKITPRO)/libnds \
               $(DEVKITPRO)/portlibs/nds

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)

export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(CURDIR)/$(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(CURDIR)/$(dir)/*.cpp)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(CURDIR)/$(dir)/*.s)))
BINFILES    := $(foreach dir,$(DATA),$(notdir $(wildcard $(CURDIR)/$(dir)/*.*)))

ifeq ($(strip $(CPPFILES)),)
    export LD := $(CC)
else
    export LD := $(CXX)
endif

export OFILES_BIN   := $(addsuffix .o,$(BINFILES))
export OFILES_SRC   := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES       := $(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN   := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@echo ""
	@echo "============================================"
	@echo "  BUILD COMPLETATA: $(TARGET).nds"
	@echo "  Copia il file .nds sulla tua scheda R4!"
	@echo "============================================"

clean:
	@echo "Pulizia..."
	@rm -rf $(BUILD) $(TARGET).nds $(TARGET).elf $(TARGET).ds.gba *.map
	@echo "Fatto."

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).nds   : $(OUTPUT).elf
$(OUTPUT).elf   : $(OFILES)

# File .c -> .o
%.o : %.c
	@echo "CC  $(notdir $<)"
	@$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

-include $(DEPENDS)

endif
#---------------------------------------------------------------------------------
