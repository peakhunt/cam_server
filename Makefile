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

MMAL_SOURCES =                                      \
src/mmal/brcmjpeg.c

APP_SOURCES =                                       \
src/main/main.c                                     \
src/main/webserver.c                                \
src/main/v4l2_camera.c                              \
src/main/camera_driver.c

ifeq ($(RPI),yes)
APP_SOURCES += src/main/frame_converter_rpi.c
APP_SOURCES += src/main/am2320.c
else
APP_SOURCES += src/main/frame_converter.c
endif

#######################################
C_DEFS  = 

#######################################
# include and lib setup
#######################################
C_INCLUDES =                              \
-Isrc/utils                               \
-Isrc/main                                \
-Isrc/mongoose

ifeq ($(RPI),yes)
C_INCLUDES += -Isrc/mmal -I/opt/vc/include -I/opt/vc/include/interface/mmal
endif

ifeq ($(RPI),yes)
LIBS = -ldl -lpthread -lwiringPi -lgd -ljpeg -lfreetype -lm -L/opt/vc/lib/ -lmmal_core -lmmal_util -lmmal_vc_client -lvcsm -lvcos
else
LIBS = -ldl -lpthread -lgd -ljpeg -lfreetype
endif
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
ifeq ($(RPI),yes)
# CFLAGS += -g $(C_DEFS) $(C_INCLUDES) -DRPI -O3
CFLAGS += -g $(C_DEFS) $(C_INCLUDES) -DRPI -O3 -DUSE_FRAME_CONVERTER
else
CFLAGS += -g $(C_DEFS) $(C_INCLUDES) -O3
endif

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

ifeq ($(RPI),yes)
TARGET_SOURCES += $(MMAL_SOURCES) src/brcmjpeg_encode.c
endif

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
