/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <KCModule>

#include <QColor>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QPushButton;
class QSpinBox;

namespace KWin
{

class WindTrailPreview;

class WindTrailConfig final : public KCModule
{
    Q_OBJECT

public:
    explicit WindTrailConfig(QObject *parent,
                             const KPluginMetaData &data);
    ~WindTrailConfig() override;

    void load() override;
    void save() override;
    void defaults() override;

private Q_SLOTS:
    void chooseColor();
    void presetChanged(int index);
    void settingsChanged();

private:
    struct Settings {
        QColor color;
        qreal thickness = 1.0;
        qreal intensity = 1.0;
        int trailDuration = 330;
        int activationSpeed = 320;
        int smoothness = 2;
        bool disableInFullscreen = true;

        bool operator==(const Settings &other) const;
        bool operator!=(const Settings &other) const;
    };

    static Settings defaultSettings();
    Settings readSettings() const;
    Settings currentSettings() const;
    void applySettings(const Settings &settings);
    void updateColorButton();
    void updatePreview();
    void updateState();
    int presetForColor(const QColor &color) const;

    QComboBox *m_preset = nullptr;
    QPushButton *m_colorButton = nullptr;
    QDoubleSpinBox *m_thickness = nullptr;
    QDoubleSpinBox *m_intensity = nullptr;
    QSpinBox *m_trailDuration = nullptr;
    QSpinBox *m_activationSpeed = nullptr;
    QSpinBox *m_smoothness = nullptr;
    QCheckBox *m_disableInFullscreen = nullptr;
    WindTrailPreview *m_preview = nullptr;

    QColor m_color;
    Settings m_savedSettings;
    bool m_loading = false;
};

} // namespace KWin
