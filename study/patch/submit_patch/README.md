AderXCoding
=======

| CSDN | GitHub |
|:----:|:------:|
| [Aderstep--紫夜阑珊-青伶巷草](http://blog.csdn.net/gatieme) | [`AderXCoding/system/tools`](https://github.com/gatieme/AderXCoding/tree/master/system/tools) |

<br>

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>

本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处, 谢谢合作

因本人技术水平和知识面有限, 内容如有纰漏或者需要修正的地方, 欢迎大家指正, 也欢迎大家提供一些其他好的调试工具以供收录, 鄙人在此谢谢啦

<br>


#1	应有的心态
-------

`Paul` 在《深入理解并行编程》一书第 `11.1.2` 节中说, 验证和测试工作都需要良好的心态. 应当以一种破坏性的、甚至带一点仇恨的心理来验证代码，有时也应当考虑到：不少人的生命依赖于我们代码正确性的几率。总之，心态对事情的成败有重要的影响。

同样的，在向社区提交补丁之前，我们也应当有正确的心态：

1.	我们的代码，可能会影响不少人的生命，所以一定要细心。

2.	悲观的说，如果补丁做得不好，会被社区大牛的口水淹死。当然，更可能的情况是，大牛懒得向你吐口水：）所以在提交补丁前，请为自己的声誉负责。

3.	乐观的说，社区牛人一般都比较Nice，如果你的水平真的比较牛的话。

4.	更进一步乐观的说，你提交的高质量的补丁，可能会为你带来良好的声誉、满意的工作。

5.	需要强调一点的是，在社区，千万不要和Maintainer作对。要象对待学校老师那样，谦恭的对待Maintainer。不管你信不信这一点，反正我信了:)
       如果你和我一样，有着近乎自大的自信，请接着读完本文^-^


#2	准备工作
-------

在开始工作之前，请准备如下工作：

#2.1	安装一份Linux
-------

不论是 `ubuntu`、`centos` 还是其他 `Linux` 发行版本, 都是可以的.

我个人习惯使用 `ubuntu 16.04` 版本. 虽然我曾经尝试在 `windows` 下面安装 `git`, 但是最终你会发现, 在 `windows` 下面使用 `git` 提交补丁几乎可以用痛苦来描述.


##2.2	安装git
-------

默认的 `Linux` 发行版, 一般都已经安装好 `git`. 如果没有, 随便找一本 `git` 的书都可以. 这里不详述. 比较好的git资料有 :

http://git.oschina.net/progit/

https://item.jd.com/11615420.html

##2.3	配置git
-------

###2.3.1	配置用户名和邮箱
-------

在配置用户名的时候, 请注意社区朋友习惯用英语沟通, 也就是名在前, 姓在后, 这一点会影响社区邮件讨论, 因此需要留意.(但是随着中文内核开发者的增多, 这点其实已经不那么重要了, 大家还是习惯 `姓 名` 的方式).

在配置邮箱时, 也要注意. 社区会将国内某些著名的邮件服务器屏蔽. 因此建议你申请一个 `gmail` 邮箱.

以下是我的配置：

xiebaoyou@ThinkPad-T440$git config -l | grep "user"
       user.email=baoyou.xie@linaro.org
       user.name=Baoyou Xie

###2.3.2	配置 `sendemail`
-------

你可以手工修改 `~/.gitconfig`, 或者 `git` 仓库下的 `.git/config`文件, 添加 `[sendemail]`节.

该配置用于指定发送补丁时用到的邮件服务器参数。

以下是我的配置, 供参考 :

```cpp
       [sendemail]
              smtp encryption= tls
              smtp server= smtp.gmail.com
              smtp user= baoyou.xie@linaro.org
              smtp serverport= 587
```

`gmail` 邮箱的配置比较麻烦, 需要按照 `google` 的说明, 制作证书. 配置完成后, 请用如下命令, 向自己发送一个测试补丁：

```cpp
git send-email your.patch --to your.mail --cc your.mail
```

##2.3	下载源码
-------

