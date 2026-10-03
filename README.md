myMusicPlayer
=============

开源音乐播放器 · 音乐魔盒（Qt Widgets + Qt Multimedia）

界面为深色玻璃拟态主题，青蓝色强调色；所有控件由布局管理，窗口可自由缩放。

环境要求
--------

| 项目 | 要求 |
| ---- | ---- |
| 最低 Qt | **6.12 LTS**（`myMusicPlayer.pro` 内有版本断言，低于该版本 qmake 直接报错） |
| 编译标准 | C++17 |
| 依赖模块 | Core、Gui、Widgets、Multimedia |

代码用到的较新 API：`QMediaPlayer::setAudioOutput()`、`playbackStateChanged()`、
`errorOccurred()`（均为 Qt 6.2 引入），`QAudioOutput`、`QRandomGenerator`（Qt 6.0）。

构建
----

```bash
qmake6 myMusicPlayer.pro && make -j        # Linux
qmake myMusicPlayer.pro && nmake            # Windows (MSVC)
```

Debian/Ubuntu 依赖（以 Qt 6.12 官方源为准）：

```
qt6-base-dev qt6-multimedia-dev qt6-base-dev-tools
```

运行后播放列表保存在应用数据目录（`QStandardPaths::AppDataLocation`）下的
`playlist.txt`；若检测到旧版本写在可执行文件目录的 `playlist.txt`，会自动兼容读取。

功能
----

- 添加（批量）/ 移除歌曲，双击列表播放
- 上一曲 / 播放暂停 / 下一曲，拖动进度条跳转（按真实播放位置驱动歌词）
- 顺序、列表循环、随机、单曲循环四种播放模式，点击「模式」按钮切换
- 音量滑杆 + 静音开关（图标随音量变化）
- LRC 歌词滚动，支持 `[mm:ss.xx]` 与 `[mm:ss.xxx]`，优先读取音频同目录同名 `.lrc`
- 登录（演示：仅验证码校验）与关于对话框

目录结构
--------

```
main.cpp           程序入口：Fusion 样式 + 暗色调色板 + 窗口居中
mymusicplayer.*    主窗口：布局、播放列表、播放控制
theme.*            深色主题配色与 QSS（改这里即可换肤）
icons.*            QPainter 矢量图标（任意 DPI 清晰，无二进制资源）
lrc/               歌词控件
login/, about.*    对话框
```

已知取舍
--------

- “玻璃拟态”通过半透明面板 + 描边 + 渐变实现，保留系统原生标题栏，以便跨平台拖拽与缩放；
  若需要整窗毛玻璃需改为无边框 + `Qt::WA_TranslucentBackground`，那需要自绘标题栏与窗口拖动。
- 歌词文件名需与音频同名同目录（或放在 `lrc/` 目录下）。
