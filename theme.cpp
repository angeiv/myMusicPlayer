#include "theme.h"

#include <QApplication>
#include <QColor>

namespace {

QString token(const QString &name, const QString &value)
{
    return QStringLiteral("@") + name + QStringLiteral("@") + value;
}

} // namespace

QString Theme::styleSheet()
{
    QString qss = QStringLiteral(R"QSS(
/* ---------- 窗口底：深色玻璃渐变 ---------- */
QWidget#myMusicPlayer {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 @BASE@, stop:0.55 @BASEMID@, stop:1 @BASEEND@);
}
QWidget#contentRoot { background: transparent; }

/* ---------- 玻璃面板 ---------- */
QFrame#glassPanel {
    background: @GLASS@;
    border: 1px solid @BORDER@;
    border-radius: 14px;
}
QFrame#divider {
    background: @DIVIDER@;
    border: none;
    max-height: 1px;
    min-height: 1px;
}

/* ---------- 文本 ---------- */
QLabel { background: transparent; color: @TEXT@; }
QLabel#songTitle {
    color: @TEXT@;
    font-size: 20pt;
    font-weight: 600;
    padding: 2px 4px;
}
QLabel#songAuthor {
    color: @TEXTDIM@;
    font-size: 11pt;
    padding: 0 4px;
}
QLabel#panelTitle {
    color: @ACCENT@;
    font-size: 9pt;
    font-weight: 600;
    letter-spacing: 2px;
}
QLabel#lyricLabel {
    color: @TEXT@;
    font-size: 13pt;
    padding: 6px 10px;
}
QLabel#timeLabel {
    color: @TEXTDIM@;
    font-size: 9pt;
}
QLabel#volumeLabel {
    color: @TEXTDIM@;
    font-size: 9pt;
    min-width: 34px;
}

/* ---------- 按钮 ---------- */
QPushButton {
    color: @TEXT@;
    border: none;
    background: transparent;
}
QPushButton#glassButton {
    background: rgba(148, 197, 255, 20);
    border: 1px solid @BORDER@;
    border-radius: 10px;
}
QPushButton#glassButton:hover {
    background: @ACCENTSOFT@;
    border: 1px solid rgba(34, 211, 238, 120);
}
QPushButton#glassButton:pressed {
    background: rgba(34, 211, 238, 70);
}
QPushButton#glassButton:disabled {
    color: @TEXTFAINT@;
    background: rgba(148, 197, 255, 10);
    border: 1px solid rgba(148, 184, 226, 24);
}
QPushButton#playButton {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 @ACCENT@, stop:1 @ACCENTDEEP@);
    border: none;
    border-radius: 31px;
}
QPushButton#playButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 @ACCENTHOVER@, stop:1 @ACCENT@);
}
QPushButton#playButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 @ACCENTDEEP@, stop:1 #0369A1);
}
QPushButton#playButton:disabled {
    background: rgba(148, 184, 226, 46);
}
QPushButton#modeButton {
    background: rgba(148, 197, 255, 16);
    border: 1px solid @DIVIDER@;
    border-radius: 10px;
}
QPushButton#modeButton:hover {
    background: @ACCENTSOFT@;
    border: 1px solid rgba(34, 211, 238, 120);
}

/* ---------- 进度条 ---------- */
QSlider { background: transparent; }
QSlider#progressSlider::groove:horizontal {
    height: 5px;
    border-radius: 3px;
    background: rgba(148, 184, 226, 56);
}
QSlider#progressSlider::sub-page:horizontal {
    height: 5px;
    border-radius: 3px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 @ACCENT@, stop:1 @ACCENTDEEP@);
}
QSlider#progressSlider::handle:horizontal {
    width: 13px;
    margin: -5px 0;
    border-radius: 7px;
    background: #EAFDFF;
    border: 1px solid rgba(34, 211, 238, 190);
}
QSlider#progressSlider::handle:horizontal:hover { width: 15px; margin: -6px 0; }
QSlider#progressSlider::handle:horizontal:pressed { width: 15px; margin: -6px 0; }

QSlider#volumeSlider::groove:horizontal {
    height: 4px;
    border-radius: 2px;
    background: rgba(148, 184, 226, 56);
}
QSlider#volumeSlider::sub-page:horizontal {
    height: 4px;
    border-radius: 2px;
    background: @ACCENT@;
}
QSlider#volumeSlider::handle:horizontal {
    width: 11px;
    margin: -4px 0;
    border-radius: 6px;
    background: #EAFDFF;
}
QSlider#volumeSlider::handle:horizontal:hover { width: 13px; margin: -5px 0; }
QSlider#volumeSlider::sub-page:horizontal:disabled,
QSlider#volumeSlider::handle:horizontal:disabled { background: rgba(148, 184, 226, 70); }

/* ---------- 播放列表 ---------- */
QTableWidget#playlistView {
    background: transparent;
    border: none;
    gridline-color: transparent;
    alternate-background-color: rgba(148, 197, 255, 8);
    outline: none;
    selection-background-color: @ACCENTSOFT@;
    selection-color: @TEXT@;
}
QTableWidget#playlistView::item { padding: 5px 6px; border: none; color: @TEXT@; }
QTableWidget#playlistView::item:selected {
    background: @ACCENTSOFT@;
    color: #F2FEFF;
}
QTableWidget#playlistView::item:hover { background: rgba(148, 197, 255, 20); }
QHeaderView { background: transparent; }
QHeaderView::section {
    background: transparent;
    color: @TEXTDIM@;
    border: none;
    border-bottom: 1px solid @DIVIDER@;
    padding: 6px 6px 8px 6px;
    font-size: 10pt;
    font-weight: 600;
}
QTableCornerButton::section { background: transparent; border: none; }

