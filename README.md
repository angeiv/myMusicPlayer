myMusicPlayer
=============

音乐魔盒（Qt6）

一个基于 Qt 6 的本地音乐播放器。

## 特性
- 播放内核：内置解码（mp3/wav/flac）+ `QAudioSink` 输出（不依赖 FFmpeg）
- UI：Qt Quick / QML（Qt Quick Controls 2）
- 播放列表：自动持久化到系统 AppData（`playlist.json`）
- 歌词：支持 `.lrc`，按播放进度同步显示（查找顺序：同目录 → `<appDir>/lrc/` → `./lrc/`）

## 构建
- 依赖：Qt 6（需要 Qt Multimedia、Qt Quick、Qt Quick Controls 2、Qt Widgets）

### CMake（推荐）
- 生成：`cmake -S . -B build`
- 编译：`cmake --build build --config Release`

> Windows 上如果找不到 Qt，可通过 `-DCMAKE_PREFIX_PATH=<Qt6>/lib/cmake` 或设置 `Qt6_DIR` 指向 Qt 的 CMake 包目录。

### qmake（兼容）
- `qmake myMusicPlayer.pro` 后再执行构建工具（Windows 常用 `mingw32-make`）

## 第三方声明
- 见 `THIRD_PARTY_NOTICES.md`
