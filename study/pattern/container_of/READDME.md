Kernel Container_of详解--通过结构体内部成员地址获取到结构体地址
=======

| CSDN | GitHub |
|:----:|:------:|
| [Kernel Container_of详解--通过结构体内部成员地址获取到结构体地址
](http://blog.csdn.net/gatieme) | [`LDD-LinuxDeviceDrivers/study/pattern/container_of`](https://github.com/gatieme/LinuxDeviceDrivers/tree/master/study/pattern/container_of) |


<br>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a>
本作品采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可, 转载请注明出处
<br>



在 `Linux` 内核 `Kernel` 中大量使用指针和结构体数据,　这时候经常需要这样的操作, 我们知道了结构体某个成员的地址信息,　怎么拿到这个结构体的地址呢 ?





最直观的, 内核中在设计链表,　树等数据结构的时候为了代码的复用和效率, 设计了一套通用的模板. 将链表和书结构分为设计为 `struct list_head` 和　`struct rb_node`, 然后在数据结构体中包含此通用结构, 比如

```cpp
typedef struct DataNode
{
    Data d;
    struct list_head ptr;
}DataNode;
```

那么这个时候, 遍历或者查询链表或者树, 访问的都是通用数据结构( `struct list_head` 和　`struct rb_node` ), 这时候如果想要得到私有数据(比如 `DataNode` 中的 `Data d`), 就需要先获取到整体结构体的地址.

#1	container_of宏函数
-------

这是一个频繁的操作, 因此内核中提供了 `container_of` 宏函数来完成这个操作, 其声明形式如下所示 :

```cpp
/////////////////////
// container_of
/////////////////////
结构体类型 container_type 中包含了一个名为 container_field 的成员字段
通过该成员的地址获取到整体结构类型 container_type 的地址
point           -=>     已知的成员数据的地址
container_type  -=>     待获取地址的结构体的类型(point为指向其成员member的指针)
container_field -=>     已知的成员数据的字段名称
container_of(point, container_type, container_field)
```

`container_of` 宏的实现参见[未填写链接](), 其主要实现如下所示

```cpp
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)


#define container_of(ptr, type, member)                                 \
({                                                                      \
        const typeof( ((type *)0)->member) *m_ptr = (ptr);              \
        (type *)( (char *)m_ptr - offsetof(type, member) );             \
})
```

其中 `offsetof` 宏用于获取结构体类型 `TYPE` 中 `member` 成员(相对于结构体首地址)的地址偏移. 由于没有实际的 `TYPE` 结构体对象, 因为使用 `NULL` 指针计算结构体成员的地址偏移, 由于 `NULL` 结构体的起始地址为 0, 因此直接索引即可得到结构体成员的地址.

#2	关于NULL指针异常的疑惑
-------

> **注意**, `NULL` 指针虽然被认为是非法访问地址, 但是对其取地址, 进行 `sizeof` 和 `typeof`运算, 计算偏移信息等等都是合法的, 只是不可以被访问(读写).

参见如下代码:

```cpp
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
        /////////////////////
        //  合法操作
        /////////////////////
        //  取地址
        printf("address = %p\n", &(*(int *)0));

        //  sizeof 运算
        printf("sizeof  = %ld\n", sizeof(*(int *)0));

        //  typeof 运算
        typeof(*(int *)0) temp1 = 8;
        printf("temp = %d\n", temp1);

        /////////////////////
        //  非法操作
        /////////////////////
        //  对 NULL 指针进行读
        typeof(*(int *)0) temp2 = *(int *)0;

        //  对 NULL 指针进行写
        *(int *)0 = 7;
        return EXIT_SUCCESS;
}
```

#3	container_of宏函数分析

了解了这点, 下面我们接着来分析 `container_of` 宏函数的实现

**已知条件**

*	条件1--已知结构体重成员字段 `container_field` 的地址 `point`

*	条件2--结构体的定义 `container_type`, 其中已知字段的成员变量名为 `container_field`

**求解**	结构体的起始地址

**思路**	根据条件 `2` 可以很清楚的拿到已知字段相对于结构体起始地址的偏移, 再结合条件 `1`, 知道了已知字段的地址减去其相对于起始地址的偏移, 就可以直接拿到结构体的起始地址.


首先获取已知字段相对于结构体起始地址的偏移

```cpp
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
```

接着我们就用已知地址减去偏移, 拿到起始地址

```cpp
#define container_of(ptr, type, member)                                 \
({                                                                      \
        const typeof( ((type *)0)->member) *m_ptr = (ptr);              \
        (type *)( (char *)m_ptr - offsetof(type, member) );             \
})
```

#4	测试程序
-------


```cpp
#include <stdio.h>
#include <stdlib.h>

/////////////////////
// container_of
/////////////////////
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)


#define container_of(ptr, type, member)                                 \
({                                                                      \
        const typeof( ((type *)0)->member) *m_ptr = (ptr);              \
        (type *)( (char *)m_ptr - offsetof(type, member) );             \
})


typedef struct CTest
{
        int     m_a;
        int     m_b;
}CTest;


/////////////////////
// test main
/////////////////////
int main(void)
{
    CTest cc = {
        .m_a = 10,
        .m_b = 20,
    };

    int *pa = &(cc.m_a);
    CTest *pc = container_of(pa, CTest, m_a);
    printf("%p %p\n", &cc, pc);
    printf("%d %d\n", pc->m_a, pc->m_b);

    return 0;
}
```

![`container_of` 测试程序](test_container_of.png)

<br>

*	本作品/博文 ( [AderStep-紫夜阑珊-青伶巷草 Copyright ©2013-2017](http://blog.csdn.net/gatieme) ), 由 [成坚(gatieme)](http://blog.csdn.net/gatieme) 创作 

*	采用<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="知识共享许可协议" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">知识共享署名-非商业性使用-相同方式共享 4.0 国际许可协议</a>进行许可. 欢迎转载、使用、重新发布, 但务必保留文章署名[成坚gatieme](http://blog.csdn.net/gatieme) ( 包含链接: http://blog.csdn.net/gatieme ), 不得用于商业目的

*	基于本文修改后的作品务必以相同的许可发布. 如有任何疑问，请与我联系.