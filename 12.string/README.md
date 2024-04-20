# 字符串功能

* 使用`char str1[20]`定义字符串时的注意事项（string测试用例）
需要注意的是，在macOS下gcc编译`char str11[20]`的用法会使用movdqa、movups命令，这两个指令在bochs中会报错。因此需要在g++编译时加上`-mno-sse`参数。