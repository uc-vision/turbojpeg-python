import os
import re
import subprocess
import sys
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

# Convert distutils Windows platform specifiers to CMake -A arguments
PLAT_TO_CMAKE = {
    "win32": "Win32",
    "win-amd64": "x64",
    "win-arm32": "ARM",
    "win-arm64": "ARM64",
}


# A CMakeExtension needs a sourcedir instead of a file list.
# The name must be the _single_ output extension from the CMake build.
# If you need multiple extensions, see scikit-build.
class CMakeExtension(Extension):
    def __init__(self, name: str, sourcedir: str = "") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(Path(sourcedir).resolve())


class CMakeBuild(build_ext):
    def build_extension(self, ext: CMakeExtension) -> None:

        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = Path.cwd() / self.get_ext_fullpath(ext.name)  # type: ignore[no-untyped-call]
        extdir = ext_fullpath.parent.resolve()

        if not os.path.exists("./build"):
            os.mkdir("./build")

        subprocess.run(
            ["cmake", "-DCMAKE_BUILD_TYPE=Release", "-DBUILD_TESTING=OFF", f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}{os.sep}", ".."], check=True, cwd="./build"
        )
        subprocess.run(
            ["cmake", "--build", ".", "--", "-j16"], check=True, cwd="./build"
        )


# The information here can also be placed in setup.cfg - better separation of
# logic and declaration, and simpler if you include description/version in a file.
setup(
    name="turbojpeg-python",
    version="0.0.1",
    author="Oliver Batchelor",
    author_email="oliver.batchelor@canterbury.ac.nz",
    description="TurboJPEG v3 bindings for 12 and 16 bit encoding",
    long_description="",
    ext_modules=[CMakeExtension("turbojpeg_python")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    extras_require={"test": ["pytest>=6.0"]},
    python_requires=">=3.8",
)
