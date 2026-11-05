# OpenPak Wii Patcher (Homebrew Channel app). Build in devkitpro/devkitppc: make -> boot.dol
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment")
endif
include $(DEVKITPPC)/wii_rules

TARGET   := boot
BUILD    := build
SOURCES  := source
APP_VERSION ?= 0.0.0

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
export OFILES := $(CPPFILES:.cpp=.o)
export LD := $(CXX)
export INCLUDE := $(foreach dir,$(SOURCES),-I$(CURDIR)/$(dir)) -I$(LIBOGC_INC)
export LIBPATHS := -L$(LIBOGC_LIB)
export APP_VERSION
.PHONY: $(BUILD) clean
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
clean:
	rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).dol
else
MACHDEP += -DGEKKO
CFLAGS   = -g -Os -Wall $(MACHDEP) $(INCLUDE) -DAPP_VERSION=\"$(APP_VERSION)\"
CXXFLAGS = $(CFLAGS) -fno-rtti -std=gnu++17
LDFLAGS  = -g $(MACHDEP) -Wl,-Map,$(notdir $@).map
LIBS     := -lfat -lwiiuse -lbte -logc -lm
DEPENDS  := $(OFILES:.o=.d)
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(DEPENDS)
endif
