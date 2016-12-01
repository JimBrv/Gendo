<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>

	<div id="register_form">
		<div class="div_register_title"></div>
		<p class="p1"> 注册成功 </p>
		<p class="p2"> 请记住您的账号：<span class="register_ok_user"> <?php echo "$username".", 昵称-"."$nickname";?> </span> </p>
		<p class="p2"> 现在可以通过客户端登录了！每月5G流量，不限速不封号，畅游十万八千里</p>
		<p class="p2">
		          活动期间(6-9月)，新注册用户有福啦，所有服务器均开放，且不限流量，还在等什么。
		</p>
	
		<p class="p2">
		          没有客户端？<?php echo anchor('http://82.211.30.236/download/GendoCloud.rar', '下载地址在这里!'); ?>
		</p>
	</div>
	
<?php $this->load->view('footer');?>	
