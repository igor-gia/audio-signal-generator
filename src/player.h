#pragma once
#include <Arduino.h>
#include "Audio.h"

const int PLAYER_MAX_ITEMS      = 64;  
const int PLAYER_MAX_NAME_LEN   = 64;  

enum FileType {
    ITEM_FILE_AUDIO,  
    ITEM_DIRECTORY,
    ITEM_UP_DIR       
};

struct FileBrowserItem {
    char name[PLAYER_MAX_NAME_LEN];
    FileType type;
    uint32_t size;    
};

struct TrackInfo {
    char name[PLAYER_MAX_NAME_LEN];
    uint32_t size;
    uint32_t duration; 
    uint32_t bitrate;  
    uint32_t sampleRate;
    char codec[16];
};

class AudioPlayer {
public:
    AudioPlayer();
    
    void init();
    bool sdInit();
    void stop();
    void tick();

    void openRoot();                    
    bool enterDir(const char* dirName); 
    bool exitToParent();               
    const char* getCurrentPath() const { return _currentPath; }

    int getItemsCount() const { return _itemsCount; }
    const FileBrowserItem* getItemSorted(int sortedIndex) const;
    void playSortedIndex(int sortedIndex);
    
    void next();                         
    void prev();                         
    void togglePause();
    bool isPlaying() const { return _isPlaying; }
    bool isPaused() const { return _isPaused; }
    
    void setVolumeDb(int vol_db);
    int getVolumeDb() const { return _currentVolumeDb; }
    void setMute(bool mute);
    bool getMute() const { return _isMute; }

    int getCurrentSortedIndex();

    void getCurrentTrackInfo(TrackInfo* info);
    uint32_t getAudioCurrentTime(); 

    void handleEof(); 
    void setInfoCallback(void (*callback)(const char*)) { _userInfoCallback = callback; }

private:
    void scanAndSortDirectory();
    bool isSupportedAudioFile(const char* filename);

    FileBrowserItem _browserList[PLAYER_MAX_ITEMS];
    int _sortedIndices[PLAYER_MAX_ITEMS]; 
    
    Audio* _audioEngine;

    int _itemsCount;
    int _currentPlayingTrackIndex; 
    int _currentVolumeDb;
    bool _isMute;
    
    bool _isPlaying;
    bool _isPaused;

    char _currentPath[256];
    void (*_userInfoCallback)(const char*);
};

extern AudioPlayer player;