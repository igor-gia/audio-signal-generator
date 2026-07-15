#include "player.h"
#include "Config.h"
#include "SD_MMC.h"


AudioPlayer player;

AudioPlayer::AudioPlayer() {
    _audioEngine = nullptr;
    _itemsCount = 0;
    _currentPlayingTrackIndex = -1;
    _currentVolumeDb = -10;
    _isMute = false;

    _isPlaying = false;
    _isPaused = false;

    _userInfoCallback = nullptr;

    strcpy(_currentPath, "/");

    memset(_sortedIndices, 0, sizeof(_sortedIndices));
}

void AudioPlayer::init() {
    _currentPlayingTrackIndex = -1;
    _isPlaying = false;
    _isPaused = false;

    // Создаем объект динамически, если его еще нет
    if (_audioEngine == nullptr) {
        _audioEngine = new Audio();
    }

    _audioEngine->audio_info_callback = [this](Audio::msg_t m) {
        if (_userInfoCallback) {
            char buf[256];
            snprintf(buf, sizeof(buf), "%s: %s", m.s ? m.s : "", m.msg ? m.msg : "");
            _userInfoCallback(buf);
        }
        if (m.s && strcasecmp(m.s, "eof") == 0) {
            handleEof();
        }
    };

    _audioEngine->setAudioTaskCore(1); // Можно вернуть обратно на Ядро 1
    _audioEngine->setPinout(BCLK_PIN, LRCK_PIN, DIN_PIN);

}

bool AudioPlayer::sdInit() {
    // Инициализация SD-карты
    static bool sdInitialized = false;
    if (!sdInitialized) {
        pinMode(SD_MMC_D0, INPUT_PULLUP);
        SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
        if (!SD_MMC.begin("/sdcard", true)) {
            return false;
        }
        sdInitialized = true;
    }

    openRoot();
    return true;
}

void AudioPlayer::tick() {
    if (_isPlaying && _audioEngine != nullptr) {
        _audioEngine->loop();
    }
}

void AudioPlayer::stop() {
    _isPlaying = false;
    _isPaused = false;

    if (_audioEngine != nullptr) {
        _audioEngine->audio_info_callback = nullptr;
        _audioEngine->stopSong();
        // ЖДЁМ пока реально остановится
        uint32_t timeout = millis();
        while (_audioEngine->isRunning()) {
            _audioEngine->loop(); 
            delay(1);
            // защита от зависания
            if (millis() - timeout > 2000) {
                break;
            }
        }

        delay(100);
        // теперь безопаснее удалять
        delete _audioEngine;
        _audioEngine = nullptr;
    }
}

int AudioPlayer::getCurrentSortedIndex() {
    if (!_isPlaying || _currentPlayingTrackIndex < 0) return -1;
    
    // Ищем, какому sortedIndex соответствует реальный играющий индекс трека
    for (int i = 0; i < _itemsCount; i++) {
        if (_sortedIndices[i] == _currentPlayingTrackIndex) {
            return i;
        }
    }
    return -1;
}

void AudioPlayer::openRoot() {
    strcpy(_currentPath, "/");
    scanAndSortDirectory();
}

bool AudioPlayer::enterDir(const char* dirName) {
    char newPath[256];

    if (dirName[0] == '/') {
        strncpy(newPath, dirName, sizeof(newPath) - 1);
    } else {
        if (strcmp(_currentPath, "/") == 0) {
            snprintf(newPath, sizeof(newPath), "/%s", dirName);
        } else {
            snprintf(newPath, sizeof(newPath), "%s/%s", _currentPath, dirName);
        }
    }

    File test = SD_MMC.open(newPath);
    if (!test || !test.isDirectory()) {
        return false;
    }
    test.close();

    strncpy(_currentPath, newPath, sizeof(_currentPath) - 1);
    _currentPath[sizeof(_currentPath) - 1] = '\0';

    scanAndSortDirectory();
    return true;
}

bool AudioPlayer::exitToParent() {
    if (strcmp(_currentPath, "/") == 0) return false;

    char* slash = strrchr(_currentPath, '/');

    if (slash == _currentPath) {
        strcpy(_currentPath, "/");
    } else if (slash) {
        *slash = '\0';
    }

    scanAndSortDirectory();
    return true;
}

