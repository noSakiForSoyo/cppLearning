# KICKPI K11C (RK3566) 完整上手流程

> **起点**：板子刷好 Ubuntu，串口能登录。
> **终点**：串口 → 网络 → SSH → 传文件 → 装包 → tmux → 板子上跑程序 → **交叉编译**
> 用途：换板子 / 重装 / 换电脑时照着走，不用重新踩坑。

---

## 0. 硬件事实（从启动日志里读出来的，可自查）

| 项目 | 值 |
|---|---|
| 板子 | KICKPI K11C **V1.2** / Rockchip RK3566 |
| CPU | 4× Cortex-A55（**aarch64**） |
| 内存 | 4 GiB（DDR3，最终 1056 MHz） |
| eMMC | 32 GB（约 29.1 GiB 可用，型号 BJTD4R） |
| GPU | Mali G52 2EE |
| NPU | RKNPU 0.9.2（1 TOPS） |
| WiFi | VS6621S（驱动名 SKWIFI6621S） |
| 网口 | 千兆，控制器 `fe010000.ethernet`，PHY `maxio_mae0621a` |
| 调试串口 | **UART2_M0**，`console=ttyFIQ0`，**1500000 8N1** |
| 系统 | Ubuntu 20.04.6 LTS，kernel 5.10.160，**glibc 2.31** |
| 主机名 | `kickpi` |
| 登录 | **root 自动登录（无密码）** → 第一件事就 `passwd` |

---

## 1. 串口连接（唯一不会失联的通道）

### 1.1 板子侧针脚

```
pin 23   GND
pin 25   UART2_RX_M0_DEBUG1     ← 芯片的"收"
pin 27   UART2_TX_M0_DEBUG1     ← 芯片的"发"
```

### 1.2 接线（**必须交叉**）

| USB-TTL 转接板 | 板子 | 说明 |
|---|---|---|
| **GND**（黑） | **pin 23** | 共地 |
| **RX** | **pin 27** | 板子发 → 转接板收 |
| **TX** | **pin 25** | 转接板发 → 板子收 |
| **VCC**（红） | **✗ 不接** | 板子由 DC 12V 独立供电 |

**口诀：同名的不要接一起，相反的才接一起。**

- 串口参数：**1500000 / 8 / N / 1**，无流控
- 转接板电平跳线必须是 **3.3V**（5V 可能打坏 SoC 引脚）
- **改线一定先断电**
- 白/绿谁是 TX 谁是 RX 因模块而异：**先看 PCB 焊盘丝印，看不见就试错**（接反不会烧，发现没输出立刻断电对调）

### 1.3 工具

Windows **MobaXterm**（免安装，串口 + SSH + SFTP 一体）。

> **关键认知**：串口**没有"握手"** —— 窗口全黑但没报错 = 已经打开了，只是没数据。

---

## 2. 第一次上电

1. **先接线，后上电**
2. 开串口终端 → 再插 DC 12V
3. 日志开始刷 = 成功

---

## 3. 网络：笔记本直连板子

### 3.1 拓扑

```
校园网 / WiFi
     │
  笔记本 ── WLAN（上网用）
     │
     └── 以太网 192.168.137.1   ← ICS 自动设置
              │  网线直连
         板子 eth0 192.168.137.100/24
              gateway / DNS = 192.168.137.1
```

### 3.2 Windows 侧：启用 ICS

`网络连接` → 右键 **WLAN** → 属性 → **共享** → 勾选"允许其他网络用户…" → 家庭网络连接选 **以太网**。

> ⚠️ **ICS 会把「以太网」强制改成 `192.168.137.1`** —— 原来的静态 IP 全废，**网段整个换掉**。

### 3.3 板子侧：nmcli 配静态 IP + 网关

```bash
nmcli con mod "Wired connection 1" \
      ipv4.method manual \
      ipv4.addresses 192.168.137.100/24 \
      ipv4.gateway 192.168.137.1 \
      ipv4.dns 192.168.137.1 \
      ipv4.never-default no

nmcli con up "Wired connection 1"
```

**验证：**
```bash
ip addr show eth0 | grep "inet "        # 192.168.137.100/24
ip route                                # 必须有 default via 192.168.137.1
nmcli device status                     # eth0 应为 connected
```

> **`gateway` 和 `never-default` 是配套的**：
> 前者说"默认路由指向谁"，后者说"要不要用它当默认路由"。**缺一个都没有默认路由。**

### 3.4 分层排查表（这套方法换个领域照样用）

