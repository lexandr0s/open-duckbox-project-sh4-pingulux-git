stmalloc-objs :=        allocator.o

ifeq ($(CONFIG_CPU_SUBTYPE_STX7200),y)
CFLAGS += -DLMI_BASE_ADDRESS=0x19000000 -DLMI_BLOCK_SIZE=0x03000000
else
ifeq ($(CONFIG_CPU_SUBTYPE_STB7100),y)
CFLAGS += -DLMI_BASE_ADDRESS=0x10800000 -DLMI_BLOCK_SIZE=0x03800000
else
endif
endif

# workaround for stm24
ifneq (,$(findstring 2.6.3,$(KERNELVERSION)))
ccflags-y += $(CCFLAGSY)
endif

obj-m += stmalloc.o

