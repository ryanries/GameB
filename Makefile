BUILD_DIR := ./build
CFLAGS := -pthread -Wall -Wno-unknown-pragmas
CPPFLAGS := -MMD -MP
LDLIBS := -lm -lX11 -lGL -lpulse
LDFLAGS := -pthread
release: CFLAGS += -O3
debug: CFLAGS += -g
debug: CPPFLAGS += -D _DEBUG

SRCS := LinuxMain.c CommonMain.c Platform.c OpeningSplashScreen.c TitleScreen.c CharacterNamingScreen.c Overworld.c ExitYesNoScreen.c Battle.c OptionsScreen.c NewGameAreYouSure.c GamepadUnplugged.c miniz.c stb_vorbis.c

OBJS := $(SRCS:.c=.o)
OBJS := $(OBJS:%=$(BUILD_DIR)/%)
DEPS := $(OBJS:.o=.d)

release: $(BUILD_DIR)/GameB
debug: $(BUILD_DIR)/GameB
MyMiniz: $(BUILD_DIR)/LinuxMyMiniz.o $(BUILD_DIR)/miniz.o
	$(CC) $^ -o $@

$(BUILD_DIR)/GameB: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
