# 进度
基础功能开发，单元测试中
# Getting started
## 获取Chis-Prime
```
git clone https://github.com/ChisBread/Chis-Prime.git
git submodule update --init --recursive
#环境安装
#sudo apt-get install clang clang-format cmake
```
## 测试
```
mkdir build && cd build
CC=/usr/local/bin/clang CXX=/usr/local/bin/clang++ cmake .. && make -j8
./TEST
```
## 发布
```
1.下载[Visual Studio 2019](https://visualstudio.microsoft.com/)(这是最简单的方案),选择安装C++/CMake
2.在Visual Studio 2019中打开"Chis-Prime", 选择"x64-release"生成pbrain-Chis++.exe
3.在[piskvork](https://gomocup.org/download-gomocup-manager/)中选择pbrain-Chis++.exe
4.做你想做的
```
# 附录
## 代码目录
- test Chis-Prime 基础设施的测试(搜索部分待完善)
- thirdparty 依赖的第三方库
- utils 通用类型与方法
- board 棋盘操作、棋盘类型等
- search 搜索算法
- resource 生成包含原始棋型的C++代码
- gomocup gomocup协议实现
## 关于patterns_by_wind23.txt
[Tianyi Hao](https://github.com/wind23) 提供的棋型表，共有14种棋型
```
XX_*X代表左边界,X_*XX代表右边界
XX_*X[O_]*X_*XX
```
|棋形(个人叫法)|编号|样例|描述|
|----|----|----|----|
|死棋 |0 |XX______XOX______XX|毫无用处的棋型|
|眠一 |11|XX______XO____X__XX|只有成眠二点的棋型|
|活一 |21|XX_____X_O____X__XX|有成活二点的棋型|
|眠二 |31|XX______XO___OX__XX|只有成眠三点的棋型|
|活二A|41|XX_____X_O__O_X__XX|有成活三A点的棋型|
|活二B|51|XX____X__O_O__X__XX|有一处成活三B点的棋型|
|活二C|61|XX___X___OO___X__XX|有两处成活三B点的棋型|
|眠三 |71|XX______XO__OOX__XX|有成眠四点的棋型(意外的是有可能会变成活四A)|
|活三A|81|XX_____X_O_OO_X__XX|有一个成活四B点的棋型|
|活三B|91|XX____X__OOO__X__XX|有两个成活四B点的棋型|
|眠四 |a1|XX______XO_OOOX__XX|有一个成五点的棋型|
|活四A|a2|XX____XO_OOO_OX__XX|有两个成五点的棋型|
|活四B|b1|XX_____X_OOOO_X__XX|有两个成五点的棋型|
|成五 |c1|XX______XOOOOOX__XX|赢了|