# Makefile
.SUFFIXES:
.SECONDARY:
.PHONY: all clean
#-------------------------------------------------------------------------------
# ToolChains
#-------------------------------------------------------------------------------
CCompiler := powerpc-eabi-gcc
CppCompiler := powerpc-eabi-g++
Linker := powerpc-eabi-g++
NameList := powerpc-eabi-nm

#-------------------------------------------------------------------------------
# Environment
#-------------------------------------------------------------------------------
DevKitPro := $(DEVKITPRO)
PortLibs := $(DevKitPro)/portlibs/ppc
Wups := $(DevKitPro)/wups
Wut := $(DevKitPro)/wut
Wums := $(DevKitPro)/wums

MachineDependent := -DESPRESSO -mcpu=750 -meabi -mhard-float

#-------------------------------------------------------------------------------
# Directories
#-------------------------------------------------------------------------------
TopDir := $(CURDIR)

Target := RmlUI_Example

SourceDir := $(TopDir)/Source
IncludeDir := $(TopDir)/Include
BuildDir := $(TopDir)/Build
DistDir := $(TopDir)/Dist

ShaderDir := $(TopDir)/Shader
UIDir := $(TopDir)/UI
FontDir := $(TopDir)/Font

ToolDir := $(TopDir)/Tool

#-------------------------------------------------------------------------------
# Macros
#-------------------------------------------------------------------------------
include $(TopDir)/Tools.mk

#-------------------------------------------------------------------------------
# Files
#-------------------------------------------------------------------------------
RmlDir := $(UIDir)/Document
RcssDir := $(UIDir)/Style

ShaderEntry := Default
GlslCompiler := $(ToolDir)/GLSLCompiler

BuildTempDir := $(BuildDir)/Temp
BuildIncludeDir := $(BuildDir)/Include
BuildObjectDir := $(BuildDir)/Object
BuildDependenceDir := $(BuildDir)/Dependence

BuildGshFile := $(foreach entry,$(ShaderEntry),$(BuildTempDir)/$(entry).gsh)
RmlFile := $(shell find $(RmlDir) -type f -name '*.rml')
RcssFile := $(shell find $(RcssDir) -type f -name '*.rcss')
FontFile := $(shell find $(FontDir) -type f -name '*.ttf')

BuildGshBase := $(foreach entry,$(ShaderEntry),$(basename $(notdir $(entry))))
RmlBase := $(foreach entry,$(RmlFile),$(basename $(notdir $(entry))))
RcssBase := $(foreach entry,$(RcssFile),$(basename $(notdir $(entry))))
FontBase := $(foreach entry,$(FontFile),$(basename $(notdir $(entry))))

BuildIncludeShaderFile := $(foreach entry,$(BuildGshBase),$(BuildIncludeDir)/Shader/$(entry).h)
BuildObjectShaderFile := $(foreach entry,$(BuildGshBase),$(BuildObjectDir)/Shader/$(entry).o)

BuildIncludeRmlFile := $(foreach entry,$(RmlBase),$(BuildIncludeDir)/UI/Document/$(entry).h)
BuildObjectRmlFile := $(foreach entry,$(RmlBase),$(BuildObjectDir)/UI/Document/$(entry).o)

BuildIncludeRcssFile := $(foreach entry,$(RcssBase),$(BuildIncludeDir)/UI/Style/$(entry).h)
BuildObjectRcssFile := $(foreach entry,$(RcssBase),$(BuildObjectDir)/UI/Style/$(entry).o)

BuildIncludeFontFile := $(foreach entry,$(FontBase),$(BuildIncludeDir)/Font/$(entry).h)
BuildObjectFontFile := $(foreach entry,$(FontBase),$(BuildObjectDir)/Font/$(entry).o)

BuildIncludeBinaryFile := $(BuildIncludeShaderFile) $(BuildIncludeRmlFile) $(BuildIncludeRcssFile) $(BuildIncludeFontFile)
BuildObjectBinaryFile := $(BuildObjectShaderFile) $(BuildObjectRmlFile) $(BuildObjectRcssFile) $(BuildObjectFontFile)

CppFile := $(shell find $(SourceDir) -type f -name '*.cpp')
CppRelative := $(shell realpath --relative-to=$(SourceDir) $(CppFile))
BuildObjectCppFile := $(patsubst %.cpp,$(BuildObjectDir)/Cpp/%.o,$(CppRelative))

BuildElfFile := $(BuildDir)/$(Target).elf
DistWpsFile := $(DistDir)/$(Target).wps

