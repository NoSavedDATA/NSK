# No Saved Kaleidoscope Compiler

Documentation: [https://nsk-lang.dev](https://nsk-lang.dev)

Official repository of: [https://arxiv.org/abs/2409.11600](https://arxiv.org/abs/2409.11600)

NSK is a LLVM/C++ programming language. 

All the code is open sourced.


<div align="center">
  <img src="assets/nsk_logo.png" alt="Logo" width="260" height="260">
</div>

Features: 
- Parallel coding with finish async expressions.
- Pythonic syntax;
- Object Oriented (no inheritance, just composition);
- The low-level background C++ functions are compiled;
- High-level uses Just-in-Time Compilation;
- Close to C++ performance;
- Easy to read threads syntax that resemble go routines;
- Mark-sweep garbage collector with a go-like memory pool;
- Ease of extending it with C++ functions.

## Install

**Make**:

- WSL 2 or Ubuntu 22

- clang version 19;

  
```bash
wget https://apt.llvm.org/llvm.sh
chmod +x *.sh
sudo ./llvm.sh 19 all

sudo apt-get install llvm clang zlib1g-dev libzstd-dev libeigen3-dev libopencv-dev
```
- Add commands `nsk` to `PATH`:

```bash
./alias.sh
```

- Update submodules:

```bash
./update_submodules.sh
```

- Make:

```bash
make -j8
```

This adds nsk to bin/nsk
