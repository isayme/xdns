# 关于 #
GFW的手段之一就是**DNS污染**，DNS污染的原理是：DNS默认是UDP协议，比如GFW想要污染google.com，当GFW监听到google.com的DNS请求时，GFW就会伪造一个响应报文，当然响应报文中的IP地址是无用的。

想要解决这个问题，通常的手段是：写个DNS转发工具（中间服务器），将UDP的DNS请求转发成TCP格式。转发工具运行在本地，然后用户需要将自己电脑的DNS服务器地址设为`127.0.0.1`。

其实上面的方法并不可靠，因为TCP虽然面向连接，但其DNS报文如果被GFW监听到仍然可以伪造响应报文。只是由于绝大多数的DNS请求时UDP协议，可能GFW当前并没有处理TCP的DNS请求，所以目前此方法还是有效的。  
这个方法我曾经用C和python简单实现过，有兴趣的可以参见：[C语言版](https://github.com/isayme/fuck_dns "C DNS TCP转发")，[Python版](https://github.com/isayme/DnsByTcp "Python DNS TCP转发")。

既然GFW可以监听并伪造数据包，那么我们其实可以通过一个中间服务器（和TCP转发的中间服务器类似），这个服务器并不处理DNS请求包的具体格式，只是简单的修改数据包，如对每一字节都**取反**，这样即使GFW监听到此报文也不会知道实际这个数据包请求的真实内容，于是就不会污染。  
这个方法我也用Python简单实现，有兴趣可以参见：[Python版](https://github.com/isayme/dns_encrypt "Python 取反DNS包")，现在用C再次实现一次（Python只会照葫芦画瓢）。

# 如何使用 #

上节说到对DNS请求包取反数据，GFW无法识别，同样DNS服务器也无法识别，所以这就要求墙外（墙内的还是可能被污染）有个服务器可以运行同样的进程，将数据包再次取反后发到诸如8.8.8.8之类的DNS服务器。

这个项目是在Linux下编译运行的，没有windows版，我自己测试时是用的虚拟机。如果没有虚拟机，可以使用上文提到的[Python版](https://github.com/isayme/dns_encrypt "Python 取反DNS包")。

所以真正使用起来的架构是（本项目生成的进程名是xdns）：  

1. 墙外服务器运行xdns，配置文件中配置其请求的DNS服务器地址为8.8.8.8或其他你所知道的服务器地址，可同时配置多个。
2. 本地将网卡的DNS服务器IP设为127.0.0.1，然后本地机器运行xdns，配置文件中配置其请求的DNS服务器地址为你墙外服务器的IP。

## 配置文件格式 ##
服务器端和本地端配置格式相同。  
**xdns.num = 2** :指名配置的DNS服务器个数；  
**xdns.srv[0] = 8.8.8.8**：配置一个真实的服务器地址；  
**xdns.srv[1] = 8.8.4.4**：配置另一个真实的服务器地址；

# 联系我 #
邮件 : isaymeorg [at] gmail [dot] com  
博客 : [www.isayme.org](www.isayme.org "www.isayme.org") [Chinese Simplified]
