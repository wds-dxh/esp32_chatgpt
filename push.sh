#传递参数为提交的信息
###
 # @Author: wds-Ubuntu22-cqu wdsnpshy@163.com
 # @Date: 2024-12-10 20:12:41
 # @Description: 
 # 邮箱：wdsnpshy@163.com 
 # Copyright (c) 2024 by ${wds-Ubuntu22-cqu}, All Rights Reserved. 
### 
# 进入/home/wds/workspace/now/Driving_module/目录
git add .
git pull
git commit -m "update_$(date +'%Y-%m-%d %H:%M:%S')_$1"
git push -u origin dev