| 层 | 怎么查 | 期望 |
|---|---|---|
| **链路层** | 串口日志 / `ip addr` 的 `LOWER_UP` | 有 `LOWER_UP` |
| **IP 层** | `ip addr show eth0 \| grep inet` | 有 IPv4 地址 |
| **路由层** | `ip route` | 有 `default via ...` |
| **DNS 层** | `ping www.baidu.com` | 能解析 |
| **应用层** | `ssh` / `apt update` | 通 |

**症状 → 层：**
- `NO-CARRIER` → **链路层**（网线 / 对端口）
- 有 `LOWER_UP` 但没 IP → **DHCP 没人应答，或没配静态 IP**
- 有 IP 但 `Network is unreachable` → **没有默认路由**
- `ping 8.8.8.8` 通、域名不通 → **DNS**
- ping 不通但服务能用 → **对端防火墙拦了 ICMP**（ping 不通 ≠ 网络不通）

### 3.5 ⚠️ WSL 到不了板子怎么办

**WSL2 默认是 NAT 模式**，它的 NAT 是为"出公网"设计的。板子所在的 `192.168.137.0/24` 是**主机网卡上的直连网段**，NAT 不转发 → WSL 看不见板子，而 **Windows cmd 看得见**。

**尝试过但失败的方案**：`networkingMode=mirrored` → 报 `0x8007054f (HNS 内部错误)`。
**原因**：装了 **VMware**，它的虚拟网络栈和 WSL mirrored 模式冲突。

**✅ 可用方案：从 WSL 调用 Windows 的 ssh/scp（WSL interop）**

```bash
# 写进 ~/.bashrc 永久生效
alias wssh='/mnt/c/Windows/System32/OpenSSH/ssh.exe'
alias wscp='/mnt/c/Windows/System32/OpenSSH/scp.exe'

wssh root@192.168.137.100

# 传文件（先落到 Windows 侧，再传）
mkdir -p /mnt/c/Users/27928/board
cp build-arm20/plain_demo /mnt/c/Users/27928/board/
wscp 'C:\Users\27928\board\plain_demo' root@192.168.137.100:~/
```

> 它们跑在 Windows 上，用 Windows 的网络栈 —— **一定能到板子**，而人还在 WSL 终端里。

---

## 4. SSH（验收项）

```bash
# 板子上
passwd                                   # 自动登录的 root 没有密码，必须设
systemctl status ssh --no-pager          # 应为 active (running)
ss -tlnp | grep :22                      # 应监听 0.0.0.0:22
```

```bash
# 笔记本（WSL，走 .exe 中继）
wssh root@192.168.137.100
```

**验收截图（一屏自证）：**
```bash
echo "=== $(date) ==="; hostname; whoami; uname -a; ip -4 addr show eth0 | grep inet
```

---

## 5. scp 传文件

```bash
wscp from_wsl.txt root@192.168.137.100:~/          # 本机 → 板子
wscp root@192.168.137.100:/etc/os-release ./       # 板子 → 本机
```

---

## 6. 板子上装包

```bash
apt update
apt install -y tmux build-essential cmake gdb
```

> 板子有外网依赖 ICS。`apt` 报 `Network is unreachable` → 回 3.4 的**路由层**。

---

## 7. tmux

```bash
tmux new -s work          # 新建会话
# Ctrl-B 然后按 D        → 脱离（detach）
tmux ls
tmux attach -t work       # 重新挂上去 —— 任务还在跑
```

**原理**：SSH 断线会发 `SIGHUP` 杀前台进程；tmux 让进程挂在 **tmux server** 上，不是挂在 sshd 上。

---

## 8. 在板子上原生编译

```bash
cat > hello.cpp <<'EOF'
#include <iostream>
int main(){ std::cout << "hello from K11C\n"; }
EOF

g++ -Wall -Wextra -std=c++17 hello.cpp -o hello && ./hello
```

**零 ABI 问题**（编译环境 = 运行环境），但慢、吃板子资源。

---

## 9. 交叉编译：glibc 版本这一课

### 9.1 问题

在 WSL 里用交叉工具链编出来的程序，拷到板子上跑：

```
./test: /lib/aarch64-linux-gnu/libc.so.6: version `GLIBC_2.34' not found
./test: /lib/aarch64-linux-gnu/libstdc++.so.6: version `GLIBCXX_3.4.32' not found
```

**原因**：

| | glibc | libstdc++ |
|---|---|---|
| WSL 的交叉工具链 | 2.39 级 | GCC 15 级 |
| 板子的 Ubuntu 20.04 | **2.31** | GCC 9 级（≤3.4.28） |

可执行文件里刻着"**我需要 GLIBC_2.34**"（符号版本化），板子只有 2.31 → **动态加载器拒绝兑现**。

> **一句话：交叉编译的真正难点不是"编译出来"，而是"运行时环境对得上"。**

### 9.2 四条路对比

