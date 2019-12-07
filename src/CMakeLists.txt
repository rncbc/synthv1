set(NAME synthv1)

include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file (cmake_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set (HEADERS
  synthv1.h
  synthv1_config.h
  synthv1_filter.h
  synthv1_formant.h
  synthv1_wave.h
  synthv1_ramp.h
  synthv1_list.h
  synthv1_fx.h
  synthv1_reverb.h
  synthv1_param.h
  synthv1_sched.h
  synthv1_tuning.h
  synthv1_programs.h
  synthv1_controls.h
)

set (SOURCES
  synthv1.cpp
  synthv1_config.cpp
  synthv1_formant.cpp
  synthv1_wave.cpp
  synthv1_param.cpp
  synthv1_sched.cpp
  synthv1_tuning.cpp
  synthv1_programs.cpp
  synthv1_controls.cpp
)

qt5_wrap_cpp (MOC_SOURCES ${HEADERS})


set (HEADERS_UI
  synthv1_ui.h
  synthv1widget.h
  synthv1widget_env.h
  synthv1widget_filt.h
  synthv1widget_wave.h
  synthv1widget_param.h
  synthv1widget_keybd.h
  synthv1widget_preset.h
  synthv1widget_status.h
  synthv1widget_programs.h
  synthv1widget_controls.h
  synthv1widget_control.h
  synthv1widget_palette.h
  synthv1widget_config.h
)

set (SOURCES_UI
  synthv1_ui.cpp
  synthv1widget.cpp
  synthv1widget_env.cpp
  synthv1widget_filt.cpp
  synthv1widget_wave.cpp
  synthv1widget_param.cpp
  synthv1widget_keybd.cpp
  synthv1widget_preset.cpp
  synthv1widget_status.cpp
  synthv1widget_programs.cpp
  synthv1widget_controls.cpp
  synthv1widget_control.cpp
  synthv1widget_palette.cpp
  synthv1widget_config.cpp
)

set (FORMS_UI
  synthv1widget.ui
  synthv1widget_control.ui
  synthv1widget_config.ui
)

set (RESOURCES_UI
  synthv1.qrc
)

qt5_wrap_ui (UI_SOURCES_UI ${FORMS_UI})
qt5_wrap_cpp (MOC_SOURCES_UI ${HEADERS_UI})
qt5_add_resources (QRC_SOURCES_UI ${RESOURCES_UI})


set (HEADERS_LV2
  synthv1_lv2.h
  synthv1_lv2ui.h
  synthv1widget_lv2.h
)

set (SOURCES_LV2
  synthv1_lv2.cpp
  synthv1_lv2ui.cpp
  synthv1widget_lv2.cpp
)

qt5_wrap_cpp (MOC_SOURCES_LV2 ${HEADERS_LV2})


set (HEADERS_JACK
  synthv1_nsm.h
  synthv1_jack.h
  synthv1widget_jack.h
)

set (SOURCES_JACK
  synthv1_nsm.cpp
  synthv1_jack.cpp
  synthv1widget_jack.cpp
)

qt5_wrap_cpp (MOC_SOURCES_JACK ${HEADERS_JACK})


add_library (${NAME} STATIC
  ${MOC_SOURCES}
  ${SOURCES}
)

add_library (${NAME}_ui STATIC
  ${UI_SOURCES_UI}
  ${MOC_SOURCES_UI}
  ${QRC_SOURCES_UI}
  ${SOURCES_UI}
)

add_library (${NAME}_lv2 MODULE
  ${MOC_SOURCES_LV2}
  ${SOURCES_LV2}
)

add_executable (${NAME}_jack
  ${MOC_SOURCES_JACK}
  ${SOURCES_JACK}
)


set_target_properties (${NAME}       PROPERTIES CXX_STANDARD 11)
set_target_properties (${NAME}_ui    PROPERTIES CXX_STANDARD 11)
set_target_properties (${NAME}_lv2   PROPERTIES CXX_STANDARD 11)
set_target_properties (${NAME}_jack  PROPERTIES CXX_STANDARD 11)

target_link_libraries (${NAME}       PUBLIC Qt5::Core Qt5::Xml)
target_link_libraries (${NAME}_ui    PUBLIC Qt5::Widgets ${NAME})
target_link_libraries (${NAME}_lv2   PRIVATE ${NAME}_ui)
target_link_libraries (${NAME}_jack  PRIVATE ${NAME}_ui)

if (CONFIG_JACK)
  target_link_libraries (${NAME}_jack PRIVATE ${JACK_LIBRARIES})
endif ()

if (CONFIG_ALSA_MIDI)
  target_link_libraries (${NAME}_jack PRIVATE ${ALSA_LIBRARIES})
endif ()

if (CONFIG_LIBLO)
  target_link_libraries (${NAME}_jack PRIVATE ${LIBLO_LIBRARIES})
endif ()


if (UNIX AND NOT APPLE AND NOT CONFIG_DEBUG)
  add_custom_command(TARGET ${NAME}_lv2  POST_BUILD
    COMMAND strip -v lib${NAME}_lv2.so)
  add_custom_command(TARGET ${NAME}_jack POST_BUILD
    COMMAND strip -v ${NAME}_jack)
endif ()


if (UNIX AND NOT APPLE)
  if (CONFIG_PREFIX MATCHES $ENV{HOME})
    set (CONFIG_LV2DIR ${CONFIG_PREFIX}/.lv2)
  else ()
    set (CONFIG_LV2DIR ${CONFIG_LIBDIR}/lv2)
  endif ()
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${NAME}_lv2.so
     RENAME ${NAME}.so
     DESTINATION ${CONFIG_LV2DIR}/${NAME}.lv2)
  install (FILES ${NAME}.lv2/manifest.ttl
     ${NAME}.lv2/${NAME}.ttl
     ${NAME}.lv2/${NAME}_ui.ttl
     DESTINATION ${CONFIG_LV2DIR}/${NAME}.lv2)
  install (TARGETS ${NAME}_jack RUNTIME
     DESTINATION ${CMAKE_INSTALL_BINDIR})
  install (FILES ${NAME}.desktop
     DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install (FILES images/${NAME}.png
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)
  install (FILES images/${NAME}.svg
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
  install (FILES appdata/${NAME}.appdata.xml
     DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
  install (FILES mimetypes/${NAME}.xml
     DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages)
  install (FILES mimetypes/application-x-${NAME}-preset.png
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/mimetypes)
  install (FILES mimetypes/application-x-${NAME}-preset.svg
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/mimetypes)
endif ()
