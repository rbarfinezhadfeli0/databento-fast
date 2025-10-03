import os
import sys
import subprocess
from pathlib import Path

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)

class CMakeBuild(build_ext):
    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        
        # Required for auto-detection of auxiliary "native" libs
        if not extdir.endswith(os.path.sep):
            extdir += os.path.sep

        cmake_args = [
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            "-DBUILD_PYTHON_BINDINGS=ON",
            "-DBUILD_TESTS=OFF",
        ]

        build_args = []
        
        # Set CMAKE_BUILD_PARALLEL_LEVEL to control build parallelism
        if "CMAKE_BUILD_PARALLEL_LEVEL" not in os.environ:
            build_args += ["-j4"]

        build_temp = Path(self.build_temp) / ext.name
        build_temp.mkdir(parents=True, exist_ok=True)

        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=build_temp
        )
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=build_temp
        )

# Read the contents of README file
this_directory = Path(__file__).parent
long_description = (this_directory / "README.md").read_text()

setup(
    name="databento-fast",
    version="1.0.0",
    author="Ramin Brfinezhadfeli",
    author_email="",
    description="Ultra-fast C++ library for parsing Databento DBN files",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/rbarfinezhadfeli0/databento-fast",
    ext_modules=[CMakeExtension("databento_cpp")],
    cmdclass={"build_ext": CMakeBuild},
    zip_safe=False,
    python_requires=">=3.7",
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
        "Topic :: Software Development :: Libraries",
        "Topic :: Scientific/Engineering",
    ],
    keywords="databento dbn parser c++ high-performance market-data",
    project_urls={
        "Bug Reports": "https://github.com/rbarfinezhadfeli0/databento-fast/issues",
        "Source": "https://github.com/rbarfinezhadfeli0/databento-fast",
    },
)