SendScript := $(TopDir)/SendPlugin.sh
Logger := udplogserver

#-------------------------------------------------------------------------------
# Libraries
#-------------------------------------------------------------------------------
LibraryEntries := wups wut notifications mappedmemory kernel rmlui freetype
LibraryDirs := $(PortLibs)/lib $(Wups)/lib $(Wut)/lib $(Wums)/lib
LibraryIncludeDirs := $(PortLibs)/include $(PortLibs)/include/freetype2 $(Wups)/include $(Wut)/include $(Wums)/include
LibraryDirFlags := $(foreach dir,$(LibraryDirs),-L$(dir))
LibraryFlags := $(foreach entry,$(LibraryEntries),-l$(entry))

#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------
IncludeDirs := $(IncludeDir) $(BuildIncludeDir) $(LibraryIncludeDirs)
IncludeFlags := $(foreach dir,$(IncludeDirs),-I$(dir))

#-------------------------------------------------------------------------------
# Cpp Flags
#-------------------------------------------------------------------------------
CppFlags := $(MachineDependent) $(IncludeFlags) $(LibraryFlags) -Wall -O3 -ffunction-sections -std=c++23

#-------------------------------------------------------------------------------
# Linker Flags
#-------------------------------------------------------------------------------
LinkerScript := -T$(Wums)/share/libmappedmemory.ld -T$(Wums)/share/libkernel.ld -T$(Wups)/share/wups.ld
Specs := -specs=$(Wut)/share/wut.specs -specs=$(Wups)/share/wups.specs
LinkerFlags := $(LinkerScript) $(Specs) -g

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------
all: $(DistWpsFile)
	@rm -rf $(BuildTempDir)

$(BuildObjectCppFile): $(BuildIncludeFile)

$(BuildTempDir)/%.gsh: $(ShaderDir)/%.vert $(ShaderDir)/%.frag
	@echo $(notdir $^)
	$(call glsl2gsh,$(GlslCompiler),$(ShaderDir)/$*.vert,$(ShaderDir)/$*.frag,$@)

$(BuildIncludeDir)/Shader/%.h $(BuildObjectDir)/Shader/%.o: $(BuildTempDir)/%.gsh
	@echo $(notdir $<)
	$(call bin2o,$<,$(BuildIncludeDir)/Shader/$*.h,$(BuildObjectDir)/Shader/$*.o,$(BuildTempDir)/$*.s)

$(BuildIncludeDir)/UI/Document/%.h $(BuildObjectDir)/UI/Document/%.o: $(RmlDir)/%.rml
	@echo $(notdir $<)
	$(call bin2o,$<,$(BuildIncludeDir)/UI/Document/$*.h,$(BuildObjectDir)/UI/Document/$*.o,$(BuildTempDir)/$*.s)

$(BuildIncludeDir)/UI/Style/%.h $(BuildObjectDir)/UI/Style/%.o: $(RcssDir)/%.rcss
	@echo $(notdir $<)
	$(call bin2o,$<,$(BuildIncludeDir)/UI/Style/$*.h,$(BuildObjectDir)/UI/Style/$*.o,$(BuildTempDir)/$*.s)

$(BuildIncludeDir)/Font/%.h $(BuildObjectDir)/Font/%.o: $(FontDir)/%.ttf
	@echo $(notdir $<)
	$(call bin2o,$<,$(BuildIncludeDir)/Font/$*.h,$(BuildObjectDir)/Font/$*.o,$(BuildTempDir)/$*.s)

$(BuildObjectDir)/Cpp/%.o: $(SourceDir)/%.cpp
	@echo $(notdir $<)
	$(call cpp2o,$<,$@,$(BuildDependenceDir)/$*.d,$(CppFlags))

$(BuildDir)/%.elf: $(BuildObjectCppFile) $(BuildObjectBinaryFile)
	@echo linking ... $(notdir $@)
	@$(call o2elf,$^,$@,$(LinkerFlags),$(LibraryDirFlags),$(LibraryFlags),$(BuildDir)/$*.map)
	@$(call elf2lst,$@,$(BuildDir)/$*.lst)

$(DistDir)/%.wps: $(BuildDir)/%.elf
	@echo building ... $(notdir $@)
	@$(call elf2wps,$<,$@)

-include $(BuildDependenceDir)/*.d

clean:
	@echo clean ...
	@rm -rf $(BuildDir) $(DistDir)

send: $(DistWpsFile)
	@echo sending ... $(notdir $<)
	@$(SendScript) $<
	@$(Logger)