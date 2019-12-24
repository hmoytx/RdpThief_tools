# 简介
针对原项目[RdpThief](https://github.com/0x09AL/RdpThief)，进行了修改，参考了3好学生的文章，增加了对win7情况下的无法写入ip的情况进行了修改。  


# 使用
编译RdpThief项目导出dll，测试时放在C盘下。编译use项目，可根据实际需求更改dll路径。  
编译前需要安装detours程序包。  
在运行mstsc的情况下运行use工具，自动完成注入。  
无论正确与否，都会记录在%temp%/data.bin文件中。  



# 注
原项目RdpThief编译出来的直接在win10中是可以用的，win7下可以使用这里的版本。  