#pragma once

#include <stdint.h>
#include <windows.h>

#define V8_M64P_FRONTEND_API 0x020106
#define V8_M64P_INPUT_API 0x020101
#define V8_M64P_PLUGIN_VERSION 0x00010000

enum m64p_error {
    M64ERR_SUCCESS = 0,
    M64ERR_NOT_INIT,
    M64ERR_ALREADY_INIT,
    M64ERR_INCOMPATIBLE,
    M64ERR_INPUT_ASSERT,
    M64ERR_INPUT_INVALID,
    M64ERR_INPUT_NOT_FOUND,
    M64ERR_NO_MEMORY,
    M64ERR_FILES,
    M64ERR_INTERNAL,
    M64ERR_INVALID_STATE,
    M64ERR_PLUGIN_FAIL,
    M64ERR_SYSTEM_FAIL,
    M64ERR_UNSUPPORTED,
    M64ERR_WRONG_TYPE
};

enum m64p_plugin_type {
    M64PLUGIN_NULL = 0,
    M64PLUGIN_RSP = 1,
    M64PLUGIN_GFX,
    M64PLUGIN_AUDIO,
    M64PLUGIN_INPUT,
    M64PLUGIN_CORE
};

enum m64p_core_param {
    M64CORE_EMU_STATE = 1,
    M64CORE_VIDEO_MODE,
    M64CORE_SAVESTATE_SLOT,
    M64CORE_SPEED_FACTOR,
    M64CORE_SPEED_LIMITER,
    M64CORE_VIDEO_SIZE,
    M64CORE_AUDIO_VOLUME,
    M64CORE_AUDIO_MUTE,
    M64CORE_INPUT_GAMESHARK,
    M64CORE_STATE_LOADCOMPLETE,
    M64CORE_STATE_SAVECOMPLETE,
    M64CORE_SCREENSHOT_CAPTURED
};

enum m64p_command {
    M64CMD_NOP = 0,
    M64CMD_ROM_OPEN,
    M64CMD_ROM_CLOSE,
    M64CMD_ROM_GET_HEADER,
    M64CMD_ROM_GET_SETTINGS,
    M64CMD_EXECUTE,
    M64CMD_STOP,
    M64CMD_PAUSE,
    M64CMD_RESUME,
    M64CMD_CORE_STATE_QUERY,
    M64CMD_STATE_LOAD,
    M64CMD_STATE_SAVE,
    M64CMD_STATE_SET_SLOT,
    M64CMD_SEND_SDL_KEYDOWN,
    M64CMD_SEND_SDL_KEYUP,
    M64CMD_SET_FRAME_CALLBACK,
    M64CMD_TAKE_NEXT_SCREENSHOT,
    M64CMD_CORE_STATE_SET,
    M64CMD_READ_SCREEN,
    M64CMD_RESET,
    M64CMD_ADVANCE_FRAME
};

typedef HMODULE m64p_dynlib_handle;
typedef void (*m64p_frame_callback)(unsigned int);
typedef void (*m64p_debug_callback)(void *, int, const char *);
typedef void (*m64p_state_callback)(void *, m64p_core_param, int);

typedef struct {
    unsigned char *HEADER;
    unsigned char *RDRAM;
    unsigned char *DMEM;
    unsigned char *IMEM;
    unsigned int *MI_INTR_REG;
    unsigned int *DPC_START_REG;
    unsigned int *DPC_END_REG;
    unsigned int *DPC_CURRENT_REG;
    unsigned int *DPC_STATUS_REG;
    unsigned int *DPC_CLOCK_REG;
    unsigned int *DPC_BUFBUSY_REG;
    unsigned int *DPC_PIPEBUSY_REG;
    unsigned int *DPC_TMEM_REG;
    unsigned int *VI_STATUS_REG;
    unsigned int *VI_ORIGIN_REG;
    unsigned int *VI_WIDTH_REG;
    unsigned int *VI_INTR_REG;
    unsigned int *VI_V_CURRENT_LINE_REG;
    unsigned int *VI_TIMING_REG;
    unsigned int *VI_V_SYNC_REG;
    unsigned int *VI_H_SYNC_REG;
    unsigned int *VI_LEAP_REG;
    unsigned int *VI_H_START_REG;
    unsigned int *VI_V_START_REG;
    unsigned int *VI_V_BURST_REG;
    unsigned int *VI_X_SCALE_REG;
    unsigned int *VI_Y_SCALE_REG;
    void (*CheckInterrupts)(void);
    unsigned int version;
    unsigned int *SP_STATUS_REG;
    const unsigned int *RDRAM_SIZE;
} GFX_INFO;