const FileBrowserItem* AudioPlayer::getItemSorted(int sortedIndex) const {
    if (sortedIndex < 0 || sortedIndex >= _itemsCount) return nullptr;
    return &_browserList[_sortedIndices[sortedIndex]];
}

void AudioPlayer::playSortedIndex(int sortedIndex) {
    if (sortedIndex < 0 || sortedIndex >= _itemsCount) return;
    int real = _sortedIndices[sortedIndex];
    if (_browserList[real].type != ITEM_FILE_AUDIO) return;

    // Если объект был удален в методе stop(), пересоздаем его перед игрой
    if (_audioEngine == nullptr) {
        init(); 
    }

    char path[512];
    if (strcmp(_currentPath, "/") == 0) {
        snprintf(path, sizeof(path), "/%s", _browserList[real].name);
    } else {
        snprintf(path, sizeof(path), "%s/%s", _currentPath, _browserList[real].name);
    }

    setVolumeDb(_currentVolumeDb);

    if (_audioEngine->connecttoFS(SD_MMC, path)) {
        _currentPlayingTrackIndex = real;
        _isPlaying = true;
        _isPaused = false;
    }
}

void AudioPlayer::next() {
    if (_itemsCount == 0) return;

    int currentSorted = -1;

    for (int i = 0; i < _itemsCount; i++) {
        if (_sortedIndices[i] == _currentPlayingTrackIndex) {
            currentSorted = i;
            break;
        }
    }

    if (currentSorted < 0) return;

    for (int i = 1; i <= _itemsCount; i++) {
        int idx = (currentSorted + i) % _itemsCount;
        int real = _sortedIndices[idx];

        if (_browserList[real].type == ITEM_FILE_AUDIO) {
            playSortedIndex(idx);
            return;
        }
    }
}

void AudioPlayer::prev() {
    if (_itemsCount == 0) return;

    int currentSorted = -1;

    for (int i = 0; i < _itemsCount; i++) {
        if (_sortedIndices[i] == _currentPlayingTrackIndex) {
            currentSorted = i;
            break;
        }
    }

    if (currentSorted < 0) return;

    for (int i = 1; i <= _itemsCount; i++) {
        int idx = (currentSorted + _itemsCount - i) % _itemsCount;
        int real = _sortedIndices[idx];

        if (_browserList[real].type == ITEM_FILE_AUDIO) {
            playSortedIndex(idx);
            return;
        }
    }
}

void AudioPlayer::togglePause() {
    if (!_isPlaying || _audioEngine == nullptr) return;
    _audioEngine->pauseResume();
    _isPaused = !_isPaused;
}

void AudioPlayer::setMute(bool mute) {
    _isMute = mute;
    if (_isMute) {
        if (_audioEngine != nullptr) {
            _audioEngine->setVolume(0); // Было . стало -> и завернуто в проверку
        }
    } else {
        setVolumeDb(_currentVolumeDb);
    }
}

void AudioPlayer::setVolumeDb(int vol_db) {
    if (vol_db < -60) vol_db = -60;
    if (vol_db > 0) vol_db = 0;
    _currentVolumeDb = vol_db;

    if (!_isMute && _audioEngine != nullptr) {
        int libVol = map(vol_db, -60, 0, 0, 21);
        _audioEngine->setVolume(libVol);
    }
}

void AudioPlayer::getCurrentTrackInfo(TrackInfo* info) {
    if (!info) return;
    if (_currentPlayingTrackIndex < 0) return;

    const FileBrowserItem& item = _browserList[_currentPlayingTrackIndex];

    strncpy(info->name, item.name, PLAYER_MAX_NAME_LEN - 1);
    info->name[PLAYER_MAX_NAME_LEN - 1] = '\0';

    info->size = item.size;

    // Запрашиваем актуальные данные у аудиодвижка, если он инициализирован
    if (_audioEngine != nullptr) {
        info->duration = _audioEngine->getAudioFileDuration();
        info->bitrate = _audioEngine->getBitRate();
        info->sampleRate = _audioEngine->getSampleRate();
    } else {
        info->duration = 0;
        info->bitrate = 0;
        info->sampleRate = 0;
    }

    // Получение кодека из расширения файла
    const char* ext = strrchr(item.name, '.');
    if (ext) {
        strncpy(info->codec, ext + 1, sizeof(info->codec) - 1);
        for(int i = 0; info->codec[i]; i++) info->codec[i] = toupper(info->codec[i]);
    } else {
        strcpy(info->codec, "UNKNOWN");
    }
}

