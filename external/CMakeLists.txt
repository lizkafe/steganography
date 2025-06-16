include(FetchContent)
FetchContent_Declare(doctest GIT_REPOSITORY https://github.com/doctest/doctest.git)
FetchContent_GetProperties(doctest)
FetchContent_MakeAvailable(doctest)