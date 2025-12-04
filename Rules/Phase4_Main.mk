# Phase5_Main.mk

ifneq ($(BUILD),$(notdir $(CURDIR)))

include $(TOPDIR)/Rules/Phase5_Prepare.mk

else

include $(TOPDIR)/Rules/Phase6_Build.mk

endif
