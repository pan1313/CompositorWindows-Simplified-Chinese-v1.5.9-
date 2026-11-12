# Compositor Windows 简体中文插件 · 黑体版

CompositorWindows v1.5.9 简体中文汉化插件｜无需修改原程序，支持一键启动、中文界面及黑体字体，兼容 Windows 安装版与便携版。

**[下载已编译的黑体版 ZIP](https://github.com/pan1313/CompositorWindows-Simplified-Chinese-v1.5.9-/raw/refs/heads/main/dist/CompositorWindows-v1.5.9-zh-Hans-SimHei.zip)** · [SHA-256 校验值](dist/SHA256SUMS.txt)

## 安装与启动

1. 保存工作并关闭 Compositor，将下载包完整解压到固定目录。
2. 双击 `Compositor-ZH.exe`。启动器会查找已安装的编辑器。便携版用户若看到选择框，请选择与 `Qt6Widgets.dll` 同目录的原版 `Compositor.exe`。
3. 打开“文件 → 图像焦点堆栈…”或“滤镜 → 镜头效果…”，即可确认中文界面已加载。

已安装前一版汉化插件的用户，也可以关闭编辑器，仅用新包中的 `plugins/generic/compositorzh.dll` 覆盖旧汉化包的同名文件。通过原快捷方式可进入英文界面。卸载时关闭程序、删除汉化包目录即可。

## 翻译与字体

覆盖主菜单、常用工具、画布、图层与蒙版、24 种混合模式，以及部分 RAW 显影和导出控件。命令面板按英文关键词搜索。部分动态错误提示、自绘控件以及用于同步勾选状态的 Pixel Grid 菜单项仍显示英文。

优先使用黑体。缺少黑体时尝试微软雅黑 UI、微软雅黑、Noto Sans CJK SC。标题与正文沿用编辑器定义的字号。

## 从源码构建

准备 MSVC x64、CMake 3.21 或更高版本，以及 **Qt 6.8.3 的 MSVC 2022 x64 开发包**。在项目目录运行以下命令，把 Qt 路径换成实际安装位置。

```powershell
cmake -S src -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build build --config Release
```

将 `build/Release/Compositor-ZH.exe` 放入目标目录，再将 `build/Release/compositorzh.dll` 放入该目录下的 `plugins/generic/`。英文到中文的词表在 `src/terms.inc`，控件处理和字体选择在 `src/plugin.cpp`。

## 验证范围

已核对官方 v1.5.9 便携包中的 Qt DLL 导出符号，并在 Linux 上使用同版本 Qt 做过离屏控件测试，覆盖菜单、快捷键、新弹窗、动态滤镜名称及字体继承。Windows 11 实机上的整机运行和逐窗口视觉验收仍待完成。

## 来源与许可

本项目为非官方汉化插件。部分基础译词来自 Windows 简体中文 fork，保留原项目 MIT 许可及版权声明，见 [上游 MIT 许可](licenses/upstream-MIT.txt)。本仓库的许可见 [LICENSE](LICENSE)。原版编辑器与 Qt 运行库由用户已有的安装提供。

- [Cengokill 的 Windows v1.5.9 源码](https://github.com/Cengokill/CompositorWindows/tree/v1.5.9)
- [Robbie Tilton 的 Mac 原作](https://github.com/robbietilton/Compositor)
- [Windows 简体中文 fork](https://github.com/jax3jie3/CompositorWindows-zh-Hans)
- [Mac 中文术语参考](https://github.com/mainanamisa/compositor-cn)
