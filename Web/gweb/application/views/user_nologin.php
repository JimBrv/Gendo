<?php if ( ! defined('BASEPATH')) exit('No direct script access allowed');
    $this->load->view('header');
?>
<div id="register_form">
    <div class="div_register_title"></div>
	<label class="register_title">请先登录</label>
	<div class="div_register_title"></div>
	<p class="p4" style="font-size:16px;">已有账号，<a id="loginname" href="#" onClick="open_login()"><strong>请猛击这里！</strong></a>
    <p class="p4" style="font-size:16px;">还没注册账号？赶快
    <a title="新开张，大促销，只要注册即送大礼！" href="<?php echo base_url();?>register" ><strong>注册一个吧</strong></a>
    ，立即享有5G/月大流量的<a title="新开张，大促销，只要注册即送大礼！" href="<?php echo base_url();?>register" ><strong>免费账号</strong></a>，还等什么。</p>
</div>

<?php $this->load->view('footer');?>	