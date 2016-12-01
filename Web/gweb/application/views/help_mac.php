<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-help" style="height:4100px;">
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
    	         <div class="nav-help-1">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/android.jpg" class="nav-os-logo"> Android</p>
    	         </div>
    	    </a>
    	    
    	    <div class="nav-help-split1"></div>
    	    <a href="<?php echo base_url();?>help/help_mac" hidefocus="true"> 
    	         <div class="nav-help-0">
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
	    <p>Mac操作系统支持的协议有PPTP/L2TP，各种协议优点和介绍，详见教程<a href="#">"V-P+N协议扫盲教程"。</a></p>
	    <p>目前版本需要用户首先记下服务器IP地址，在后续的设置需要用户填入IP地址信息。</p>
	    <br>
	    <p class="help-step">第一步：点击Dock上的“系统设置”图标进入设置，也可以通过“系统偏好设定”找到入口：</p>
	    <p><img src="<?php echo base_url();?>css/images/mac1.png" width="400"> </p>
	    <br>
	    <p class="help-step">第二步：找到 "网络"选项，进入：</p>
	    <p><img src="<?php echo base_url();?>css/images/mac2.png" width="500"> </p>
	    <br>
	    <p class="help-step">第三步：点击“创建新服务”：</p>
	    <p><img src="<?php echo base_url();?>css/images/mac3.png" width="400"> </p>
	    <br>
	    <p class="help-step">第四步：在“接口”栏中选择“V-P+N”，在“V-P+N类型”栏中选择 “PPTP”，在“服务名称”中填入gendo，然后点击“创建”</p>
	    <p><img src="<?php echo base_url();?>css/images/mac4.png" width="500"> </p>
	    <br>
	    <p class="help-step">第五步：按照下面的示意图，在服务器栏中填入服务器线路地址<a href="<?php echo base_url();?>server" style="color:red"><strong>（点击这里查看服务器线路地址）</strong></a></p>
	    <p class="help-step">在账户栏中填入跟斗云用户名，然后点击“验证设置”</p>
	    <p><img src="<?php echo base_url();?>css/images/mac5.png" width="500"> </p>
	    <br>
	    <p class="help-step">第六步：在出现的对话框中输入账户密码，然后点击“确定”</p>
	    <p><img src="<?php echo base_url();?>css/images/mac6.png" width="500"> </p>
	    <br>
	    <p class="help-step">第七步：点击“高级”按钮，对使用方式进行设置，选中“使用V-P+N连接发送所有流量”，然后点击“确定”</p>
	    <p><img src="<?php echo base_url();?>css/images/mac7.png" width="500"> </p>
	    <br>
	    <p class="help-step">第八步：点击“连接”按钮，连接线路服务器</p>
	    <p><img src="<?php echo base_url();?>css/images/mac8.png" width="500"> </p>	    
	    
	</div>
</div>

<?php $this->load->view('footer');?>