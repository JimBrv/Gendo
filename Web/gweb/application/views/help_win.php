<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-help" style="height:2300px;">
	<div class="div-tab-nav">
    	<div class="nav-help-split0"></div>
    	    <a href="<?php echo base_url();?>help/help_win" hidefocus="true"> 
    	         <div class="nav-help-0">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/win8.jpg" class="nav-os-logo"> XP/Win7,8</p>
    	         </div>
    	    </a>
    	    
    	    <div class="nav-help-split1"></div>
    	    <a href="<?php echo base_url();?>help/help_iphone" hidefocus="true"> 
    	         <div class="nav-help-1">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/iphone.jpg" class="nav-os-logo"> Iphone/Ipad</p>
    	         </div>
    	    </a>
    
    	    <div class="nav-help-split1"></div>
    	    <a href="<?php echo base_url();?>help/help_android" hidefocus="true"> 
    	         <div class="nav-help-1">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/android.jpg" class="nav-os-logo"> Android</p>
    	         </div>
    	    </a>
    	    
    	    <div class="nav-help-split1"></div>
    	    <a href="<?php echo base_url();?>help/help_mac" hidefocus="true"> 
    	         <div class="nav-help-1">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/iphone.jpg" class="nav-os-logo"> MacOS</p>
    	         </div>
    	    </a>
    	    
    	    <div class="nav-help-split1"></div>
    	    <a href="<?php echo base_url();?>help/help_ubuntu" hidefocus="true"> 
    	         <div class="nav-help-1">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/ubuntu.jpg" class="nav-os-logo"> Ubuntu</p>
   	             </div>
    	    </a>   	    
    	<div class="nav-help-split2"></div>
	</div>
	
	<div class="help-content">
	    
	    <p>客户端软件目前支持所有Windows系列(XP, Win7, Win8)，建议Windows用户均通过客户端软件。</p>	    
	    <p>当前支持的协议有PPTP/L2TP/SSL-TCP/SSL-UDP，各种协议优点和介绍，详见教程<a href="#">"V-P+N协议扫盲教程"。</a></p>
	    <p>Win8用户登录后，如果服务器延迟测试失败，请右键选择"管理员方式运行"程序。</p>
	    <p>连接成功后，由于亲爱的墙存在，偶尔会发生想去的网站到不了的情况，请主动断开，再连接一次。</p>
	    <br>
	    <p class="help-step">第一步：在主页下载跟斗云软件包，并解压到任意目录：</p>
	    <p><img src="<?php echo base_url();?>css/images/gd-download.jpg" width="600"> </p>
	    <p><img src="<?php echo base_url();?>css/images/gd-unrar.jpg" width="400"> </p>
	    <p class="help-step">第二步：执行gendo.exe程序：</p>
	    <p><img src="<?php echo base_url();?>css/images/gd-open.jpg" width="400"> </p>
	    <p class="help-step">第三步：登录跟斗云账户，可注册新账户：</p>
	    <p><img src="<?php echo base_url();?>css/images/gd-login.jpg" width="450"> </p>
	    <p class="help-step">第四步：根据服务器延迟和自己需求，可选择延迟小的服务器连接，另外可选择各种支持的协议（目前建议选择PPTP方式）：</p>
	    <p><img src="<?php echo base_url();?>css/images/gd-proto.jpg" width="450"> </p>
	    <p class="help-step">第五步：连接成功后，即可畅游互联网：</p>
	    <p><img src="<?php echo base_url();?>css/images/gd-main.jpg" width="450"> </p>
	    
	</div>
	
	
</div>

<?php $this->load->view('footer');?>