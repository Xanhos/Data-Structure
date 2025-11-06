include(FetchContent)



function(download_package PACKAGE_NAME GIT VERSION)
    find_package(${PACKAGE_NAME} ${VERSION} QUIET)

    if(NOT ${PACKAGE_NAME}_FOUND)
        FetchContent_Declare(${PACKAGE_NAME}
                GIT_REPOSITORY ${GIT}
                GIT_TAG ${VERSION}
                SOURCE_DIR "${DEPS_SOURCE_DIR}/${PACKAGE_NAME}-src")

        FetchContent_MakeAvailable(${PACKAGE_NAME})
    endif()
endfunction()