cmake_minimum_required(VERSION 3.16)
project(G_htop VERSION 1.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(packaging/cpack/Packaging.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
find_package(Threads REQUIRED)

add_executable(G_htop main.cpp)

target_link_libraries(G_htop PRIVATE PkgConfig::GTKMM Threads::Threads ${CMAKE_DL_LIBS})
if(WIN32)
  target_compile_definitions(G_htop PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0A00)
  target_link_libraries(G_htop PRIVATE advapi32 psapi iphlpapi ws2_32 pdh dxgi dxguid)
  set_target_properties(G_htop PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
target_compile_definitions(G_htop PRIVATE G_HTOP_VERSION="${PROJECT_VERSION}")

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

# Community hardware fixtures and focused GTK acceptance tools.
include(CTest)
if(BUILD_TESTING AND NOT WIN32)
  add_library(community_nvml_fixture SHARED tests/nvml_fixture.cpp)
  add_executable(community_gpu_tests tests/gpu_tests.cpp)
  target_include_directories(community_gpu_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(community_gpu_tests PRIVATE PkgConfig::GTKMM ${CMAKE_DL_LIBS})
  target_compile_options(community_gpu_tests PRIVATE -UNDEBUG)
  foreach(scenario amd-w5500 amd-wx3200 intel-integrated intel-discrete nvidia-nouveau nvidia-no-driver nvidia-professional missing-invalid replacement identity drm-pci-merge)
    add_test(NAME community-${scenario} COMMAND community_gpu_tests ${scenario})
  endforeach()
  add_test(NAME community-nvml COMMAND community_gpu_tests nvml $<TARGET_FILE:community_nvml_fixture>)
  add_executable(community_gpu_shared_panel tests/gpu_shared_panel.cpp)
  target_include_directories(community_gpu_shared_panel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(community_gpu_shared_panel PRIVATE PkgConfig::GTKMM ${CMAKE_DL_LIBS})
  target_compile_options(community_gpu_shared_panel PRIVATE -UNDEBUG)
  add_executable(community_about_test tests/about_test.cpp)
  target_include_directories(community_about_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(community_about_test PRIVATE PkgConfig::GTKMM)
  target_compile_definitions(community_about_test PRIVATE G_HTOP_VERSION="${PROJECT_VERSION}")
  target_compile_options(community_about_test PRIVATE -UNDEBUG)
  add_executable(community_gpu_gtk tests/gpu_gtk.cpp)
  target_include_directories(community_gpu_gtk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(community_gpu_gtk PRIVATE PkgConfig::GTKMM Threads::Threads ${CMAKE_DL_LIBS})
  target_compile_options(community_gpu_gtk PRIVATE -UNDEBUG)
endif()

if(BUILD_TESTING AND WIN32)
  foreach(test windows_collectors gpu_shared_panel about_test)
    add_executable(community_${test} tests/${test}.cpp)
    target_include_directories(community_${test} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(community_${test} PRIVATE PkgConfig::GTKMM Threads::Threads advapi32 psapi iphlpapi ws2_32 pdh dxgi dxguid)
    target_compile_definitions(community_${test} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0A00 G_HTOP_VERSION="${PROJECT_VERSION}")
    target_compile_options(community_${test} PRIVATE -UNDEBUG)
  endforeach()
  add_test(NAME windows-collectors COMMAND community_windows_collectors)
  add_test(NAME windows-shared-graph COMMAND community_gpu_shared_panel "${CMAKE_BINARY_DIR}/test-results/graph")
  add_test(NAME windows-about-link COMMAND community_about_test "${CMAKE_BINARY_DIR}/test-results/about")
endif()
