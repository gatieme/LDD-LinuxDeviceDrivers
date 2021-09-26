---

title: 工具
date: 2021-06-26 09:40
author: gatieme
tags:
    - linux
    - tools
categories:
        - 技术积累
thumbnail:
blogexcerpt: 虚拟化 & KVM 子系统

---

<br>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 鄙人在此谢谢啦

**转载请务必注明出处, 谢谢, 不胜感激**

<br>

| 日期 | 作者 | GitHub| CSDN | BLOG |
| ------- |:-------:|:-------:|:-------:|:-------:|
| 2021-02-15 | [成坚-gatieme](https://kernel.blog.csdn.net) | [`AderXCoding/system/tools/fzf`](https://github.com/gatieme/AderXCoding/tree/master/system/tools/fzf) | [使用模糊搜索神器 FZF 来提升办公体验](https://blog.csdn.net/gatieme/article/details/113828826) | [Using FZF to Improve Productivit](https://oskernellab.com/2021/02/15/2021/0215-0001-Using_FZF_to_Improve_Productivity)|


<br>

   **工具**
=====================




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*　重要功能和时间点　-*-*-*-*-*-*-*-*-*-*-*-*-*-*-***





下文将按此目录分析 Linux 内核中 MM 的重要功能和引入版本:




**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* 正文 -*-*-*-*-*-*-*-*-*-*-*-*-*-*-***


# 1 开发工具
-------


社区一直在寻求一种非邮件列表的内核开发方式, [Pulling GitHub into the kernel process](https://lwn.net/Articles/860607)



# 2 编译
-------

## 2.1 zero initialization for stack variables
-------

| 时间  | 作者 | 特性 | 描述 | 是否合入主线 | 链接 |
|:----:|:----:|:---:|:----:|:---------:|:----:|
| 2019/01/23 | Kees Cook <keescook@chromium.org> | [gcc-plugins: Introduce stackinit plugin](https://patchwork.kernel.org/project/linux-kbuild/cover/20190123110349.35882-1-keescook@chromium.org) | 使用 GCC 编译内核时, 提供插件来实现早期提议的 gcc 选项 [`-finit-local-vars`](https://gcc.gnu.org/ml/gcc-patches/2014-06/msg00615.html)(未被 GNU/gcc 社区接纳)类似的功能, 以实现[未初始化局部变量初始化为 0 的诉求](https://gcc.gnu.org/ml/gcc-patches/2014-06/msg00615.html). 通过 CONFIG_GCC_PLUGIN_STACKINIT 来开启. | v1 ☐ | [PatchWork](https://patchwork.kernel.org/project/linux-kbuild/cover/20190123110349.35882-1-keescook@chromium.org) |
| 2019/04/10 | Kees Cook <keescook@chromium.org> | [security: Implement Clang's stack initialization](https://patchwork.kernel.org/project/linux-kbuild/cover/20190423194925.32151-1-keescook@chromium.org) | clang 中提供了 `-ftrivial-auto-var-init` 来进行未初始化局部变量的默认初始化, 引入 CONFIG_INIT_STACK_ALL 来为内核开启此选项, 这比 CONFIG_GCC_PLUGINS_STRUCTLEAK_BYREF_ALL 的覆盖更广. 当启用 CONFIG_INIT_STACK_ALL 时, 当前补丁仅使用 "pattern" 模式. 开发人员可以通过使用 `__attribute__((uninitialized))` 在每个变量的基础上选择不使用该特性. | v1 ☑ 5.2-rc1 | [PatchWork v1,3/3](https://patchwork.kernel.org/project/linux-kbuild/patch/20190410161612.18545-4-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[PatchWork v2,3/3](https://patchwork.kernel.org/project/linux-kbuild/patch/20190411180117.27704-4-keescook@chromium.org)<br>*-*-*-*-*-*-*-* <br>[PatchWork v3,3/3](https://patchwork.kernel.org/project/linux-kbuild/patch/20190423194925.32151-4-keescook@chromium.org), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=709a972efb01efaeb97cad1adc87fe400119c8ab) |
| 2020/06/14 | Alexander Potapenko <glider@google.com> | [security: allow using Clang's zero initialization for stack variables](https://lore.kernel.org/patchwork/cover/1255765) | 支持 clang 的局部变量零初始化. 通过 CONFIG_INIT_STACK_ALL_ZERO 来启用, clang 可以通过选项 `-ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang` 来保证未初始化局部变量初始化为 0. | RFC v2 ☑ 5.9-rc1 | [PatchWork v2,RFC](https://lore.kernel.org/patchwork/cover/1255765)<br>*-*-*-*-*-*-*-* <br>[PatchWork RFC](https://lore.kernel.org/patchwork/patch/1256566), [commit](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=f0fe00d4972a8cd4b98cc2c29758615e4d51cdfe) |
| 2021/02/18 | NA | [security: allow using Clang's zero initialization for stack variables](https://lore.kernel.org/patchwork/cover/1255765) | gcc 也引入了 [Auto Initialize Automatic Variables](https://www.phoronix.com/scan.php?page=news_item&px=GCC-12-Auto-Var-Init) 通过 `-ftrivial-auto-var-init` 选项将未初始化的[变量默认初始化为 0](https://gcc.gnu.org/pipermail/gcc-patches/2021-February/565514.html). | RFC v2 ☑ 5.9-rc1 | NA |


<br>

*   本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作.

*   采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*   基于本文修改后的作品务必以相同的许可发布. 如有任何疑问, 请与我联系.

*   **转载请务必注明出处, 谢谢, 不胜感激**
<br>