/* ---------- 滚动条 ---------- */
QScrollBar { background: transparent; }
QScrollBar:vertical { width: 10px; margin: 4px 2px 4px 0; }
QScrollBar:horizontal { height: 10px; margin: 0 2px 4px 4px; }
QScrollBar::handle:vertical,
QScrollBar::handle:horizontal {
    background: rgba(148, 184, 226, 110);
    border-radius: 5px;
    min-height: 32px;
    min-width: 32px;
}
QScrollBar::handle:vertical:hover,
QScrollBar::handle:horizontal:hover { background: rgba(34, 211, 238, 170); }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

/* ---------- 菜单栏 / 菜单 ---------- */
QMenuBar { background: transparent; color: @TEXTDIM@; }
QMenuBar::item { background: transparent; padding: 5px 12px; border-radius: 7px; }
QMenuBar::item:selected { background: @ACCENTSOFT@; color: @TEXT@; }
QMenu {
    background: @GLASSSTRONG@;
    border: 1px solid @BORDER@;
    border-radius: 10px;
    padding: 6px;
}
QMenu::item { padding: 6px 24px 6px 20px; border-radius: 7px; color: @TEXT@; }
QMenu::item:selected { background: @ACCENTSOFT@; color: #F2FEFF; }
QMenu::item:disabled { color: @TEXTFAINT@; }
QMenu::separator { height: 1px; background: @DIVIDER@; margin: 5px 8px; }

/* ---------- 对话框 / 输入控件 ---------- */
QDialog { background: #0C1424; }
QLineEdit {
    background: rgba(148, 197, 255, 22);
    border: 1px solid @BORDER@;
    border-radius: 8px;
    padding: 7px 10px;
    color: @TEXT@;
    selection-background-color: @ACCENT@;
    selection-color: #052B33;
}
QLineEdit:focus { border: 1px solid @ACCENT@; background: rgba(148, 197, 255, 32); }
QTextEdit, QTextBrowser, QPlainTextEdit {
    background: rgba(148, 197, 255, 14);
    border: 1px solid @DIVIDER@;
    border-radius: 10px;
    padding: 8px;
    color: @TEXT@;
    selection-background-color: @ACCENT@;
}
QGroupBox {
    border: 1px solid @DIVIDER@;
    border-radius: 10px;
    margin-top: 14px;
    color: @TEXTDIM@;
}
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 4px; color: @ACCENT@; }
QCheckBox, QRadioButton { color: @TEXT@; spacing: 8px; }
QMessageBox { background: #0C1424; }
QMessageBox QLabel { color: @TEXT@; }
QToolTip {
    background: #0B1424;
    color: @TEXT@;
    border: 1px solid rgba(34, 211, 238, 110);
    border-radius: 6px;
    padding: 5px 9px;
}
QStatusBar { background: transparent; color: @TEXTDIM@; }
)QSS");

    qss.replace(token(QStringLiteral("BASE"), Base));
    qss.replace(token(QStringLiteral("BASEMID"), BaseMid));
    qss.replace(token(QStringLiteral("BASEEND"), BaseEnd));
    qss.replace(token(QStringLiteral("GLASS"), Glass));
    qss.replace(token(QStringLiteral("GLASSSTRONG"), GlassStrong));
    qss.replace(token(QStringLiteral("BORDER"), Border));
    qss.replace(token(QStringLiteral("DIVIDER"), Divider));
    qss.replace(token(QStringLiteral("TEXT"), Text));
    qss.replace(token(QStringLiteral("TEXTDIM"), TextDim));
    qss.replace(token(QStringLiteral("TEXTFAINT"), TextFaint));
    qss.replace(token(QStringLiteral("ACCENT"), Accent));
    qss.replace(token(QStringLiteral("ACCENTHOVER"), AccentHover));
    qss.replace(token(QStringLiteral("ACCENTDEEP"), AccentDeep));
    qss.replace(token(QStringLiteral("ACCENTSOFT"), AccentSoft));

    return qss;
}

QPalette Theme::darkPalette()
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(Base));
    p.setColor(QPalette::WindowText, QColor(Text));
    p.setColor(QPalette::Base, QColor(BaseMid));
    p.setColor(QPalette::AlternateBase, QColor(Base));
    p.setColor(QPalette::Text, QColor(Text));
    p.setColor(QPalette::Button, QColor(Glass));
    p.setColor(QPalette::ButtonText, QColor(Text));
    p.setColor(QPalette::Highlight, QColor(Accent));
    p.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#052B33")));
    p.setColor(QPalette::ToolTipBase, QColor(BaseMid));
    p.setColor(QPalette::ToolTipText, QColor(Text));
    p.setColor(QPalette::PlaceholderText, QColor(TextFaint));

    // 禁用态单独处理，避免未选中项出现高亮亮块
    p.setColor(QPalette::Disabled, QPalette::Text, QColor(TextFaint));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(TextFaint));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(TextFaint));
    p.setColor(QPalette::Disabled, QPalette::Highlight, QColor(Glass));
    p.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(TextFaint));

    return p;
}

void Theme::applyApplicationTheme()
{
    // Fusion 是跨平台唯一能被 QSS 完整接管的基础样式，必须先设置。
    QApplication::setStyle(QStringLiteral("Fusion"));
    qApp->setPalette(darkPalette());
}
