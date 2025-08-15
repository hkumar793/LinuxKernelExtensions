# Linux Kernel Programming Assignment

## Environment Setup
 
Check kernel version:
```bash
uname -r
```

### Options

#### 1. Native Installation  
Have a Linux machine running kernel version 6.1.x-x with root access.

#### 2. Virtual Machine Setup  
- **VM Image**: x86-64 (VirtualBox Image) — [link]  
- **Install VirtualBox**: https://www.virtualbox.org/wiki/Downloads  
- **Credentials**:  
  - `username`: cs695  
  - `password`: 1234  
  - `root password`: 1234

**Enable SSH (for VirtualBox)**:  
1. Devices → Network → Network Settings → **Bridged Adapter**  
2. Reboot VM  
3. Use provided `internet.sh` script for IITB network login (LDAP credentials).  
4. SSH:  
```bash
ssh cs695@<vm-ip>
```

---

## Task 1: Linux Kernel Modules (LKM)

### 1.0 Hello World Module
- `helloworld.ko`: Prints **"Hello World"** on load, **"Module Unloaded"** on unload.

### 1.1 Runnable Processes
- `lkm1.c`: List all runnable processes (PID, name).

### 1.2 Child Process State
- `lkm2.c`: Given `pid`, print each child PID and state.

### 1.3 Virtual to Physical Address
- `lkm3.c`: Given `pid` and `vaddr`, print mapping (if mapped).

### 1.4 Virtual & Physical Memory Size
- `lkm4.c`: Given `pid`, print total VAS size & mapped physical size.  
- `test1.c`: Allocates memory, writes with stride.  
- Output graph: `lkm4.png`  
- Observations: `lkm4.txt`

### 1.5 Transparent Huge Pages
- `lkm5.c`: Given `pid`, report huge page count & size (THP on/off).  
- `test2.c`: Allocates large anonymous memory.

**Note:** Common `Makefile` or `Kbuild` for all modules.

---

## Task 2: IOCTL Drivers

### 2.1 Virtual ↔ Physical Address Translation
- IOCTL driver functions:
  - Translate VA → PA (current process).
  - Write values to given PA list.
- User program:
  - Allocate & initialize block.
  - Print VA, PA, and values.
  - Modify via PA & verify.
- Script: `spock.sh` (build, init device, run, cleanup).

### 2.2 Process Parent Change & Kill Children
- IOCTL driver:
  - Change parent PID (foot soldier → control station).
  - Terminate all children of given PID (emergency).
- Provided soldier & control station programs + `run_dr_bloom.sh` (modified for device init/cleanup).

---

## Task 3: procfs & sysfs

### 3.0.1 `/proc/hello_procfs`
- `hello_procfs.c`: Read returns "Hello World!".

### 3.0.2 `/sys/kernel/hello_sysfs`
- `hello_sysfs.c`: Directory with:
  - `hello_int` (integer, read/write)
  - `hello_string` (string, read/write)

### 3.1 Page Faults
- `get_pgfaults.c`: `/proc/get_pgfaults` shows total OS page faults since boot.

### 3.2 Memory Stats
- `get_memstats.c`: `/sys/kernel/mem_stats` directory:
  - `pid`: PID to monitor (-1 default)
  - `virtmem`: Virtual memory size
  - `physmem`: Physical memory size
  - `unit`: "B", "K", or "M"

---

## References
- [Linux Kernel Labs](https://linux-kernel-labs.github.io/refs/heads/master/labs/kernel_modules.html)  
- [Linux Source Code v6.1 (Bootlin)](https://elixir.bootlin.com/linux/v6.1/source)  
- procfs & sysfs guides for kernel ≥ 5.x.x
