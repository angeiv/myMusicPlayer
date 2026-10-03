#ifndef THEME_H
#define THEME_H

#include <QPalette>
#include <QString>

/*!
 * \brief 深色玻璃拟态主题（配色 + 样式表）。
 *
 * 所有控件都通过 objectName 与本模块的 QSS 对应，
 * 新增控件时保持命名一致即可自动获得统一外观。
 */
namespace Theme {

// 颜色令牌：修改这里即可整体换肤
inline const QString Base        = QStringLiteral("#0B1120");  // 窗口底色（渐变起点）
inline const QString BaseMid     = QStringLiteral("#111C31");  // 窗口底色（渐变中段）
inline const QString BaseEnd     = QStringLiteral("#08161F");  // 窗口底色（渐变终点）
inline const QString Glass       = QStringLiteral("rgba(148, 197, 255, 26)");   // 玻璃面板底
inline const QString GlassStrong = QStringLiteral("rgba(9, 15, 28, 190)");      // 强玻璃层（弹窗/菜单）
inline const QString Border      = QStringLiteral("rgba(148, 184, 226, 56)");   // 玻璃描边
inline const QString Divider     = QStringLiteral("rgba(148, 184, 226, 40)");   // 分隔线
inline const QString Text        = QStringLiteral("#E8F2FF");  // 主文本
inline const QString TextDim     = QStringLiteral("#93A7C4");  // 次要文本
inline const QString TextFaint   = QStringLiteral("#63779A");  // 禁用/极弱文本
inline const QString Accent      = QStringLiteral("#22D3EE");  // 青蓝强调色
inline const QString AccentHover = QStringLiteral("#67E8F9");
inline const QString AccentDeep  = QStringLiteral("#0284C7");
inline const QString AccentSoft  = QStringLiteral("rgba(34, 211, 238, 46)");   // 选中行/悬停底
inline const QString Danger      = QStringLiteral("#F87171");

/// 生成完整 QSS（@TOKEN@ 会被替换为对应颜色）。
QString styleSheet();

/// 供 QSS 之外的原生绘制部分（滚动条轨道、消息框等）使用的暗色调色板。
QPalette darkPalette();

/// 一次性把暗色调色板安装到整个应用。
void applyApplicationTheme();

} // namespace Theme

#endif // THEME_H
