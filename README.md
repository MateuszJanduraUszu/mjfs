# MJFS

MJFS is an internal module designed for use in future projects.
It is written in C++20.

## Setup

1. Download the appropriate package based on your system architecture:

    * For 64-bit systems, download `Bin-x64.zip`.
    * For 32-bit systems, download `Bin-x86.zip`.

2. Extract the downloaded package. You should see the following directories:

    * `bin` - Debug and release binaries.
    * `inc` - Include headers.

3. Include the `inc\mjfs` directory in your project as an additional include directory.
4. Link `bin\{Debug|Release}\mjfs.lib` library to your project.
5. Don't forget to include the `bin\{Debug|Release}\mjfs.dll`. in your project's
   output directory if your application relies on it during runtime.

## Usage

To integrate MJFS into your project, you can include the appropriate header files
based on your requirements:

* **<mjfs/api.hpp>**: Export/import macro, don't include it directly.
* **<mjfs/bitmask.hpp>**: Bitmask operations and utilities.
* **<mjfs/directory.hpp>**: Directory utilities.
* **<mjfs/file.hpp>**: `file` class.
* **<mjfs/file_stream.hpp>**: `file_stream` class.
* **<mjfs/path>**: Filesystem path utilities.
* **<mjfs/status.hpp>**: Filesystem object status utilities.

## Compatibility

MJFS is compatible with Windows Vista and later operating systems,
and it requires C++20 support.

## Questions and support

If you have any questions, encounter issues, or need assistance with the MJFS,
feel free to reach out. You can reach me through the `Issues` section or email
([mjandura03@gmail.com](mailto:mjandura03@gmail.com)).

## License

Copyright © Mateusz Jandura.

SPDX-License-Identifier: Apache-2.0