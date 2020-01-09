macro_optional_find_package(Cdparanoia)
macro_log_feature(CDPARANOIA_FOUND "cdparanoia" "Reads digital audio directly from a CD, then writes the data to a file or pipe in several different formats" "http://www.xiph.org/paranoia/" FALSE "" "Required for building the AudioCD ioslave.")


if (CDPARANOIA_FOUND)
    add_subdirectory( audiocd )
endif(CDPARANOIA_FOUND)