首先，请用如下命令, 拉取 `linus` 维护的 `Linux` 主分支代码到本地 :

```cpp
git clone ssh://git@dev-private.git.linaro.org/zte/kernel.git
```


![`kernel` 官网的内核库](kernel_org_linus_git.png)


这个过程比较长，请耐心等待。


##2.4	linux-next 分支
-------

一般情况下, `Linux` 主分支代码不够新, 如果你基于这个代码制作补丁, 很有可能不会顺利的合入到 `Maintainer` 那里, 换句话说, `Maintainer` 会将补丁发回给你, 要求你重新制作.

所以, 一般情况下, 你需要再用以下命令, 添加其他分支, 特别是 `linux-next` 分支. 强调一下， 你需要习惯基于 `linux-next` 分支进行工作.


![`Linux Next` 分支](linux-next-git.png)

详细的信息, 请参见 [`Working with linux-next`](https://www.kernel.org/doc/man-pages/linux-next.html)

```cpp
# linux-next
git remote add linux-next https://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next.git

# linux-staging
git remote add staging https://git.kernel.org/pub/scm/linux/kernel/git/gregkh/staging.git

# net tree
git remote add net git://git.kernel.org/pub/scm/linux/kernel/git/davem/net.git
```

然后用如下命令拉取这三个分支的代码到本地：

```cpp
git fetch --tags linux-next

git fetch --tags staging

git fetch --tags net
```

有些 `Maintainer` 维护了自己的代码分支, 那么, 你可以在内核源码目录 `\MAINTAINERS` 文件中, 找一下相应文件的维护者, 及其 `git` 地址.


例如, `watchdog` 模块的信息如下 :

```cpp
       WATCHDOGDEVICE DRIVERS
       M:      Wim Van Sebroeck <wim@iguana.be>
       R:      Guenter Roeck <linux@roeck-us.net>
       L:      linux-watchdog@vger.kernel.org
       W:      http://www.linux-watchdog.org/
       T:      gitgit://www.linux-watchdog.org/linux-watchdog.git
       S:      Maintained
       F:      Documentation/devicetree/bindings/watchdog/
       F:      Documentation/watchdog/
       F:      drivers/watchdog/
       F:      include/linux/watchdog.h
       F:      include/uapi/linux/watchdog.h
```

其中, `git://www.linux-watchdog.org/linux-watchdog.git` 是其git地址。

你可以用如下命令拉取 `watchdog` 代码到本地 :

```cpp
git remote add watchdog git://www.linux-watchdog.org/linux-watchdog.git

gitfetch --tags watchdog
```

当然, 这里友情提醒一下, `MAINTAINERS` 里面的信息可能不一定准确, 这时候你可能需要借助 `google`, 或者问一下社区的朋友, 或者直接问一下作者本人. 不过, 一般情况, 基于 `linux-next` 分支工作不会有太大的问题. 实在有问题再去打扰作者本人.

4、阅读Documentation/SubmittingPatches，这很重要。

5、检出源码

查看 `linux-next` 的最新 `tag`

```cpp
git tag -l "next-*" | tail -n1
```

![查看 `linux-next` 的最新 `tag`](git_tag_next.png)

可以看到 `linux-next` 最新的代码是 `next-20170901`

```cpp
git branch my_local_branch next-20170901
```

这个命令表示将 `linux-next` 分支的 `20170816` 这个 `tag` 作为本地 `my_local_branch` 的基础.

```cpp
git checkout  my_local_branch
```

这个命令检索出 `linux-next` 的代码到新创建的  `my_local_branch`.

上面两个命令可以直接使用如下命令代替

```cpp
git checkout -b my_local_branch next-20170901
```

这条命令将在本地创建分支 `my_local_branch` 与远程分支 `next-20170901` 相关联, 并直接检出代码.



#3	寻找软柿子
-------


如果没有奇遇, 大厨一般都是从小工做起的. 我们不可能一开始就维护一个重要的模块, 或者修复一些非常重要的故障. 那么我们应当怎么样入手参与社区?

这当然要寻找软柿子了. 拿着软柿子做出来的补丁, 可以让 `Maintainer` 无法拒绝合入你的补丁. 当然, 这么做主要还是为了在 `Maintainer` 那里混个脸熟. 否则, 以后你发的重要补丁, 人家可能不会理你.

什么样的柿子最软？下面是答案：

1.	消除编译警告

2.	编码格式, 例如注释里面的单词拼写错误、对齐不规范、代码格式不符合社区要求.

建议是从"消除编译警告"入手. 社区很多大牛, 都是这样成长起来的.

我们平时编译内核, 基本上遇不到编译警告. 是不是内核非常完美, 没有编译警告, 非矣! 你用下面这个步骤试一下 :

首先, 配置内核, 选择所有模块 :

```cpp
make ARCH=arm64 allmodconfig
```

请注意其中 "allmodconfig", 很有用的配置, 我们暂且可以理解为, 将所有模块都编译. 这样我们就可以查找所有模块中的编译警告了.


下面这个命令开始编译所有模块 :

```cpp
make ARCH=arm64 EXTRA_CFLAGS="-Wmissing-declarations -Wmissing-prototypes" CROSS_COMPILE=/toolchains/aarch64-linux-gnu/bin/aarch64-linux-gnu-
```

其中 `EXTRA_CFLAGS="-Wmissing-declarations-Wmissing-prototypes"` 参数表示追踪所有 `missing-declarations`、 `missing-prototypes`类型的警告.


`CROSS_COMPILE=/toolchains/aarch64-linux-gnu/bin/aarch64-linux-gnu-` 是指定交叉编译工具链路径, 需要根据你的实际情况修改. 当然, 如果是 `x86` 架构, 则不需要指定此参数.

在编译的过程中, 我们发现如下错误 :


```cpp
scripts/Makefile.build:311:recipe for target 'drivers/staging/fsl-mc/bus/dpio/qbman-portal.o' failed
```

我们可以简单的忽略 `drivers/staging/fsl-mc/bus/dpio/qbman-portal.c` 这个文件.

在 `drivers/staging/fsl-mc/bus/dpio/Makefile` 文件中, 发现这个文件的编译依赖于宏 `CONFIG_FSL_MC_DPIO`.

于是，我们修改编译命令，以如下命令继续编译：

```cpp
make CONFIG_ACPI_SPCR_TABLE=n ARCH=arm64 EXTRA_CFLAGS="-Wmissing-declarations -Wmissing-prototypes" CROSS_COMPILE=/toolchains/aarch64-linux-gnu/bin/aarch64-linux-gnu-
```

请注意该命令中的 "CONFIG_ACPI_SPCR_TABLE=n", 它强制关闭了 `CONFIG_ACPI_SPCR_TABLE` 配置.

当编译完成以后, 我们是不是发现有很多警告? 特别是在 `drivers` 目录下.

下面是我在 `next-20170807` 版本中发现的警告 :

```cpp
/dimsum/git/kernel.next/drivers/clk/samsung/clk-s3c2410.c:363:13:warning: no previous prototype for 's3c2410_common_clk_init'[-Wmissing-prototypes]
 void__init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
            ^
 CC     drivers/clk/samsung/clk-s3c2412.o
/dimsum/git/kernel.next/drivers/clk/samsung/clk-s3c2412.c:254:13:warning: no previous prototype for 's3c2412_common_clk_init'[-Wmissing-prototypes]
 void__init s3c2412_common_clk_init(struct device_node *np, unsigned long xti_f,
            ^
 CC     drivers/clk/samsung/clk-s3c2443.o
/dimsum/git/kernel.next/drivers/clk/samsung/clk-s3c2443.c:388:13:warning: no previous prototype for 's3c2443_common_clk_init' [-Wmissing-prototypes]
 void__init s3c2443_common_clk_init(struct device_node *np, unsigned long xti_f,
```


下一节，我们就基于这几个警告来制作补丁。

#4	制作补丁
-------


##4.1	修改内核代码消除警告
-------

要消除这几个警告, 当然很简单了. 将这几个函数声明为 `static` 即可. 下面是我的修改 :

```cpp
git diff
diff --git a/drivers/clk/samsung/clk-s3c2410.c b/drivers/clk/samsung/clk-s3c2410.c
index e0650c3..8f4fc5a 100644
--- a/drivers/clk/samsung/clk-s3c2410.c
+++ b/drivers/clk/samsung/clk-s3c2410.c
@@ -360,7 +360,7 @@ static void __inits3c2410_common_clk_register_fixed_ext(
       samsung_clk_register_alias(ctx, &xti_alias, 1);
 }
 
-void __init s3c2410_common_clk_init(structdevice_node *np, unsigned long xti_f,
+static void __init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
                                    intcurrent_soc,
                                    void__iomem *base)
 {
diff --git a/drivers/clk/samsung/clk-s3c2412.c b/drivers/clk/samsung/clk-s3c2412.c
index b8340a4..2a2ce06 100644
--- a/drivers/clk/samsung/clk-s3c2412.c
+++ b/drivers/clk/samsung/clk-s3c2412.c
@@ -251,7 +251,7 @@ static void __init s3c2412_common_clk_register_fixed_ext(
       samsung_clk_register_alias(ctx, &xti_alias, 1);
 }
 
-void __init s3c2412_common_clk_init(struct device_node *np, unsigned long xti_f,
+static void __init s3c2412_common_clk_init(struct device_node *np, unsigned long xti_f,
                                    unsigned long ext_f, void __iomem *base)
 {
       struct samsung_clk_provider *ctx;
diff --gita/drivers/clk/samsung/clk-s3c2443.c b/drivers/clk/samsung/clk-s3c2443.c
index abb935c..f0b88bf 100644
--- a/drivers/clk/samsung/clk-s3c2443.c
+++ b/drivers/clk/samsung/clk-s3c2443.c
@@ -385,7 +385,7 @@ static void __inits3c2443_common_clk_register_fixed_ext(
                               ARRAY_SIZE(s3c2443_common_frate_clks));
 }
 
-void __init s3c2443_common_clk_init(struct device_node *np, unsigned long xti_f,
+static void __init s3c2443_common_clk_init(struct device_node *np, unsigned long xti_f,
                                    int current_soc,
                                    void __iomem *base)
```

再编译一次, 警告果然被消除了. 原来, 社区工作如此简单.


但是请允许我浇一盆冷水!

##4.2	提交代码
-------

你先试着用下面的命令做一个补丁出来看看!


```cpp
git add drivers/clk/samsung/clk-s3c2410.c
git add drivers/clk/samsung/clk-s3c2412.c
git add drivers/clk/samsung/clk-s3c2443.c
git commit drivers/clk/samsung/

[zxic/67184930591] this is my test
3 files changed, 3 insertions(+), 3deletions(-)

gitformat-patch -s -v 1 -1
```

生成的补丁内容如下：

```cpp
cat v1-0001-this-is-my-test.patch
From 493059190e9ca691cf08063ebaf945627a5568c7 Mon Sep 17 00:00:00 2001
From: Baoyou Xie<baoyou.xie@linaro.org>
Date: Thu, 17 Aug 2017 19:23:13 +0800
Subject: [PATCH v1] this is my test
 
Signed-off-by: Baoyou Xie<baoyou.xie@linaro.org>
---
 drivers/clk/samsung/clk-s3c2410.c | 2 +-
 drivers/clk/samsung/clk-s3c2412.c | 2 +-
 drivers/clk/samsung/clk-s3c2443.c | 2 +-
 3files changed, 3 insertions(+), 3 deletions(-)
 
diff --git a/drivers/clk/samsung/clk-s3c2410.c b/drivers/clk/samsung/clk-s3c2410.c
index e0650c3..8f4fc5a 100644
--- a/drivers/clk/samsung/clk-s3c2410.c
+++ b/drivers/clk/samsung/clk-s3c2410.c
@@ -360,7 +360,7 @@ static void __init s3c2410_common_clk_register_fixed_ext(
      samsung_clk_register_alias(ctx,&xti_alias, 1);
 }
 
-void __init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
+static void __init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
                               int current_soc,
                               void __iomem *base)
 {
diff --git a/drivers/clk/samsung/clk-s3c2412.c b/drivers/clk/samsung/clk-s3c2412.c
index b8340a4..2a2ce06 100644
--- a/drivers/clk/samsung/clk-s3c2412.c
+++ b/drivers/clk/samsung/clk-s3c2412.c
@@ -251,7 +251,7 @@ static void __inits3c2412_common_clk_register_fixed_ext(
      samsung_clk_register_alias(ctx,&xti_alias, 1);
 }
 
-void __init s3c2412_common_clk_init(structdevice_node *np, unsigned long xti_f,
+static void __init s3c2412_common_clk_init(struct device_node *np, unsigned long xti_f,
                               unsigned long ext_f, void __iomem *base)
 {
      struct samsung_clk_provider *ctx;
diff --git a/drivers/clk/samsung/clk-s3c2443.c b/drivers/clk/samsung/clk-s3c2443.c
index abb935c..f0b88bf 100644
--- a/drivers/clk/samsung/clk-s3c2443.c
+++ b/drivers/clk/samsung/clk-s3c2443.c
@@ -385,7 +385,7 @@ static void __inits3c2443_common_clk_register_fixed_ext(
                           ARRAY_SIZE(s3c2443_common_frate_clks));
 }
 
-void __init s3c2443_common_clk_init(structdevice_node *np, unsigned long xti_f,
+static void __init s3c2443_common_clk_init(struct device_node *np, unsigned long xti_f,
                               int current_soc,
                               void __iomem *base)
 {
--
2.7.4
```

你可以试着用

```cpp
git send-email v1-0001-this-is-my-test.patch --to baoyou.xie@linaro.org
```

将补丁发给 `Maintainer`. 记得准备好一个盆子, 接大家的口水:)

