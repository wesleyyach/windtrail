kcoreaddons_add_plugin(proxyx_windtrail
    INSTALL_NAMESPACE "kwin/effects/plugins"
)

target_sources(proxyx_windtrail PRIVATE
    main.cpp
    windtrail.cpp
    windtrail.h
)

# CMake's automoc scanner does not know this KWin macro by default.
set_property(TARGET proxyx_windtrail APPEND PROPERTY
    AUTOMOC_MACRO_NAMES KWIN_EFFECT_FACTORY)

target_link_libraries(proxyx_windtrail PRIVATE
    KF6::CoreAddons
    KF6::ConfigCore
    KWin::kwin
)

kcoreaddons_add_plugin(kwin_proxyx_windtrail_config
    INSTALL_NAMESPACE "kwin/effects/configs"
)

target_sources(kwin_proxyx_windtrail_config PRIVATE
    windtrail_config.cpp
    windtrail_config.h
)

target_link_libraries(kwin_proxyx_windtrail_config PRIVATE
    Qt6::DBus
    Qt6::Gui
    Qt6::Widgets
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::KCMUtils
)