typedef struct {
    int Present;
    int RawData;
    int Plugin;
    int Type;
} CONTROL;

typedef union {
    unsigned int Value;
    struct {
        unsigned R_DPAD : 1;
        unsigned L_DPAD : 1;
        unsigned D_DPAD : 1;
        unsigned U_DPAD : 1;
        unsigned START_BUTTON : 1;
        unsigned Z_TRIG : 1;
        unsigned B_BUTTON : 1;
        unsigned A_BUTTON : 1;
        unsigned R_CBUTTON : 1;
        unsigned L_CBUTTON : 1;
        unsigned D_CBUTTON : 1;
        unsigned U_CBUTTON : 1;
        unsigned R_TRIG : 1;
        unsigned L_TRIG : 1;
        unsigned Reserved1 : 1;
        unsigned Reserved2 : 1;
        signed X_AXIS : 8;
        signed Y_AXIS : 8;
    };
} BUTTONS;

typedef struct {
    CONTROL *Controls;
} CONTROL_INFO;

typedef struct {
    unsigned char *RDRAM;
    unsigned char *DMEM;
    unsigned char *IMEM;
    unsigned int *MI_INTR_REG;
    unsigned int *AI_DRAM_ADDR_REG;
    unsigned int *AI_LEN_REG;
    unsigned int *AI_CONTROL_REG;
    unsigned int *AI_STATUS_REG;
    unsigned int *AI_DACRATE_REG;
    unsigned int *AI_BITRATE_REG;
    void (*CheckInterrupts)(void);
} AUDIO_INFO;

typedef struct {
    unsigned char *RDRAM;
    unsigned char *DMEM;
    unsigned char *IMEM;
    unsigned int *MI_INTR_REG;
    unsigned int *SP_MEM_ADDR_REG;
    unsigned int *SP_DRAM_ADDR_REG;
    unsigned int *SP_RD_LEN_REG;
    unsigned int *SP_WR_LEN_REG;
    unsigned int *SP_STATUS_REG;
    unsigned int *SP_DMA_FULL_REG;
    unsigned int *SP_DMA_BUSY_REG;
    unsigned int *SP_PC_REG;
    unsigned int *SP_SEMAPHORE_REG;
    unsigned int *DPC_START_REG;
    unsigned int *DPC_END_REG;
    unsigned int *DPC_CURRENT_REG;
    unsigned int *DPC_STATUS_REG;
    unsigned int *DPC_CLOCK_REG;
    unsigned int *DPC_BUFBUSY_REG;
    unsigned int *DPC_PIPEBUSY_REG;
    unsigned int *DPC_TMEM_REG;
    void (*CheckInterrupts)(void);
    void (*ProcessDlistList)(void);
    void (*ProcessAlistList)(void);
    void (*ProcessRdpList)(void);
    void (*ShowCFB)(void);
} RSP_INFO;

typedef struct {
    uint32_t magic;
    volatile LONG frame;
    volatile LONG stopFrame;
} V8N64Shared;

#define V8_N64_SHARED_MAGIC 0x56384E36u
#define V8_N64_PLUGIN_NONE 1

typedef m64p_error (*ptr_CoreStartup)(
    int, const char *, const char *, void *, m64p_debug_callback,
    void *, m64p_state_callback);
typedef m64p_error (*ptr_CoreShutdown)(void);
typedef m64p_error (*ptr_CoreAttachPlugin)(m64p_plugin_type, HMODULE);
typedef m64p_error (*ptr_CoreDetachPlugin)(m64p_plugin_type);
typedef m64p_error (*ptr_CoreDoCommand)(m64p_command, int, void *);
typedef const char *(*ptr_CoreErrorMessage)(m64p_error);

typedef m64p_error (*ptr_PluginStartup)(
    HMODULE, void *, void (*)(void *, int, const char *));
typedef m64p_error (*ptr_PluginShutdown)(void);
typedef m64p_error (*ptr_PluginGetVersion)(
    m64p_plugin_type *, int *, int *, const char **, int *);
