myMusicPlayer
=============

音乐魔盒（Qt6）

一个基于 Qt 6 的本地音乐播放器。

## 特性
- 播放内核：`QMediaPlayer` + `QAudioOutput`
- UI：Qt Quick / QML（Qt Quick Controls 2）
- 播放列表：自动持久化到系统 AppData（`playlist.json`）
- 歌词：支持 `.lrc`，按播放进度同步显示（查找顺序：同目录 → `<appDir>/lrc/` → `./lrc/`）

## 构建
- 依赖：Qt 6（需要 Qt Multimedia、Qt Quick、Qt Quick Controls 2）
- 使用 qmake：`qmake myMusicPlayer.pro` 后再执行构建工具（Windows 常用 `mingw32-make`）
