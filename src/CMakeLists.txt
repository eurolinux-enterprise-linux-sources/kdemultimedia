project(kdemultimedia)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
find_package(Alsa)

set(PULSEAUDIO_MINIMUM_VERSION "0.9.12")
macro_optional_find_package(PulseAudio)
macro_log_feature(PULSEAUDIO_FOUND "PulseAudio" "PulseAudio Audio Server" "http://www.pulseaudio.org/" FALSE "0.9.12" "libpulse is needed to let KMix control PulseAudio")
find_package(GLIB2)
macro_optional_find_package(MusicBrainz)
macro_log_feature(MUSICBRAINZ_FOUND "MusicBrainz" "A library that provides access to metadata lookup on the MusicBrainz server" "http://www.musicbrainz.org" FALSE "" "Music metadata lookup for KDE multimedia applications. You need version 2.x of libmusicbrainz.")

alsa_configure_file(${CMAKE_BINARY_DIR}/config-alsa.h)

macro_optional_find_package(Xine)
macro_log_feature(XINE_XCB_FOUND "Xine XCB" "Xine libraries" "http://xinehq.de/" FALSE "" "Xine and phonon-xine must be compiled with XCB support.")

macro_optional_find_package(Taglib)
macro_log_feature(TAGLIB_FOUND "taglib" "Library for reading and editing audio meta data." "http://ktown.kde.org/~wheeler/taglib/" FALSE "1.4" "Required to build Juk and many Strigi Analyzers.")

macro_optional_find_package(OggVorbis)
macro_log_feature(OGGVORBIS_FOUND "OggVorbis" "Libraries for Ogg Vorbis general audio compression codec" "http://www.xiph.org" FALSE "" "Required for the AudioCD Ogg Vorbis plugin.")

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
if(MSVC)
    include_directories( ${TAGLIB_INCLUDES} )
endif(MSVC)

include (ConfigureChecks.cmake)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

add_subdirectory(cmake)
macro_optional_add_subdirectory(doc)
add_subdirectory(libkcddb)
add_subdirectory(libkcompactdisc)
add_subdirectory(mplayerthumbs)

if (TAGLIB_FOUND)
  macro_optional_add_subdirectory(juk)
endif(TAGLIB_FOUND)

if (NOT APPLE)
  if(MUSICBRAINZ_FOUND)
     macro_optional_add_subdirectory(kscd)
  endif(MUSICBRAINZ_FOUND)
endif (NOT APPLE)

macro_optional_add_subdirectory(strigi-analyzer)
macro_optional_add_subdirectory(kioslave)

if(NOT WIN32)
  find_package(LibKNotificationItem-1)
  macro_log_feature(LIBKNOTIFICATIONITEM-1_FOUND "LibKNotificationItem-1" "An experimental library" "svn://anonsvn.kde.org/home/kde/trunk/KDE/kdelibs/experimental" TRUE "" "LibKNotificationItem-1 is an unfinished library which is required by kmix.")

  if (LIBKNOTIFICATIONITEM-1_FOUND)
     macro_optional_add_subdirectory(kmix)
  endif (LIBKNOTIFICATIONITEM-1_FOUND)
endif(NOT WIN32)
macro_optional_add_subdirectory(dragonplayer)

macro_display_feature_log()
