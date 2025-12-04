# Phase3_Shader.mk

# ----- Paths -----
SHADER_SRC_DIR := $(TOPDIR)/Shader
SHADER_BUILD_DIR := $(TOPDIR)/GSH
GLSL_COMPILER_PATH := $(TOPDIR)/Tool/GLSLCompiler

WSL_DISTRO := Ubuntu-24.04

# Convert paths to WSL format
WSL_TOP_DIR := $(shell wsl wslpath "$(TOPDIR)")
WSL_SHADER_SRC_DIR := $(shell wsl -d $(WSL_DISTRO) wslpath "$(SHADER_SRC_DIR)")
WSL_SHADER_BUILD_DIR := $(shell wsl -d $(WSL_DISTRO) wslpath "$(SHADER_BUILD_DIR)")
WSL_GLSL_COMPILER_PATH := $(shell wsl -d $(WSL_DISTRO) wslpath -a "$(GLSL_COMPILER_PATH)")

GLSL_COMPILER := MSYS2_ARG_CONV_EXCL=* wsl -d $(WSL_DISTRO) --cd "$(WSL_TOP_DIR)" -- "$(WSL_GLSL_COMPILER_PATH)"

$(shell mkdir -p $(SHADER_BUILD_DIR))

SHADER_GSH := $(foreach s,$(SHADERS),$(SHADER_BUILD_DIR)/$(s).gsh)
SHADER_H := $(foreach s,$(SHADERS),$(SHADER_BUILD_DIR)/$(s)_gsh.h)

.PHONY: shader clean-shader

# ----- compile each shader -----

$(SHADER_BUILD_DIR)/%.gsh: $(SHADER_SRC_DIR)/%.vert $(SHADER_SRC_DIR)/%.frag
	@echo $(notdir $<)
	@$(GLSL_COMPILER) \
		-vs $(WSL_SHADER_SRC_DIR)/$*.vert \
		-ps $(WSL_SHADER_SRC_DIR)/$*.frag \
		-o $(WSL_SHADER_BUILD_DIR)/$*.gsh

$(BUILD)/%_gsh.o $(BUILD)/%_gsh.h: $(SHADER_BUILD_DIR)/%.gsh
	@echo $(notdir $<)
	@$(bin2o)

shader: $(SHADER_GSH)
	@echo "All shaders built"

clean-shader:
	rm -f $(SHADER_BUILD_DIR)/*
