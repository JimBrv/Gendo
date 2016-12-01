<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>
  
	<div id="register_form">
	    <div class="div_register_title"></div>
	    <label class="register_title">用户登录失败</label>
	    <div class="div_register_title"></div>
        <p class="p3" style="color:red;">用户名或密码错误！</p>
    </div>

<?php $this->load->view('footer');?>