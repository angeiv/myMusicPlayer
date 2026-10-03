#include "icons.h"

#include <QGuiApplication>
#include <QHash>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPolygonF>
#include <QScreen>

namespace {

/// 图标逻辑坐标系边长（所有图形按 24x24 设计后整体缩放）
constexpr qreal kDesign = 24.0;
/// 常规描边宽度（逻辑单位）
constexpr qreal kStroke = 2.0;

QPainterPath roundedRectPath(const QRectF &r, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(r, radius, radius);
    return path;
}

/// 实心箭头头部，尖端在 at，沿 angleDeg 方向（屏幕坐标，0° 指向 +X）。
void drawArrowHead(QPainter &p, QPointF at, qreal angleDeg, const QColor &c,
                   qreal len = 4.4, qreal halfWidth = 2.5)
{
    const qreal rad = qDegreesToRadians(angleDeg);
    const QPointF dir(qCos(rad), qSin(rad));
    const QPointF back = at - dir * len;
    const QPointF normal(-dir.y(), dir.x()) * halfWidth;

    QPainterPath path;
    path.moveTo(at);
    path.lineTo(back + normal);
    path.lineTo(back - normal);
    path.closeSubpath();

    p.fillPath(path, c);
}

void drawSpeaker(QPainter &p, const QColor &c)
{
    QPainterPath path;
    path.moveTo(4.0, 9.5);
    path.lineTo(7.5, 9.5);
    path.lineTo(11.5, 5.6);
    path.lineTo(11.5, 18.4);
    path.lineTo(7.5, 14.5);
    path.lineTo(4.0, 14.5);
    path.closeSubpath();
    p.fillPath(path, c);
}

void drawSoundWaves(QPainter &p, const QColor &c, int waves)
{
    p.setPen(QPen(c, 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QRectF arcRect(10.0, 5.0, 14.0, 14.0); // 圆心 (17,12)，半径 7
    for (int i = 0; i < waves; ++i) {
        p.drawArc(arcRect, int(-45 * 16), int(90 * 16));
        arcRect.adjust(2.8, 2.8, -2.8, -2.8);
    }
}

void drawPlay(QPainter &p, const QColor &c)
{
    QPainterPath path;
    path.moveTo(8.0, 5.6);
    path.lineTo(19.2, 12.0);
    path.lineTo(8.0, 18.4);
    path.closeSubpath();
    p.fillPath(path, c);
}

void drawPause(QPainter &p, const QColor &c)
{
    p.fillPath(roundedRectPath(QRectF(7.4, 5.2, 3.4, 13.6), 1.3), c);
    p.fillPath(roundedRectPath(QRectF(13.2, 5.2, 3.4, 13.6), 1.3), c);
}

void drawPrevious(QPainter &p, const QColor &c)
{
    p.fillPath(roundedRectPath(QRectF(4.6, 5.2, 2.6, 13.6), 1.1), c);
    QPainterPath path;
    path.moveTo(19.0, 12.0);
    path.lineTo(9.0, 5.8);
    path.lineTo(9.0, 18.2);
    path.closeSubpath();
    p.fillPath(path, c);
}

void drawNext(QPainter &p, const QColor &c)
{
    QPainterPath path;
    path.moveTo(5.0, 12.0);
    path.lineTo(15.0, 5.8);
    path.lineTo(15.0, 18.2);
    path.closeSubpath();
    p.fillPath(path, c);
    p.fillPath(roundedRectPath(QRectF(16.8, 5.2, 2.6, 13.6), 1.1), c);
}

void drawPlus(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(12.0, 6.6), QPointF(12.0, 17.4));
    p.drawLine(QPointF(6.6, 12.0), QPointF(17.4, 12.0));
}

void drawTrash(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(QPointF(4.6, 7.0), QPointF(19.4, 7.0));
    p.drawPath(roundedRectPath(QRectF(6.4, 7.0, 11.2, 12.0), 2.0));
    p.drawPath(roundedRectPath(QRectF(9.0, 4.2, 6.0, 2.6), 1.1));
}

void drawLoopArc(QPainter &p, const QColor &c, bool withDot)
{
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::FlatCap, Qt::RoundJoin));
    const QRectF r(5.6, 5.6, 12.8, 12.8);
    // 从 -60° 逆时针画 270°，缺口留在右下角
    p.drawArc(r, int(-60 * 16), int(270 * 16));

    // 弧线终点（210°）及其切线方向
    const qreal rad = qDegreesToRadians(210.0);
    const QPointF end(r.center().x() + r.width() / 2.0 * qCos(rad),
                      r.center().y() - r.height() / 2.0 * qSin(rad));
    const qreal tangent = qRadiansToDegrees(qAtan2(qCos(rad), -qSin(rad)));
    drawArrowHead(p, end, tangent, c, 4.2, 2.4);

    if (withDot) {
        p.setPen(Qt::NoPen);
        p.setBrush(c);
        p.drawEllipse(QPointF(12.0, 12.0), 2.0, 2.0);
    }
}

void drawShuffle(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const QPointF crossA(18.4, 16.6);
    const QPointF crossB(18.4, 7.4);
    p.drawPolyline(QPolygonF({QPointF(4.6, 7.5), QPointF(9.0, 7.5), crossA}));
    p.drawPolyline(QPolygonF({QPointF(4.6, 16.5), QPointF(9.0, 16.5), crossB}));

    p.setPen(Qt::NoPen);
    drawArrowHead(p, crossA, 45.0, c);
    drawArrowHead(p, crossB, -45.0, c);
}

