# wtf8

Convert to and from Wobbly UTF-8.

## Installation

If you're using cmake, you can fetch the library via FetchContent.

    FetchContent_Declare(
        avakar_wtf8
        GIT_REPOSITORY https://github.com/avakar/wtf8.git
        GIT_TAG master
        GIT_SHALLOW YES
        )
    FetchContent_MakeAvailable(avakar_wtf8)

Then, link against avakar::wtf8.
