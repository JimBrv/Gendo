<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

<div class="div-help" style="height:1900px;">
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
    	         <div class="nav-help-1">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/iphone.jpg" class="nav-os-logo"> MacOS</p>
    	         </div>
    	    </a>
    	    
    	    <div class="nav-help-split1"></div>
    	    <a href="<?php echo base_url();?>help/help_ubuntu" hidefocus="true"> 
    	         <div class="nav-help-0">
    	         <p style="margin:0;padding:0"><img src="<?php echo base_url();?>css/images/ubuntu.jpg" class="nav-os-logo"> Ubuntu</p>
    	         </div>
    	    </a>   	    
    	<div class="nav-help-split2"></div>
	</div>
	
	<div class="help-content">	    
	    <p>Linux用户一样可以使用V-P+N服务，鉴于各种Linux发行版本较多，此处以Ubuntu桌面版为例说明。</p>	    
	    <p>Ubuntu几乎支持所有的协议，各种协议优点和介绍，详见教程<a href="#">"V-P+N协议扫盲教程"。</a></p>
	    <p>由于Linux用户极少，未开发相应客户端，需要用户记下服务器IP地址完成设置。</p>	    
	    <br>
	    
	    <p class="help-step">第一步：单击右上角网络图标，“配置V-P+N”一项：</p>
	    <p><img src="<?php echo base_url();?>css/images/ubt1.gif" width="280"> </p>
	    <br>
	    <p class="help-step">第二步：单击“添加”，会弹出添加窗口，选择Point-To-Point(PPTP)，并单击“新建”：</p>
	    <p><img src="<?php echo base_url();?>css/images/ubt2.gif" width="280"> </p>
	    <br>
	    <p class="help-step">第三步：在配置窗口，Gateway网关一项，输入服务器IP<a href="<?php echo base_url();?>server" style="color:red"><strong>（官网获取IP地址）</strong></a>：</p>
	    <p class="help-step">账号/密码请填入跟斗云账号密码，其它保持不变：</p>
	    <p><img src="<?php echo base_url();?>css/images/ubt3.gif" width="280"> </p>
	    <br>
	    <p class="help-step">第四步：点击“Advanced”选项，在弹出窗口中，取消“EAP”选项，勾选“Use Point-to-Point Encryption(MPPE)”选项，点击“确定”：</p>
	    <p><img src="<?php echo base_url();?>css/images/ubt4.gif" width="280"> </p>
	    <br>
	    <p class="help-step">第五步：点击右下角网络图标，可切换到“V-P-N”模式，连接成功后会有图标显示：</p>
	    <p><img src="<?php echo base_url();?>css/images/ubt5.gif" width="200"> </p>
	    
	</div>
	
</div>

<?php $this->load->view('footer');?>