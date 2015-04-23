set(url "file://E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_repo/Tests/CMakeTests/FileDownloadInput.png")
set(dir "E:/repo/WindowsStore/dava.framework/Tools/WinStoreCMake/cmake_build/Tests/CMakeTests/downloads")

file(DOWNLOAD
  ${url}
  ${dir}/file3.png
  TIMEOUT 2
  STATUS status
  EXPECTED_HASH SHA1=5555555555555555555555555555555555555555
  )
