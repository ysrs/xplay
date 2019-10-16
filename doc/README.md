# 文档说明

## Linux(Ubuntu)下安装FFmpeg
其它依赖库就不多说了，按照提示安装对应的依赖即可。
使用git下载源码：
git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg

进入到ffmpeg目录中，依次执行下面命令
./configure --enable-shared --prefix=/usr/local/ffmpeg
make -j8
make install

