#include "Platform.h"

#include <stdarg.h>

bool gMusicIsPaused = false;

#ifdef _WIN32

// A global handle to the game window.
HWND gGameWindow = NULL;

// This critical section is used to synchronize LogMessageA between multiple threads.
CRITICAL_SECTION gLogCritSec;

// Any values that are read from and stored in Windows registry go in this struct.
REGISTRYPARAMS gRegistryParams;

// The background thread that loads assets from the compressed archive during the splash screen.
HANDLE gAssetLoadingThreadHandle = NULL;

// This event gets signalled/set after the most essential assets have been loaded.
// "Essential" means the assets required to render the splash screen, which is
// the basic font and the weird noise that plays at the beginning of the splash screen.
HANDLE gEssentialAssetsLoadedEvent = NULL;

// COM interfaces for the XAudio2 library.
// Having 4 source voices for sound effects means we can play up to 
// 4 sound effects simultaneously.
IXAudio2SourceVoice* gXAudioSFXSourceVoice[NUMBER_OF_SFX_SOURCE_VOICES] = { 0 };

// We only need 1 source voice for music because we never play overlapping music tracks.
IXAudio2SourceVoice* gXAudioMusicSourceVoice = { 0 };

void PlayGameSound(GAMESOUND* GameSound)
{
    static int SFXSourceVoiceSelector = 0;

    gXAudioSFXSourceVoice[SFXSourceVoiceSelector]->lpVtbl->SubmitSourceBuffer(gXAudioSFXSourceVoice[SFXSourceVoiceSelector], &GameSound->Buffer, NULL);

    gXAudioSFXSourceVoice[SFXSourceVoiceSelector]->lpVtbl->Start(gXAudioSFXSourceVoice[SFXSourceVoiceSelector], 0, XAUDIO2_COMMIT_NOW);

    SFXSourceVoiceSelector++;

    if (SFXSourceVoiceSelector > (NUMBER_OF_SFX_SOURCE_VOICES - 1))
    {
        SFXSourceVoiceSelector = 0;
    }
}

void PlayGameMusic(GAMESOUND* GameSound, bool Loop, bool Immediate)
{
    if (gMusicIsPaused == false)
    {
        if (Immediate)
        {
            gXAudioMusicSourceVoice->lpVtbl->Stop(gXAudioMusicSourceVoice, 0, 0);

            gXAudioMusicSourceVoice->lpVtbl->FlushSourceBuffers(gXAudioMusicSourceVoice);
        }

        if (Loop)
        {
            GameSound->Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
        }
        else
        {
            GameSound->Buffer.LoopCount = 0;
        }

        gXAudioMusicSourceVoice->lpVtbl->SubmitSourceBuffer(gXAudioMusicSourceVoice, &GameSound->Buffer, NULL);

        gXAudioMusicSourceVoice->lpVtbl->Start(gXAudioMusicSourceVoice, 0, XAUDIO2_COMMIT_NOW);
    }
    else
    {
        gXAudioMusicSourceVoice->lpVtbl->Start(gXAudioMusicSourceVoice, 0, XAUDIO2_COMMIT_NOW);
    }

    gMusicIsPaused = false;
}

void StopAllGameSounds(void)
{
    for (int i = 0; i < NUMBER_OF_SFX_SOURCE_VOICES; i++)
    {
        gXAudioSFXSourceVoice[i]->lpVtbl->Stop(gXAudioSFXSourceVoice[i], 0, 0);

        gXAudioSFXSourceVoice[i]->lpVtbl->FlushSourceBuffers(gXAudioSFXSourceVoice[i]);
    }
}

void PauseMusic(void)
{
    gXAudioMusicSourceVoice->lpVtbl->Stop(gXAudioMusicSourceVoice, 0, 0);

    gMusicIsPaused = true;
}

void StopMusic(void)
{
    gXAudioMusicSourceVoice->lpVtbl->Stop(gXAudioMusicSourceVoice, 0, 0);

    gXAudioMusicSourceVoice->lpVtbl->FlushSourceBuffers(gXAudioMusicSourceVoice);

    gMusicIsPaused = false;
}

