<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-help" style="height:2800px;"">
	<div class="div-tab-nav">
    	<div class="nav-help-split0"></div>
    	    <a href="<?php echo base_url();?>help/help_win" hidefocus="true"> 
    	         <div class="nav-help-1">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/win8.jpg" class="nav-os-logo"> XP/Win7,8</p>
    	         </div>
    	    </a>
    	    
    	    <div class="nav-help-split1"></div>
    	    <a href="<?php echo base_url();?>help/help_iphone" hidefocus="true"> 
    	         <div class="nav-help-0">
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
	    <p>移动终端Iphone用户推荐使用PPTP方式，设置极为简单，创建V-P+N连接后，系统会自动记住所有设置，今后再开启服务只需一键开关。</p>	    
	    <p>Iphone/Ipad终端支持的协议有PPTP/L2TP，各种协议优点和介绍，详见教程<a href="#">"V-P+N协议扫盲教程"。</a></p>
	    <p>移动终端目前任然需要用户首先记下服务器IP地址，如果想切换服务器，相对PC客户端麻烦一些。</p>
	    <p>我们正在加紧开发iOS和Android版本的客户端，让大家少去设置的麻烦！</p>
	    <br>
	    <p class="help-step">第一步：进入设置 -> 通用：</p>
	    <p><img src="<?php echo base_url();?>css/images/iphone1.jpg" width="280"> </p>
	    <br>
	    <p class="help-step">第二步：找到 "网络"选项，进入：</p>
	    <p><img src="<?php echo base_url();?>css/images/iphone2.jpg" width="280"> </p>
	    <br>
	    <p class="help-step">第三步：选择"V-P+N"，系统会弹出相关设置：</p>
	    <p><img src="<?php echo base_url();?>css/images/iphone3.jpg" width="280"> </p>
	    <br>
	    <p class="help-step">第四步：V-P+N设置部分，服务器可以填入从<a href="<?php echo base_url();?>server" style="color:red"><strong>官网得到的IP地址</strong></a>：</p>
	    <p class="help-step">账号/密码请填入跟斗云账号密码，其它保持不变</p>
	    <p><img src="<?php echo base_url();?>css/images/iphone4.jpg" width="280"> </p>
	    <br>
	    <p class="help-step">第五步：设置完成后，即可以通过V-P+N开关一键开启/关闭服务了，如果连上了，右上角会V-P+N"字样</p>
	    <p><img src="<?php echo base_url();?>css/images/iphone5.jpg" width="280"> </p>
	    
	</div>
	
</div>

<?php $this->load->view('footer');?>