在制作正确的补丁之前，我们需要这个错误的补丁错在何处:

1.	应该将它拆分成三个补丁。

也许这一点值得商酌, 因为这三个文件都是同一个驱动 : `clk: samsung`. 也许 `Maintainer` 认为它是同一个驱动, 做成一个补丁也是可以的.

我觉得应该拆分成三个.

当然了, 应当以 `Maintainer` 的意见为准. 不同的 `Maintainer` 也许会有不同的意见.

2.	补丁描述实在太 `LOW`

3.	补丁格式不正确

4.	补丁内容不正确


下一节我们逐个解决这几个问题.

但是首先我们应当将补丁回退. 使用如下命令 :

```cpp
git reset HEAD~1
```

#5	制作正确的补丁
-------

##5.1	补丁描述
-------


补丁第一行是标题, 比较重要. 它首先应当是模块名称.

但是我们怎么找到 `drivers/clk/samsung/clk-s3c2412.c` 文件属于哪个模块?

可以试试下面这个命令, 看看 `drivers/clk/samsung/clk-s3c2412.c` 文件的历史补丁 :

```cpp
#git log drivers/clk/samsung/clk-s3c2412.c

commit 02c952c8f95fd0adf1835704db95215f57cfc8e6
Author:Martin Kaiser <martin@kaiser.cx>
Date:   Wed Jan 25 22:42:25 2017 +0100


clk: samsung:mark s3c...._clk_sleep_init() as __init
```

