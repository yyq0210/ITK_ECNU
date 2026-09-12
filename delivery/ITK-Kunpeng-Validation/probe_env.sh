#!/bin/bash
set -u
CACHE=/home/pub/yyq/ITK-5.4.0/build-yyq/CMakeCache.txt
echo "=== CMAKE CACHE (selected) ==="
grep -E '^(CMAKE_BUILD_TYPE|CMAKE_C_COMPILER|CMAKE_CXX_COMPILER|CMAKE_C_FLAGS|CMAKE_CXX_FLAGS|CMAKE_MAKE_PROGRAM|CMAKE_CXX_STANDARD|BUILD_SHARED_LIBS|BUILD_STATIC_LIBS|BUILD_TESTING|BUILD_EXAMPLES|ITK_BUILD_DEFAULT_MODULES|ITK_DEFAULT_THREADER|ITK_USE_GPU|ITK_USE_FFTWF|ITK_USE_FFTWD|ITK_USE_SYSTEM_FFTW|ITK_USE_SYSTEM_PNG|ITK_USE_SYSTEM_ZLIB|ITK_USE_SYSTEM_HDF5|ITK_USE_SYSTEM_JPEG|ITK_USE_SYSTEM_TIFF|ITK_WRAP_PYTHON|ITK_ENABLE_MIXED_PRECISION|ITK_USE_FLOAT_ACCUMULATION|Module_ITKGPU|Module_ITKTBB|TBB_DIR|PNG_LIBRARY|ZLIB_LIBRARY|JPEG_LIBRARY|TIFF_LIBRARY|HDF5_|FFTW):' "$CACHE" | grep -v ADVANCED
echo "=== GPU MODULES ==="
grep -E '^Module_ITKGPU' "$CACHE" | grep -v ADVANCED
echo "=== FFTW ==="
grep -i fftw "$CACHE" | grep -v ADVANCED | head -30
echo "=== SYS LIBS RPM ==="
rpm -qa | grep -Ei '^(zlib|libpng|libjpeg|libtiff|hdf5|fftw|tbb|openmpi|OpenCL|python3)-' | sort
echo "=== HEADER EXISTS ==="
ls /usr/include/png.h /usr/include/zlib.h /usr/include/jpeglib.h /usr/include/tiff.h /usr/include/fftw3.h 2>/dev/null
echo "=== MPI ==="
command -v mpirun; mpirun --version 2>/dev/null | head -2
echo "=== SYSTEM GCC ==="
/usr/bin/gcc --version 2>/dev/null | head -1
echo "=== KUNPENG SIMD ==="
head -30 /etc/profile.d/kunpeng-simd.sh 2>/dev/null
echo "=== LDD BENCH ==="
ldd /home/pub/yyq/itk_hybrid_precision_demo/build/precision_remain_bench 2>/dev/null | head -30
echo "=== ITK CONFIGURE ==="
grep -E 'ITK_USE_|ITK_ENABLE_|ITK_BUILD' /home/pub/yyq/ITK-5.4.0/build-yyq/Modules/Core/Common/itkConfigure.h 2>/dev/null | head -40
