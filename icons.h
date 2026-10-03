#ifndef ICONS_H
#define ICONS_H

#include <QColor>
#include <QIcon>

/*!
 * \brief 用 QPainter 绘制的矢量图标。
 *
 * 全部图标在 24x24 逻辑坐标系内绘制后按需缩放，
 * 因此不存在多倍图资源，任何 DPI 下都清晰，且可随主题换色。
 */
namespace Icons {

enum class Icon {
    Play,
    Pause,
    Previous,
    Next,
    Volume,
    VolumeLow,
    Muted,
    Add,
    Remove,
    Loop,        ///< 列表循环
    LoopOne,     ///< 单曲循环
    Shuffle,     ///< 随机播放
    Sequential,  ///< 顺序播放
    Playlist,
    Login,
    Info,
    MusicNote
};

/// 取图标；color 决定前景色（描边/填充）。
QIcon icon(Icon id, const QColor &color = QColor(Qt::white), int size = 32);

/// 播放模式枚举 -> 图标（避免上层重复 switch）。
QIcon playbackModeIcon(int mode, const QColor &color = QColor(Qt::white), int size = 32);

} // namespace Icons

#endif // ICONS_H