`ok`, 模块名称是 "clk:samsung"


下面是我为这个补丁添加的描述, 其中第一行是标题 :

```cpp
clk: samsung: mark symbols static where possible for s3c2410

We get 1 warnings when building kernel withW=1:
/dimsum/git/kernel.next/drivers/clk/samsung/clk-s3c2410.c:363:13:warning: no previous prototype for 's3c2410_common_clk_init'[-Wmissing-prototypes]
 void __init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,

In fact, this function is only used in thefile in which they are
declared and don't need a declaration, butcan be made static.
So this patch marks these functions with 'static'.
```

这段描述是我从其他补丁中拷贝出来的，有几下几点需要注意：

1.	标题中故意添加了 "for s3c2410", 以区别于另外两个补丁

2.	"1 warnings" 这个单词中, 错误的使用了复数, 这是因为复制的原因

3.	"/dimsum/git/kernel.next/" 这个路径名与我的本地路径相关, 不应当出现在补丁中.

4.	警告描述超过了 `80` 个字符, 但是这是一个特例, 这里允许超过 `80` 字符.

5.	这些问题, 如果不处理的话, `Maintainer` 会不高兴的! 如果 `Maintainer` 表示了不满, 而你不修正的话, 这个补丁就会被忽略.

修正后的补丁描述如下：

