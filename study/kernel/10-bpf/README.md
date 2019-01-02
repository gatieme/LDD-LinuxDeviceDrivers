
```cpp
make -C ~/Work/Kernel/linux/build/x86_64 M=`pwd` BPF_SAMPLES_PATH=`pwd`
```

静态编译


```cpp
KBUILD_HOSTLDLIBS              += $(LIBBPF) -lelf -static -lz
```


#参考资料
-------

[BPF Documentation and Resources](https://facebookmicrosites.github.io/bpf/docs/bpf-docs)

[](https://cilium.readthedocs.io/en/latest/bpf/)
