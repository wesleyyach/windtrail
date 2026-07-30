/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "effect/effect.h"

#include <QColor>
#include <QElapsedTimer>
#include <QPointF>

#include <deque>
#include <vector>

namespace KWin
{

class WindTrailEffect final : public Effect
{
    Q_OBJECT

public:
    WindTrailEffect();
    ~WindTrailEffect() override;

    void reconfigure(ReconfigureFlags flags) override;
    void prePaintScreen(ScreenPrePaintData &data) override;
    void paintScreen(const RenderTarget &renderTarget,
                     const RenderViewport &viewport,
                     int mask,
                     const Region &deviceRegion,
                     LogicalOutput *screen) override;
    void postPaintScreen() override;

    bool isActive() const override;

private Q_SLOTS:
    void onMouseChanged(const QPointF &pos,
                        const QPointF &oldPos,
                        Qt::MouseButtons buttons,
                        Qt::MouseButtons oldButtons,
                        Qt::KeyboardModifiers modifiers,
                        Qt::KeyboardModifiers oldModifiers);

private:
    struct Sample {
        QPointF position;
        qint64 timestampMs = 0;
        qreal speed = 0.0;
    };

    bool isSuppressed() const;
    void clearTrail();
    void pruneExpired(qint64 nowMs);
    std::vector<Sample> smoothedSamples() const;

    void drawRibbonPass(const std::vector<Sample> &samples,
                        const RenderTarget &renderTarget,
                        const RenderViewport &viewport,
                        qreal widthScale,
                        const QColor &baseColor,
                        qreal baseAlpha,
                        qint64 nowMs);

    static QPointF interpolatePoint(const QPointF &a, const QPointF &b, qreal t);
    static qreal interpolateValue(qreal a, qreal b, qreal t);
    static QColor mixColors(const QColor &a, const QColor &b, qreal amount);

    qreal lifetimeForSpeed(qreal speed) const;
    qreal fullWidthSpeed() const;
    qreal stopFadeDuration() const;

    std::deque<Sample> m_samples;
    QElapsedTimer m_clock;

    QPointF m_lastPosition;
    qint64 m_lastEventMs = 0;
    qreal m_filteredSpeed = 0.0;
    bool m_haveLastPosition = false;

    QColor m_baseColor = QColor(255, 6, 12);
    qreal m_thickness = 1.0;
    qreal m_intensity = 1.0;
    qreal m_activationSpeed = 320.0;
    qint64 m_trailDurationMs = 330;
    int m_smoothingPasses = 2;
    bool m_disableInFullscreen = true;

    static constexpr int MaximumSamples = 84;
    static constexpr qreal MinimumDistance = 1.25;
    static constexpr qreal MaximumJumpDistance = 420.0;
};

} // namespace KWin
