1. Make sure you have [boost](http://www.boost.org/), [gflags](https://gflags.github.io/gflags/), [glog](https://github.com/google/glog) successfully installed. Of course you can use `brew`, `apt-get` or `yum`.
2. Build: `make`
3. Download test data: `wget http://xunzhangthu.org/data/kjv12.txt`
4. Run sequential word cound example: `./bin/wc_test --method sequential --nmap 4 --nreduce 3 --file kjv12.txt` 
