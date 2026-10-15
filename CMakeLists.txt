cmake_minimum_required(VERSION 3.16)
project(G_htop VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(packaging/cpack/Packaging.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
find_package(Threads REQUIRED)

add_executable(G_htop main.cpp)

target_link_libraries(G_htop PRIVATE PkgConfig::GTKMM Threads::Threads)

install(TARGETS G_htop
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES data/org.ghtop.Monitor.desktop
  DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

install(FILES data/icons/G_htop.svg
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

install(FILES LICENSE NOTICE
  DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/g_htop)

install(FILES README.md CHANGELOG.md LICENSES.md RELEASE_REVIEW.md
  RELEASE_NOTES.md RELEASE_READINESS_REVIEW.md LICENSE
  DESTINATION ${CMAKE_INSTALL_DOCDIR})

install(FILES docs/STAGED_LOCAL_LLM_DEVELOPMENT.md docs/DEVELOPMENT_OVERVIEW.md
  DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs)