uint32_t AudioPlayer::getAudioCurrentTime() {
    if (_audioEngine != nullptr) {
        return _audioEngine->getAudioCurrentTime();
    }
    return 0;
}
void AudioPlayer::handleEof() {
    if (cfg.playModeLoop) {
        playSortedIndex(getCurrentSortedIndex()); // Крутим текущий заново
    } else {
        next(); // Переходим на следующий в папке
    }
}

bool AudioPlayer::isSupportedAudioFile(const char* filename) {
    static const char* ext[] = { ".mp3", ".wav", ".flac", ".ogg" };

    size_t len = strlen(filename);

    for (int i = 0; i < 4; i++) {
        size_t el = strlen(ext[i]);

        if (len >= el &&
            strcasecmp(filename + len - el, ext[i]) == 0) {
            return true;
        }
    }

    return false;
}

void AudioPlayer::scanAndSortDirectory() {
    _itemsCount = 0;

    File root = SD_MMC.open(_currentPath);
    if (!root || !root.isDirectory()) return;

    if (strcmp(_currentPath, "/") != 0) {
        strcpy(_browserList[_itemsCount].name, "..");
        _browserList[_itemsCount].type = ITEM_UP_DIR;
        _browserList[_itemsCount].size = 0;
        _itemsCount++;
    }

    while (_itemsCount < PLAYER_MAX_ITEMS) {
        File f = root.openNextFile();
        if (!f) break;

        const char* name = f.name();
        bool isDir = f.isDirectory();
        uint32_t size = f.size();

        if (isDir) {
            if (strcasecmp(name, "System Volume Information") == 0) {
                f.close();
                continue;
            }

            strncpy(_browserList[_itemsCount].name,
                    name,
                    PLAYER_MAX_NAME_LEN - 1);
            _browserList[_itemsCount].name[PLAYER_MAX_NAME_LEN - 1] = '\0'; 
            _browserList[_itemsCount].type = ITEM_DIRECTORY;
            _browserList[_itemsCount].size = 0;
        } else {
            if (!isSupportedAudioFile(name)) {
                f.close();
                continue;
            }

            strncpy(_browserList[_itemsCount].name,
                    name,
                    PLAYER_MAX_NAME_LEN - 1);
            _browserList[_itemsCount].name[PLAYER_MAX_NAME_LEN - 1] = '\0'; 
            _browserList[_itemsCount].type = ITEM_FILE_AUDIO;
            _browserList[_itemsCount].size = size;
        }

        _itemsCount++;
        f.close();
    }

    root.close();

    for (int i = 0; i < _itemsCount; i++) {
        _sortedIndices[i] = i;
    }

    int start = (strcmp(_currentPath, "/") != 0) ? 1 : 0;

    for (int i = start; i < _itemsCount - 1; i++) {
        for (int j = start; j < _itemsCount - 1; j++) {

            int a = _sortedIndices[j];
            int b = _sortedIndices[j + 1];

            bool swap = false;

            if (_browserList[a].type == ITEM_FILE_AUDIO &&
                _browserList[b].type == ITEM_DIRECTORY) {
                swap = true;
            }
            else if (_browserList[a].type == _browserList[b].type) {
                if (strcasecmp(_browserList[a].name,
                               _browserList[b].name) > 0) {
                    swap = true;
                }
            }

            if (swap) {
                int t = _sortedIndices[j];
                _sortedIndices[j] = _sortedIndices[j + 1];
                _sortedIndices[j + 1] = t;
            }
        }
    }
}