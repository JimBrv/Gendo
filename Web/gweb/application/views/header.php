<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');?>

<!DOCTYPE html>
<html lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <title>跟斗云</title>
    <link rel="shortcut icon" href="<?php echo base_url();?>favicon.ico" />
    <link rel="stylesheet" href="<?php echo base_url();?>css/home.css" type="text/css" media="all" />
    
    <script type="text/javascript"  src="<?php echo base_url();?>js/jquery-1.9.0.js"> </script>
    <script type="text/javascript"  src="<?php echo base_url();?>js/jquery.validate.js"> </script>
    <script type="text/javascript"  src="<?php echo base_url();?>js/jquery-ui-1.8.6.core.widget.js"></script>
	<script type="text/javascript"  src="<?php echo base_url();?>js/jqueryui.bannerize.js"></script>
	<script type="text/javascript"  src="<?php echo base_url();?>js/code.js"></script>
    <script type="text/javascript"  src="<?php echo base_url();?>js/login.js"></script>
   
</head>

<body >

<div class="header-wrap"> 
	
	   <div class="header-body">
	   
	       <div class="header-login">
<!--	           <div class="username">
	           用户：<input class="input" name="username" type="text" maxlength="32"> </input></div>
	           <div class="password">
	           密码：<input class="input" name="password" type="password" maxlength="16"> </input>
	           <input style="width:45px; height:25px; color:black;" type="button" name="login" class="login" value="登录" id="login1" onclick="login_sub()"/>
	           <strong>|</strong>
	           <a style="color:white;" href="http://localhost/gweb/register"><strong>注册</strong> </a>
	           </div>-->

	           <div style="margin-top:40px;margin-left:5%;width:60px;float:left;">
  	               <a href="https://twitter.com/GendoCloud" class="twitter-follow-button" data-show-count="false" data-lang="zh-cn" data-show-screen-name="false" title="在Tweeter上关注 @GendoCloud" target="_blank"><img src="<?php echo base_url();?>css/images/twt-logo.JPG" style="width:45px;" /></a>
                   <!--<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0],p=/^http:/.test(d.location)?'http':'https';if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src=p+'://platform.twitter.com/widgets.js';fjs.parentNode.insertBefore(js,fjs);}}(document, 'script', 'twitter-wjs');</script>-->
               </div>

	           <div class="login-username">
	               <?php 
	               if (isset($this->session->userdata['username'])) { 
	                    echo "<span>欢迎您，<a href=\"".base_url()."user\" style=\"color:white;text-decoration:none;\"> <strong>".$this->session->userdata['username']."</strong></a></span>"; 
	                    echo "<strong> |</strong>"."<a style=\"color:white;text-decoration:none;\" href=\"".base_url()."login/logout\">"." <strong>退出</strong> </a>";
	                   } else { 	                       
	                    echo "<a id=\"loginname\" href=\"#\" onClick=\"open_login()\" style=\"color:white\"><strong>登录 </strong></a>";	                    
	                    echo "<strong>|</strong>"."<a style=\"color:white;\" href=\"".base_url()."register\">"."<strong> 注册</strong> </a>";
	                   }
	               ?>
	           </div>
	       </div>
	       <div class="header-logo">
	           <img src="<?php echo base_url();?>css/images/logo-256.png" style="vertical-align:middle;width:75px;margin-left:5%;margin-top:1px;"></img>
	           <span style="font-size:22px;"><strong>跟斗云</strong></span><span style="font-size:18px;">，随行而至</span>
	       </div>
	            
	       <div class="header-nav">
        	   <ul class="header-nav-ul">
        	       <li><a href="<?php echo base_url();?>home">首页</a><span></span></li>
        	       <li><a href="<?php echo base_url();?>user">个人中心</a></li>
        	       <li><a href="<?php echo base_url();?>service">服务订购</a></li>
        	       <li><a href="<?php echo base_url();?>server">线路信息</a></li>
        	       <li><a href="<?php echo base_url();?>help">帮助中心</a></li>
        	       <li><a href="http://download.gendocloud.com/download/GendoCloud.rar" target="_blank">下载中心</a></li>
        	       <li><a href="http://blog.gendocloud.com/" target="_blank">官方博客</a></li>        	       
        	   </ul>
    	   </div>
        </div>
</div>

<?php $this->load->view('login'); ?>
