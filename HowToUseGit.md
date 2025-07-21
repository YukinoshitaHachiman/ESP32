
# Git 使用教程

## 一、创建项目的基本流程

### 1. 创建远程仓库

在 GitHub 创建一个仓库，例如：  
`https://github.com/YukinoshitaHachiman/ESP32`

### 2. 创建并初始化本地仓库

在本地写好代码后，例如：`/test/readme.md`，进入项目根目录（即未来 `.git` 所在目录），执行以下命令：

```bash
git init                         # 初始化本地仓库，创建 .git 文件夹
git add .                        # 添加所有更改进入暂存区
git commit -m "这是一个test"     # 提交暂存区更改到本地仓库
git remote add origin https://github.com/YukinoshitaHachiman/ESP32.git  # 关联远程仓库
git push -u origin main          # 推送代码到远程仓库的 main 分支，并建立跟踪关系
```

之后再次提交只需要使用 `git push` 即可。

---

## 二、拉取远程项目的基本流程

确保本地分支与远程分支存在关联。拉取远程分支代码到当前分支：

```bash
git pull origin main
```

该命令相当于：`git fetch` + `git merge`，即拉取远程代码并尝试合并到当前分支。

> 本地暂存不会影响 pull 行为，但若本地代码未提交，合并时可能出现冲突。

---

## 三、本地分支操作及合并流程

### 1. 创建和切换分支

```bash
git branch                 # 查看本地所有分支
git branch dev            # 创建 dev 分支
git checkout dev          # 切换到 dev 分支
# 或一步到位：
git checkout -b dev       # 创建并切换到 dev 分支
```

### 2. 合并分支

#### 无冲突合并

当两个分支间无代码冲突时，可以直接合并。  
首先切换到主分支，再执行合并：

```bash
git checkout main
git merge dev
```

#### 有冲突合并

若两个分支修改了相同位置的代码，执行 `git merge` 会出现冲突：

```bash
git checkout main
git merge dev
```

此时 Git 会提示：  
`Automatic merge failed; fix conflicts and then commit the result.`

编辑器中将出现以下标记，标注冲突内容：

```text
<<<<<<< HEAD  (当前更改)
代码 A
=======
代码 B
>>>>>>> dev  (传入更改)
```

你需要手动修改为期望的代码状态，并删除这些标记。然后执行：

```bash
git add .
git commit -m "解决冲突"
```

合并完成。

---

## 四、git pull 与 git fetch 的区别

| 命令       | 含义                           | 是否合并 | 是否可能产生冲突 |
|------------|--------------------------------|----------|------------------|
| `git fetch`| 拉取远程更新到本地（仅更新缓存） | 否       | 否               |
| `git pull` | 拉取远程更新并合并到当前分支     | 是       | 是               |

> 建议：使用 `git fetch` 查看远程变更后再决定是否合并，可避免不必要的冲突。

---

## 五、 .gitignore 文件的使用

很多时候我们的代码在编译后会产生一个 /build 文件夹，存放着编译产生的各种链接文件，可执行文件等构筑文件，这个文件夹体积非常大，上传时会耗费很多时间，而且也没必要。

又或者在编译的时候会指定要下载的组件，譬如 ESP32 的开发会把需要下载的组件写到 idf_component.yml 中，我们希望别人自己去下载这些组件，而不是依赖于直接 clone 原文件。

还或者在不同的编辑器或者开发环境中会产生一些用户配置文件，如 .vscode .cursor 等，我们同样不希望这些文件被一同上传到远程仓库。

此时就需要用到.gitignore，顾名思义，我们可以吧需要忽略的文件写到里面，这样 在提交更新时 git 会自动忽略这些文件，所有的更改都会被无视。
