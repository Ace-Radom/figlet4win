cmake_minimum_required(VERSION 3.10)
project(figlet4win VERSION 1.0.2 LANGUAGES C CXX)

option(FIGLET_BUILD_FIGINSTALL "Build FIGinstall - FIGfont and FIGlet control file installer" ON)
option(FIGLET_ENABLE_TLF_FONTS "Enable support for Sam Hocevar's TOIlet fonts" ON)
option(FIGLET_INSTALL_LIBWINPTHREAD "Install libwinpthread dynamic library in package" OFF)

if(DEFINED DEFAULT_FONT_FILE)
    set(FIGLET_DEFAULT_FONT_FILE ${DEFAULT_FONT_FILE})
else()
    set(FIGLET_DEFAULT_FONT_FILE "standard")
endif()
if(DEFINED DEFAULT_FONT_DIR)
    set(FIGLET_DEFAULT_FONT_DIR ${DEFAULT_FONT_DIR})
else()
    set(FIGLET_DEFAULT_FONT_DIR "fonts")
endif()

file(
    WRITE ${CMAKE_SOURCE_DIR}/VERSION
    ${PROJECT_VERSION}
)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-value")

set(FIGLET_SRC_LIST
    figlet.c
    zipio.c
    crc.c
    inflate.c
)

if(${FIGLET_BUILD_FIGINSTALL})
    find_package(OpenSSL REQUIRED)
    if(NOT OPENSSL_FOUND)
        message(FATAL "Cannot find OpenSSL, please check environment")
    endif()
    get_filename_component(OPENSSL_ROOT_DIR ${OPENSSL_INCLUDE_DIR} DIRECTORY)
    set(OPENSSL_BIN_DIR "${OPENSSL_ROOT_DIR}/bin")
    # get openssl bin dir to search dynamic libs
    file(GLOB OPENSSL_CRYPTO_DLL "${OPENSSL_BIN_DIR}/libcrypto*.dll")
    file(GLOB OPENSSL_SSL_DLL "${OPENSSL_BIN_DIR}/libssl*.dll")
    # get target dlls with special name tags
    set(CURL_ZLIB OFF CACHE STRING "" FORCE)
    set(CURL_USE_LIBPSL OFF CACHE STRING "" FORCE)
    set(CURL_USE_LIBSSH2 OFF CACHE STRING "" FORCE)
    set(CURL_USE_LIBSSH OFF CACHE STRING "" FORCE)
    set(CURL_ENABLE_SSL ON CACHE INTERNAL "" FORCE)
    set(CURL_USE_OPENSSL ON CACHE STRING "" FORCE)
    set(CURL_ENABLE_EXPORT_TARGET OFF CACHE STRING "" FORCE)
    set(CURL_DISABLE_INSTALL ON CACHE STRING "" FORCE)
    # force disable curl install function
    set(BUILD_CURL_EXE OFF CACHE INTERNAL "" FORCE)
    set(BUILD_TESTING OFF)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
        cmake_policy(SET CMP0010 NEW)
        cmake_policy(SET CMP0135 NEW)
    endif()
    include(FetchContent)
    FetchContent_Declare(curl
        URL https://github.com/curl/curl/releases/download/curl-8_5_0/curl-8.5.0.tar.xz
        URL_HASH SHA256=42ab8db9e20d8290a3b633e7fbb3cec15db34df65fd1015ef8ac1e4723750eeb
        USES_TERMINAL_DOWNLOAD TRUE
    )
    FetchContent_MakeAvailable(curl)
    # download and unpack curl 8.5.0 src package
endif()
# if figintall isn't a build target, no need to build curl either

find_package(Python3 REQUIRED)
if(NOT PYTHON3_FOUND)
    message(FATAL "Cannot find Python3, cannot do tests")
endif()

if(${FIGLET_ENABLE_TLF_FONTS})
    add_definitions(-DTLF_FONTS)
    set(FIGLET_SRC_LIST
        ${FIGLET_SRC_LIST}
        utf8.c
    )
endif()

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
add_definitions(-DDEFAULTFONTDIR="${FIGLET_DEFAULT_FONT_DIR}")
add_definitions(-DDEFAULTFONTFILE="${FIGLET_DEFAULT_FONT_FILE}")
add_definitions(-DFIGLET4WINVERSION="${CMAKE_PROJECT_VERSION}")

if(NOT ${CMAKE_PROJECT_VERSION_MAJOR} LESS 1)
    set(FIGLET4WINVERSIONINT ${CMAKE_PROJECT_VERSION_MAJOR})
    if(${CMAKE_PROJECT_VERSION_MINOR} LESS 10)
        string(APPEND FIGLET4WINVERSIONINT "0")
    endif()
    string(APPEND FIGLET4WINVERSIONINT ${CMAKE_PROJECT_VERSION_MINOR})
