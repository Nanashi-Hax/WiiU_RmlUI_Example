# Phase4_Main.mk

ifneq ($(BUILD),$(notdir $(CURDIR)))

include $(TOPDIR)/Rules/Phase5-0_Prepare.mk

else

include $(TOPDIR)/Rules/Phase5-1_Build.mk

endif
