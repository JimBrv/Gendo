<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-help" style="height:2600px;">
	<div class="div-tab-nav">
    	<div class="nav-help-split0"></div>
    	    <a href="<?php echo base_url();?>help/help_win" hidefocus="true">
    	         <div class="nav-help-1">
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
    	         <div class="nav-help-0">
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
	    <p>Android用户推荐使用PPTP方式，设置极为简单，创建V-P+N连接后，系统会自动记住所有设置，今后再开启服务只需一键开关。</p>	    
	    <p>Android终端支持的协议有PPTP/L2TP/IPSec，各种协议优点和介绍，详见教程<a href="#">"V-P+N协议扫盲教程"。</a></p>
	    <p>移动终端目前任然需要用户首先记下服务器IP地址，如果想切换服务器，相对PC客户端麻烦一些。</p>
	    <p>我们正在加紧开发iOS和Android版本的客户端，让大家少去设置的麻烦！</p>
	    <p>由于市面Android版本较多，各版本V-P+N设置稍有区别，此处以魅族MX2 Flyme系统讲解</p>
	    <br>
	    <p class="help-step">第一步：进入设置 -> 网络 -> V-P+N选项：</p>
	    <p><img src="<?php echo base_url();?>css/images/android1.jpg" width="280"> </p>
	    <br>
	    <p class="help-step">第二步：选择“添加V-P+N网络”，并选择协议“PPTP”：</p>
	    <p><img src="<?php echo base_url();?>css/images/android2.jpg" width="280"> </p>
	    <br>
	    <p class="help-step">第三步：V-P+N设置部分，服务器可以填入从<a href="<?php echo base_url();?>server" style="color:red"><strong>官网得到的IP地址</strong></a>：</p>
	    <p class="help-step">账号/密码请填入跟斗云账号密码</p>
	    <p class="help-step">PPP加密选项打开（MPPE加密方式）</p>
	    <p><img src="<?php echo base_url();?>css/images/android3.jpg" width="280"> </p>
	    <br>
	    <p class="help-step">第四步：输入跟斗云账号/密码，点击“连接”：</p>
	    <p><img src="<?php echo base_url();?>css/images/android4.jpg" width="280"> </p>
	    <br>	    
	    <p class="help-step">第五步：连接成功，右上角会有"V-P+N"提示</p>
	    <p><img src="<?php echo base_url();?>css/images/android5.jpg" width="280"> </p>
	    
	</div>
	
</div>

<?php $this->load->view('footer');?>