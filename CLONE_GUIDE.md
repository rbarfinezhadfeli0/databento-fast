# 📥 How to Clone databento-fast on Another PC

This guide shows how to get databento-fast in different scenarios.

---

## 🎯 **Three Ways to Clone:**

### **Option 1: Clone github-v1 with Submodules (Full Workspace)**

Get everything including databento-fast as a linked submodule:

```bash
# Clone with all submodules
git clone --recursive https://github.com/rbarfinezhadfeli0/github-v1.git
cd github-v1

# databento-fast is automatically cloned to:
# github-v1/my_github_libraries/databento-fast/
```

**If you forgot `--recursive`, add it later:**
```bash
git clone https://github.com/rbarfinezhadfeli0/github-v1.git
cd github-v1

# Initialize and update submodules
git submodule init
git submodule update

# Or in one command:
git submodule update --init --recursive
```

---

### **Option 2: Clone databento-fast Only (Just the Library)**

Get only databento-fast without the full github-v1 workspace:

```bash
# Clone standalone repository
git clone https://github.com/rbarfinezhadfeli0/databento-fast.git
cd databento-fast

# Build and install
./build.sh
pip install -e .
```

---

### **Option 3: Install from PyPI (Easiest)**

Just install the library without source code:

```bash
# Install from PyPI
pip install databento-fast

# Use it immediately
python -c "import databento_cpp; print(databento_cpp.__version__)"
```

---

## 🔄 **Updating Submodules:**

If databento-fast gets updated and you want the latest version in github-v1:

```bash
cd /path/to/github-v1

# Update all submodules to latest
git submodule update --remote

# Or update specific submodule
cd my_github_libraries/databento-fast
git pull origin main

# Commit the update in parent repo
cd /path/to/github-v1
git add my_github_libraries/databento-fast
git commit -m "Update databento-fast to latest version"
git push
```

---

## 📂 **What You'll Get:**

### **Full Clone (github-v1 --recursive):**
```
github-v1/
├── tradning/
│   └── rithmic_v1/
│       ├── databento_fastest.py
│       └── dbn/
├── my_github_libraries/
│   ├── databento-fast/           ← Submodule (separate repo)
│   │   ├── include/
│   │   ├── src/
│   │   ├── examples/
│   │   ├── python/
│   │   └── README.md
│   └── other_libraries/
└── README.md
```

### **Standalone Clone (databento-fast):**
```
databento-fast/
├── include/databento/
│   ├── dbn.hpp
│   └── parser.hpp
├── src/
│   └── parser.cpp
├── examples/
│   ├── ultra_fast_real_data.cpp
│   ├── ultimate_fast.cpp
│   └── ...
├── python/
│   ├── databento_py.cpp
│   └── minimal_example.py
├── README.md
└── setup.py
```

---

## 🛠️ **Setting Up on New PC:**

### **For Development (Full Workspace):**

```bash
# 1. Clone with submodules
git clone --recursive https://github.com/rbarfinezhadfeli0/github-v1.git
cd github-v1/my_github_libraries/databento-fast

# 2. Build C++ library
./build.sh

# 3. Install Python bindings
pip install -r requirements.txt
pip install -e .

# 4. Test it works
./build/standalone_test
python -c "import databento_cpp; print(databento_cpp.__version__)"
```

### **For Usage Only (PyPI):**

```bash
# Just install from PyPI
pip install databento-fast

# Use it
python -c "import databento_cpp; print(databento_cpp.__version__)"
```

---

## 🔍 **Understanding Submodules:**

### **What is a Submodule?**

A **submodule** is a reference to another repository inside your repository:

```
github-v1/.gitmodules contains:
    [submodule "my_github_libraries/databento-fast"]
        path = my_github_libraries/databento-fast
        url = https://github.com/rbarfinezhadfeli0/databento-fast.git

This tells git: "This folder is actually a separate repository"
```

### **Benefits:**

1. **databento-fast exists independently**
   - Has its own commits, releases, tags
   - Can be cloned standalone
   - Can be published to PyPI

2. **Also part of github-v1**
   - Shows up in your workspace
   - Easy to access alongside other projects
   - Version is tracked (commit hash)

3. **Best of both worlds**
   - Users can clone just databento-fast
   - You can clone full workspace with everything

---

## 📝 **Common Commands:**

```bash
# Clone workspace with all submodules
git clone --recursive https://github.com/rbarfinezhadfeli0/github-v1.git

# If already cloned, get submodules:
git submodule update --init --recursive

# Update submodule to latest:
cd my_github_libraries/databento-fast
git pull origin main

# Check submodule status:
cd github-v1
git submodule status

# Work in submodule:
cd my_github_libraries/databento-fast
# This is a full git repo, use normal git commands
git status
git commit
git push
```

---

## ⚠️ **Important Notes:**

1. **Submodule has its own .git folder**
   - It's a complete repository
   - Has its own commits and branches
   - Push/pull independently

2. **Parent repo tracks specific commit**
   - github-v1 stores which commit of databento-fast it uses
   - Shows as `databento-fast @ 52cca71` on GitHub
   - Update parent when you update submodule

3. **Two separate pushes needed:**
   ```bash
   # Push submodule
   cd databento-fast && git push
   
   # Push parent (to update reference)
   cd github-v1 && git push
   ```

---

## 🎉 **After You Push:**

Once you push both repositories to GitHub, anyone can clone with:

```bash
# Get everything
git clone --recursive https://github.com/rbarfinezhadfeli0/github-v1.git

# Or get just databento-fast
git clone https://github.com/rbarfinezhadfeli0/databento-fast.git
```

**Both will work!** The first gives them your full workspace, the second just the library.

