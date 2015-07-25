## Domi Server 项目结构

基于libevent网络库，也参照了很多工作接触到的服务器框架。

-----
### 1、目录结构

	3rd			-- 第三方库，包括google protobuf、libevent,需要自己去下载，这里不提供
	build		-- 自动构建工具生成目录
	DomiEngine 	-- 服务器引擎部分，编译为lib
	DomiServer	-- 服务器程序，编译为exe可执行程序
	Output		-- 编译输出目录

	├─3rd
	│  ├─include
	│  │  ├─google
	│  │  │  └─protobuf
	│  │  │      ├─compiler
	│  │  │      │  ├─cpp
	│  │  │      │  ├─java
	│  │  │      │  ├─javanano
	│  │  │      │  ├─python
	│  │  │      │  └─ruby
	│  │  │      ├─io
	│  │  │      └─stubs
	│  │  └─libevent
	│  │      ├─compat
	│  │      │  └─sys
	│  │      ├─include
	│  │      │  └─event2
	│  │      └─WIN32-Code
	│  │          ├─event2
	│  │          └─nmake
	│  │              └─event2
	│  └─lib
	├─build
	├─DomiEngine
	│  ├─common
	│  │  ├─basic
	│  │  ├─log
	│  │  ├─pool
	│  │  ├─serverroot
	│  │  ├─singleton
	│  │  ├─thread
	│  │  └─timer
	│  └─network
	├─DomiServer
	│  ├─logic
	│  │  └─logicnet
	│  └─main
	└─Output

### 2、命名规范

	类似golang规则:
	public： 首字母大写
	private： 首字母小写

###3、编译
- cmake  
  >cmake 3.3版本，这货不好写，后面不用了，先放在里面，当成学习之用。已经写好了 generate bat，直接运行。

- premake
  >这货是后面发现挺好用，加上我比较喜欢lua，后面都用这个耍。也写好了bat。
  
