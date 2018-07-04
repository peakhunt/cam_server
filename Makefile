include Rules.mk

#######################################
# list of source files
########################################
LIB_UTILS_SOURCES =                                 \
src/utils/circ_buffer.c                             \
src/utils/cli.c                                     \
src/utils/cli_telnet.c                              \
src/utils/io_driver.c                               \
src/utils/sock_util.c                               \
src/utils/soft_timer.c                              \
src/utils/stream.c                                  \
src/utils/tcp_server.c                              \
src/utils/tcp_server_ipv4.c                         \
src/utils/telnet_reader.c

MONGOOSE_SOURCES =                                  \
src/mongoose/mongoose.c                             \
src/mongoose/mongoose_util.c                        \
src/mongoose/frozen.c

APP_SOURCES =                                       \
src/main/main.c                                     \
src/main/webserver.c                                \
src/main/l4v2_camera.c                              \
src/main/camera_driver.c

#######################################
C_DEFS  = 

#######################################
# include and lib setup
#######################################
C_INCLUDES =                              \
-Isrc/utils                               \
-Isrc/main                                \
-Isrc/mongoose

LIBS = -ldl -lpthread -lm
LIBDIR = 

#######################################
# for verbose output
#######################################
# Prettify output
V = 0
ifeq ($V, 0)
  Q = @
  P = > /dev/null
else
  Q =
  P =
endif

#######################################
# build directory and target setup
#######################################
BUILD_DIR = build
TARGET    = cam_server

#######################################
# compile & link flags
#######################################
CFLAGS += -g $(C_DEFS) $(C_INCLUDES)

# Generate dependency information
CFLAGS += -MMD -MF .dep/$(*F).d

LDFLAGS +=  $(LIBDIR) $(LIBS)

#######################################
# build target
#######################################
all: $(BUILD_DIR)/$(TARGET)

#######################################
# target source setup
#######################################
TARGET_SOURCES := $(LIB_UTILS_SOURCES) $(MONGOOSE_SOURCES) $(APP_SOURCES)
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(TARGET_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(TARGET_SOURCES)))

#######################################
# C source build rule
#######################################
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	@echo "[CC]         $(notdir $<)"
	$Q$(CC) -c $(CFLAGS) $< -o $@

#######################################
# main target
#######################################
$(BUILD_DIR)/$(TARGET): $(OBJECTS) Makefile
	@echo "[LD]         $@"
	$Q$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET)_rpi: $(OBJECTS) Makefile
	@echo "[LD]         $@"
	$Q$(CC) $(OBJECTS) $(LDFLAGS) -lwiringPi -o $@

$(BUILD_DIR):
	@echo "MKDIR          $(BUILD_DIR)"
	$Qmkdir $@

#######################################
# clean up
#######################################
clean:
	@echo "[CLEAN]          $(TARGET) $(BUILD_DIR) .dep"
	$Q-rm -fR .dep $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
