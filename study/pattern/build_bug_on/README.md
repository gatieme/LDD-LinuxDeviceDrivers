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
| BUILD_BUG_ON_ZERO(e) | 如果表示式e不为0, 则编译报错 |
| BUILD_BUG_ON_NULL(e) | 如果表达式e不为NULL, 则编译报错 |
| BUILD_BUG_ON_INVALID(e) | 如果表示式e不合法, 则编译报错 |
| BUILD_BUG_ON_MSG(cond, msg) | 表达式cond为FALSE, 则提示错误信息msg |
| BUILD_BUG_ON(condition) | 如果表示式condition为FALSE, 则停止编译 |
| BUILD_BUG() | 如果使用, 则永远报错 |
| MAYBE_BUILD_BUG_ON(cond) | |


#2	实现技巧
-------



##2.1	BUILD_BUG_ON
-------

##2.1.1	实现
-------


BUILD_BUG_ON是最基本的断言, 其实现是其他多数断言的基础, 参见[`include/linux/bug.h, version 4.10, line 56`](http://lxr.free-electrons.com/source/include/linux/bug.h?v=4.10#L56)

```cpp
/**
 * BUILD_BUG_ON - break compile if a condition is true.
 * @condition: the condition which the compiler should know is false.
 *
 * If you have some code which relies on certain constants being equal, or
 * some other compile-time-evaluated condition, you should use BUILD_BUG_ON to
 * detect if someone changes it.
 *
 * The implementation uses gcc's reluctance to create a negative array, but gcc
 * (as of 4.4) only emits that error for obvious cases (e.g. not arguments to
 * inline functions).  Luckily, in 4.3 they added the "error" function
 * attribute just for this type of case.  Thus, we use a negative sized array
 * (should always create an error on gcc versions older than 4.4) and then call
 * an undefined function with the error attribute (should always create an
 * error on gcc 4.3 and later).  If for some reason, neither creates a
 * compile-time error, we'll still have a link-time error, which is harder to
 * track down.
 */
#ifndef __OPTIMIZE__
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#else
#define BUILD_BUG_ON(condition) \
        BUILD_BUG_ON_MSG(condition, "BUILD_BUG_ON failed: " #condition)
#endif
```

###2.1.2	实现技巧分析
-------

`BUILD_BUG_ON_MSG` 的实现技巧我们不去讨论, 其定义在[`include/linux/bug.h, version=4.10, line 54`](http://lxr.free-electrons.com/source/include/linux/bug.h?v=4.10#L54


于是 `BUILD_BUG_ON` 定义如下

```cpp
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
```

1.	condition为一合法的表达式

2.	!!(condition) 对其两次取非, 则表示式condition若为0, 两次取非后仍为0, 否则原表示式不为0, 两次取非后为1

3.	1-2*!!(condition)	若表达式condition若为0, 该式结果为1-2*0=1, 否则原表示式不为0, 结果为1-2*1=-1

4	char[1 - 2*!!(condition)] 当conndition为0, 则该式为char[1], 否则为char[-1]

5	对上式子做sizeof运行, 当conndition为0, 结果为1, 否则出错



$$
        f(n)
        = sizeof(char[1 - 2*!!(con)]
        =
        \begin{cases}
        	sizeof(char[1-2*!!0])=sizeof(char[1])=1,
            & \text {if $con$ = 0}
            \\
        sizeof(char[1-2*!!1])=sizeof(char[-1])=ERROR,
        & \text{if $con \neq$ 0 }
        \end{cases}
$$

再总结一下, `BUILD_BUG_ON(condition)` 表示的就是

*	若表达式`condition`结果为`0`(不成立), 则编译通过, 该宏的值也为 `0`;

*	若表达式`condition`的结果不为0(成立), 则编译不通过.



###2.1.3	示例
-------




###2.1.4	编译时断言与运行时断言
-------



这会让人联想到C语言中 assert 宏的用法：


>void assert(int expression);

如果参数`expression`计算的结果为`0`(假), 不满足表示式要求, 它先向`stderr`打印一条出错信息, 然后通过调用 `abort` 来终止程序运行；否则断言成立，继续执行。

我们讨论的宏与 `assert`本质区别在于, 我们的宏在编译时进行测试, 而 `assert` 宏是在运行时测试. 我们希望能尽早地捕获到我们编译时的错误, 而不是推迟到运行时. 我们管这种宏用法叫做**编译时断言**, `assert`为**运行时断言**.


##2.1	BUILD_BUG_ON_ZERO && BUILD_BUG_ON_NULL
-------


用于判断表达式是否为 `0/NULL`, 定义如下[`include/linux/bug.h, version=4.10, line 33`](http://lxr.free-electrons.com/source/include/linux/bug.h?v=4.10#L33)

```CPP
/* Force a compilation error if condition is true, but also produce a
   result (of value 0 and type size_t), so the expression can be used
   e.g. in a structure initializer (or where-ever else comma expressions
   aren't permitted). */
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int:-!!(e); }))
#define BUILD_BUG_ON_NULL(e) ((void *)sizeof(struct { int:-!!(e); }))
```



实现的形式基本相同, 我们以第一个 `BUILD_BUG_ON_ZERO` 分析， 它表示的是

>检查表达式e是否为0
>
>*	为0编译通过且返回0;
>
>*	如果不为0，则编译不通过.

可能从这个宏的名字上看可能容易理解错, 因此有人曾提议将名称改为 `"BUILD_BUG_OR_ZERO", 参见[patch])(http://lkml.indiana.edu/hypermail/linux/kernel/0703.1/1546.html), 但未能被社区接受.


我们且不管这个宏定义名字怎样,来逐步分析一下这个宏是如何来实现的 "

>sizeof(struct { int : –!!(e); } )


1.	(e)	表达式 `e` 的声明

2.	!!(e)	对 `e` 的结果进行两次求非. 即如果 `e` 开始是 `0` 的话, 结果就是`0`; 如果 `e` 不为 `0`, 则结果为`1`

3.	–!!(e)  再乘以 `-1`, 如果第`2`步结果为`0`, 则仍为`0`; 否则结果为`-1`

4.	`struct { int : –!!(0); }  -=> struct { int : 0; } 如果e的结果为0，则我们声明一个结构体拥有一个int型的数据域，并且规定它所占的位的个数为0。这没有任何问题，我们认为一切正常。

5. struct { int : –!!(1); }  -=>  struct { int : –1; } 如果e的结果非0，结构体的int型数据域的位域将变为一个负数，将位域声明为负数这是一个语法的错误。

6. 现在要么结果为声明了一个位域为 `0`的结构体, 要么出现位域为负数编译出错; 如果能正确编译, 然后我们对该结构体进行sizeof操作，得到一个类型为 `size_t` 的结果, 值为 `0`


$$
f(n)=sizeof(struct \{int:–!!(e);\})=
\begin{cases}
	sizeof(struct \{int:–!!(0);\})
    =sizeof(struct \{int:0;\})
    =0
	& \text {if $con$ = 0}
	\\
	sizeof(struct \{int:–!!(!0);\})
    =sizeof(struct \{int:-1;\})
    =ERROR
    & \text{if $con \neq$ 0 }
\end{cases}
$$


再总结一下, `BUILD_BUG_ON_ZERO(e)` 表示的就是

*	若表达式e结果为`0`, 则编译通过, 该宏的值也为 `0`;

*	若表达式`e`的结果不为0，则编译不通过。



理解了上面之后，再来看看第二个 `BUILD_BUG_ON_NULL(e)`宏, 与第一个类似, 用来在编译时断言 `e` 是否为 `NULL`

*	若是这个宏返回 `(void *)0` (即`NULL`，与第一个宏的区别)

*	不为NULL时编译出错。



##2.2	BUILD_BUG_ON_NOT_POWER_OF_2
-------


```CPP
/* Force a compilation error if a constant expression is not a power of 2 */
#define __BUILD_BUG_ON_NOT_POWER_OF_2(n)        \
        BUILD_BUG_ON(((n) & ((n) - 1)) != 0)
#define BUILD_BUG_ON_NOT_POWER_OF_2(n)          \
        BUILD_BUG_ON((n) == 0 || (((n) & ((n) - 1)) != 0))
```



可以看到 `BUILD_BUG_ON_NOT_POWER_OF_2` 是通过`BUILD_BUG_ON`, 其中 n & (n-1) != 0, 用于判断


http://www.cnblogs.com/hazir/p/static_assert_macro.html

http://www.cnblogs.com/westfly/p/4210231.html

http://blog.chinaunix.net/uid-20608849-id-3027980.html





<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
<br>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可