```cpp
clk: samsung: mark symbols static wherepossible for s3c2410
 
We get 1 warning when building kernel withW=1:
drivers/clk/samsung/clk-s3c2410.c:363:13:warning: no previous prototype for 's3c2410_common_clk_init'[-Wmissing-prototypes]
 void__init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
 
In fact, this function is only used in thefile in which they are
declared and don't need a declaration, but can be made static.
So this patch marks these functions with 'static'.
```


我们的补丁描述一定要注意用词, 不要出现将 "unused" 写为 "no used" 这样的错误.

反复使用 `git add，git commit` 将补丁提交到 `git` 仓库.


终于快成功, 是不是想庆祝一下. 用 `git` 命令看看我们刚才提交的三个补丁 :

```cpp
root@ThinkPad-T440:/dimsum/git/kernel.next#git log drivers/clk/samsung/
commit0539c5bc17247010d17394b0dc9f788959381c8f
Author: Baoyou Xie<baoyou.xie@linaro.org>
Date:  Thu Aug 17 20:43:09 2017 +0800

   clk: samsung: mark symbols static where possible for s3c2443

   We get 1 warning when building kernel with W=1:
   drivers/clk/samsung/clk-s3c2443.c:388:13: warning: no previous prototypefor 's3c2443_common_clk_init' [-Wmissing-prototypes]
    void __init s3c2443_common_clk_init(struct device_node *np, unsignedlong xti_f,

   In fact, this function is only used in the file in which they are
   declared and don't need a declaration, but can be made static.
   So this patch marks these functions with 'static'.

commitc231d40296b4ee4667e3559e34b00f738cae1e58
Author: Baoyou Xie<baoyou.xie@linaro.org>
Date:  Thu Aug 17 20:41:38 2017 +0800

   clk: samsung: mark symbols static where possible for s3c2412

   We get 1 warning when building kernel with W=1:
   drivers/clk/samsung/clk-s3c2412.c:254:13: warning: no previous prototypefor 's3c2412_common_clk_init' [-Wmissing-prototypes]
    void __init s3c2412_common_clk_init(struct device_node *np, unsignedlong xti_f,

   In fact, this function is only used in the file in which they are
   declared and don't need a declaration, but can be made static.
   So this patch marks these functions with 'static'.

commit ff8ea5ed4947d9a643a216d51f14f6cb87abcb97
Author: Baoyou Xie<baoyou.xie@linaro.org>
Date:  Thu Aug 17 20:40:50 2017 +0800

   clk: samsung: mark symbols static where possible for s3c2410
```


