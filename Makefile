# Makefile
TOPDIR ?= $(CURDIR)

.DEFAULT_GOAL := all

include $(TOPDIR)/Rules/Phase1_Setup.mk

TARGET		:=	RmlUI_Example
BUILD		:=	Build
SOURCES		:=	Code/Source
DATA		:=	Data
INCLUDES	:=	Code/Include Shader/Build
SHADERS		:=	rmlui

include $(TOPDIR)/Rules/Phase2_Config.mk
include $(TOPDIR)/Rules/Phase3_Shaders.mk
include $(TOPDIR)/Rules/Phase4_Main.mk