| 方案 | 体积 | 兼容板子 | 多进程共享内存 | 用在哪 |
|---|---|---|---|---|
| WSL 原生交叉编译 | 79 KB | ❌ | ✅ | 已废弃 |
| `-static` 静态链接 | 2000 KB | ✅ | ❌ | 应急、单文件小工具 |
| **Docker 固定 20.04 环境** | **79 KB** | ✅ | ✅ | ✅ **日常主力** |
| 手动抽 sysroot | 79 KB | ✅ | ✅ | 理解原理用 |

**静态链接的代价**：代码**不能被多个进程共享** —— 板子上跑 10 个进程就带 10 份 libc。想压体积可以 `strip`（2 MB → ~700 KB），但符号表没了，gdb 就用不了。

### 9.3 ✅ 推荐：Docker 固定构建环境

**原理**：容器里的 `g++-aarch64-linux-gnu` 来自 Ubuntu 20.04 的仓库，链接的 glibc 就是 **2.31**，和板子完全一致。

**① 让 Docker 能拉镜像**（国内必备）

阿里云 → 容器镜像服务 ACR → 镜像工具 → 镜像加速器 → 拿专属地址：

```bash
sudo mkdir -p /etc/docker
sudo tee /etc/docker/daemon.json <<'EOF'
{ "registry-mirrors": ["https://<你的ID>.mirror.aliyuncs.com"] }
EOF

sudo service docker restart
docker info | grep -A3 "Registry Mirrors"      # 必须看到你的地址
docker pull ubuntu:20.04
```

**② 造专用构建镜像**

```bash
mkdir -p ~/docker/rk3566-builder && cd ~/docker/rk3566-builder

cat > Dockerfile <<'EOF'
FROM ubuntu:20.04

RUN sed -i 's|//archive.ubuntu.com|//mirrors.tuna.tsinghua.edu.cn|g; \
           s|//security.ubuntu.com|//mirrors.tuna.tsinghua.edu.cn|g' \
      /etc/apt/sources.list \
 && apt-get update \
 && apt-get install -y --no-install-recommends \
      g++-aarch64-linux-gnu cmake make file binutils-aarch64-linux-gnu \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work
EOF

sudo docker build -t rk3566-builder .
```

**③ 编译单个文件**

```bash
cd <放源码的目录>

sudo docker run --rm -v "$PWD:/work" -w /work rk3566-builder \
  aarch64-linux-gnu-g++ -Wall -O2 -o test_2004 test.cpp

sudo chown $USER:$USER test_2004
```

> **`docker run` 格式**：`docker run [选项] 镜像名 [在容器里执行的命令]`
> - `--rm` 用完自动删容器
> - `-v "$PWD:/work"` 把宿主机当前目录挂进容器（**这就是"开门"**）
> - `-w /work` 容器的工作目录

**④ 构建整个 CMake 项目**（**CMakeLists 和 toolchain file 一个字都不用改**）

```bash
cd ~/projects/2.my_vec

sudo docker run --rm -v "$PWD:/work" -w /work rk3566-builder \
  bash -c "cmake -S . -B build-arm20 -DCMAKE_TOOLCHAIN_FILE=cmake/aarch64-toolchain.cmake && \
           cmake --build build-arm20"

sudo chown -R $USER:$USER build-arm20
```

**⑤ 验证产物**

```bash
ls -lh test_2004        # ~79 KB，不是 2000 KB
file test_2004          # dynamically linked，不是 statically

sudo docker run --rm -v "$PWD:/work" -w /work rk3566-builder \
  aarch64-linux-gnu-readelf --version-info test_2004 | grep GLIBC | head
# 期望：最高 GLIBC_ ≤ 2.31，没有 2.34
```

**⑥ 传到板子上跑**

```bash
wscp test_2004 root@192.168.137.100:~/
# 板子上
./test_2004
```

> ⚠️ **千万别把 WSL 的新 libc 拷到板子上覆盖** —— 几乎必然把系统搞坏。

### 9.4 板子上自查

```bash
ldd --version                                   # 板子的 glibc 版本
aarch64-linux-gnu-readelf --version-info ./app  # 二进制"欠"了哪些版本
```

---

## 10. 踩坑记录（症状 → 原因 → 解法）

