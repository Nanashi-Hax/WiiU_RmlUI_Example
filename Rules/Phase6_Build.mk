# Phase7_Build.mk

.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all: $(OUTPUT).wps

$(OUTPUT).wps: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

$(OFILES_SRC): $(HFILES_BIN)

$(BUILD)/%_bin.o $(BUILD)/%_bin.h: %.bin
	@echo $(notdir $<)
	@$(bin2o)

$(BUILD)/%_rml.o $(BUILD)/%_rml.h: %.rml
	@echo $(notdir $<)
	@$(bin2o)

$(BUILD)/%_rcss.o $(BUILD)/%_rcss.h: %.rcss
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)