bool MusicIsPlaying(void)
{
    XAUDIO2_VOICE_STATE State = { 0 };

    gXAudioMusicSourceVoice->lpVtbl->GetState(gXAudioMusicSourceVoice, &State, 0);    
    
    if (State.BuffersQueued > 0 && (gMusicIsPaused == false))
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

bool WIN32_AssetThreadFailed(void)
{
    DWORD ThreadExitCode = ERROR_SUCCESS;
    GetExitCodeThread(gAssetLoadingThreadHandle, &ThreadExitCode);
    return ThreadExitCode != ERROR_SUCCESS;
}

void LogMessageA(LOGLEVEL LogLevel, char* Message, ...)
{
    size_t MessageLength = strlen(Message);

    SYSTEMTIME Time = { 0 };

    HANDLE LogFileHandle = INVALID_HANDLE_VALUE;

    DWORD EndOfFile = 0;

    DWORD NumberOfBytesWritten = 0;

    char DateTimeString[96] = { 0 };

    char SeverityString[8] = { 0 };

    char FormattedString[4096] = { 0 };
    

    if (gRegistryParams.LogLevel < (DWORD)LogLevel)
    {
        return;
    }

    if (MessageLength < 1 || MessageLength > 4095)
    {
        ASSERT(FALSE, "Message was either too short or too long!");
    }

    switch (LogLevel)
    {
        case LL_NONE:
        {
            return;
        }
        case LL_INFO:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[INFO]");

            break;
        }
        case LL_WARNING:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[WARN]");

            break;
        }
        case LL_ERROR:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[ERROR]");

            break;
        }
        case LL_DEBUG:
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[DEBUG]");

            break;
        }
        default:
        {
            ASSERT(FALSE, "Unrecognized log level!");
        }
    }

    GetLocalTime(&Time);

    va_list ArgPointer = NULL;

    va_start(ArgPointer, Message);

    _vsnprintf_s(FormattedString, sizeof(FormattedString), _TRUNCATE, Message, ArgPointer);

    va_end(ArgPointer);

    _snprintf_s(DateTimeString, sizeof(DateTimeString), _TRUNCATE, "\r\n[%02u/%02u/%u %02u:%02u:%02u.%03u]", Time.wMonth, Time.wDay, Time.wYear, Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds);

    EnterCriticalSection(&gLogCritSec);

    if ((LogFileHandle = CreateFileA(LOG_FILE_NAME, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(NULL, "Failed to access log file!", "ERROR!", MB_ICONERROR | MB_OK);

        ASSERT(FALSE, "Failed to access log file!");        
    }

    EndOfFile = SetFilePointer(LogFileHandle, 0, NULL, FILE_END);

    WriteFile(LogFileHandle, DateTimeString, (DWORD)strlen(DateTimeString), &NumberOfBytesWritten, NULL);

    WriteFile(LogFileHandle, SeverityString, (DWORD)strlen(SeverityString), &NumberOfBytesWritten, NULL);

    WriteFile(LogFileHandle, FormattedString, (DWORD)strlen(FormattedString), &NumberOfBytesWritten, NULL);

    if (LogFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(LogFileHandle);
    }   
    
    LeaveCriticalSection(&gLogCritSec);
}

// Currently only supports DWORD registry values.
uint32_t LoadSettings(void)
{
    uint32_t Result = ERROR_SUCCESS;

    typedef struct REGSETTING
    {
        char* Name;

        DWORD* Destination;

        DWORD Default;

    } REGSETTING;

    REGSETTING RegSettings[] = {
        { "LogLevel",    &gRegistryParams.LogLevel,    LL_NONE },
        { "ScaleFactor", &gRegistryParams.ScaleFactor, 0 },
        { "SFXVolume",   &gRegistryParams.SFXVolume,   50 },
        { "MusicVolume", &gRegistryParams.MusicVolume, 50 },
        { "DebugKey",    &gRegistryParams.DebugKey,    VK_F1 }
    }; 

    HKEY RegKey = NULL;

    DWORD RegDisposition = 0;

    DWORD RegBytesRead = sizeof(DWORD);

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\" GAME_NAME, 0, NULL, 0, KEY_READ, NULL, &RegKey, &RegDisposition);

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (RegDisposition == REG_CREATED_NEW_KEY)
    {
        LogMessageA(LL_INFO, "[%s] Registry key did not exist; created new key HKCU\\SOFTWARE\\%s.", __FUNCTION__, GAME_NAME);
    }
    else
    {
        LogMessageA(LL_INFO, "[%s] Opened existing registry key HCKU\\SOFTWARE\\%s", __FUNCTION__, GAME_NAME);
    }

    for (int i = 0; i < _countof(RegSettings); i++)
    {
        Result = RegGetValueA(RegKey, NULL, RegSettings[i].Name, RRF_RT_DWORD, NULL, (BYTE*)RegSettings[i].Destination, &RegBytesRead);

        if (Result != ERROR_SUCCESS)
        {
            if (Result == ERROR_FILE_NOT_FOUND)
            {
                Result = ERROR_SUCCESS;

                LogMessageA(LL_INFO, "[%s] Registry value '%s' not found. Using default of %d.", __FUNCTION__, RegSettings[i].Name, RegSettings[i].Default);

                *(RegSettings[i].Destination) = RegSettings[i].Default;                                
            }
            else
            {
                LogMessageA(LL_ERROR, "[%s] Failed to read the '%s' registry value! Error 0x%08lx!", __FUNCTION__, RegSettings[i].Name, Result);

                goto Exit;
            }
        }
    }

Exit:

    if (RegKey)
    {
        RegCloseKey(RegKey);
    }

    return(Result);
}

uint32_t SaveSettings(void)
{
    uint32_t Result = ERROR_SUCCESS;

    HKEY RegKey = NULL;

    DWORD RegDisposition = 0;    

    DWORD SFXVolume = (DWORD)(gSFXVolume * 100.0f);
    
    DWORD MusicVolume = (DWORD)(gMusicVolume * 100.0f);

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\" GAME_NAME, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &RegKey, &RegDisposition);

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] Reg key open for save.", __FUNCTION__);

    Result = RegSetValueExA(RegKey, "SFXVolume", 0, REG_DWORD, (const BYTE*)&SFXVolume, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set 'SFXVolume' in the registry! Error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] SFXVolume saved: %d.", __FUNCTION__, SFXVolume);

    Result = RegSetValueExA(RegKey, "MusicVolume", 0, REG_DWORD, (const BYTE*)&MusicVolume, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set 'MusicVolume' in the registry! Error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] MusicVolume saved: %d.", __FUNCTION__, MusicVolume);

    Result = RegSetValueExA(RegKey, "ScaleFactor", 0, REG_DWORD, &gPerformanceData.CurrentScaleFactor, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(LL_ERROR, "[%s] Failed to set 'ScaleFactor' in the registry! Error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(LL_INFO, "[%s] ScaleFactor saved: %d.", __FUNCTION__, gPerformanceData.CurrentScaleFactor);


Exit:

    if (RegKey)
    {
        RegCloseKey(RegKey);
    }

    return(Result);
}

#else

#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Prepend getenv("HOME") to this before using.
#define LINUX_CONFIG_DIR "/.config/" GAME_NAME
#define LINUX_CONFIG_FILE LINUX_CONFIG_DIR "/config.bin"
#define LINUX_CONFIG_SIZE (sizeof(gSFXVolume) + sizeof(gMusicVolume) + sizeof(gPerformanceData.CurrentScaleFactor))

pa_threaded_mainloop *gPulseLoop;
PulseSound gBackgroundMusic[MUSIC_QUEUE_SIZE];
PulseSound gSFX[NUMBER_OF_SFX_SOURCE_VOICES];
int gMusicIndex;
volatile AssetThreadStatus gAssetThreadStatus;
pthread_mutex_t gLogMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Linux does not have strncpy_s and strncpy has issues.
 * Main one being that if dst is not large enough, it does not get null terminated.
 * Wrote my own replacement function to solve that issue.
 */
void safe_strncpy(char *restrict dst, size_t dst_size, const char *restrict src, size_t count)
{
    if (dst == NULL || src == NULL || dst_size == 0 || count == 0) {
        return;
    }
    count = min(count, dst_size - 1);
    size_t i = 0;
    while (i < count && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

void PlayGameSound(GAMESOUND* GameSound)
{
    static int SFXSourceVoiceSelector = 0;
    if (gPulseLoop == NULL) {
        return;
    }
    pa_threaded_mainloop_lock(gPulseLoop);
    gSFX[SFXSourceVoiceSelector].samples = GameSound->Buffer.AudioBytes / 2;
    gSFX[SFXSourceVoiceSelector].position = 0;
    gSFX[SFXSourceVoiceSelector].data = (int16_t *)GameSound->Buffer.pAudioData;
    SFXSourceVoiceSelector++;
    if (SFXSourceVoiceSelector > (NUMBER_OF_SFX_SOURCE_VOICES - 1)) {
        SFXSourceVoiceSelector = 0;
    }
    pa_threaded_mainloop_unlock(gPulseLoop);
}

void PlayGameMusic(GAMESOUND* GameSound, bool Loop, bool Immediate)
{
    static int writeIndex = 0;

    if (gPulseLoop == NULL) {
        return;
    }

    pa_threaded_mainloop_lock(gPulseLoop);

    if (gMusicIsPaused) {
        gMusicIsPaused = false;
        pa_threaded_mainloop_unlock(gPulseLoop);
        return;
    }

    if (Immediate) {
        gMusicIndex = 0;
        writeIndex = 0;
        memset(gBackgroundMusic, 0, sizeof(PulseSound) * MUSIC_QUEUE_SIZE);
    } else {
        writeIndex++;
        if (writeIndex > (MUSIC_QUEUE_SIZE - 1)) {
            writeIndex = 0;
        }
    }

    gBackgroundMusic[writeIndex].loop = Loop;
    gBackgroundMusic[writeIndex].samples = GameSound->Buffer.AudioBytes / 2;
    gBackgroundMusic[writeIndex].position = 0;
    gBackgroundMusic[writeIndex].data = (int16_t *)GameSound->Buffer.pAudioData;

    pa_threaded_mainloop_unlock(gPulseLoop);
}

void StopAllGameSounds(void)
{
    if (gPulseLoop == NULL) {
        return;
    }
    pa_threaded_mainloop_lock(gPulseLoop);
    memset(gSFX, 0, sizeof(PulseSound) * NUMBER_OF_SFX_SOURCE_VOICES);
    pa_threaded_mainloop_unlock(gPulseLoop);
}

void PauseMusic(void)
{
    if (gPulseLoop == NULL) {
        return;
    }
    pa_threaded_mainloop_lock(gPulseLoop);
    gMusicIsPaused = true;
    pa_threaded_mainloop_unlock(gPulseLoop);
}

void StopMusic(void)
{
    if (gPulseLoop == NULL) {
        return;
    }
    pa_threaded_mainloop_lock(gPulseLoop);
    gMusicIsPaused = false;
    memset(gBackgroundMusic, 0, sizeof(PulseSound) * MUSIC_QUEUE_SIZE);
    pa_threaded_mainloop_unlock(gPulseLoop);
}

bool MusicIsPlaying(void)
{
    if (gPulseLoop == NULL) {
        return false;
    }
    pa_threaded_mainloop_lock(gPulseLoop);
    bool playing = !gMusicIsPaused && gBackgroundMusic[gMusicIndex].data != NULL;
    pa_threaded_mainloop_unlock(gPulseLoop);
    return playing;
}

void LogMessageA(LOGLEVEL LogLevel, char *Message, ...)
{
    char buffer[4096];
    const char *severity;

    switch(LogLevel) {
        case LL_NONE:
            return;
        case LL_INFO:
            severity = "[INFO]";
            break;
        case LL_WARNING:
            severity = "[WARN]";
            break;
        case LL_ERROR:
            severity = "[ERROR]";
            break;
        case LL_DEBUG:
            severity = "[DEBUG]";
            break;
        default:
            severity = "[UNKNOWN]";
            ASSERT(false, "Unrecognized log level!")
    }

    struct timespec unix_time;
    clock_gettime(CLOCK_REALTIME, &unix_time);
    int milliseconds = unix_time.tv_nsec / 1000000;

    pthread_mutex_lock(&gLogMutex);

    // localtime is not thread safe so make sure we lock the mutex first.
    struct tm *time = localtime(&unix_time.tv_sec);
    int month = time->tm_mon + 1;
    int year = time->tm_year + 1900;
    snprintf(buffer, 96, "[%02d/%02d/%d %02d:%02d:%02d.%03d]", month, time->tm_mday, year, time->tm_hour, time->tm_min, time->tm_sec, milliseconds);
    size_t buffer_len = strlen(buffer);
    size_t severity_len = strlen(severity);
    memcpy(buffer + buffer_len, severity, severity_len);
    buffer_len += severity_len;

    va_list ArgPointer;
    va_start(ArgPointer, Message);
    vsnprintf(buffer + buffer_len, sizeof(buffer) - buffer_len - 1, Message, ArgPointer);
    va_end(ArgPointer);

    buffer_len = strlen(buffer);
    buffer[buffer_len] = '\n';
    buffer_len++;

    int fd = open(LOG_FILE_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror(LOG_FILE_NAME);
    } else {
        if (write(fd, buffer, buffer_len) == -1) {
            perror(LOG_FILE_NAME);
        }
        close(fd);
    }

    pthread_mutex_unlock(&gLogMutex);
}

uint32_t LoadSettings(void)
{
    char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        LogMessageA(LL_ERROR, "[%s] Failed to get HOME environment variable", __FUNCTION__);
        return ERROR_FILE_INVALID;
    }
    char file_location[256];
    size_t home_len = strlen(home_dir);
    size_t file_len = strlen(LINUX_CONFIG_FILE);
    if (home_len > sizeof(file_location) - file_len - 1) {
        LogMessageA(LL_ERROR, "[%s] HOME enviornment variable is too long", __FUNCTION__);
        return ERROR_FILE_INVALID;
    }
    memcpy(file_location, home_dir, home_len);
    memcpy(file_location + home_len, LINUX_CONFIG_FILE, file_len + 1);
    int fd = open(file_location, O_RDONLY);
    if (fd == -1) {
        LogMessageA(LL_ERROR, "[%s] Failed to open config file at %s", __FUNCTION__, file_location);
        return ERROR_FILE_INVALID;
    }
    uint8_t config_data[LINUX_CONFIG_SIZE];
    if (read(fd, config_data, LINUX_CONFIG_SIZE) != LINUX_CONFIG_SIZE) {
        close(fd);
        LogMessageA(LL_ERROR, "[%s] Failed to read from config file at %s", __FUNCTION__, file_location);
        return ERROR_FILE_INVALID;
    }
    close(fd);
    uint8_t *ptr = config_data;
    memcpy(&gSFXVolume, ptr, sizeof(gSFXVolume));
    ptr += sizeof(gSFXVolume);
    memcpy(&gMusicVolume, ptr, sizeof(gMusicVolume));
    ptr += sizeof(gMusicVolume);
    memcpy(&gPerformanceData.CurrentScaleFactor, ptr, sizeof(gPerformanceData.CurrentScaleFactor));
    LogMessageA(LL_INFO, "[%s] Config data successfully read from %s", __FUNCTION__, file_location);
    return ERROR_SUCCESS;
}

uint32_t SaveSettings(void)
{
    char *home_dir = getenv("HOME");
    if (home_dir == NULL) {
        LogMessageA(LL_ERROR, "[%s] Failed to get HOME environment variable", __FUNCTION__);
        return ERROR_FILE_INVALID;
    }
    char file_location[256];
    size_t home_len = strlen(home_dir);
    size_t file_len = strlen(LINUX_CONFIG_FILE);
    if (home_len > sizeof(file_location) - file_len - 1) {
        LogMessageA(LL_ERROR, "[%s] HOME enviornment variable is too long", __FUNCTION__);
        return ERROR_FILE_INVALID;
    }
    memcpy(file_location, home_dir, home_len);
    memcpy(file_location + home_len, LINUX_CONFIG_FILE, file_len + 1);
    int fd = open(file_location, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        char *file_begin = file_location + home_len + strlen(LINUX_CONFIG_DIR);
        *file_begin = '\0';
        if (mkdir(file_location, 0755) == -1) {
            LogMessageA(LL_ERROR, "[%s] Failed to create config directory at %s", __FUNCTION__, file_location);
            return ERROR_FILE_INVALID;
        }
        LogMessageA(LL_INFO, "[%s] Config directory created at %s", __FUNCTION__, file_location);
        *file_begin = '/';
        fd = open(file_location, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            LogMessageA(LL_ERROR, "[%s] Failed to open config file at %s", __FUNCTION__, file_location);
            return ERROR_FILE_INVALID;
        }
    }
    uint8_t config_data[LINUX_CONFIG_SIZE];
    uint8_t *ptr = config_data;
    memcpy(ptr, &gSFXVolume, sizeof(gSFXVolume));
    ptr += sizeof(gSFXVolume);
    memcpy(ptr, &gMusicVolume, sizeof(gMusicVolume));
    ptr += sizeof(gMusicVolume);
    memcpy(ptr, &gPerformanceData.CurrentScaleFactor, sizeof(gPerformanceData.CurrentScaleFactor));
    if (write(fd, config_data, LINUX_CONFIG_SIZE) != LINUX_CONFIG_SIZE) {
        close(fd);
        LogMessageA(LL_ERROR, "[%s] Failed to write to config file at %s", __FUNCTION__, file_location);
        return ERROR_FILE_INVALID;
    }
    close(fd);
    LogMessageA(LL_INFO, "[%s] Config file saved successfully at %s", __FUNCTION__, file_location);
    return ERROR_SUCCESS;
}

#endif