void drawSequential(QPainter &p, const QColor &c)
{
    const QPointF tip(17.0, 12.0);
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(QPointF(4.8, 6.4), QPointF(4.8, 17.6)); // 终点竖线，表示“播完即止”
    p.drawLine(QPointF(4.8, 12.0), tip);
    p.setPen(Qt::NoPen);
    drawArrowHead(p, tip, 0.0, c);
}

void drawPlaylist(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(4.0, 7.0), QPointF(13.6, 7.0));
    p.drawLine(QPointF(4.0, 12.0), QPointF(13.6, 12.0));
    p.drawLine(QPointF(4.0, 17.0), QPointF(13.6, 17.0));
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawEllipse(QPointF(17.6, 15.2), 2.2, 1.7);
    p.drawRect(QRectF(18.8, 6.6, 1.7, 9.8));
}

void drawLogin(QPainter &p, const QColor &c)
{
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawEllipse(QPointF(10.0, 8.6), 3.4, 3.4);
    QPainterPath body;
    body.addRoundedRect(QRectF(4.2, 13.0, 11.6, 8.0), 5.0, 5.0);
    p.drawPath(body);
}

void drawInfo(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap));
    p.drawEllipse(QPointF(12.0, 12.0), 8.4, 8.4);
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawEllipse(QPointF(12.0, 7.6), 1.25, 1.25);
    p.drawRoundedRect(QRectF(10.8, 10.2, 2.4, 6.6), 1.2, 1.2);
}

void drawMusicNote(QPainter &p, const QColor &c)
{
    p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(10.0, 17.0), QPointF(10.0, 5.6));
    p.drawPolyline(QPolygonF({QPointF(10.0, 5.6), QPointF(18.4, 7.6), QPointF(18.4, 12.4)}));
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawEllipse(QPointF(7.6, 18.2), 2.6, 2.1);
    p.drawEllipse(QPointF(16.0, 18.2), 2.6, 2.1);
}

void paint(QPainter &p, Icons::Icon id, const QColor &c)
{
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);
    p.setPen(Qt::NoPen);
    p.setBrush(c);

    switch (id) {
    case Icons::Icon::Play:       drawPlay(p, c); break;
    case Icons::Icon::Pause:      drawPause(p, c); break;
    case Icons::Icon::Previous:   drawPrevious(p, c); break;
    case Icons::Icon::Next:       drawNext(p, c); break;
    case Icons::Icon::Volume:     drawSpeaker(p, c); drawSoundWaves(p, c, 2); break;
    case Icons::Icon::VolumeLow:  drawSpeaker(p, c); drawSoundWaves(p, c, 1); break;
    case Icons::Icon::Muted:
        drawSpeaker(p, c);
        p.setPen(QPen(c, kStroke, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(15.4, 9.4), QPointF(20.2, 15.6));
        p.drawLine(QPointF(20.2, 9.4), QPointF(15.4, 15.6));
        break;
    case Icons::Icon::Add:        drawPlus(p, c); break;
    case Icons::Icon::Remove:     drawTrash(p, c); break;
    case Icons::Icon::Loop:       drawLoopArc(p, c, false); break;
    case Icons::Icon::LoopOne:    drawLoopArc(p, c, true); break;
    case Icons::Icon::Shuffle:    drawShuffle(p, c); break;
    case Icons::Icon::Sequential: drawSequential(p, c); break;
    case Icons::Icon::Playlist:   drawPlaylist(p, c); break;
    case Icons::Icon::Login:      drawLogin(p, c); break;
    case Icons::Icon::Info:       drawInfo(p, c); break;
    case Icons::Icon::MusicNote:  drawMusicNote(p, c); break;
    }
}

qreal renderRatio()
{
    static const qreal ratio = []() -> qreal {
        if (const QScreen *screen = QGuiApplication::primaryScreen())
            return qBound<qreal>(1.0, screen->devicePixelRatio(), 3.0);
        return 1.0;
    }();
    return ratio;
}

} // namespace

QIcon Icons::icon(Icon id, const QColor &color, int size)
{
    const int px = qMax(8, size);
    static QHash<QString, QIcon> cache;

    const QString key = QStringLiteral("%1|%2|%3")
                            .arg(static_cast<int>(id))
                            .arg(color.rgba(), 0, 16)
                            .arg(px);
    const auto it = cache.constFind(key);
    if (it != cache.constEnd())
        return it.value();

    const qreal ratio = renderRatio();
    QPixmap pm(qRound(px * ratio), qRound(px * ratio));
    pm.setDevicePixelRatio(ratio);
    pm.fill(Qt::transparent);

    {
        QPainter p(&pm);
        p.scale(pm.width() / kDesign, pm.height() / kDesign);
        paint(p, id, color);
    }

    QIcon result(pm);
    cache.insert(key, result);
    return result;
}

QIcon Icons::playbackModeIcon(int mode, const QColor &color, int size)
{
    // 与 myMusicPlayer::PlaybackMode 取值对应：0=顺序 1=列表循环 2=随机 3=单曲循环
    switch (mode) {
    case 1:  return icon(Icon::Loop, color, size);
    case 2:  return icon(Icon::Shuffle, color, size);
    case 3:  return icon(Icon::LoopOne, color, size);
    case 0:
    default: return icon(Icon::Sequential, color, size);
    }
}
