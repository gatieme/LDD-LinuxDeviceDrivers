Linux内核中的奇技淫巧--BUILD_BUG_ON编译时断言
=======

| CSDN | GitHub |
|:----:|:------:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) |

`Linux` 内核是由 `C` 语言和少量的汇编 `ASM` 代码写成的, 内核的协作开发, 展示了各路大神"高度发达的脑细胞"和"奇技淫巧", 在阅读 `Linux` 内核源代码过程中, 总是能发现一些比较难以理解但又设计巧妙的代码片段, 因此写下这一系列博文

*	一来通过内核代码学习一下C语言及汇编语言的高级用法

*	二来学习内核开发大牛们书写代码的风格及思路

> 注意  本系列博文, 不关注 `OS` 的各个模块的设计思想, 而仅关注于内核中在解决问题时提出的和使用的 `C` 和 `ASM` 的高级设计技巧.


<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处
<br>


#1	BUILD_BUG_ON内核中的编译时断言
-------


尽管在大多数时候只需关心代码运行的正确性, 但是很多时候需要在编译期间就发现一些潜在的致命错误. 为此内核提供了有力的宏定义, 位于[include/linux/bug.h, version =4.10, line 18](http://lxr.free-electrons.com/source/include/linux/bug.h?v=4.10#L18)


| 宏 | 描述 |
|:--:|:---:|
| BUILD_BUG_ON_NOT_POWER_OF_2(n) | 如果表达式n不是2的幂, 则编译报错 |
| BUILD_BUG_ON_ZERO(e) | 如果表示式e为0, 则编译报错 |
| BUILD_BUG_ON_NULL(e) | 如果表达式e为NULL, 则编译报错 |
| BUILD_BUG_ON_INVALID(e) | 如果表示式e不合法, 则编译报错 |
| BUILD_BUG_ON_MSG(cond, msg) | 表达式cond为TRUE, 则提示错误信息msg |
| BUILD_BUG_ON(condition) | 如果表示式condition为TRUE, 则停止编译 |
| BUILD_BUG() | 如果使用, 则永远报错 |
| MAYBE_BUILD_BUG_ON(cond) | |





http://www.cnblogs.com/hazir/p/static_assert_macro.html

http://www.cnblogs.com/westfly/p/4210231.html

http://blog.chinaunix.net/uid-20608849-id-3027980.html





<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
<br>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可


