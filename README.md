# dtrtmp
rtmp streaming & flv mux

## Installation

### Linux
* cd librtmp & make & cd ..
* mkdir build & cd build
* cmake ..
* make -j8

### Android
* Install [NDK](https://github.com/peterfuture/dttv-android/wiki/1-%E5%AE%89%E8%A3%85android-arm%E4%BA%A4%E5%8F%89%E7%BC%96%E8%AF%91%E9%93%BE)
* cd librtmp
* Modify Makefile: #CROSS_COMPILE=arm-linux-androideabi- => CROSS_COMPILE=arm-linux-androideabi-
* make & cd ..
* mkdir build & cd build
* cmake -DCMAKE_TOOLCHAIN_FILE=../android_toolchain.cmake ..
* make -j8

## Thanks
* [flvmuxer](https://github.com/rainfly123/flvmuxer)

# Licence

GPL v3.0
