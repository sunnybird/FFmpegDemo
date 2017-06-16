## FFmpegDemo
----
介绍
----
这个项目是把 FFmpeg 移植到 Android 平台的简单示例代码
项目结构如下
libffmpeg 为 ffmpeg 编译生成的 h文件和 so文件.
jniffmpeg 为 jni 层调用 ffpmeg API 的封装，是一个 Android Library 项目.
ffmpegapp 为简单的 Android Application 项目，其依赖于 jniffmpeg .

参考资料
-----
[libffmpeg 编译参考资料](http://sunnybird.github.io/2017/04/14/%E5%9F%BA%E4%BA%8EAndroid%E5%B9%B3%E5%8F%B0%E7%BC%96%E8%AF%91FFmpeg-3-2-4/)
