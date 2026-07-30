/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "windtrail.h"

#include "core/colorspace.h"
#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "effect/effecthandler.h"
#include "effect/effectwindow.h"
#include "opengl/glshader.h"
#include "opengl/glshadermanager.h"
#include "opengl/glvertexbuffer.h"

#include <KConfigGroup>

#include <QColor>
#include <QList>
#include <QVector2D>

#include <epoxy/gl.h>

#include <algorithm>
#include <array>
#include <cmath>

namespace KWin
{

namespace
{

qreal clamp01(qreal value)
{
    return std::clamp(value, qreal(0.0), qreal(1.0));
}

qreal smoothStep(qreal value)
{
    const qreal x = clamp01(value);
    return x * x * (3.0 - 2.0 * x);
}

void appendQuad(QList<QVector2D> &vertices,
                const QPointF &a,
                const QPointF &b,
                qreal widthA,
                qreal widthB,
                qreal scale)
{
    const QPointF direction = b - a;
    const qreal length = std::hypot(direction.x(), direction.y());

    if (length < 0.001) {
        return;
    }

    const QPointF normal(-direction.y() / length, direction.x() / length);
    const QPointF offsetA = normal * (widthA * 0.5);
    const QPointF offsetB = normal * (widthB * 0.5);

    const QPointF aLeft = a + offsetA;
    const QPointF aRight = a - offsetA;
    const QPointF bLeft = b + offsetB;
    const QPointF bRight = b - offsetB;

    vertices.push_back(QVector2D(aLeft.x() * scale, aLeft.y() * scale));
    vertices.push_back(QVector2D(aRight.x() * scale, aRight.y() * scale));
    vertices.push_back(QVector2D(bRight.x() * scale, bRight.y() * scale));

    vertices.push_back(QVector2D(aLeft.x() * scale, aLeft.y() * scale));
    vertices.push_back(QVector2D(bRight.x() * scale, bRight.y() * scale));
    vertices.push_back(QVector2D(bLeft.x() * scale, bLeft.y() * scale));
}

} // namespace

WindTrailEffect::WindTrailEffect()
{
    m_clock.start();

    connect(effects,
            &EffectsHandler::mouseChanged,
            this,
            &WindTrailEffect::onMouseChanged);

    reconfigure(ReconfigureAll);
}

WindTrailEffect::~WindTrailEffect() = default;

void WindTrailEffect::reconfigure(ReconfigureFlags)
{
    const KConfigGroup group(effects->config(),
                             QStringLiteral("Effect-proxyx_windtrail"));

    m_baseColor = group.readEntry("Color", QColor(255, 6, 12));
    if (!m_baseColor.isValid()) {
        m_baseColor = QColor(255, 6, 12);
    }
    // Opacity is controlled by the Intensity setting, not the color picker.
    m_baseColor.setAlpha(255);

    m_thickness = std::clamp(group.readEntry("Thickness", 1.0), 0.35, 3.0);
    m_intensity = std::clamp(group.readEntry("Intensity", 1.0), 0.15, 2.0);
    m_activationSpeed = std::clamp(
        qreal(group.readEntry("ActivationSpeed", 320)),
        qreal(100.0),
        qreal(1600.0));
    m_trailDurationMs = std::clamp<qint64>(
        group.readEntry("TrailDuration", 330),
        120,
        650);
    m_smoothingPasses = std::clamp(
        group.readEntry("Smoothness", 2),
        0,
        4);
    m_disableInFullscreen = group.readEntry("DisableInFullscreen", true);

    clearTrail();
    effects->addRepaintFull();
}

void WindTrailEffect::prePaintScreen(ScreenPrePaintData &data)
{
    if (isSuppressed()) {
        clearTrail();
    } else {
        pruneExpired(m_clock.elapsed());
    }

    effects->prePaintScreen(data);
}

void WindTrailEffect::paintScreen(const RenderTarget &renderTarget,
                                  const RenderViewport &viewport,
                                  int mask,
                                  const Region &deviceRegion,
                                  LogicalOutput *screen)
{
    effects->paintScreen(renderTarget, viewport, mask, deviceRegion, screen);

    if (!effects->isOpenGLCompositing()
        || m_samples.size() < 2
        || isSuppressed()) {
        return;
    }

    const qint64 nowMs = m_clock.elapsed();
    const std::vector<Sample> samples = smoothedSamples();

    if (samples.size() < 2) {
        return;
    }

    const QColor haloColor = mixColors(m_baseColor, QColor(0, 0, 0), 0.28);
    const QColor bodyColor = m_baseColor;
    const QColor coreColor = mixColors(m_baseColor, QColor(255, 255, 255), 0.72);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawRibbonPass(samples,
                   renderTarget,
                   viewport,
                   2.05 * m_thickness,
                   haloColor,
                   0.11 * m_intensity,
                   nowMs);

    drawRibbonPass(samples,
                   renderTarget,
                   viewport,
                   1.00 * m_thickness,
                   bodyColor,
                   0.38 * m_intensity,
                   nowMs);

    drawRibbonPass(samples,
                   renderTarget,
                   viewport,
                   0.27 * m_thickness,
                   coreColor,
                   0.78 * m_intensity,
                   nowMs);

    glDisable(GL_BLEND);
}

void WindTrailEffect::postPaintScreen()
{
    effects->postPaintScreen();

    if (!m_samples.empty() && !isSuppressed()) {
        effects->addRepaintFull();
    }
}

bool WindTrailEffect::isActive() const
{
    return !isSuppressed() && m_samples.size() >= 2;
}

void WindTrailEffect::onMouseChanged(const QPointF &pos,
                                     const QPointF &oldPos,
                                     Qt::MouseButtons,
                                     Qt::MouseButtons,
                                     Qt::KeyboardModifiers,
                                     Qt::KeyboardModifiers)
{
    if (isSuppressed()) {
        clearTrail();
        return;
    }

    const qint64 nowMs = m_clock.elapsed();

    if (!m_haveLastPosition) {
        m_lastPosition = oldPos;
        m_lastEventMs = nowMs;
        m_haveLastPosition = true;
    }

    const QPointF delta = pos - m_lastPosition;
    const qreal distance = std::hypot(delta.x(), delta.y());
    const qint64 elapsedMs = std::max<qint64>(1, nowMs - m_lastEventMs);
    const qreal rawSpeed = distance * 1000.0 / qreal(elapsedMs);

    if (distance > MaximumJumpDistance) {
        clearTrail();
        m_haveLastPosition = true;
        m_lastPosition = pos;
        m_lastEventMs = nowMs;
        return;
    }

    if (m_filteredSpeed <= 0.0) {
        m_filteredSpeed = rawSpeed;
    } else {
        constexpr qreal NewSampleWeight = 0.46;
        m_filteredSpeed = m_filteredSpeed * (1.0 - NewSampleWeight)
            + rawSpeed * NewSampleWeight;
    }

    if (distance >= MinimumDistance) {
        if (m_samples.empty() && m_filteredSpeed >= m_activationSpeed) {
            m_samples.push_back(Sample{
                .position = m_lastPosition,
                .timestampMs = std::max<qint64>(0, nowMs - elapsedMs),
                .speed = m_filteredSpeed,
            });
        }

        if (!m_samples.empty() || m_filteredSpeed >= m_activationSpeed) {
            m_samples.push_back(Sample{
                .position = pos,
                .timestampMs = nowMs,
                .speed = m_filteredSpeed,
            });
        }
    }

    while (m_samples.size() > MaximumSamples) {
        m_samples.pop_front();
    }

    pruneExpired(nowMs);

    m_lastPosition = pos;
    m_lastEventMs = nowMs;

    if (!m_samples.empty()) {
        effects->addRepaintFull();
    }
}

bool WindTrailEffect::isSuppressed() const
{
    if (effects->isScreenLocked()) {
        return true;
    }

    if (m_disableInFullscreen) {
        EffectWindow *window = effects->activeWindow();
        if (window && window->isFullScreen()) {
            return true;
        }
    }

    return false;
}

void WindTrailEffect::clearTrail()
{
    m_samples.clear();
    m_filteredSpeed = 0.0;
    m_haveLastPosition = false;
}

void WindTrailEffect::pruneExpired(qint64 nowMs)
{
    while (!m_samples.empty()
           && nowMs - m_samples.front().timestampMs > m_trailDurationMs) {
        m_samples.pop_front();
    }

    if (m_samples.size() == 1
        && nowMs - m_samples.front().timestampMs > m_trailDurationMs / 2) {
        m_samples.clear();
    }
}

std::vector<WindTrailEffect::Sample> WindTrailEffect::smoothedSamples() const
{
    std::vector<Sample> current(m_samples.begin(), m_samples.end());

    if (current.size() < 3 || m_smoothingPasses <= 0) {
        return current;
    }

    const auto chaikinPass = [](const std::vector<Sample> &input) {
        std::vector<Sample> output;
        output.reserve(input.size() * 2);
        output.push_back(input.front());

        for (std::size_t index = 0; index + 1 < input.size(); ++index) {
            const Sample &a = input[index];
            const Sample &b = input[index + 1];

            output.push_back(Sample{
                .position = interpolatePoint(a.position, b.position, 0.25),
                .timestampMs = qRound64(interpolateValue(
                    qreal(a.timestampMs), qreal(b.timestampMs), 0.25)),
                .speed = interpolateValue(a.speed, b.speed, 0.25),
            });

            output.push_back(Sample{
                .position = interpolatePoint(a.position, b.position, 0.75),
                .timestampMs = qRound64(interpolateValue(
                    qreal(a.timestampMs), qreal(b.timestampMs), 0.75)),
                .speed = interpolateValue(a.speed, b.speed, 0.75),
            });
        }

        output.push_back(input.back());
        return output;
    };

    for (int pass = 0; pass < m_smoothingPasses; ++pass) {
        current = chaikinPass(current);
    }

    return current;
}

void WindTrailEffect::drawRibbonPass(const std::vector<Sample> &samples,
                                     const RenderTarget &renderTarget,
                                     const RenderViewport &viewport,
                                     qreal widthScale,
                                     const QColor &baseColor,
                                     qreal baseAlpha,
                                     qint64 nowMs)
{
    if (samples.size() < 2) {
        return;
    }

    const qint64 newestAgeMs = std::max<qint64>(
        0, nowMs - samples.back().timestampMs);
    const qreal newestLifetime = lifetimeForSpeed(samples.back().speed);
    const qreal fadeDuration = stopFadeDuration();
    const qreal fadeStartMs = std::max<qreal>(
        0.0, newestLifetime - fadeDuration);

    qreal stopFade = 1.0;
    if (qreal(newestAgeMs) > fadeStartMs) {
        stopFade = clamp01(
            (newestLifetime - qreal(newestAgeMs)) / fadeDuration);
    }

    if (stopFade <= 0.001) {
        return;
    }

    std::array<QList<QVector2D>, 8> buckets;
    const qreal scale = viewport.scale();
    const qreal widthSpeed = fullWidthSpeed();

    for (std::size_t index = 0; index + 1 < samples.size(); ++index) {
        const Sample &a = samples[index];
        const Sample &b = samples[index + 1];

        const qreal ageA = qreal(std::max<qint64>(
            0, nowMs - a.timestampMs));
        const qreal ageB = qreal(std::max<qint64>(
            0, nowMs - b.timestampMs));

        const qreal lifeA = clamp01(
            1.0 - ageA / lifetimeForSpeed(a.speed));
        const qreal lifeB = clamp01(
            1.0 - ageB / lifetimeForSpeed(b.speed));

        const qreal speedA = smoothStep(
            (a.speed - m_activationSpeed)
            / (widthSpeed - m_activationSpeed));
        const qreal speedB = smoothStep(
            (b.speed - m_activationSpeed)
            / (widthSpeed - m_activationSpeed));

        const qreal tailA = std::pow(smoothStep(lifeA), 1.55);
        const qreal tailB = std::pow(smoothStep(lifeB), 1.55);

        const qreal responsiveSpeedA = std::sqrt(speedA);
        const qreal responsiveSpeedB = std::sqrt(speedB);

        const qreal widthA = widthScale
            * (0.18 + 17.5 * responsiveSpeedA * tailA);
        const qreal widthB = widthScale
            * (0.18 + 17.5 * responsiveSpeedB * tailB);

        const qreal segmentLife = (tailA + tailB) * 0.5;
        if (segmentLife <= 0.002) {
            continue;
        }

        const int bucket = std::clamp(
            int(std::floor(segmentLife * qreal(buckets.size()))),
            0,
            int(buckets.size()) - 1);

        appendQuad(buckets[bucket],
                   a.position,
                   b.position,
                   widthA,
                   widthB,
                   scale);
    }

    ShaderBinder binder(ShaderTrait::UniformColor
                        | ShaderTrait::TransformColorspace);
    binder.shader()->setUniform(
        GLShader::Mat4Uniform::ModelViewProjectionMatrix,
        viewport.projectionMatrix());
    binder.shader()->setColorspaceUniforms(
        ColorDescription::sRGB,
        renderTarget.colorDescription(),
        RenderingIntent::Perceptual);

    GLVertexBuffer *vbo = GLVertexBuffer::streamingBuffer();

    for (std::size_t bucket = 0; bucket < buckets.size(); ++bucket) {
        QList<QVector2D> &vertices = buckets[bucket];

        if (vertices.isEmpty()) {
            continue;
        }

        const qreal bucketOpacity = std::pow(
            qreal(bucket + 1) / qreal(buckets.size()),
            1.75);

        QColor color = baseColor;
        color.setAlphaF(clamp01(baseAlpha * bucketOpacity * stopFade));

        binder.shader()->setUniform(
            GLShader::ColorUniform::Color,
            color);

        vbo->reset();
        vbo->setVertices(vertices);
        vbo->render(GL_TRIANGLES);
    }
}

QPointF WindTrailEffect::interpolatePoint(const QPointF &a,
                                          const QPointF &b,
                                          qreal t)
{
    return a * (1.0 - t) + b * t;
}

qreal WindTrailEffect::interpolateValue(qreal a, qreal b, qreal t)
{
    return a * (1.0 - t) + b * t;
}

QColor WindTrailEffect::mixColors(const QColor &a,
                                  const QColor &b,
                                  qreal amount)
{
    const qreal t = clamp01(amount);

    return QColor::fromRgbF(
        interpolateValue(a.redF(), b.redF(), t),
        interpolateValue(a.greenF(), b.greenF(), t),
        interpolateValue(a.blueF(), b.blueF(), t),
        interpolateValue(a.alphaF(), b.alphaF(), t));
}

qreal WindTrailEffect::lifetimeForSpeed(qreal speed) const
{
    const qreal normalized = smoothStep(
        (speed - m_activationSpeed)
        / (fullWidthSpeed() - m_activationSpeed));

    const qreal minimumLifetime = std::max<qreal>(
        90.0, qreal(m_trailDurationMs) * 0.53);

    return interpolateValue(
        minimumLifetime,
        qreal(m_trailDurationMs),
        normalized);
}

qreal WindTrailEffect::fullWidthSpeed() const
{
    return m_activationSpeed + 1530.0;
}

qreal WindTrailEffect::stopFadeDuration() const
{
    return std::clamp(
        qreal(m_trailDurationMs) * 0.44,
        qreal(65.0),
        qreal(220.0));
}

} // namespace KWin

#include "moc_windtrail.cpp"