else()
    set(FIGLET4WINVERSIONINT ${CMAKE_PROJECT_VERSION_MINOR})
endif()
if(${CMAKE_PROJECT_VERSION_PATCH} LESS 10)
    string(APPEND FIGLET4WINVERSIONINT "0")
endif()
string(APPEND FIGLET4WINVERSIONINT ${CMAKE_PROJECT_VERSION_PATCH})
add_definitions(-DFIGLET4WINVERSIONINT=${FIGLET4WINVERSIONINT})
# version num

string(TIMESTAMP COMPILE_TIME "%d %b %Y")
add_definitions(-DFIGLET4WINBUILDTIME="${COMPILE_TIME}")

include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(fonts)

add_executable(figlet
    ${FIGLET_SRC_LIST}
)
if(${FIGLET_ENABLE_TLF_FONTS})
    target_link_libraries(figlet
        ws2_32
    )
endif()
add_executable(chkfont
    chkfont.c
)
if(${FIGLET_BUILD_FIGINSTALL})
    add_executable(figinstall
        figinstall.cpp
    )
    target_link_libraries(figinstall
        CURL::libcurl
        OpenSSL::Crypto
    )
    target_include_directories(figinstall
        PRIVATE
            ${CURL_INCLUDE_DIR}
            ${OPENSSL_INCLUDE_DIR}
    )
endif()

enable_testing()
add_subdirectory(tests)

install(
    TARGETS figlet chkfont
    RUNTIME DESTINATION .
)

if(${FIGLET_BUILD_FIGINSTALL})
    install(
        FILES ${OPENSSL_CRYPTO_DLL} ${OPENSSL_SSL_DLL}
        DESTINATION .
    )
    install(
        TARGETS figinstall libcurl_shared
        RUNTIME DESTINATION .
    )
    # if using curl-8.5.0 src package directly, the name of the shared curl lib is libcurl_shared

    include(cmake/find_file_from_mingw.cmake)

    find_file_from_mingw("libgcc_s_seh-1.dll" MINGW_LIBGCC_S_SEH_PATH)
    if(MINGW_LIBGCC_S_SEH_PATH)
        message(STATUS "MinGW libgcc_s_seh path: ${MINGW_LIBGCC_S_SEH_PATH}")
    else()
        message(FATAL_ERROR "MinGW libgcc_s_seh dll not found!")
    endif()
    find_file_from_mingw("libstdc++-6.dll" MINGW_LIBSTDCPP_PATH)
    if(MINGW_LIBSTDCPP_PATH)
        message(STATUS "MinGW libstdc++ path: ${MINGW_LIBSTDCPP_PATH}")
    else()
        message(FATAL_ERROR "MinGW libstdc++ dll not found!")
    endif()
    install(
        FILES ${MINGW_LIBGCC_S_SEH_PATH} ${MINGW_LIBSTDCPP_PATH}
        DESTINATION .
    )
    # pack libgcc_s_seh and libstdc++

    if(${FIGLET_INSTALL_LIBWINPTHREAD})
        find_file_from_mingw("libwinpthread-1.dll" MINGW_LIBWINPTHREAD_PATH)
        if(MINGW_LIBWINPTHREAD_PATH)
            message(STATUS "MinGW libwinpthread path: ${MINGW_LIBWINPTHREAD_PATH}")
        else()
            message(FATAL_ERROR "MinGW libwinpthread dll not found!")
        endif()
        
        install(
            FILES ${MINGW_LIBWINPTHREAD_PATH}
            DESTINATION .
        )
    endif()
    # pack libwinpthread if needed
endif()

install(
    FILES ${FIGLET_FONT_CONTROL_FILES} ${FIGLET_FONT_FILES}
    DESTINATION ${FIGLET_DEFAULT_FONT_DIR}
)
install(
    FILES ${CMAKE_SOURCE_DIR}/figlist.bat ${CMAKE_SOURCE_DIR}/showfigfonts.bat
    DESTINATION .
)

if(EXISTS ${CMAKE_SOURCE_DIR}/doc/figlet4win.chm)
    install(
        FILES ${CMAKE_SOURCE_DIR}/figman.bat
        DESTINATION .
    )
    install(
        FILES ${CMAKE_SOURCE_DIR}/doc/figlet4win.chm
        DESTINATION doc
    )
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Ace-Radom")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_PACKAGE_DISCRIPTION_SUMMARY "FIGlet for Windows")
set(CPACK_WIX_UPGRADE_GUID "CC11CD4A-3AEC-4D65-81ED-65790266A448")
set(CPACK_WIX_PRODUCT_GUID "CC11CD4A-3AEC-4D65-81ED-65790266A451")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
set(CPACK_WIX_PATCH_FILE ${CMAKE_SOURCE_DIR}/wix-patch.xml)
include(CPack)
