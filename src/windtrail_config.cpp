/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "windtrail_config.h"

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

K_PLUGIN_CLASS(KWin::WindTrailConfig)

namespace KWin
{

namespace
{

QColor mixColors(const QColor &a, const QColor &b, qreal amount)
{
    const qreal t = std::clamp(amount, qreal(0.0), qreal(1.0));

    const auto interpolate = [t](qreal first, qreal second) {
        return first * (1.0 - t) + second * t;
    };

    return QColor::fromRgbF(
        interpolate(a.redF(), b.redF()),
        interpolate(a.greenF(), b.greenF()),
        interpolate(a.blueF(), b.blueF()),
        interpolate(a.alphaF(), b.alphaF()));
}

QString colorStyle(const QColor &color)
{
    const qreal luminance = 0.2126 * color.redF()
        + 0.7152 * color.greenF()
        + 0.0722 * color.blueF();

    const QString textColor = luminance > 0.58
        ? QStringLiteral("#151515")
        : QStringLiteral("#ffffff");

    return QStringLiteral(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid rgba(127,127,127,0.7);"
        "  border-radius: 6px;"
        "  padding: 6px 12px;"
        "}")
        .arg(color.name(QColor::HexRgb), textColor);
}

} // namespace

class WindTrailPreview final : public QFrame
{
public:
    explicit WindTrailPreview(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setMinimumHeight(118);
        setFrameShape(QFrame::StyledPanel);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

    void setAppearance(const QColor &color,
                       qreal thickness,
                       qreal intensity)
    {
        m_color = color;
        m_thickness = thickness;
        m_intensity = intensity;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QFrame::paintEvent(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF area = rect().adjusted(22, 18, -22, -18);
        if (area.width() <= 10 || area.height() <= 10) {
            return;
        }

        QPainterPath path;
        path.moveTo(area.left(), area.center().y() + area.height() * 0.22);
        path.cubicTo(area.left() + area.width() * 0.22,
                     area.top() - area.height() * 0.02,
                     area.left() + area.width() * 0.56,
                     area.bottom() + area.height() * 0.08,
                     area.right(),
                     area.center().y() - area.height() * 0.18);

        const QColor halo = mixColors(m_color, QColor(0, 0, 0), 0.28);
        const QColor core = mixColors(m_color, QColor(255, 255, 255), 0.72);

        auto drawLayer = [&](const QColor &source,
                             qreal width,
                             qreal alpha) {
            QColor color = source;
            color.setAlphaF(std::clamp(alpha * m_intensity,
                                       qreal(0.0),
                                       qreal(1.0)));

            QPen pen(color,
                     width * m_thickness,
                     Qt::SolidLine,
                     Qt::RoundCap,
                     Qt::RoundJoin);
            painter.setPen(pen);
            painter.drawPath(path);
        };

        drawLayer(halo, 19.0, 0.12);
        drawLayer(m_color, 9.0, 0.42);
        drawLayer(core, 2.4, 0.82);
    }

private:
    QColor m_color = QColor(255, 6, 12);
    qreal m_thickness = 1.0;
    qreal m_intensity = 1.0;
};

WindTrailConfig::WindTrailConfig(QObject *parent,
                                 const KPluginMetaData &data)
    : KCModule(parent, data)
{
    auto *rootLayout = new QVBoxLayout(widget());

    auto *description = new QLabel(
        QStringLiteral(
            "Customize the wind ribbon that follows fast cursor movement. "
            "Changes are applied when you click Apply."),
        widget());
    description->setWordWrap(true);
    rootLayout->addWidget(description);

    m_preview = new WindTrailPreview(widget());
    rootLayout->addWidget(m_preview);

    auto *form = new QFormLayout();
    rootLayout->addLayout(form);

    m_preset = new QComboBox(widget());
    m_preset->addItem(QStringLiteral("Crimson Slash"));
    m_preset->addItem(QStringLiteral("Wind White"));
    m_preset->addItem(QStringLiteral("Ice Blue"));
    m_preset->addItem(QStringLiteral("Custom"));
    form->addRow(QStringLiteral("Preset:"), m_preset);

    m_colorButton = new QPushButton(widget());
    form->addRow(QStringLiteral("Main color:"), m_colorButton);

    m_thickness = new QDoubleSpinBox(widget());
    m_thickness->setRange(0.35, 3.0);
    m_thickness->setSingleStep(0.05);
    m_thickness->setDecimals(2);
    m_thickness->setSuffix(QStringLiteral("×"));
    form->addRow(QStringLiteral("Thickness:"), m_thickness);

    m_intensity = new QDoubleSpinBox(widget());
    m_intensity->setRange(0.15, 2.0);
    m_intensity->setSingleStep(0.05);
    m_intensity->setDecimals(2);
    m_intensity->setSuffix(QStringLiteral("×"));
    form->addRow(QStringLiteral("Intensity:"), m_intensity);

    m_trailDuration = new QSpinBox(widget());
    m_trailDuration->setRange(120, 650);
    m_trailDuration->setSingleStep(10);
    m_trailDuration->setSuffix(QStringLiteral(" ms"));
    form->addRow(QStringLiteral("Trail length / duration:"), m_trailDuration);

    m_activationSpeed = new QSpinBox(widget());
    m_activationSpeed->setRange(100, 1600);
    m_activationSpeed->setSingleStep(20);
    m_activationSpeed->setSuffix(QStringLiteral(" px/s"));
    form->addRow(QStringLiteral("Minimum speed:"), m_activationSpeed);

    m_smoothness = new QSpinBox(widget());
    m_smoothness->setRange(0, 4);
    m_smoothness->setSingleStep(1);
    m_smoothness->setSpecialValueText(QStringLiteral("No smoothing"));
    form->addRow(QStringLiteral("Curve smoothness:"), m_smoothness);

    m_disableInFullscreen = new QCheckBox(
        QStringLiteral("Disable in full-screen apps and games"),
        widget());
    rootLayout->addWidget(m_disableInFullscreen);

    auto *about = new QLabel(
        QStringLiteral("WindTrail 1.0.0 · by ProxyX · GPL-3.0-or-later"),
        widget());
    about->setAlignment(Qt::AlignRight);
    about->setEnabled(false);
    rootLayout->addWidget(about);
    rootLayout->addStretch(1);

    connect(m_colorButton,
            &QPushButton::clicked,
            this,
            &WindTrailConfig::chooseColor);
    connect(m_preset,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &WindTrailConfig::presetChanged);

    connect(m_thickness,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,
            &WindTrailConfig::settingsChanged);
    connect(m_intensity,
            qOverload<double>(&QDoubleSpinBox::valueChanged),
            this,
            &WindTrailConfig::settingsChanged);
    connect(m_trailDuration,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &WindTrailConfig::settingsChanged);
    connect(m_activationSpeed,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &WindTrailConfig::settingsChanged);
    connect(m_smoothness,
            qOverload<int>(&QSpinBox::valueChanged),
            this,
            &WindTrailConfig::settingsChanged);
    connect(m_disableInFullscreen,
            &QCheckBox::toggled,
            this,
            &WindTrailConfig::settingsChanged);

    load();
}

WindTrailConfig::~WindTrailConfig() = default;

bool WindTrailConfig::Settings::operator==(const Settings &other) const
{
    return color == other.color
        && qFuzzyCompare(thickness + 1.0, other.thickness + 1.0)
        && qFuzzyCompare(intensity + 1.0, other.intensity + 1.0)
        && trailDuration == other.trailDuration
        && activationSpeed == other.activationSpeed
        && smoothness == other.smoothness
        && disableInFullscreen == other.disableInFullscreen;
}

bool WindTrailConfig::Settings::operator!=(const Settings &other) const
{
    return !(*this == other);
}

WindTrailConfig::Settings WindTrailConfig::defaultSettings()
{
    return Settings{
        .color = QColor(255, 6, 12),
        .thickness = 1.0,
        .intensity = 1.0,
        .trailDuration = 330,
        .activationSpeed = 320,
        .smoothness = 2,
        .disableInFullscreen = true,
    };
}

WindTrailConfig::Settings WindTrailConfig::readSettings() const
{
    const KSharedConfigPtr config = KSharedConfig::openConfig(
        QStringLiteral("kwinrc"));
    const KConfigGroup group(
        config,
        QStringLiteral("Effect-proxyx_windtrail"));

    Settings settings = defaultSettings();

    settings.color = group.readEntry("Color", settings.color);
    settings.thickness = group.readEntry(
        "Thickness", settings.thickness);
    settings.intensity = group.readEntry(
        "Intensity", settings.intensity);
    settings.trailDuration = group.readEntry(
        "TrailDuration", settings.trailDuration);
    settings.activationSpeed = group.readEntry(
        "ActivationSpeed", settings.activationSpeed);
    settings.smoothness = group.readEntry(
        "Smoothness", settings.smoothness);
    settings.disableInFullscreen = group.readEntry(
        "DisableInFullscreen", settings.disableInFullscreen);

    return settings;
}

WindTrailConfig::Settings WindTrailConfig::currentSettings() const
{
    return Settings{
        .color = m_color,
        .thickness = m_thickness->value(),
        .intensity = m_intensity->value(),
        .trailDuration = m_trailDuration->value(),
        .activationSpeed = m_activationSpeed->value(),
        .smoothness = m_smoothness->value(),
        .disableInFullscreen = m_disableInFullscreen->isChecked(),
    };
}

void WindTrailConfig::applySettings(const Settings &settings)
{
    m_loading = true;

    m_color = settings.color.isValid()
        ? settings.color
        : defaultSettings().color;

    m_preset->setCurrentIndex(presetForColor(m_color));
    m_thickness->setValue(settings.thickness);
    m_intensity->setValue(settings.intensity);
    m_trailDuration->setValue(settings.trailDuration);
    m_activationSpeed->setValue(settings.activationSpeed);
    m_smoothness->setValue(settings.smoothness);
    m_disableInFullscreen->setChecked(
        settings.disableInFullscreen);

    updateColorButton();
    updatePreview();

    m_loading = false;
}

void WindTrailConfig::load()
{
    const Settings settings = readSettings();
    applySettings(settings);
    m_savedSettings = currentSettings();

    setNeedsSave(false);
    setRepresentsDefaults(
        m_savedSettings == defaultSettings());
}

void WindTrailConfig::save()
{
    const Settings settings = currentSettings();

    const KSharedConfigPtr config = KSharedConfig::openConfig(
        QStringLiteral("kwinrc"));
    KConfigGroup group(
        config,
        QStringLiteral("Effect-proxyx_windtrail"));

    group.writeEntry("Color", settings.color);
    group.writeEntry("Thickness", settings.thickness);
    group.writeEntry("Intensity", settings.intensity);
    group.writeEntry("TrailDuration", settings.trailDuration);
    group.writeEntry("ActivationSpeed", settings.activationSpeed);
    group.writeEntry("Smoothness", settings.smoothness);
    group.writeEntry("DisableInFullscreen",
                     settings.disableInFullscreen);
    group.sync();
    config->sync();

    QDBusInterface interface(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/Effects"),
        QStringLiteral("org.kde.kwin.Effects"),
        QDBusConnection::sessionBus());

    interface.call(
        QDBus::NoBlock,
        QStringLiteral("reconfigureEffect"),
        QStringLiteral("proxyx_windtrail"));

    m_savedSettings = settings;
    setNeedsSave(false);
    setRepresentsDefaults(
        settings == defaultSettings());
}

void WindTrailConfig::defaults()
{
    applySettings(defaultSettings());
    updateState();
}

void WindTrailConfig::chooseColor()
{
    const QColor selected = QColorDialog::getColor(
        m_color,
        widget(),
        QStringLiteral("Choose WindTrail color"));

    if (!selected.isValid()) {
        return;
    }

    m_color = selected;
    m_preset->setCurrentIndex(presetForColor(m_color));
    updateColorButton();
    settingsChanged();
}

void WindTrailConfig::presetChanged(int index)
{
    if (m_loading) {
        return;
    }

    if (index == 0) {
        m_color = QColor(255, 6, 12);
    } else if (index == 1) {
        m_color = QColor(238, 242, 247);
    } else if (index == 2) {
        m_color = QColor(67, 184, 255);
    } else {
        updateState();
        return;
    }

    updateColorButton();
    settingsChanged();
}

void WindTrailConfig::settingsChanged()
{
    if (m_loading) {
        return;
    }

    updatePreview();
    updateState();
}

void WindTrailConfig::updateColorButton()
{
    m_colorButton->setText(
        m_color.name(QColor::HexRgb).toUpper());
    m_colorButton->setStyleSheet(colorStyle(m_color));
}

void WindTrailConfig::updatePreview()
{
    m_preview->setAppearance(
        m_color,
        m_thickness->value(),
        m_intensity->value());
}

void WindTrailConfig::updateState()
{
    const Settings current = currentSettings();
    setNeedsSave(current != m_savedSettings);
    setRepresentsDefaults(current == defaultSettings());
}

int WindTrailConfig::presetForColor(const QColor &color) const
{
    if (color.rgb() == QColor(255, 6, 12).rgb()) {
        return 0;
    }

    if (color.rgb() == QColor(238, 242, 247).rgb()) {
        return 1;
    }

    if (color.rgb() == QColor(67, 184, 255).rgb()) {
        return 2;
    }

    return 3;
}

} // namespace KWin

#include "windtrail_config.moc"
#include "moc_windtrail_config.cpp"