但是, 你发现补丁描述里面还有什么不正确的吗?

不过 `Maintainer` 也许发现不了这个问题, 然后 `~~~~` 然后这个补丁也可能被接收入内核.


##5.2	生成补丁
-------

下面我们生成补丁 :

```cpp
#git format-patch -s -3
0001-clk-samsung-mark-symbols-static-where-possible-for-s.patch
0002-clk-samsung-mark-symbols-static-where-possible-for-s.patch
0003-clk-samsung-mark-symbols-static-where-possible-for-s.patch
```

实际上, 我们的补丁仍然是错误的.

##5.3	检查补丁
-------

在发送补丁前, 我们需要用脚本检查一下补丁 :

```cpp
./scripts/checkpatch.pl 000*
---------------------------------------------------------------
0001-clk-samsung-mark-symbols-static-where-possible-for-s.patch
---------------------------------------------------------------
WARNING: Possible unwrapped commit description (prefer a maximum 75 chars per line)
#9:
 void__init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
 
WARNING: line over 80 characters
#29: FILE:drivers/clk/samsung/clk-s3c2410.c:363:
+static void __init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,

total: 0 errors, 2 warnings, 8 lineschecked
```

请留意输出警告, 其中第一个警告是说我们的描述中, 有过长的语句. 前面已经提到, 这个警告可以忽略.