| # | 症状 | 原因 | 解法 |
|---|---|---|---|
| 1 | 串口全黑 | TX/RX 没交叉 / 板子没上电 / COM 口选错 | 对调白绿；查电源灯 |
| 2 | 串口乱码 | 波特率不对 | 改 1500000 |
| 3 | 设备管理器没有 COM 口 | CH340 驱动没装 | 装 `CH341SER.EXE` |
| 4 | Windows 认不到来烧录的板子 | 充电线 / 驱动没装 / 按键手法 | 换**数据线**、装 `DriverAssitant`、按 MASKROM 再上电 |
| 5 | `RKFW` 开头的 img 写不进 TF 卡 | 那是 RKDevTool 用的打包固件 | 走 USB 烧 eMMC |
| 6 | 直连笔记本 `DHCPDISCOVER` 无应答 | **笔记本不是 DHCP 服务器** | 两端静态 IP，或开 ICS |
| 7 | `ip addr add` 的地址过一会儿消失 | **NetworkManager 清掉了**（不属任何连接配置） | 用 `nmcli con mod` |
| 8 | 有 IP 但 `Network is unreachable` | **没有默认路由** | `ipv4.gateway` 和 `never-default` 都要配对 |
| 9 | 板子 ping 不通笔记本，Windows ping 板子通 | **Windows 防火墙拦入站 ICMP** | 正常现象，改测 `ping 8.8.8.8` |
| 10 | Windows 写完 Linux 镜像后卡顿、打不开盘 | Windows 不认识 ext4 | **别点格式化**，直接拔掉 |
| 11 | WSL ping/ssh 不到板子，cmd 可以 | WSL2 NAT 不转发主机直连网段 | 用 `.exe` 中继（3.5） |
| 12 | `networkingMode=mirrored` 报 `0x8007054f`、WSL 完全没网 | **与 VMware 虚拟网络栈冲突**。⚠️ VMware 服务**开机自启**，禁用了、重启后也会自己回来 | **退回 NAT**：删掉/改掉 `%USERPROFILE%\.wslconfig` 里的 `networkingMode=mirrored`，再 `wsl --shutdown`。**别试第三次** |
| 13 | `docker build` 报 `permission denied ... docker.sock` | 用户不在 docker 组 | `sudo`，或 `usermod -aG docker $USER` |
| 14 | `docker pull` 超时 `dial tcp ... i/o timeout` | Docker Hub 国内不可达 | 配阿里云加速器（9.3 ①） |
| 15 | 板子上跑交叉编译产物报 `GLIBC_2.34 not found` | 工具链 glibc 比板子新 | Docker 固定 20.04 环境（9.3） |

---

## 11. 一页速查

```bash
# —— 串口 ——
# 1500000 8N1 ; 黑→pin23(GND)  白→pin27(TX)  绿→pin25(RX)  红→不接

# —— 板子：静态 IP + 网关（含外网）——
nmcli con mod "Wired connection 1" \
      ipv4.method manual ipv4.addresses 192.168.137.100/24 \
      ipv4.gateway 192.168.137.1 ipv4.dns 192.168.137.1 \
      ipv4.never-default no
nmcli con up "Wired connection 1"

# —— 三层验证 ——
ip addr show eth0 | grep "inet "   # IP
ip route                           # default via 192.168.137.1
ping -c 3 8.8.8.8                  # 外网
ping -c 3 www.baidu.com            # DNS

# —— 从 WSL 进入（走 Windows 的 ssh）——
alias wssh='/mnt/c/Windows/System32/OpenSSH/ssh.exe'
alias wscp='/mnt/c/Windows/System32/OpenSSH/scp.exe'
wssh root@192.168.137.100

# —— 交叉编译（Docker，主力）——
sudo docker run --rm -v "$PWD:/work" -w /work rk3566-builder \
  aarch64-linux-gnu-g++ -Wall -O2 -o app app.cpp
```

---

## 12. 清单状态

### Week 2（板子到货 + 网络与远程）
- [x] 烧录系统镜像（RKDevTool → eMMC）
- [x] 串口连接 + 看启动日志
- [x] 板子联网
- [x] SSH 登录 + scp 传文件
- [x] tmux
- [x] 板子上编译并运行 hello world
- [ ] **验收截图存档**（板子点亮 + SSH 成功）

### Week 3–4（C++ 工程化改造）
- [x] 多文件工程（include / src 分离）
- [x] CMake：`target_include_directories` / `target_link_libraries`
- [x] 编译链接原理（预处理 → 编译 → 汇编 → 链接）
- [x] 静态库 vs 动态库（`.a` / `.so`）
- [x] 交叉编译（toolchain file + **glibc 匹配**）
- [x] 智能指针 + RAII（myVec 重写，少 40 行）
- [x] gdb（断点 / `bt` / core dump）
- [x] Git 分支工作流（feature branch → merge）
- [ ] `add_subdirectory` 拆分
- [ ] `find_package`（引 OpenCV）
- [ ] 继承 + 虚函数（小类层次）

### 下一步
- [ ] MIPI 摄像头（OV5647 / IMX415 / OV13850 / GC5035 资料都在手边）
- [ ] 板子上跑 OpenCV
- [ ] RKNN + NPU（1 TOPS）
