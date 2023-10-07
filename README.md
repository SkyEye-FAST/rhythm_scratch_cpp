# 音游猜曲名刮刮乐

**Python 版本：<https://github.com/SkyEye-FAST/rhythm_scratch>**

此脚本适用于即时通讯的群组/聊天室/服务器等处，**需要主持人来操作**，而非自主进行游戏。

执行与题目相关的命令后会自动将输出内容复制到剪贴板，无需手动复制。

题目和答案会在执行过程中以文件形式出现在**输出文件夹**（默认为`output`）内。

刮开的字符如果为字母（包括拉丁、希腊、西里尔），则会同时刮开大小写。

## 需求

代码以 C++20（202002L）标准编写，编译器为`clang`，仅在 Windows 10/11 进行过测试，不保证在其他平台的可用性。

使用了库[toml11](https://github.com/ToruNiina/toml11)以解析`toml`文件，相关文件已经放在仓库内，使用 MIT 协议分发。

## 命令列表

- help | ? - 显示帮助
- exit | e - 退出
- version | ver | v - 列出曲库使用音游版本号
- (heart | h) add [amount] - 增加可用刮开次数
- (heart | h) remove [amount] - 减少可用刮开次数
- (open | o) [character] - 刮开指定字符
- openspace | os - 刮开空格
- (check | c) [index] - 将某题全部刮开
- (show | s) - 显示题目

## 曲库

目前仓库内提供的音游曲库如下：

- Arcaea（版本：v5.0.1）
- Muse Dash（版本：3.9.0）
- Orzmic（版本：2.19.28）
- Phiros（版本：3.1.3）
- RYCEAM（版本：v1.0.5）

### 自定义曲库

在曲库文件夹（默认为`song_dict`）下，请按照以下结构存放曲库：

- 曲库文件夹
  - 曲库 1
    - `dict`
      - 曲库文件 1
      - 曲库文件 2
    - `dict.toml`
  - 曲库 2
  - 曲库 3

`dict.toml`的格式如下：

```toml
name = "游戏名称"
version = "游戏版本"
dicts = ["曲库文件1", "曲库文件2"]
```

曲库文件为纯文本，一行一个曲名。

## 配置文件

配置文件名为`configuration.toml`，位置与脚本同级。

| 键名              | 默认值 | 说明         |
| ----------------- | ------ | ------------ |
| `generate_amount` | `10`   | 生成曲目数量 |
| `guess_chancess`  | `10`   | 可用刮开次数 |

| 键名            | 默认值      | 说明       |
| --------------- | ----------- | ---------- |
| `output_folder` | `output`    | 输出文件夹 |
| `dict_folder`   | `song_dict` | 曲库文件夹 |