但是第二个警告告诉我们代码行超过 `80` 个字符了. 这是不能忽略的警告, 必须处理.


##5.4	修改补丁
-------

使用 "git resetHEAD~3" 命令将三个补丁回退. 重新修改代码 :

```cpp
static void __init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
       修改为
       static void __init
       s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f,
       也可以修改为
       static void __init s3c2410_common_clk_init(struct device_node *np, unsigned long xti_f，
```

我个人习惯用第一种方法, 因为看其他代码也是第一种修改方法.

重新提交补丁, 并用 `git format-patch` 命令生成补丁.

#6	发送补丁
-------

##6.1	获取 `Maintainer`
-------


生成正确的补丁后, 请再次用 `checkpatch.pl` 检查补丁正确性.

确保无误后, 可以准备将它发送给 `Maintainer` 了.

但是应该将补丁发给谁? 这可以用 `get_maintainer.pl` 来查看 :


```cpp
./scripts/get_maintainer.pl 000*
Kukjin Kim <kgene@kernel.org>(maintainer:ARM/SAMSUNG EXYNOS ARM ARCHITECTURES)
Krzysztof Kozlowski <krzk@kernel.org>(maintainer:ARM/SAMSUNG EXYNOS ARM ARCHITECTURES)
Sylwester Nawrocki<s.nawrocki@samsung.com> (supporter:SAMSUNG SOC CLOCK DRIVERS)
Tomasz Figa <tomasz.figa@gmail.com>(supporter:SAMSUNG SOC CLOCK DRIVERS)
Chanwoo Choi <cw00.choi@samsung.com>(supporter:SAMSUNG SOC CLOCK DRIVERS)
Michael Turquette<mturquette@baylibre.com> (maintainer:COMMON CLK FRAMEWORK)
Stephen Boyd <sboyd@codeaurora.org>(maintainer:COMMON CLK FRAMEWORK)
linux-arm-kernel@lists.infradead.org(moderated list:ARM/SAMSUNG EXYNOS ARM ARCHITECTURES)
linux-samsung-soc@vger.kernel.org (moderatedlist:ARM/SAMSUNG EXYNOS ARM ARCHITECTURES)
linux-clk@vger.kernel.org (open list:COMMONCLK FRAMEWORK)
linux-kernel@vger.kernel.org (open list)
```

##6.2	发送补丁
-------

接下来, 可以用 `git send-email` 命令发送补丁了 :

```cpp
git send-email 000* --tokgene@kernel.org,krzk@kernel.org,s.nawrocki@samsung.com,tomasz.figa@gmail.com,cw00.choi@samsung.com,mturquette@baylibre.com,sboyd@codeaurora.org--cc linux-arm-kernel@lists.infradead.org,linux-samsung-soc@vger.kernel.org,linux-clk@vger.kernel.org,linux-kernel@vger.kernel.org
```


注意, 哪些人应当作为邮件接收者, 哪些人应当作为抄送者.

在本例中, 补丁是属于实验性质的, 可以不抄送给邮件列表帐户.

提醒 : 你应当将补丁先发给自己, 检查无误后再发出去. 如果你有朋友在社区有较高的威望, 也可以抄送给他, 必要的时候, 也许他能给你一些帮助. 这有助于将补丁顺利的合入社区.


>重要提醒 : 本文讲述的, 主要是实验性质的补丁, 用于打开社区大门. 真正重要的补丁, 可能需要经过反复修改, 才能合入社区. 我知道有一些补丁, 超过两年时间都没能合入社区, 因为总是有需要完善的地方, 也许还涉及一些社区政治:)

<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作;

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的.

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问，请与我联系.