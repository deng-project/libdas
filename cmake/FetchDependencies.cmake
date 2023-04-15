# libdas: DENG asset management library
# licence: Apache, see LICENCE file
# file: FetchDependencies.cmake - dependency fetching cmake configuration file
# author: Karl-Mihkel Ott

include(FetchContent)
set(CMAKE_TLS_VERIFY true)

function(GitClone git_url git_tag target_dir repo_name)
    set(GIT_REPO "git_repo_${repo_name}")
    message("Cloning git repository from ${git_url} to ${target_dir}")
    FetchContent_Declare(${GIT_REPO}
        SOURCE_DIR ${target_dir}
        GIT_REPOSITORY ${git_url}
        GIT_TAG ${git_tag}
    )
    FetchContent_MakeAvailable(${GIT_REPO})
endfunction()

GitClone(https://git.dengproject.org/deng/minimal-ascii-reader 9c0ee1df7af34e543f30fd178f19937f1775ca0b ${CMAKE_CURRENT_SOURCE_DIR}/deps/mar mar)
GitClone(https://git.dengproject.org/deng/trs-headers 898e58a49f8f8f76dc3b07407b879cb6836ddc93 ${CMAKE_CURRENT_SOURCE_DIR}/deps/trs trs)
